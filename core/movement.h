#pragma once

#include <stdint.h>
#include <stdbool.h>

struct focuser_config_s
{
    int32_t steps_per_mm;
};

enum focuser_state_e
{
    FOCUSER_IDLE = 0,
    FOCUSER_MOVE_TO_TARGET,
    FOCUSER_MOVE_WITH_SPEED,
};

struct focuser_state_s
{
    enum focuser_state_e state;
    int32_t steps_current_position;
    int32_t steps_target_position;
    int32_t speed_time_delay_ms;
    bool    speed_direction;
};

struct focuser_actions_s
{
    void (*set_dir)(bool dir);
    void (*make_step)(void);
    void (*unforce)(void);
};

struct focuser_descriptor_s
{
    struct focuser_config_s  *conf;
    struct focuser_state_s   *state;
    struct focuser_actions_s *actions;
};

int32_t focuser_timer_handler(struct focuser_descriptor_s *desc);

void focuser_set_speed(struct focuser_descriptor_s *desc, int32_t um_per_sec);
void focuser_move_to_target_um(struct focuser_descriptor_s *desc, int32_t target_um);
void focuser_move_with_speed(struct focuser_descriptor_s *desc, bool dir);
void focuser_stop(struct focuser_descriptor_s *desc);

int32_t focuser_get_position_um(struct focuser_descriptor_s *desc);
void focuser_set_position_um(struct focuser_descriptor_s *desc, int32_t pos_um);


void focuser_unforce(struct focuser_descriptor_s *desc);
bool focuser_is_busy(struct focuser_descriptor_s *desc);

void focuser_init(struct focuser_descriptor_s *desc,
                  int32_t steps_per_mm,
                  int32_t speed_um_per_sec,
                  void (*set_dir)(bool dir),
                  void (*make_step)(void),
                  void (*unforce)(void));
