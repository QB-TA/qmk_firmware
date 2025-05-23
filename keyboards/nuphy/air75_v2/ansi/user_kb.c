/*
Copyright 2023 @ Nuphy <https://nuphy.com/>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "user_kb.h"
#include "ansi.h"
#include "usb_main.h"
#include "mcu_pwr.h"
#include "color.h"

kb_config_t     kb_config;
DEV_INFO_STRUCT dev_info = {
    .rf_battery = 100,
    .link_mode  = LINK_USB,
    .rf_state   = RF_IDLE,
};
bool f_bat_hold        = 0;
bool f_sys_show        = 0;
bool f_sleep_show      = 0;
bool f_send_channel    = 0;
bool f_dial_sw_init_ok = 0;
bool f_rf_sw_press     = 0;
bool f_dev_reset_press = 0;
bool f_rgb_test_press  = 0;
bool f_bat_num_show    = 0;

uint8_t        rf_blink_cnt          = 0;
uint8_t        rf_sw_temp            = 0;
uint8_t        host_mode             = 0;
uint16_t       rf_linking_time       = 0;
uint16_t       rf_link_show_time     = 0;
uint16_t       no_act_time           = 0;
uint16_t       dev_reset_press_delay = 0;
uint16_t       rf_sw_press_delay     = 0;
uint16_t       rgb_test_press_delay  = 0;
uint16_t       rgb_led_last_act      = 0;
uint16_t       side_led_last_act     = 0;
uint16_t       sleep_time_delay      = SLEEP_TIME_DELAY;
host_driver_t *m_host_driver         = 0;
rgb_t          bat_pct_rgb           = {.r = 0x80, .g = 0x80, .b = 0x00};

extern host_driver_t rf_host_driver;

/**
 * @brief  gpio initial.
 */
void gpio_init(void) {
    /* power on all LEDs */
    pwr_rgb_led_on();
    pwr_side_led_on();

    /* set side led pin output low */
    gpio_set_pin_output(DRIVER_SIDE_PIN);
    gpio_write_pin_low(DRIVER_SIDE_PIN);
    /* config RF module pin */
    gpio_set_pin_output(NRF_WAKEUP_PIN);
    gpio_write_pin_high(NRF_WAKEUP_PIN);
    gpio_set_pin_input_high(NRF_TEST_PIN);

    /* reset RF module */
    gpio_set_pin_output(NRF_RESET_PIN);
    gpio_write_pin_low(NRF_RESET_PIN);
    wait_ms(50);
    gpio_write_pin_high(NRF_RESET_PIN);
    /* config dial switch pin */
    gpio_set_pin_input_high(DEV_MODE_PIN);
    gpio_set_pin_input_high(SYS_MODE_PIN);
}

/**
 * @brief  long press key process.
 */
void long_press_key(void) {
    static uint32_t long_press_timer = 0;

    if (timer_elapsed32(long_press_timer) < 100) return;
    long_press_timer = timer_read32();

    // Open a new RF device
    if (f_rf_sw_press) {
        rf_sw_press_delay++;
        if (rf_sw_press_delay >= RF_LONG_PRESS_DELAY) {
            f_rf_sw_press = 0;

            dev_info.link_mode   = rf_sw_temp;
            dev_info.rf_channel  = rf_sw_temp;
            dev_info.ble_channel = rf_sw_temp;

            uint8_t timeout = 5;
            while (timeout--) {
                uart_send_cmd(CMD_NEW_ADV, 0, 1);
                wait_ms(20);
                uart_receive_pro();
                if (f_rf_new_adv_ok) break;
            }
        }
    } else {
        rf_sw_press_delay = 0;
    }

    // The device is restored to factory Settings
    if (f_dev_reset_press) {
        dev_reset_press_delay++;
        if (dev_reset_press_delay >= DEV_RESET_PRESS_DELAY) {
            f_dev_reset_press = 0;

            if (dev_info.link_mode != LINK_USB) {
                if (dev_info.link_mode != LINK_RF_24) {
                    dev_info.link_mode   = LINK_BT_1;
                    dev_info.ble_channel = LINK_BT_1;
                    dev_info.rf_channel  = LINK_BT_1;
                }
            } else {
                dev_info.ble_channel = LINK_BT_1;
                dev_info.rf_channel  = LINK_BT_1;
            }

            uart_send_cmd(CMD_SET_LINK, 10, 10);
            wait_ms(500);
            uart_send_cmd(CMD_CLR_DEVICE, 10, 10);

            void device_reset_show(void);
            void device_reset_init(void);

            eeconfig_init();
            device_reset_show();
            device_reset_init();

            if (dev_info.sys_sw_state == SYS_SW_MAC) {
                default_layer_set(1 << 0);
                keymap_config.nkro = 0;
            } else {
                default_layer_set(1 << 8);
                keymap_config.nkro = 1;
            }
        }
    } else {
        dev_reset_press_delay = 0;
    }

    // Enter the RGB test mode
    // if (f_rgb_test_press) {
    //     rgb_test_press_delay++;
    //     if (rgb_test_press_delay >= RGB_TEST_PRESS_DELAY) {
    //         f_rgb_test_press = 0;
    //         rgb_test_show();
    //     }
    // } else {
    //     rgb_test_press_delay = 0;
    // }
}

// RF repeat incase key break doesn't register properly...
void rf_repeat_key_break(void) {
    if (dev_info.link_mode == LINK_USB) return;
    for (uint8_t i = 0; i < 10; i++) {
        uart_send_report_repeat();
        wait_ms(5);
        uart_receive_pro();
        wait_ms(5);
    }
}

/**
 * @brief  Release all keys, clear keyboard report.
 */
void break_all_key(void) {
    // bool nkro_temp = keymap_config.nkro;

    // break current keyboard mode
    clear_weak_mods();
    clear_mods();
    clear_keyboard(); // this already sends the report.
    wait_ms(10);
    rf_repeat_key_break();

    // break the other keyboard mode
    // probably not necessary, commenting out for now.
    /*
    keymap_config.nkro = !keymap_config.nkro;
    clear_keyboard();
    wait_ms(10);

    keymap_config.nkro = nkro_temp;
    */
    void clear_report_buffer_and_queue(void);
    clear_report_buffer_and_queue();
}

/**
 * @brief  switch device link mode.
 * @param mode : link mode
 */
void switch_dev_link(uint8_t mode) {
    if (mode > LINK_USB) return;
    no_act_time = 0;
    break_all_key();
    dev_info.link_mode = mode;

    dev_info.rf_state = RF_IDLE;
    f_send_channel    = 1;

    if (mode == LINK_USB) {
        host_mode = HOST_USB_TYPE;
        host_set_driver(m_host_driver);
        rf_link_show_time = 0;
    } else {
        host_mode = HOST_RF_TYPE;
        host_set_driver(&rf_host_driver);
    }
}

/**
 * @brief  scan dial switch.
 */
void dial_sw_scan(void) {
    uint8_t         dial_scan       = 0;
    static uint8_t  dial_save       = 0xf0;
    static uint8_t  debounce        = 0;
    static uint32_t dial_scan_timer = 0;
    static bool     f_first         = true;

    if (!f_first) {
        if (timer_elapsed32(dial_scan_timer) < 20) return;
    }
    dial_scan_timer = timer_read32();

    gpio_set_pin_input_high(DEV_MODE_PIN);
    gpio_set_pin_input_high(SYS_MODE_PIN);

    if (gpio_read_pin(DEV_MODE_PIN)) dial_scan |= 0X01;
    if (gpio_read_pin(SYS_MODE_PIN)) dial_scan |= 0X02;

    if (dial_save != dial_scan) {
        break_all_key();

        no_act_time     = 0;
        rf_linking_time = 0;

        dial_save         = dial_scan;
        debounce          = 25;
        f_dial_sw_init_ok = 0;
        return;
    } else if (debounce) {
        debounce--;
        return;
    }

    if (dial_scan & 0x01) {
        if (dev_info.link_mode != LINK_USB) {
            switch_dev_link(LINK_USB);
        }
    } else {
        if (dev_info.link_mode != dev_info.rf_channel) {
            switch_dev_link(dev_info.rf_channel);
        }
    }

    if (dial_scan & 0x02) {
        if (dev_info.sys_sw_state != SYS_SW_MAC) {
            f_sys_show = 1;
            default_layer_set(1 << 0);
            dev_info.sys_sw_state = SYS_SW_MAC;
            break_all_key();
            keymap_config.nkro = 0;
        }
    } else {
        if (dev_info.sys_sw_state != SYS_SW_WIN) {
            f_sys_show = 1;
            default_layer_set(1 << 8);
            dev_info.sys_sw_state = SYS_SW_WIN;
            break_all_key();
            keymap_config.nkro = 1;
        }
    }

    if (f_dial_sw_init_ok == 0) {
        f_dial_sw_init_ok = 1;
        f_first           = false;

        if (dev_info.link_mode != LINK_USB) {
            host_set_driver(&rf_host_driver);
        }
    }
}

/**
 * @brief  power on scan dial switch.
 */
void dial_sw_fast_scan(void) {
    uint8_t dial_scan_dev  = 0;
    uint8_t dial_scan_sys  = 0;
    uint8_t dial_check_dev = 0;
    uint8_t dial_check_sys = 0;
    uint8_t debounce       = 0;

    gpio_set_pin_input_high(DEV_MODE_PIN);
    gpio_set_pin_input_high(SYS_MODE_PIN);

    // Debounce to get a stable state
    for (debounce = 0; debounce < 10; debounce++) {
        dial_scan_dev = 0;
        dial_scan_sys = 0;
        if (gpio_read_pin(DEV_MODE_PIN))
            dial_scan_dev = 0x01;
        else
            dial_scan_dev = 0;
        if (gpio_read_pin(SYS_MODE_PIN))
            dial_scan_sys = 0x01;
        else
            dial_scan_sys = 0;
        if ((dial_scan_dev != dial_check_dev) || (dial_scan_sys != dial_check_sys)) {
            dial_check_dev = dial_scan_dev;
            dial_check_sys = dial_scan_sys;
            debounce       = 0;
        }
        wait_ms(1);
    }

    // RF link mode
    if (dial_scan_dev) {
        if (dev_info.link_mode != LINK_USB) {
            switch_dev_link(LINK_USB);
        }
    } else {
        if (dev_info.link_mode != dev_info.rf_channel) {
            switch_dev_link(dev_info.rf_channel);
        }
    }

    // Win or Mac
    if (dial_scan_sys) {
        if (dev_info.sys_sw_state != SYS_SW_MAC) {
            break_all_key();
            default_layer_set(1 << 0);
            dev_info.sys_sw_state = SYS_SW_MAC;
            keymap_config.nkro    = 0;
        }
    } else {
        if (dev_info.sys_sw_state != SYS_SW_WIN) {
            break_all_key();
            default_layer_set(1 << 8);
            dev_info.sys_sw_state = SYS_SW_WIN;
            keymap_config.nkro    = 1;
        }
    }
}

/**
 * @brief  timer process.
 */
void timer_pro(void) {
    static uint32_t interval_timer = 0;
    static bool     f_first        = true;

    if (f_first) {
        f_first        = false;
        interval_timer = timer_read32();
        m_host_driver  = host_get_driver();
    }

    // step 10ms
    if (timer_elapsed32(interval_timer) < 10) return;
    interval_timer = timer_read32();

    if (rf_link_show_time < RF_LINK_SHOW_TIME) rf_link_show_time++;

    if (no_act_time < 0xffff) no_act_time++;

    if (rf_linking_time < 0xffff) rf_linking_time++;

    if (rgb_led_last_act < 0xffff) rgb_led_last_act++;

    if (side_led_last_act < 0xffff) side_led_last_act++;
}

/**
 * @brief  load eeprom data.
 */
void load_eeprom_data(void) {
    eeconfig_read_kb_datablock(&kb_config);
    if (kb_config.init_flag != 0xA5) {
        kb_config_reset();
    }
}

/**
 * @brief User config to default setting.
 */
void kb_config_reset(void) {
    /* first power on, set rgb matrix brightness 100% */
    rgb_matrix_sethsv(255, 255, 255);

    kb_config.init_flag       = 0xA5;
    kb_config.side_mode       = 3;
    kb_config.side_light      = 3;
    kb_config.side_speed      = 2;
    kb_config.side_rgb        = 1;
    kb_config.side_colour     = 6;
    kb_config.sleep_mode      = SLEEP_MODE_DEEP;
    kb_config.rf_link_timeout = LINK_TIMEOUT_ALT;
    eeconfig_update_kb_datablock(&kb_config);
}

/**
 * @brief  Show battery percentage LEDs
 */
void bat_pct_led_kb(void) {
    uint8_t bat_percent = dev_info.rf_battery;

    if (bat_percent >= 100) {
        bat_percent = 100;
    }

    uint8_t led_idx_tens = (bat_percent + 4) / 10;
    // uint8_t led_idx_ones = bat_percent % 10;i

    // set F key for battery percentage tens (e.g, 10%)

    for (uint8_t i = 0; i <= led_idx_tens; i++) {
        user_set_rgb_color(30 - i, bat_pct_rgb.r, bat_pct_rgb.g, bat_pct_rgb.b);
    }
    

    // set number key for battery percentage ones (e.g., 5 in 15%)
    // if (led_idx_ones == 0) {
    //     user_set_rgb_color(20, bat_pct_rgb.r, bat_pct_rgb.g, bat_pct_rgb.b);
    // } else {
    //     user_set_rgb_color(30 - led_idx_ones, bat_pct_rgb.r, bat_pct_rgb.g, bat_pct_rgb.b);
    // }
}

/**
 * @brief Updates RGB value and sets current bat percentage.
 */
void update_bat_pct_rgb(uint8_t bat_percent) {
    static uint8_t  bat_pct          = 0;
    static uint32_t bat_per_debounce = 0;

    if (bat_percent > 100) {
        bat_percent = 100;
    }

    // no change, update timer and move on.
    if (bat_pct == bat_percent) {
        bat_per_debounce = timer_read32();
        return;
    }

    // update only when battery stabilizes. Battery level could blip.
    if (timer_elapsed32(bat_per_debounce) <= 1000) {
        return;
    }

    bat_pct             = bat_percent;
    dev_info.rf_battery = bat_percent;

    // 120 hue is green, 0 is red on a 360 degree wheel but QMK is a uint8_t
    // so it needs to convert to relative to 255 - so green is actually 85.
    uint8_t h = 85; // green
    if (bat_pct <= 20) {
        h = 0; // red
    } else if (bat_pct <= 40) {
        h = 21; // orange same as charging
    } else if (bat_pct <= 99) {
        h = 85; // green
    } else if (bat_pct == 100) {
        h = 170; // blue
    }

    hsv_t hsv = {
        .h = h,
        .s = 255,
        .v = 255, // 100% max brightness
    };

    bat_pct_rgb = hsv_to_rgb_nocie(hsv); // this results in same calculation as colour pickers.
}

/**
 * @brief Wrapper for rgb_matrix_set_color for sleep.c logic usage.
 */
void user_set_rgb_color(int index, uint8_t red, uint8_t green, uint8_t blue) {
    if (red || green || blue) {
        rgb_led_last_act = 0;
        pwr_rgb_led_on(); // turn on LEDs
    }
    rgb_matrix_set_color(index, red, green, blue);
}

/**
 * @brief Dims RGB to 0 or 100
 */
void rgb_switch(void) {
    if (rgblight_get_val() == 0) {
        for (uint8_t i = 0; i <= 10; i++) {
            rgblight_increase_val();
        }
    } else {
        for (uint8_t i = 1; i <= 10; i++) {
            rgblight_decrease_val();
        }
    }
}

/**
 * @brief Handle LED power
 * @note Turn off LEDs if not used to save some power. This is ported
 *       from older Nuphy leaks.
 */
void led_power_handle(void) {
    static uint32_t interval = 0;

    if (timer_elapsed32(interval) < 500) // only check once in a while, less flickering for unhandled cases
        return;

    interval = timer_read32();

    if (rgb_led_last_act > 100) { // 10ms intervals
        if (rgb_matrix_is_enabled() && rgb_matrix_get_val() != 0) {
            pwr_rgb_led_on();
        } else { // brightness is 0 or RGB off.
            pwr_rgb_led_off();
        }
    }

    if (side_led_last_act > 100) { // 10ms intervals
        if (kb_config.side_light == 0) {
            pwr_side_led_off();
        } else {
            pwr_side_led_on();
        }
    }
}

void toggle_sleep_mode(void) {
    if (kb_config.sleep_mode > SLEEP_MODE_OFF) {
        kb_config.sleep_mode--;
    } else {
        kb_config.sleep_mode = SLEEP_MODE_DEEP;
    }
    f_sleep_show = 1;
    eeconfig_update_kb_datablock(&kb_config);
}

void link_mode_set(void) {
    dev_info.link_mode   = rf_sw_temp;
    dev_info.rf_channel  = rf_sw_temp;
    dev_info.ble_channel = rf_sw_temp;
    uart_send_cmd(CMD_SET_LINK, 10, 20);
}