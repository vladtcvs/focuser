#include <movement.h>

static int32_t move_to_target(struct focuser_descriptor_s *desc)
{
    int32_t delta = desc->state->steps_target_position - desc->state->steps_current_position;
    if (delta == 0)
    {
        desc->state->state = FOCUSER_IDLE;
        return -1;
    }
    desc->state->speed_direction = (delta > 0);
    desc->actions->set_dir(desc->state->speed_direction);
    desc->actions->make_step();
    if (desc->state->speed_direction)
        desc->state->steps_current_position++;
    else
        desc->state->steps_current_position--;

    return desc->state->speed_time_delay_ms;
}

static int32_t move_with_speed(struct focuser_descriptor_s *desc)
{
    desc->actions->set_dir(desc->state->speed_direction);
    desc->actions->make_step();
    if (desc->state->speed_direction)
        desc->state->steps_current_position++;
    else
        desc->state->steps_current_position--;

    return desc->state->speed_time_delay_ms;
}

int32_t focuser_timer_handler(struct focuser_descriptor_s *desc)
{
    switch (desc->state->state)
    {
        case FOCUSER_IDLE:
            return -1;
        case FOCUSER_MOVE_TO_TARGET:
            return move_to_target(desc);
        case FOCUSER_MOVE_WITH_SPEED:
            return move_with_speed(desc);
    }
}

void focuser_set_speed(struct focuser_descriptor_s *desc, int32_t um_per_sec)
{
    if (um_per_sec <= 0)
    {
        focuser_stop(desc);
        return;
    }
    int32_t steps_per_1000sec = um_per_sec * desc->conf->steps_per_mm;
    int32_t delay_ms = 1000000UL / steps_per_1000sec;

    desc->state->speed_time_delay_ms = delay_ms;
}

void focuser_move_to_target_um(struct focuser_descriptor_s *desc, int32_t target_um)
{
    desc->state->steps_target_position = target_um * desc->conf->steps_per_mm / 1000;
    desc->state->state = FOCUSER_MOVE_TO_TARGET;
}

void focuser_move_with_speed(struct focuser_descriptor_s *desc, bool dir)
{
    desc->state->speed_direction = dir;
    desc->state->state = FOCUSER_MOVE_WITH_SPEED;
}

void focuser_stop(struct focuser_descriptor_s *desc)
{
    desc->state->state = FOCUSER_IDLE;
}

int32_t focuser_get_position_um(struct focuser_descriptor_s *desc)
{
    return desc->state->steps_current_position * 1000 / desc->conf->steps_per_mm;
}

void focuser_set_position_um(struct focuser_descriptor_s *desc, int32_t pos_um)
{
    desc->state->steps_current_position = pos_um * desc->conf->steps_per_mm / 1000;
}

void focuser_unforce(struct focuser_descriptor_s *desc)
{
    desc->actions->unforce();
}

bool focuser_is_busy(struct focuser_descriptor_s *desc)
{
    return desc->state->state != FOCUSER_IDLE;
}

void focuser_init(struct focuser_descriptor_s *desc,
                  int32_t steps_per_mm,
                  int32_t speed,
                  void (*set_dir)(bool dir),
                  void (*make_step)(void),
                  void (*unforce)(void))
{
    desc->conf->steps_per_mm = steps_per_mm;
    desc->state->steps_current_position = 0;
    desc->state->steps_target_position = 0;
    desc->state->state = FOCUSER_IDLE;
    desc->state->speed_direction = false;
    desc->actions->set_dir = set_dir;
    desc->actions->make_step = make_step;
    desc->actions->unforce = unforce;
    focuser_set_speed(desc, speed);
}
