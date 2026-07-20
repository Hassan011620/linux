#ifndef __PS4_BRIDGE_QUIRKS_H__
#define __PS4_BRIDGE_QUIRKS_H__

#include <drm/drm_edid.h>

bool ps4_bridge_quirk_force_1080p(const struct edid *edid);

#endif
