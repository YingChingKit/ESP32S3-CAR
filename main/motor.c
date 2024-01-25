
#include "motor.h"

motor_handle_t *brushed_motor_default_config(motor_handle_t *handle)
{
        handle->ch_A_pin = GPIO_NUM_NC;
        handle->ch_B_pin = GPIO_NUM_NC;
        handle->mcpwm_config.cmpr_a = 0;
        handle->mcpwm_config.cmpr_b = 0;
        handle->mcpwm_config.counter_mode = MCPWM_UP_COUNTER;
        handle->mcpwm_config.duty_mode = MCPWM_DUTY_MODE_0;
        handle->mcpwm_config.frequency = 60;

        handle->pcnt_pin = GPIO_NUM_NC;
        handle->pcnt_unit_config.high_limit = __INT16_MAX__;
        handle->pcnt_unit_config.low_limit = ~__INT16_MAX__;
        handle->pcnt_glitch_filter_config.max_glitch_ns = 1000;
        handle->pcnt_inverted = false;
        return handle;
}

void brushed_motor_init(motor_handle_t *handle)
{
        esp_err_t ret;
        ret = mcpwm_gpio_init(handle->mcpwm_unit, handle->mcpwm_ch_A_signal, handle->ch_A_pin);
        ESP_ERROR_CHECK(ret);
        ret = mcpwm_gpio_init(handle->mcpwm_unit, handle->mcpwm_ch_B_signal, handle->ch_B_pin);
        ESP_ERROR_CHECK(ret);
        ret = mcpwm_init(handle->mcpwm_unit, handle->mcpwm_timer, &handle->mcpwm_config);
        ESP_ERROR_CHECK(ret);
        ret = mcpwm_deadtime_disable(handle->mcpwm_unit, handle->mcpwm_timer);
        ESP_ERROR_CHECK(ret);

        if (handle->pcnt_pin == GPIO_NUM_NC)
                return;
        ret = pcnt_new_unit(&handle->pcnt_unit_config, &handle->pcnt_unit_handle);
        ESP_ERROR_CHECK(ret);
        ret = pcnt_unit_set_glitch_filter(handle->pcnt_unit_handle, &handle->pcnt_glitch_filter_config);
        ESP_ERROR_CHECK(ret);

        pcnt_chan_config_t pcnt_chan_config = {
            .edge_gpio_num = handle->pcnt_pin,
            .level_gpio_num = -1,
            .flags.invert_edge_input = handle->pcnt_inverted,
        };
        pcnt_channel_handle_t pcnt_channel_handle;
        ret = pcnt_new_channel(handle->pcnt_unit_handle, &pcnt_chan_config, &pcnt_channel_handle);
        ESP_ERROR_CHECK(ret);
        ret = pcnt_channel_set_edge_action(pcnt_channel_handle, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE);
        ESP_ERROR_CHECK(ret);
        ret = pcnt_unit_enable(handle->pcnt_unit_handle);
        ESP_ERROR_CHECK(ret);
        ret = pcnt_unit_clear_count(handle->pcnt_unit_handle);
        ESP_ERROR_CHECK(ret);
        ret = pcnt_unit_start(handle->pcnt_unit_handle);
        ESP_ERROR_CHECK(ret);
}

void brushed_motor_set(motor_handle_t *handle, float duty_cycle)
{
        if (duty_cycle < 0.0F)
                brushed_motor_backward(handle, -duty_cycle);
        else if (duty_cycle > 0.0F)
                brushed_motor_forward(handle, duty_cycle);
        else
                brushed_motor_brake(handle);
}

void brushed_motor_forward(motor_handle_t *handle, float duty_cycle)
{
        esp_err_t ret;
        ret = mcpwm_set_signal_low(handle->mcpwm_unit, handle->mcpwm_timer, MCPWM_OPR_A);
        ESP_ERROR_CHECK(ret);
        ret = mcpwm_set_duty(handle->mcpwm_unit, handle->mcpwm_timer, MCPWM_OPR_B, duty_cycle);
        ESP_ERROR_CHECK(ret);
        ret = mcpwm_set_duty_type(handle->mcpwm_unit, handle->mcpwm_timer, MCPWM_OPR_B, handle->mcpwm_config.duty_mode); // call this each time, if operator was previously in low/high state
        ESP_ERROR_CHECK(ret);
}

void brushed_motor_backward(motor_handle_t *handle, float duty_cycle)
{
        esp_err_t ret;
        ret = mcpwm_set_signal_low(handle->mcpwm_unit, handle->mcpwm_timer, MCPWM_OPR_B);
        ESP_ERROR_CHECK(ret);
        ret = mcpwm_set_duty(handle->mcpwm_unit, handle->mcpwm_timer, MCPWM_OPR_A, duty_cycle);
        ESP_ERROR_CHECK(ret);
        ret = mcpwm_set_duty_type(handle->mcpwm_unit, handle->mcpwm_timer, MCPWM_OPR_A, handle->mcpwm_config.duty_mode); // call this each time, if operator was previously in low/high state
        ESP_ERROR_CHECK(ret);
}

void brushed_motor_brake(motor_handle_t *handle)
{
        esp_err_t ret;
        ret = mcpwm_set_signal_high(handle->mcpwm_unit, handle->mcpwm_timer, MCPWM_OPR_A);
        ESP_ERROR_CHECK(ret);
        ret = mcpwm_set_signal_high(handle->mcpwm_unit, handle->mcpwm_timer, MCPWM_OPR_B);
        ESP_ERROR_CHECK(ret);
}

void brushed_motor_coast(motor_handle_t *handle)
{
        esp_err_t ret;
        ret = mcpwm_set_signal_low(handle->mcpwm_unit, handle->mcpwm_timer, MCPWM_OPR_A);
        ESP_ERROR_CHECK(ret);
        ret = mcpwm_set_signal_low(handle->mcpwm_unit, handle->mcpwm_timer, MCPWM_OPR_B);
        ESP_ERROR_CHECK(ret);
}

int brushed_motor_get_count(motor_handle_t *handle)
{
        int pulse_count;
        ESP_ERROR_CHECK(pcnt_unit_get_count(handle->pcnt_unit_handle, &pulse_count));
        return pulse_count;
}