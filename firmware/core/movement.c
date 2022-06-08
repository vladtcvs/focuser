#include <movement.h>

static int32_t process_step(struct focuser_descriptor_s *desc)
{
    if (desc->state->speed_direction)
    {
        if (desc->state->steps_current_position >= desc->state->steps_maxpos)
        {
            desc->state->state = FOCUSER_IDLE;
            return -1;
        }
        else
        {
            desc->state->steps_current_position++;
            desc->actions->make_step();
        }
    }
    else
    {
        if (desc->state->steps_current_position <= desc->state->steps_minpos)
        {
            desc->state->state = FOCUSER_IDLE;
            return -1;
        }
        else
        {
            desc->state->steps_current_position--;
            desc->actions->make_step();
        }
    }
    return desc->state->speed_time_delay_ms;
}

static int32_t move_to_target(struct focuser_descriptor_s *desc)
{
    int32_t delta = desc->state->steps_target_position - desc->state->steps_current_position;
    if (delta == 0)
    {
        desc->state->state = FOCUSER_IDLE;
        return -1;
    }
    desc->state->speed_direction = (delta > 0);

    return process_step(desc);
}

static int32_t move_with_speed(struct focuser_descriptor_s *desc)
{
    desc->actions->set_dir(desc->state->speed_direction);
    
    return process_step(desc);
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

int focuser_move_to_target_um(struct focuser_descriptor_s *desc, int32_t target_um)
{
    if (target_um < desc->state->min_position)
        return -1;
    if (target_um > desc->state->max_position)
        return -1;
    
    desc->state->steps_target_position = target_um * desc->conf->steps_per_mm / 1000;
    desc->state->state = FOCUSER_MOVE_TO_TARGET;
    return 0;
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

int focuser_set_position_um(struct focuser_descriptor_s *desc, int32_t pos_um)
{
    if (pos_um < desc->state->min_position)
        return -1;
    if (pos_um > desc->state->max_position)
        return -1;
    desc->state->steps_current_position = pos_um * desc->conf->steps_per_mm / 1000;
    return 0;
}

void focuser_set_minpos_um(struct focuser_descriptor_s *desc, int32_t um)
{
    desc->state->min_position = um;
    desc->state->steps_minpos = um * desc->conf->steps_per_mm / 1000;
}

void focuser_set_maxpos_um(struct focuser_descriptor_s *desc, int32_t um)
{
    desc->state->max_position = um;
    desc->state->steps_maxpos = um * desc->conf->steps_per_mm / 1000;
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
                  int32_t default_minpos,
                  int32_t default_maxpos,
                  void (*set_dir)(bool dir),
                  void (*make_step)(void),
                  void (*unforce)(void))
{
    desc->state->state = FOCUSER_IDLE;

    desc->actions->make_step = make_step;
    desc->actions->unforce = unforce;
    desc->actions->set_dir = set_dir;

    desc->conf->steps_per_mm = steps_per_mm;
    focuser_set_speed(desc, speed);
    focuser_set_maxpos_um(desc, default_maxpos);
    focuser_set_minpos_um(desc, default_minpos);
    focuser_set_position_um(desc, default_minpos);
    desc->state->steps_target_position = desc->state->steps_current_position;

    desc->state->speed_direction = false;

    focuser_unforce(desc);
}
