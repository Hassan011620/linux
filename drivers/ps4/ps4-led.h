/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _PS4_LED_H
#define _PS4_LED_H

#define PS4_LED_ICC_MAJOR       0x09
#define PS4_LED_ICC_MINOR       0x20
#define PS4_LED_PAYLOAD_LEN     35

#define PS4_LED_TEMP_COOL_MAX   65
#define PS4_LED_TEMP_WARM_MAX   80

#define PS4_LED_TEMP_ICC_MAJOR  0x0B
#define PS4_LED_TEMP_ICC_MINOR  0x01
#define PS4_LED_TEMP_REPLY_LEN  0x10
#define PS4_LED_TEMP_BYTE       3

#define PS4_ICC_STATUS_OK       0x00

#define PS4_LED_THERMAL_INTERVAL_MS_DEFAULT     2000U

enum ps4_led_mode {
	MODE_STATIC,
	MODE_THERMAL,
};

#endif /* _PS4_LED_H */
