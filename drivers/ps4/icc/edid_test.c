#include <linux/i2c.h>
#include <linux/printk.h>
#include <drm/drm_edid.h>

#include "../aeolia.h"
#include "edid_test.h"

static int icc_edid_read_byte(struct i2c_adapter *adap, u8 offset, u8 *val)
{
	union i2c_smbus_data data;
	int ret;

	ret = i2c_smbus_xfer(adap, DDC_ADDR, 0, I2C_SMBUS_READ, offset,
			      I2C_SMBUS_BYTE_DATA, &data);
	if (ret < 0)
		return ret;

	*val = data.byte;
	return 0;
}

void icc_edid_dump_test(struct apcie_dev *sc)
{
	u8 buf[EDID_LENGTH];
	int i, ret;

	for (i = 0; i < EDID_LENGTH; i++) {
		ret = icc_edid_read_byte(&sc->icc.i2c, i, &buf[i]);
		if (ret) {
			sc_err("icc-edid-test: read failed at offset %d: %d\n",
			       i, ret);
			return;
		}
	}

	if (buf[0] != 0x00 || buf[1] != 0xff || buf[2] != 0xff || buf[3] != 0xff ||
	    buf[4] != 0xff || buf[5] != 0xff || buf[6] != 0xff || buf[7] != 0x00)
		sc_warn("icc-edid-test: header magic mismatch, likely not a real EDID\n");
	else
		sc_info("icc-edid-test: header magic OK\n");

	print_hex_dump(KERN_INFO, "icc-edid-test: ", DUMP_PREFIX_OFFSET, 16, 1,
		       buf, EDID_LENGTH, false);
}
