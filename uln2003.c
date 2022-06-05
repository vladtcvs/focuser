#include <avr/io.h>
#include <uln2003.h>

static bool phases[8][4] = {
    {false, false, false, true },
    {false, false, true,  true },
    {false, false, true,  false},
    {false, true,  true,  false},
    {false, true,  false, false},
    {true,  true,  false, false},
    {true,  false, false, false},
    {true,  false, false, true },
};

static int phase = 0;
static int pd = 0;

static void set_phase(int phase)
{
    uint8_t val = ((phases[phase][0] << 3) | (phases[phase][1] << 2) | (phases[phase][2] << 1) | (phases[phase][3] << 0)) << 3;
    uint8_t mask = 0x0F << 3;

    PORTD &= ~mask;
    PORTD |= val;
}

void uln2003_init(void)
{
    phase = 0;
    pd = 1;
    DDRD |= (0x0F << 3);
    set_phase(phase);
}

void uln2003_make_step(void)
{
    phase = phase + pd;
    if (phase >= 8)
        phase -= 8;
    if (phase < 0)
        phase += 8;

    set_phase(phase);
}

void uln2003_set_dir(bool dir)
{
    if (dir)
        pd = 1;
    else
        pd = -1;
}

void uln2003_unforce(void)
{
    uint8_t mask = 0x0F << 3;
    PORTD &= ~mask;
}