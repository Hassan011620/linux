#ifndef __PS4_BRIDGE_QUIRKS_H__
#define __PS4_BRIDGE_QUIRKS_H__

#include <drm/drm_edid.h>

enum ps4_bridge_forced_mode {
	PS4_BRIDGE_FORCED_MODE_NONE,
	PS4_BRIDGE_FORCED_MODE_1080P,
	PS4_BRIDGE_FORCED_MODE_720P,
};

enum ps4_bridge_forced_mode ps4_bridge_quirk_forced_mode(const struct edid *edid);

#endif
