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
#include "hal_usb.h"
#include "usb_main.h"
#include "mcu_pwr.h"
#include "rf_queue.h"

extern kb_config_t     kb_config;
extern DEV_INFO_STRUCT dev_info;
extern uint16_t        rf_linking_time;
extern uint16_t        rf_link_timeout;
extern uint16_t        no_act_time;
extern bool            f_goto_sleep;
extern bool            f_wakeup_prepare;
extern uint16_t        sleep_time_delay;

void side_rgb_set_color_all(uint8_t r, uint8_t g, uint8_t b);
void side_rgb_refresh(void);

void deep_sleep_handle(void) {
    // break_all_key(); // reset keys before sleeping for new QMK lifecycle to handle on wake.

    // Visual cue for deep sleep on side LED.
    // pwr_side_led_on();
    // wait_ms(50); // give some time to ensure LED powers on.
    // side_rgb_set_color_all(0x99, 0x00, 0x00);
    // side_rgb_refresh();
    // wait_ms(500);

    // Sync again before sleeping. Without this, the wake keystroke is more likely to be lost.
    dev_sts_sync();

    enter_deep_sleep(); // puts the board in WFI mode and pauses the MCU
    exit_deep_sleep();  // This gets called when there is an interrupt (wake) event.

    no_act_time = 0; // required to not cause an immediate sleep on first wake
}

/**
 * @brief  Sleep Handle.
 */
void sleep_handle(void) {
    static uint32_t delay_step_timer     = 0;
    static uint8_t  usb_suspend_debounce = 0;
    static uint32_t rf_disconnect_time   = 0;

    /* 50ms interval */
    if (timer_elapsed32(delay_step_timer) < 50) return;
    delay_step_timer = timer_read32();

    // sleep process;
    if (f_goto_sleep) {
        // reset all counters
        f_goto_sleep         = 0;
        rf_disconnect_time   = 0;
        rf_linking_time      = 0;
        usb_suspend_debounce = 0;

        // don't deep sleep if charging on wireless, charging interrupts and wakes the MCU
        if (kb_config.sleep_mode != SLEEP_MODE_OFF && dev_info.link_mode < LINK_USB && dev_info.rf_charge & 0x01) {
            enter_light_sleep();
            // Don't deep sleep if in USB mode. Board may have issues waking as reported by others. I assume it's being
            // powered if USB port is on, or otherwise it's disconnected at the hardware level if USB port is off..
        } else if (kb_config.sleep_mode != SLEEP_MODE_OFF && dev_info.link_mode == LINK_USB) {
            enter_light_sleep();
        } else if (kb_config.sleep_mode == SLEEP_MODE_DEEP) {
            deep_sleep_handle();
            return; // don't need to do anything else
        } else if (kb_config.sleep_mode == SLEEP_MODE_LIGHT) {
            enter_light_sleep();
        }
        f_wakeup_prepare = 1; // only if light sleep.
    }

    // sleep check, won't reach here on deep sleep.
    if (f_goto_sleep || f_wakeup_prepare) return;

    if (dev_info.link_mode == LINK_USB) {
        if (USB_DRIVER.state == USB_SUSPENDED) {
            usb_suspend_debounce++;
            if (usb_suspend_debounce >= 20) {
                f_goto_sleep = 1;
            }
        } else {
            usb_suspend_debounce = 0;
        }
    } else if (no_act_time >= sleep_time_delay) {
        f_goto_sleep = 1;
    } else if (rf_linking_time >= kb_config.rf_link_timeout) {
        f_goto_sleep = 1;
    } else if (dev_info.rf_state == RF_DISCONNECT) {
        rf_disconnect_time++;
        if (rf_disconnect_time > 5 * 20) { // 5 seconds
            f_goto_sleep = 1;
        }
    } else if (dev_info.rf_state == RF_CONNECT) {
        rf_disconnect_time = 0;
    }
}
