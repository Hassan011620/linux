// SPDX-License-Identifier: GPL-2.0-only

#include <linux/module.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include "aeolia.h"
#include "ps4-led.h"

static const u8 led_off[] = {
	0x03, 0x01, 0x00, 0x00, 0x10, 0x01, 0x02, 0x00,
	0x02, 0x01, 0x00, 0x11, 0x01, 0x02, 0x00, 0x02,
	0x01, 0x00, 0x02, 0x03, 0x01, 0x00, 0x04, 0x01,
	0xbf, 0x02, 0x00, 0x05, 0x01, 0xff, 0x02, 0x00,
	0x05, 0x01, 0xff
};

static const u8 led_blue[] = {
	0x03, 0x01, 0x00, 0x00, 0x10, 0x01, 0x02, 0xff,
	0x02, 0x01, 0x00, 0x11, 0x01, 0x02, 0x00, 0x02,
	0x01, 0x00, 0x02, 0x03, 0x01, 0x00, 0x04, 0x01,
	0xbf, 0x02, 0x00, 0x05, 0x01, 0xff, 0x02, 0x00,
	0x05, 0x01, 0xff
};

static const u8 led_white[] = {
	0x03, 0x01, 0x00, 0x00, 0x10, 0x01, 0x02, 0x00,
	0x02, 0x01, 0x00, 0x11, 0x01, 0x02, 0xff, 0x02,
	0x01, 0x00, 0x02, 0x03, 0x01, 0x00, 0x04, 0x01,
	0xbf, 0x02, 0x00, 0x05, 0x01, 0xff, 0x02, 0x00,
	0x05, 0x01, 0xff
};

static const u8 led_orange[] = {
	0x03, 0x01, 0x00, 0x00, 0x10, 0x01, 0x02, 0x00,
	0x02, 0x01, 0x00, 0x11, 0x01, 0x02, 0x00, 0x02,
	0x01, 0x00, 0x02, 0x03, 0x02, 0xff, 0x02, 0x01,
	0x00, 0x02, 0xff, 0x05, 0x01, 0xff, 0x02, 0xff,
	0x05, 0x01, 0x00
};

static const u8 led_orange_blue[] = {
	0x03, 0x01, 0x00, 0x00, 0x10, 0x01, 0x02, 0xff,
	0x02, 0x01, 0x00, 0x11, 0x01, 0x02, 0x00, 0x02,
	0x01, 0x00, 0x02, 0x03, 0x01, 0x00, 0x04, 0x01,
	0xbf, 0x02, 0xff, 0x05, 0x01, 0xff, 0x02, 0x00,
	0x05, 0x01, 0xff
};

static const u8 led_orange_white[] = {
	0x03, 0x01, 0x00, 0x00, 0x10, 0x01, 0x02, 0x00,
	0x02, 0x01, 0x00, 0x11, 0x01, 0x02, 0xff, 0x02,
	0x01, 0x00, 0x02, 0x03, 0x01, 0x00, 0x04, 0x01,
	0xbf, 0x02, 0xff, 0x05, 0x01, 0xff, 0x02, 0x00,
	0x05, 0x01, 0xff
};

static const u8 led_pulsate_orange[] = {
	0x03, 0x01, 0x00, 0x00, 0x10, 0x01, 0x02, 0x00,
	0x02, 0x01, 0x00, 0x11, 0x01, 0x02, 0x00, 0x02,
	0x01, 0x00, 0x02, 0x03, 0x01, 0xff, 0x04, 0x01,
	0x00, 0x02, 0xff, 0x05, 0x01, 0xff, 0x02, 0xff,
	0x05, 0x01, 0x00
};

static const u8 led_orange_white_blue[] = {
	0x03, 0x01, 0x00, 0x00, 0x10, 0x01, 0x02, 0xff,
	0x02, 0x01, 0x00, 0x11, 0x01, 0x02, 0xff, 0x02,
	0x01, 0x00, 0x02, 0x03, 0x01, 0x00, 0x04, 0x01,
	0xbf, 0x02, 0xff, 0x05, 0x01, 0xff, 0x02, 0x00,
	0x05, 0x01, 0xff
};

static const u8 led_white_blue[] = {
	0x03, 0x01, 0x00, 0x00, 0x10, 0x01, 0x02, 0xff,
	0x02, 0x01, 0x00, 0x11, 0x01, 0x02, 0xff, 0x02,
	0x01, 0x00, 0x02, 0x03, 0x01, 0x00, 0x04, 0x01,
	0xbf, 0x02, 0x00, 0x05, 0x01, 0xff, 0x02, 0x00,
	0x05, 0x01, 0xff
};

static const u8 led_violet_blue[] = {
	0x03, 0x01, 0x00, 0x00, 0x10, 0x01, 0x02, 0x57,
	0x02, 0x01, 0x00, 0x11, 0x01, 0x02, 0x00, 0x02,
	0x01, 0x00, 0x02, 0x03, 0x02, 0xff, 0x02, 0x01,
	0x00, 0x02, 0xff, 0x05, 0x01, 0xff, 0x02, 0xff,
	0x05, 0x01, 0x00
};

static const u8 led_pink[] = {
	0x03, 0x01, 0x00, 0x00, 0x10, 0x01, 0x02, 0x00,
	0x02, 0x01, 0x00, 0x11, 0x01, 0x02, 0x30, 0x02,
	0x01, 0x00, 0x02, 0x03, 0x02, 0xff, 0x02, 0x01,
	0x00, 0x02, 0xff, 0x05, 0x01, 0xff, 0x02, 0xff,
	0x05, 0x01, 0x00
};

static const u8 led_pink_blue[] = {
	0x03, 0x01, 0x00, 0x00, 0x10, 0x01, 0x02, 0x20,
	0x02, 0x01, 0x00, 0x11, 0x01, 0x02, 0x00, 0x02,
	0x01, 0x00, 0x02, 0x03, 0x02, 0xff, 0x02, 0x01,
	0x00, 0x02, 0xff, 0x05, 0x01, 0xff, 0x02, 0xff,
	0x05, 0x01, 0x00
};

struct ps4_led_priv {
	struct platform_device  *pdev;
	struct delayed_work      thermal_work;
	struct mutex             lock;
	enum ps4_led_mode        mode;
	unsigned int             thermal_interval_ms;
};

static void ps4_thermal_work_fn(struct work_struct *work)
{
	struct ps4_led_priv *priv =
		container_of(to_delayed_work(work),
			     struct ps4_led_priv, thermal_work);
	u8 temp_reply[PS4_LED_TEMP_REPLY_LEN];
	u8 led_reply[0x30];
	const u8 *payload;
	unsigned int interval_ms;
	int ret;

	mutex_lock(&priv->lock);
	if (priv->mode != MODE_THERMAL) {
		mutex_unlock(&priv->lock);
		return;
	}
	interval_ms = priv->thermal_interval_ms;
	mutex_unlock(&priv->lock);

	memset(temp_reply, 0, sizeof(temp_reply));

	ret = apcie_icc_cmd(PS4_LED_TEMP_ICC_MAJOR, PS4_LED_TEMP_ICC_MINOR,
			    NULL, 0, temp_reply, sizeof(temp_reply));
	if (ret < 0 || temp_reply[PS4_ICC_STATUS_OK] != 0x00) {
		dev_dbg(&priv->pdev->dev,
			"thermal: ICC temp read failed (ret=%d status=0x%02x)\n",
			ret, temp_reply[0]);
		goto reschedule;
	}

	if (temp_reply[PS4_LED_TEMP_BYTE] < PS4_LED_TEMP_COOL_MAX)
		payload = led_blue;
	else if (temp_reply[PS4_LED_TEMP_BYTE] < PS4_LED_TEMP_WARM_MAX)
		payload = led_white;
	else
		payload = led_orange;

	dev_dbg(&priv->pdev->dev,
		"thermal: APU %u°C => %s\n",
		temp_reply[PS4_LED_TEMP_BYTE],
		(payload == led_blue)  ? "blue"  :
		(payload == led_white) ? "white" : "orange");

	apcie_icc_cmd(PS4_LED_ICC_MAJOR, PS4_LED_ICC_MINOR,
		      (void *)payload, PS4_LED_PAYLOAD_LEN,
		      led_reply, sizeof(led_reply));

reschedule:
	schedule_delayed_work(&priv->thermal_work,
			      msecs_to_jiffies(interval_ms));
}

static void ps4_led_set(struct led_classdev *led_cdev,
			enum led_brightness value)
{
	const u8 *data = led_off;
	u8 reply[0x30];

	if (value != LED_OFF) {
		if (strstr(led_cdev->name, "orange_white_blue"))
			data = led_orange_white_blue;
		else if (strstr(led_cdev->name, "pulsate_orange"))
			data = led_pulsate_orange;
		else if (strstr(led_cdev->name, "orange_white"))
			data = led_orange_white;
		else if (strstr(led_cdev->name, "orange_blue"))
			data = led_orange_blue;
		else if (strstr(led_cdev->name, "white_blue"))
			data = led_white_blue;
		else if (strstr(led_cdev->name, "violet_blue"))
			data = led_violet_blue;
		else if (strstr(led_cdev->name, "pink_blue"))
			data = led_pink_blue;
		else if (strstr(led_cdev->name, "pink"))
			data = led_pink;
		else if (strstr(led_cdev->name, "orange"))
			data = led_orange;
		else if (strstr(led_cdev->name, "white"))
			data = led_white;
		else if (strstr(led_cdev->name, "blue"))
			data = led_blue;
	}

	apcie_icc_cmd(PS4_LED_ICC_MAJOR, PS4_LED_ICC_MINOR,
		      (void *)data, PS4_LED_PAYLOAD_LEN,
		      reply, sizeof(reply));
}

static struct led_classdev ps4_led_nodes[] = {
	{ .name = "ps4:blue:status",              .brightness_set = ps4_led_set },
	{ .name = "ps4:white:status",             .brightness_set = ps4_led_set },
	{ .name = "ps4:orange:status",            .brightness_set = ps4_led_set },
	{ .name = "ps4:orange_blue:status",       .brightness_set = ps4_led_set },
	{ .name = "ps4:orange_white:status",      .brightness_set = ps4_led_set },
	{ .name = "ps4:pulsate_orange:status",    .brightness_set = ps4_led_set },
	{ .name = "ps4:orange_white_blue:status", .brightness_set = ps4_led_set },
	{ .name = "ps4:white_blue:status",        .brightness_set = ps4_led_set },
	{ .name = "ps4:violet_blue:status",       .brightness_set = ps4_led_set },
	{ .name = "ps4:pink:status",              .brightness_set = ps4_led_set },
	{ .name = "ps4:pink_blue:status",         .brightness_set = ps4_led_set },
};

static ssize_t mode_show(struct device *dev,
			 struct device_attribute *attr, char *buf)
{
	struct ps4_led_priv *priv = dev_get_drvdata(dev);
	const char *s;

	mutex_lock(&priv->lock);
	s = (priv->mode == MODE_THERMAL) ? "thermal" : "static";
	mutex_unlock(&priv->lock);

	return sysfs_emit(buf, "%s\n", s);
}

static ssize_t mode_store(struct device *dev,
			  struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct ps4_led_priv *priv = dev_get_drvdata(dev);
	bool want_thermal;

	if (sysfs_streq(buf, "thermal"))
		want_thermal = true;
	else if (sysfs_streq(buf, "static"))
		want_thermal = false;
	else
		return -EINVAL;

	mutex_lock(&priv->lock);

	if (want_thermal && priv->mode != MODE_THERMAL) {
		priv->mode = MODE_THERMAL;
		mutex_unlock(&priv->lock);
		schedule_delayed_work(&priv->thermal_work, 0);
	} else if (!want_thermal && priv->mode == MODE_THERMAL) {
		priv->mode = MODE_STATIC;
		mutex_unlock(&priv->lock);
		cancel_delayed_work_sync(&priv->thermal_work);
	} else {
		mutex_unlock(&priv->lock);
	}

	return count;
}
static DEVICE_ATTR_RW(mode);

static ssize_t thermal_interval_ms_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct ps4_led_priv *priv = dev_get_drvdata(dev);
	unsigned int v;

	mutex_lock(&priv->lock);
	v = priv->thermal_interval_ms;
	mutex_unlock(&priv->lock);

	return sysfs_emit(buf, "%u\n", v);
}

static ssize_t thermal_interval_ms_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count)
{
	struct ps4_led_priv *priv = dev_get_drvdata(dev);
	unsigned int val;

	if (kstrtouint(buf, 10, &val) || val == 0)
		return -EINVAL;

	mutex_lock(&priv->lock);
	priv->thermal_interval_ms = val;
	mutex_unlock(&priv->lock);

	return count;
}
static DEVICE_ATTR_RW(thermal_interval_ms);

static struct attribute *ps4_led_attrs[] = {
	&dev_attr_mode.attr,
	&dev_attr_thermal_interval_ms.attr,
	NULL,
};
ATTRIBUTE_GROUPS(ps4_led);

static int ps4_led_probe(struct platform_device *pdev)
{
	struct ps4_led_priv *priv;
	int i, ret;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->pdev                = pdev;
	priv->mode                = MODE_STATIC;
	priv->thermal_interval_ms = PS4_LED_THERMAL_INTERVAL_MS_DEFAULT;

	mutex_init(&priv->lock);
	INIT_DELAYED_WORK(&priv->thermal_work, ps4_thermal_work_fn);

	platform_set_drvdata(pdev, priv);

	for (i = 0; i < ARRAY_SIZE(ps4_led_nodes); i++) {
		ret = devm_led_classdev_register(&pdev->dev,
						 &ps4_led_nodes[i]);
		if (ret) {
			dev_err(&pdev->dev,
				"failed to register LED node %d: %d\n",
				i, ret);
			return ret;
		}
	}

	dev_info(&pdev->dev,
		 "PS4 LED driver ready. "
		 "Write 'thermal' to mode attribute to enable thermal indicator.\n");

	return 0;
}

static void ps4_led_remove(struct platform_device *pdev)
{
	struct ps4_led_priv *priv = platform_get_drvdata(pdev);

	cancel_delayed_work_sync(&priv->thermal_work);
}

static struct platform_driver ps4_led_driver = {
	.probe  = ps4_led_probe,
	.remove = ps4_led_remove,
	.driver = {
		.name       = "ps4-led",
		.dev_groups = ps4_led_groups,
	},
};

static struct platform_device *ps4_led_pdev;

static int __init ps4_led_init(void)
{
	int ret;

	ret = platform_driver_register(&ps4_led_driver);
	if (ret) {
		pr_err("ps4-led: failed to register platform driver: %d\n",
		       ret);
		return ret;
	}

	ps4_led_pdev = platform_device_register_simple("ps4-led", -1,
						       NULL, 0);
	if (IS_ERR(ps4_led_pdev)) {
		ret = PTR_ERR(ps4_led_pdev);
		pr_err("ps4-led: failed to register platform device: %d\n",
		       ret);
		platform_driver_unregister(&ps4_led_driver);
		ps4_led_pdev = NULL;
		return ret;
	}

	return 0;
}

static void __exit ps4_led_exit(void)
{
	if (ps4_led_pdev)
		platform_device_unregister(ps4_led_pdev);

	platform_driver_unregister(&ps4_led_driver);
}

module_init(ps4_led_init);
module_exit(ps4_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("rmux <armandas.kvietkus@proton.me>");
MODULE_DESCRIPTION("PS4 Aeolia front panel LED driver with Thermal Indicator Mode");
MODULE_ALIAS("platform:ps4-led");
