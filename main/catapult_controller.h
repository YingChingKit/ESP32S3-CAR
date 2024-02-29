
#pragma once

#include "servo.h"
#include "button.h"
#include "pindef.h"
#include "laser.h"

#define CATAPULT_TURN_DEFAULT_ANGLE (90)
#define CATAPULT_WIND_DEFAULT_ANGLE (145)
#define CATAPULT_LOCK_LOCKED_ANGLE (90)
#define CATAPULT_LOCK_UNLOCKED_ANGLE (0)
#define CATAPULT_LASER_DEFAULT_ANGLE (0)
#define CATAPULT_TURN_ANGLE_OFFSET (-40.0F)
#define CATAPULT_RESET_TIMEOUT_US (1 * 1e6)

#define CATAPULT_TILT_LEFT_ANGLE (50)
#define CATAPULT_TILT_RIGHT_ANGLE (130)

typedef enum
{
        STATE_RESET = 0,
        STATE_BASE_TURN,
        STATE_BASE_LEFT,
        STATE_BASE_RIGHT,
        STATE_CATAPULT_ACCUMULATE,
        STATE_CATAPULT_SHOOT,
        STATE_CATAPULT_DECAY,
        STATE_RESET_DELAY,
        STATE_MAX,
} sm_state_t;

typedef struct
{
        servo_handle_t *lock;
        servo_handle_t *wind;
        servo_handle_t *laser;
        servo_handle_t *turn;

        laser_handle_t *aiming_laser;

        sm_state_t state;
        float wind_angle;
        float turn_angle;
        float laser_angle;
        float laser_duty_cycle;
        uint64_t catapult_shot_time;

} catapult_controller_handle_t;

catapult_controller_handle_t *catapult_controller_default_config(catapult_controller_handle_t *handle);
void catapult_controller_init(catapult_controller_handle_t *handle);

float laser_controller(float wind_angle);

void catapult_controller(catapult_controller_handle_t *handle, button_event_t *event);