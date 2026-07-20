#include <linux/array_size.h>
#include <linux/kernel.h>
#include <asm/byteorder.h>

#include "ps4_bridge_quirks.h"

struct ps4_bridge_monitor_quirk {
	u16 manufacturer_name;
	u16 product_code;
	enum ps4_bridge_forced_mode forced_mode;
};

static const struct ps4_bridge_monitor_quirk ps4_bridge_monitor_quirks[] = {
	{ 0x4a8b, 0x1366, PS4_BRIDGE_FORCED_MODE_1080P },
	{ 0x5a63, 0x8a31, PS4_BRIDGE_FORCED_MODE_720P },
};

enum ps4_bridge_forced_mode ps4_bridge_quirk_forced_mode(const struct edid *edid)
{
	u16 mfg, prod;
	size_t i;

	if (!edid)
		return PS4_BRIDGE_FORCED_MODE_NONE;

	mfg = be16_to_cpu(edid->product_id.manufacturer_name);
	prod = le16_to_cpu(edid->product_id.product_code);

	for (i = 0; i < ARRAY_SIZE(ps4_bridge_monitor_quirks); i++) {
		if (ps4_bridge_monitor_quirks[i].manufacturer_name == mfg &&
		    ps4_bridge_monitor_quirks[i].product_code == prod)
			return ps4_bridge_monitor_quirks[i].forced_mode;
	}

	return PS4_BRIDGE_FORCED_MODE_NONE;
}
