/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2019 Olimex Ltd.
 *   Author: Stefan Mavrodiev <stefan@olimex.com>
 */

#include <common.h>
#include <backlight.h>
#include <dm.h>
#include <i2c.h>
#include <panel.h>
#include <asm/gpio.h>
#include <power/regulator.h>

#define LCD_OLINUXINO_HEADER_MAGIC      0x4F4CB727

struct lcd_olinuxino_mode {
	u32 pixelclock;
	u32 hactive;
	u32 hfp;
	u32 hbp;
	u32 hpw;
	u32 vactive;
	u32 vfp;
	u32 vbp;
	u32 vpw;
	u32 refresh;
	u32 flags;
};

struct lcd_olinuxino_info {
	char name[32];
	u32 width_mm;
	u32 height_mm;
	u32 bpc;
	u32 bus_format;
	u32 bus_flag;
} __attribute__((__packed__));

struct lcd_olinuxino_eeprom {
	u32 header;
	u32 id;
	char revision[4];
	u32 serial;
	struct lcd_olinuxino_info info;
	u32 num_modes;
	u8 reserved[180];
	u32 checksum;
} __attribute__((__packed__));


#define OLINUXINO_PANEL(_id, _name, _pclk, \
	_hactive, _hfp, _hbp, _hpw, \
	_vactive, _vfp, _vbp, _vpw, _flags) \
	{ \
		.id = _id, \
		.name = _name, \
		{ \
			.pixelclock = _pclk, \
			.hactive = _hactive, \
			.hfp = _hfp, \
			.hbp = _hbp, \
			.hpw = _hpw, \
			.vactive = _vactive, \
			.vfp = _vfp, \
			.vbp = _vbp, \
			.vpw = _vpw, \
			.flags = _flags \
		} \
	}

static struct lcd_olinuxino_board {
	uint32_t id;
	char name[32];
	struct lcd_olinuxino_mode mode;
} lcd_olinuxino_b[] = {
	OLINUXINO_PANEL(7859, "LCD-OLinuXino-4.3TS", 12000, 480, 8, 23, 20, 272, 4, 13, 10, 0),
	OLINUXINO_PANEL(8630, "LCD-OLinuXino-5", 33000, 800, 210, 26, 20, 480, 2, 13, 10, 0),
	OLINUXINO_PANEL(7864, "LCD-OLinuXino-7", 33000, 800, 210, 26, 20, 480, 2, 13, 10, 0),
	OLINUXINO_PANEL(9278, "LCD-OLinuXino-7CTS", 45000, 1024, 10, 160, 6, 600, 1, 22, 1, 0),
	OLINUXINO_PANEL(7862, "LCD-OLinuXino-10", 45000, 1024, 10, 160, 6, 600, 1, 22, 1, 0),
	OLINUXINO_PANEL(9284, "LCD-OLinuXino-10CTS", 45000, 1024, 10, 160, 6, 600, 1, 22, 1, 0),
};

struct olinuxino_panel_priv {
	struct udevice *reg;
	struct udevice *backlight;
	struct gpio_desc enable;
	struct lcd_olinuxino_eeprom eeprom;
};

static int olinuxino_panel_get_display_timing(struct udevice *dev,
					      struct display_timing *timing)
{
	struct olinuxino_panel_priv *priv = dev_get_priv(dev);
	struct lcd_olinuxino_mode *mode =
		(struct lcd_olinuxino_mode *)priv->eeprom.reserved;

	memset(timing, 0, sizeof(*timing));

	timing->pixelclock.typ = mode->pixelclock * 1000;
	timing->pixelclock.min = timing->pixelclock.typ;
	timing->pixelclock.max = timing->pixelclock.typ;

	timing->hactive.typ = mode->hactive;
	timing->hactive.min = timing->hactive.typ;
	timing->hactive.max = timing->hactive.typ;

	timing->hfront_porch.typ = mode->hfp;
	timing->hfront_porch.min = timing->hfront_porch.typ;
	timing->hfront_porch.max = timing->hfront_porch.typ;

	timing->hback_porch.typ = mode->hbp;
	timing->hback_porch.min = timing->hback_porch.typ;
	timing->hback_porch.max = timing->hback_porch.typ;

	timing->hsync_len.typ = mode->hpw;
	timing->hsync_len.min = timing->hsync_len.typ;
	timing->hsync_len.max = timing->hsync_len.typ;

	timing->vactive.typ = mode->vactive;
	timing->vactive.min = timing->vactive.typ;
	timing->vactive.max = timing->vactive.typ;

	timing->vfront_porch.typ = mode->vfp;
	timing->vfront_porch.min = timing->vfront_porch.typ;
	timing->vfront_porch.max = timing->vfront_porch.typ;

	timing->vback_porch.typ = mode->vbp;
	timing->vback_porch.min = timing->vback_porch.typ;
	timing->vback_porch.max = timing->vback_porch.typ;

	timing->vsync_len.typ = mode->vpw;
	timing->vsync_len.min = timing->vsync_len.typ;
	timing->vsync_len.max = timing->vsync_len.typ;

	timing->flags = mode->flags;
	timing->hdmi_monitor = false;

	return 0;
}

static int olinuxino_panel_enable_backlight(struct udevice *dev)
{
	struct olinuxino_panel_priv *priv = dev_get_priv(dev);
	int ret;

	debug("%s: start, backlight = '%s'\n", __func__, priv->backlight->name);
	dm_gpio_set_value(&priv->enable, 1);
	ret = backlight_enable(priv->backlight);
	debug("%s: done, ret = %d\n", __func__, ret);
	if (ret)
		return ret;

	return 0;
}

static int olinuxino_panel_set_backlight(struct udevice *dev, int percent)
{
	struct olinuxino_panel_priv *priv = dev_get_priv(dev);
	int ret;

	debug("%s: start, backlight = '%s'\n", __func__, priv->backlight->name);
	dm_gpio_set_value(&priv->enable, 1);
	ret = backlight_set_brightness(priv->backlight, percent);
	debug("%s: done, ret = %d\n", __func__, ret);
	if (ret)
		return ret;

	return 0;
}

static int olinuxino_panel_ofdata_to_platdata(struct udevice *dev)
{
	struct olinuxino_panel_priv *priv = dev_get_priv(dev);
	int ret;

	if (IS_ENABLED(CONFIG_DM_REGULATOR)) {
		ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
						   "power-supply", &priv->reg);
		if (ret) {
			debug("%s: Warning: cannot get power supply: ret=%d\n",
			      __func__, ret);
			if (ret != -ENOENT)
				return ret;
		}
	}
	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
					   "backlight", &priv->backlight);
	if (ret) {
		debug("%s: Cannot get backlight: ret=%d\n", __func__, ret);
		return log_ret(ret);
	}
	ret = gpio_request_by_name(dev, "enable-gpios", 0, &priv->enable,
				   GPIOD_IS_OUT);
	if (ret) {
		debug("%s: Warning: cannot get enable GPIO: ret=%d\n",
		      __func__, ret);
		if (ret != -ENOENT)
			return log_ret(ret);
	}

	return 0;
}

static int olinuxino_panel_probe(struct udevice *dev)
{
	struct olinuxino_panel_priv *priv = dev_get_priv(dev);
	struct udevice *chip;
	int ret;
	u32 crc;

	memset((u8 *)&priv->eeprom, 0, 256);

	/**
	 * If the panel is subnode of i2c device, then try autodetect
	 * function. Otherwise, get lcd_olinuxino env variable and try
	 * manual timing setting.
	 */
	if (device_get_uclass_id(dev->parent) == UCLASS_I2C) {
		ret = dm_i2c_probe(dev->parent, 0x50, 0, &chip);
		if (ret)
			return -ENODEV;

		ret = dm_i2c_read(chip, 0x00, (u8 *)&priv->eeprom, 256);
		if (ret)
			return -ENODEV;

		if (priv->eeprom.header != LCD_OLINUXINO_HEADER_MAGIC)
			return -ENODEV;

		crc = crc32(0L, (u8 *)&priv->eeprom, 252);
		if (priv->eeprom.checksum != crc)
			return -ENODEV;

		if (IS_ENABLED(CONFIG_DM_REGULATOR) && priv->reg) {
			debug("%s: Enable regulator '%s'\n", __func__, priv->reg->name);
			ret = regulator_set_enable(priv->reg, true);
			if (ret)
				return ret;
		}

		printf("LCD: %s, Rev.%s, Serial:%08x\n",
		       priv->eeprom.info.name,
		       priv->eeprom.revision,
		       priv->eeprom.serial);
	} else {
		u32 id = env_get_ulong("lcd_olinuxino", 10, 0);
		u32 i;

		if (!id)
			return -ENODEV;

		for (i = 0; i < ARRAY_SIZE(lcd_olinuxino_b); i++) {
			if (lcd_olinuxino_b[i].id == id) {
				printf("LCD: %s\n", lcd_olinuxino_b[i].name);
				memcpy((u8 *)priv->eeprom.reserved, (u8 *)&lcd_olinuxino_b[i].mode, sizeof(struct lcd_olinuxino_mode));
				return 0;
			}
		}

		return -ENODEV;
	}

	return 0;
}

static const struct panel_ops olinuxino_panel_ops = {
	.get_display_timing	= olinuxino_panel_get_display_timing,
	.enable_backlight	= olinuxino_panel_enable_backlight,
	.set_backlight		= olinuxino_panel_set_backlight,
};

static const struct udevice_id olinuxino_panel_ids[] = {
	{ .compatible = "olimex,lcd-olinuxino" },
	{ }
};

U_BOOT_DRIVER(olinuxino_panel) = {
	.name	= "olinuxino_panel",
	.id	= UCLASS_PANEL,
	.of_match = olinuxino_panel_ids,
	.ops	= &olinuxino_panel_ops,
	.ofdata_to_platdata	= olinuxino_panel_ofdata_to_platdata,
	.probe		= olinuxino_panel_probe,
	.priv_auto_alloc_size	= sizeof(struct olinuxino_panel_priv),
};
