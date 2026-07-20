#ifndef __PS4_BRIDGE_QUIRKS_H__
#define __PS4_BRIDGE_QUIRKS_H__

#include <linux/bits.h>
#include <drm/drm_edid.h>

#define PS4_BRIDGE_QUIRK_MODE_1080P60	BIT(0)
#define PS4_BRIDGE_QUIRK_MODE_1080P100	BIT(1)
#define PS4_BRIDGE_QUIRK_MODE_720P	BIT(2)

unsigned int ps4_bridge_quirk_forced_modes(const struct edid *edid);

#endif
