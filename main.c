#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

#include <focuser_core.h>
#include <config.h>
#include <uln2003.h>

static void response(const char *msg)
{

}

static void set_dir(bool dir)
{
    uln2003_set_dir(dir);
}

static void make_step(void)
{
    uln2003_make_step();
}

static void unforce(void)
{
    uln2003_unforce();
}

static struct focuser_config_s conf;
static struct focuser_state_s state;
static struct focuser_actions_s actions;

static struct focuser_descriptor_s desc =
{
    .conf = &conf,
    .state = &state,
    .actions = &actions,
};

static void timer_init(void)
{
    TCNT0 = 0;
    TCCR1A = 0x00;
    TCCR1B = 0x08;
    TCCR1C = 0x00;
    TIMSK1 = 0x02;
    OCR1A = 0xFFFF;
}

static void timer_stop(void)
{
    TCCR1B &= ~0x07;
    TCNT1 = 0;
}

static void timer_set_delay(int32_t delay_ms)
{
    if (delay_ms <= 0)
        return;
    uint32_t cnt = delay_ms * F_CPU / 1000UL / 256UL;
    if (cnt > 65535UL)
        cnt = 65535UL;
    OCR1A = cnt;
    TCCR1B |= 0x04; // /256
}

static void process_step(void)
{
    int32_t delay_ms = focuser_timer_handler(&desc);

    if (delay_ms < 0)
    {
        timer_stop();
    }
    else
    {
        timer_set_delay(delay_ms);
    }
}

ISR(TIMER1_COMPA_vect)
{
    process_step();
}

static void on_command_receive(const char *cmd, size_t len)
{
    command_process(&desc, cmd, len);
    process_step();
}

int main(void)
{
    DDRD |= 1 << 0;
    uln2003_init();
    focuser_init(&desc, STEPS_PER_MM, DEFAULT_SPEED_UM_PER_SEC, set_dir, make_step, unforce);
    command_init(response);
    timer_init();

    sei();

    on_command_receive("SS100", 5);
    on_command_receive("MDT", 5);

    while (1)
    {
    }

    return 0;
}
