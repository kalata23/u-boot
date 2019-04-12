/*
 * Device Tree fixup for A20-OLinuXino
 *
 * Copyright (C) 2018 Olimex Ltd.
 *   Author: Stefan Mavrodiev <stefan@olimex.com>
 *
 * SPDX-License-Identifier: (GPL-2.0+ OR MIT)
 */

#include <common.h>
#include <fdt_support.h>
#include <malloc.h>
#include <mtd_node.h>
#include <asm/arch/gpio.h>
#include <jffs2/load_kernel.h>
#include <linux/libfdt.h>
#include <linux/sizes.h>

#include "../common/lcd_olinuxino.h"
#include "../common/board_detect.h"
#include "../common/boards.h"

DECLARE_GLOBAL_DATA_PTR;

static int board_fix_model(void *blob)
{
	int offset;
	int ret;
	char temp[10];

	offset = fdt_path_offset(blob, "/");
	if (offset < 0)
		return offset;

	/**
	 * / {
	 * 	model = <name>;
	 *	id = <id>;
	 *	revision = <revision>;
	 */
	sprintf(temp, "%c%c", eeprom->revision.major, eeprom->revision.minor);
	ret = fdt_setprop_string(blob, offset, "revision", (const char *)temp);
	sprintf(temp, "%d", eeprom->id);
	ret |= fdt_setprop_string(blob, offset, "id",(const char *)temp);
	ret |= fdt_setprop_string(blob, offset, "model", olimex_get_board_name());

	return ret;

}

static int board_fix_atecc508a(void *blob)
{
	int offset;
	int ret;

	/**
	 * Enabled on:
	 *   - A20-SOM204-1Gs16Me16G-MC (8958)
	 */
	if (eeprom->id != 8958)
		return 0;

	/**
	 * Add the following node:
	 * &i2c {
	 *     atecc508a@60 {
	 *         compatible = "atmel,atecc508a";
	 *         reg = <0x60>;
	 * };
	 */
	offset = fdt_path_offset(blob, "/soc@1c00000/i2c@1c2b400");
 	if (offset < 0)
 		return offset;

	offset = fdt_add_subnode(blob, offset, "atecc508a@60");
	if (offset < 0)
		return offset;

	ret = fdt_setprop_u32(blob, offset, "reg", 0x60);
	ret |= fdt_setprop_string(blob, offset, "compatible", "atmel,atecc508a");

	return ret;
}

static int board_fix_spi_flash(void *blob)
{
	uint32_t phandle;
	int offset, ret = 0;

	/**
	 * Some boards, have both eMMC and SPI flash
	 */
	if (!olimex_board_has_spi())
		return 0;

	/*
	 * Find /soc@01c00000/pinctrl@01c20800
	 * Add following properties:
	 *     spi0-pc-pins {
	 *         pins = "PC0", "PC1", "PC2", "PC23";
	 *         function = "spi0";
	 *     };
	 *
	 * Test:
	 * fdt print /soc@01c00000/pinctrl@01c20800/spi0@1
	 */

	offset = fdt_path_offset(blob, "/soc/pinctrl@1c20800");
	if (offset < 0)
		return offset;

	offset = fdt_add_subnode(blob, offset, "spi0-pc-pins");
	if (offset < 0)
		return offset;

	/* Generate phandle */
	phandle = fdt_create_phandle(blob, offset);
	if (!phandle)
		return -1;

	ret |= fdt_setprop_string(blob, offset, "function" , "spi0");
	ret |= fdt_setprop_string(blob, offset, "pins" , "PC0");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PC1");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PC2");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PC23");
	if (ret < 0)
		return ret;

	/**
	 * Find /soc@01c00000/spi@01c05000
	 *
	 * Change following properties:
	 *   - pinctrl-names = "default";
	 *   - pinctrl-0 = <&spi0@1>;
	 *   - spi-max-frequency = <20000000>;
	 *   - status = "okay";
	 *
	 * Test:
	 * fdt print /soc@01c00000/spi@01c05000
	 */
	offset = fdt_path_offset(blob, "/soc/spi@1c05000");
 	if (offset < 0)
 		return offset;

	/* Change status to okay */
	ret |= fdt_set_node_status(blob, offset, FDT_STATUS_OKAY, 0);
	ret |= fdt_setprop_u32(blob, offset, "spi-max-frequency", 20000000);
	ret |= fdt_setprop_u32(blob, offset, "pinctrl-0", phandle);
	ret |= fdt_setprop_string(blob, offset, "pinctrl-names", "default");
	if (ret < 0)
		return ret;

	/**
	 * Add the following node:
	 * spi-nor@0 {
	 *     #address-cells = <1>;
	 *     #size-cells = <1>;
	 *     compatible = "winbond,w25q128", "jedec,spi-nor", "spi-flash";
	 *     reg = <0>;
	 *     spi-max-frequency = <20000000>;
	 *     status = "okay";
	 * }
	 */
	offset = fdt_add_subnode(blob, offset, "spi-nor@0");
	if (offset < 0)
		return offset;

	ret |= fdt_set_node_status(blob, offset, FDT_STATUS_OKAY, 0);
	ret |= fdt_setprop_u32(blob, offset, "spi-max-frequency", 20000000);
	ret |= fdt_setprop_u32(blob, offset, "reg", 0);
	ret |= fdt_setprop_u32(blob, offset, "#size-cells", 1);
	ret |= fdt_setprop_u32(blob, offset, "#address-cells", 1);
	ret |= fdt_setprop_string(blob, offset, "compatible", "winbond,w25q128");
	ret |= fdt_appendprop_string(blob, offset, "compatible", "jedec,spi-nor");
	ret |= fdt_appendprop_string(blob, offset, "compatible", "spi-flash");
	if (ret < 0)
		return ret;

	/* Aliases should be modified only before relocation */
	if (!gd->flags & GD_FLG_RELOC)
		return 0;
	/*
	 * Add alias property
	 *
	 * fdt print /aliases
	 *     spi0 = "/soc@01c00000/spi@01c05000"
	 */
	offset = fdt_path_offset(blob, "/aliases");
	if (offset < 0)
		return offset;

	return fdt_setprop_string(blob, offset, "spi0", "/soc/spi@1c05000");
}

static int board_fix_nand(void *blob)
{
	int offset;
	uint32_t phandle;
	int ret = 0;

	/* Modify only boards with nand storage */
	if (eeprom->config.storage != 'n')
		return 0;

	/*
	 * Find /soc@01c00000/pinctrl@01c20800
	 * Add following properties:
	 *     nand0@0 {
	 *         pins = "PC0", "PC1", "PC2", PC4, "PC5", PC6, "PC8",
	 *		"PC9", "PC10", "PC11", "PC12", "PC13",
	 *		"PC14", "PC15", "PC16";
	 *         function = "nand0";
	 *     };
	 *
	 * Test:
	 * fdt print /soc@01c00000/pinctrl@01c20800/nand0@0
	 */

	offset = fdt_path_offset(blob, "/soc/pinctrl@1c20800");
	if (offset < 0)
		return offset;

 	offset = fdt_add_subnode(blob, offset, "nand-pins");
 	if (offset < 0)
 		return offset;

	phandle = fdt_create_phandle(blob, offset);
	if (!phandle)
		return -1;

 	ret |= fdt_setprop_string(blob, offset, "function" , "nand0");

 	ret |= fdt_setprop_string(blob, offset, "pins" , "PC0");
 	ret |= fdt_appendprop_string(blob, offset, "pins", "PC1");
 	ret |= fdt_appendprop_string(blob, offset, "pins", "PC2");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PC4");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PC5");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PC6");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PC8");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PC9");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PC10");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PC11");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PC12");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PC13");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PC14");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PC15");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PC16");
 	if (ret < 0)
 		return ret;

	/**
	 * Find /soc@01c00000/nand@01c03000
	 *
	 * Change following properties:
	 *   - pinctrl-names = "default";
	 *   - pinctrl-0 = <&nand0@0>;
	 *   - #address-cells = <1>;
	 *   - #size-cells = <0>;
	 *   - status = "okay";
	 *
	 * Test:
	 * fdt print /soc@01c00000/nand@01c03000
	 */

	offset = fdt_path_offset(blob, "/soc/nand@1c03000");
 	if (offset < 0)
 		return offset;

	/* Change status to okay */
	ret |= fdt_set_node_status(blob, offset, FDT_STATUS_OKAY, 0);
	ret |= fdt_setprop_u32(blob, offset, "#size-cells", 0);
	ret |= fdt_setprop_u32(blob, offset, "#address-cells", 1);
	ret |= fdt_setprop_u32(blob, offset, "pinctrl-0", phandle);
	ret |= fdt_setprop_string(blob, offset, "pinctrl-names", "default");
	if (ret < 0)
		return ret;

	/**
	 * Add the following node:
	 * nand@0 {
	 *     reg = <0>;
	 *     allwinner,rb = <0>;
	 *     nand-ecc-mode = "hw";
	 *     nand-on-flash-bbt;
	 *    }
	 */
	offset = fdt_add_subnode(blob, offset, "nand@0");
	if (offset < 0)
		return offset;

	ret |= fdt_setprop_empty(blob, offset, "nand-on-flash-bbt");
	ret |= fdt_setprop_string(blob, offset, "nand-ecc-mode", "hw");
	ret |= fdt_setprop_u32(blob, offset, "allwinner,rb", 0);
	ret |= fdt_setprop_u32(blob, offset, "reg", 0);
	if (ret < 0)
		return ret;

	offset = fdt_add_subnode(blob, offset, "partitions");
	if (offset < 0)
		return offset;

	ret |= fdt_setprop_string(blob, offset, "compatible" , "fixed-partitions");
	ret |= fdt_setprop_u32(blob, offset, "#size-cells", 2);
	ret |= fdt_setprop_u32(blob, offset, "#address-cells", 2);
	return ret;
}

static int (*olinuxino_fixes[]) (void *blob) = {
	board_fix_model,
	board_fix_spi_flash,
	board_fix_atecc508a,
	board_fix_nand,
};

int olinuxino_fdt_fixup(void *blob)
{
	uint8_t i;
	int ret;

	ret = fdt_increase_size(blob, 65535);
	if (ret < 0)
		return ret;

	/* Apply fixes */
	for (i = 0; i < ARRAY_SIZE(olinuxino_fixes); i++) {
		ret = olinuxino_fixes[i](blob);
		if (ret < 0)
			return ret;
	}

	return 0;
}

#ifdef CONFIG_VIDEO_LCD_PANEL_OLINUXINO
static int board_fix_lcd_olinuxino_lvds(void *blob)
{
	struct lcd_olinuxino_board *lcd = lcd_olinuxino_get_data();

	uint32_t backlight_phandle;
	uint32_t ccu_phandle;
	uint32_t panel_endpoint_phandle;
	uint32_t pinctrl_phandle;
	uint32_t pins_phandle[2] = {};
	uint32_t power_supply_phandle;
	uint32_t pwm_phandle;
	uint32_t tcon0_endpoint_phandle;

	fdt32_t ccu[2];
	fdt32_t gpios[4];
	fdt32_t levels[11];
	fdt32_t phandles[2];

	int gpio;
	int i;
	int offset;
	int ret = 0;

	offset = fdt_path_offset(blob, "/soc/pinctrl@1c20800");
 	if (offset < 0)
 		return offset;

	pinctrl_phandle = fdt_get_phandle(blob, offset);
	if (pinctrl_phandle < 0)
 		return offset;

	offset = fdt_path_offset(blob, "/soc/clock@1c20000");
 	if (offset < 0)
 		return offset;

	ccu_phandle = fdt_get_phandle(blob, offset);
	if (ccu_phandle < 0)
 		return offset;


	offset = fdt_path_offset(blob, "/vcc5v0");
 	if (offset < 0)
 		return offset;

	power_supply_phandle = fdt_get_phandle(blob, offset);
	if (power_supply_phandle < 0)
 		return offset;

	/**
	 * &pwm {
	 * 	pinctrl-names = "default";
	 *	pinctrl-0 = <&pwm0_pins_a>;
	 *	status = "okay";
	 * };
	 */

	offset = fdt_path_offset(blob, "/soc/pinctrl@1c20800/pwm0-pin");
	if (offset < 0)
		return offset;

	pins_phandle[0] = fdt_create_phandle(blob, offset);
	if (!pins_phandle[0])
		return -1;

	offset = fdt_path_offset(blob, "/soc/pwm@1c20e00");
	if (offset < 0)
		return offset;

	ret |= fdt_set_node_status(blob, offset, FDT_STATUS_OKAY, 0);
	ret |= fdt_setprop_u32(blob, offset, "pinctrl-0", pins_phandle[0]);
	ret |= fdt_setprop_string(blob, offset, "pinctrl-names", "default");
	if (ret < 0)
		return ret;

	pwm_phandle = fdt_create_phandle(blob, offset);
	if (!pwm_phandle)
		return -1;

	/**
	 * backlight: backlight {
	 * 	compatible = "pwm-backlight";
	 * 	power-supply = <&reg_vcc5v0>;
	 * 	pwms = <&pwm 0 50000 0>;
	 * 	brightness-levels = <0 10 20 30 40 50 60 70 80 90 100>;
	 *	default-brightness-level = <10>;
	 * };
	 */

	offset = fdt_path_offset(blob, "/");
	if (offset < 0)
		return offset;

	offset = fdt_add_subnode(blob, offset, "backlight");
	if (offset < 0)
		return offset;

	gpios[0] = cpu_to_fdt32(pwm_phandle);
	gpios[1] = cpu_to_fdt32(0);
	gpios[2] = cpu_to_fdt32(50000);
	gpios[3] = cpu_to_fdt32(0);
	ret = fdt_setprop(blob, offset, "pwms", gpios, sizeof(gpios));

	for (i = 0; i < 11; i++)
		levels[i] = cpu_to_fdt32(i * 10);
	ret |= fdt_setprop(blob, offset, "brightness-levels", levels, sizeof(levels));
	ret |= fdt_setprop_u32(blob, offset, "default-brightness-level", 10);
	ret |= fdt_setprop_u32(blob, offset, "power-supply", power_supply_phandle);
	ret |= fdt_setprop_string(blob, offset, "compatible", "pwm-backlight");
	if (ret < 0)
		return ret;

	backlight_phandle = fdt_create_phandle(blob, offset);
	if (!backlight_phandle)
		return -1;


	/**
	 * lcd0_lvds0_pins: lcd0_lvds0_pins@0 {
	 * 	pins = "PD0", "PD1", "PD2", "PD3", "PD4", "PD5",
	 * 		"PD6", "PD7", "PD8", "PD9";
	 * 	function = "lvds0";
	 * };
	 *
	 * lcd0_lvds1_pins: lcd0_lvds1_pins@0 {
	 * 	pins = "PD10", "PD11", "PD12", "PD13", "PD14", "PD15",
	 * 		"PD16", "PD17", "PD18", "PD19";
	 * 	function = "lvds1";
	 * };
	 */

	offset = fdt_path_offset(blob, "/soc/pinctrl@1c20800");
 	if (offset < 0)
 		return offset;

	offset = fdt_add_subnode(blob, offset, "lcd0_lvds0_pins");
	if (offset < 0)
		return offset;

	pins_phandle[0] = fdt_create_phandle(blob, offset);
	if (!pins_phandle[0])
		return -1;

	ret = fdt_setprop_string(blob, offset, "function" , "lvds0");
	ret |= fdt_setprop_string(blob, offset, "pins" , "PD0");
 	ret |= fdt_appendprop_string(blob, offset, "pins", "PD1");
 	ret |= fdt_appendprop_string(blob, offset, "pins", "PD2");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD4");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD5");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD6");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD7");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD8");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD9");
	if (ret < 0)
 		return ret;

	offset = fdt_path_offset(blob, "/soc/pinctrl@1c20800");
 	if (offset < 0)
 		return offset;

	offset = fdt_add_subnode(blob, offset, "lcd0_lvds1_pins");
	if (offset < 0)
		return offset;

	pins_phandle[1] = fdt_create_phandle(blob, offset);
	if (!pins_phandle[1])
		return -1;

	ret = fdt_setprop_string(blob, offset, "function" , "lvds1");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD10");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD11");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD12");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD13");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD14");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD15");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD16");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD17");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD18");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD19");
	if (ret < 0)
 		return ret;


	/**
	 * panel {
	 * 	compatible = "panel-lvds";
	 *
	 * 	width-mm = <344>;
	 *	height-mm = <193>;
	 *	data-mapping = "jeida-18";
	 *
	 * 	#address-cells = <1>;
	 * 	#size-cells = <0>;
	 * 	reg = <0x50>;
	 *
	 * 	pinctrl-names = "default";
	 * 	pinctrl-0 = <&lcd0_lvds1_pins &lcd0_lvds0_pins>;
	 *
	 * 	power-supply = <&reg_vcc5v0>;
	 *
	 *	enable-gpios = <&pio 7 8 GPIO_ACTIVE_HIGH>;
	 * 	backlight = <&backlight>;
	 * 	status = "okay";
	 *	panel-timing {
	 * 		clock-frequency = <71000000>;
	 * 		hactive = <1280>;
	 * 		vactive = <800>;
	 * 		hsync-len = <70>;
	 * 		hfront-porch = <20>;
	 * 		hback-porch = <70>;
	 * 		vsync-len = <5>;
	 * 		vfront-porch = <3>;
	 * 		vback-porch = <15>;
	 *	};
	 *
	 * 	port@0 {
	 * 		#address-cells = <1>;
	 * 		#size-cells = <0>;
	 * 		reg = <0>;
	 *
	 * 		panel_in_tcon0: endpoint@0 {
	 * 			#address-cells = <1>;
	 * 			#size-cells = <0>;
	 * 			reg = <0>;
	 * 			remote-endpoint = <&tcon0_out_panel>;
	 * 			};
	 *		};
	 *	};
	 * };
	 */

	offset = fdt_path_offset(blob, "/");
	if (offset < 0)
		return offset;

	offset = fdt_add_subnode(blob, offset, "panel");
	if (offset < 0)
		return offset;

	ret = fdt_setprop_string(blob, offset, "compatible", "panel-lvds");

	ret |= fdt_setprop_u32(blob, offset, "width-mm", 362);
	ret |= fdt_setprop_u32(blob, offset, "height-mm", 193);
	ret |= fdt_setprop_string(blob, offset, "data-mapping", "jeida-18");

	ret |= fdt_setprop_u32(blob, offset, "#address-cells", 1);
	ret |= fdt_setprop_u32(blob, offset, "#size-cells", 0);
	ret |= fdt_setprop_string(blob, offset, "pinctrl-names", "default");
	phandles[0] = cpu_to_fdt32(pins_phandle[0]);
	phandles[1] = cpu_to_fdt32(pins_phandle[1]);
	ret |= fdt_setprop(blob, offset, "pinctrl-0", phandles, sizeof(phandles));

	ret |= fdt_setprop_u32(blob, offset, "power-supply", power_supply_phandle);
	ret |= fdt_setprop_u32(blob, offset, "backlight", backlight_phandle);

	gpios[0] = cpu_to_fdt32(pinctrl_phandle);
	gpio = sunxi_name_to_gpio(olimex_get_lcd_pwr_pin());
	gpios[1] = cpu_to_fdt32(gpio >> 5);
	gpios[2] = cpu_to_fdt32(gpio & 0x1F);
	gpios[3] = cpu_to_fdt32(0);
	ret |= fdt_setprop(blob, offset, "enable-gpios", gpios, sizeof(gpios));
	ret |= fdt_set_node_status(blob, offset, FDT_STATUS_OKAY, 0);
	if (ret < 0)
 		return ret;


	offset = fdt_add_subnode(blob, offset, "panel-timing");

	ret = fdt_setprop_u32(blob, offset, "clock-frequency", lcd->mode.pixelclock * 1000);
	ret |= fdt_setprop_u32(blob, offset, "hactive", lcd->mode.hactive);
	ret |= fdt_setprop_u32(blob, offset, "vactive", lcd->mode.vactive);
	ret |= fdt_setprop_u32(blob, offset, "hsync-len", lcd->mode.hpw);
	ret |= fdt_setprop_u32(blob, offset, "hfront-porch", lcd->mode.hfp);
	ret |= fdt_setprop_u32(blob, offset, "hback-porch", lcd->mode.hbp);
	ret |= fdt_setprop_u32(blob, offset, "vsync-len", lcd->mode.vpw);
	ret |= fdt_setprop_u32(blob, offset, "vfront-porch", lcd->mode.vfp);
	ret |= fdt_setprop_u32(blob, offset, "vback-porch", lcd->mode.vbp);
	if (lcd->id == 7894) {
		ret |= fdt_setprop_u32(blob, offset, "hsync-active", 1);
		ret |= fdt_setprop_u32(blob, offset, "vsync-active", 1);
	}
	if (ret < 0)
 		return ret;

	offset = fdt_path_offset(blob, "/panel");
	if (offset < 0)
		return offset;

	offset = fdt_add_subnode(blob, offset, "port@0");
	if (offset < 0)
		return offset;

	ret = fdt_setprop_u32(blob, offset, "reg", 0);
	ret |= fdt_setprop_u32(blob, offset, "#size-cells", 0);
	ret |= fdt_setprop_u32(blob, offset, "#address-cells", 1);
	if (ret < 0)
 		return ret;

	offset = fdt_add_subnode(blob, offset, "endpoint@0");
	if (offset < 0)
		return offset;
	ret = fdt_setprop_u32(blob, offset, "reg", 0);
	ret |= fdt_setprop_u32(blob, offset, "#size-cells", 0);
	ret |= fdt_setprop_u32(blob, offset, "#address-cells", 1);
	if (ret < 0)
 		return ret;

	panel_endpoint_phandle = fdt_create_phandle(blob, offset);
	if (!panel_endpoint_phandle)
		return -1;


	/**
	* &tcon0_out {
	* 	#address-cells = <1>;
	* 	#size-cells = <0>;
	*
	* 	tcon0_out_panel: endpoint@0 {
	* 		#address-cells = <1>;
	* 		#size-cells = <0>;
	* 		reg = <0>;
	* 		remote-endpoint = <&panel_in_tcon0>;
	* 	};
	* };
	*/

	offset = fdt_path_offset(blob, "/soc/lcd-controller@1c0c000");
  	if (offset < 0)
  		return offset;

	ccu[0] = cpu_to_fdt32(ccu_phandle);
	ccu[1] = cpu_to_fdt32(18);
	ret |= fdt_appendprop(blob, offset, "resets", ccu, sizeof(ccu));
	ret |= fdt_appendprop_string(blob, offset, "reset-names", "lvds");
	if (ret)
		return ret;

	offset = fdt_subnode_offset(blob, offset, "ports");
	if (offset < 0)
		return offset;

	offset = fdt_subnode_offset(blob, offset, "port@1");
	if (offset < 0)
		return offset;

	offset = fdt_add_subnode(blob, offset, "endpoint@0");
	if (offset < 0)
		return offset;

	ret = fdt_setprop_u32(blob, offset, "remote-endpoint", panel_endpoint_phandle);
	ret |= fdt_setprop_u32(blob, offset, "reg", 0);
	ret |= fdt_setprop_u32(blob, offset, "#size-cells", 0);
	ret |= fdt_setprop_u32(blob, offset, "#address-cells", 1);
	if (lcd->id == 7894)
		ret |= fdt_setprop_empty(blob, offset, "allwinner,lvds-dual-link");
	if (ret < 0)
 		return ret;

	tcon0_endpoint_phandle  = fdt_create_phandle(blob, offset);
	if (!tcon0_endpoint_phandle)
		return -1;

	offset = fdt_path_offset(blob, "/panel/port@0/endpoint@0");
	if (offset < 0)
		return offset;

	ret = fdt_setprop_u32(blob, offset, "remote-endpoint", tcon0_endpoint_phandle);
	if (ret < 0)
 		return ret;

	return ret;

}

static int board_fix_lcd_olinuxino_rgb(void *blob)
{
	struct lcd_olinuxino_board *lcd = lcd_olinuxino_get_data();

	uint32_t backlight_phandle;
	uint32_t panel_endpoint_phandle;
	uint32_t pinctrl_phandle;
	uint32_t pins_phandle;
	uint32_t pwm_phandle;
	uint32_t tcon0_endpoint_phandle;

	fdt32_t gpios[4];
	fdt32_t irq[3];
	fdt32_t levels[11];

	int gpio;
	int i;
	int offset;
	int ret = 0;

	offset = fdt_path_offset(blob, "/soc/pinctrl@1c20800");
 	if (offset < 0)
 		return offset;

	pinctrl_phandle = fdt_get_phandle(blob, offset);
	if (pinctrl_phandle < 0)
 		return offset;

	/**
	 * &pwm {
	 * 	pinctrl-names = "default";
	 *	pinctrl-0 = <&pwm0_pins_a>;
	 *	status = "okay";
	 * };
	 */

	offset = fdt_path_offset(blob, "/soc/pinctrl@1c20800/pwm0-pin");
 	if (offset < 0)
 		return offset;

 	pins_phandle = fdt_create_phandle(blob, offset);
 	if (!pins_phandle)
 		return -1;

	offset = fdt_path_offset(blob, "/soc/pwm@1c20e00");
  	if (offset < 0)
  		return offset;

	ret |= fdt_set_node_status(blob, offset, FDT_STATUS_OKAY, 0);
	ret |= fdt_setprop_u32(blob, offset, "pinctrl-0", pins_phandle);
	ret |= fdt_setprop_string(blob, offset, "pinctrl-names", "default");
	if (ret < 0)
		return ret;

	pwm_phandle = fdt_create_phandle(blob, offset);
	if (!pwm_phandle)
		return -1;

	/**
	 * backlight: backlight {
	 * 	compatible = "pwm-backlight";
	 * 	power-supply = <&reg_vcc5v0>;
	 * 	pwms = <&pwm 0 50000 1>;
	 * 	brightness-levels = <0 10 20 30 40 50 60 70 80 90 100>;
	 *	default-brightness-level = <10>;
	 * };
	 */

	offset = fdt_path_offset(blob, "/");
 	if (offset < 0)
 		return offset;

	offset = fdt_add_subnode(blob, offset, "backlight");
	if (offset < 0)
		return offset;

	gpios[0] = cpu_to_fdt32(pwm_phandle);
	gpios[1] = cpu_to_fdt32(0);
	gpios[2] = cpu_to_fdt32(50000);
	gpios[3] = cpu_to_fdt32(1);
	ret = fdt_setprop(blob, offset, "pwms", gpios, sizeof(gpios));

	for (i = 0; i < 11; i++)
		levels[i] = cpu_to_fdt32(i * 10);
	ret |= fdt_setprop(blob, offset, "brightness-levels", levels, sizeof(levels));
	ret |= fdt_setprop_u32(blob, offset, "default-brightness-level", 10);
	ret |= fdt_setprop_string(blob, offset, "compatible", "pwm-backlight");
	if (ret < 0)
		return ret;

	backlight_phandle = fdt_create_phandle(blob, offset);
	if (!backlight_phandle)
		return -1;

	/**
	 * lcd0_rgb888_pins: lcd0_rgb888_pins@0 {
	 * 	pins = "PD0", "PD1", "PD2", "PD3", "PD4", "PD5",
	 * 		"PD6", "PD7", "PD8", "PD9", "PD10",
	 * 		"PD11", "PD12", "PD13", "PD14", "PD15",
	 * 		"PD16", "PD17", "PD18", "PD19", "PD20",
	 * 		"PD21", "PD22", "PD23", "PD24", "PD25",
	 * 		"PD26", "PD27";
	 * 	function = "lcd0";
	 * };
	 */


	offset = fdt_path_offset(blob, "/soc/pinctrl@1c20800");
 	if (offset < 0)
 		return offset;

	offset = fdt_add_subnode(blob, offset, "lcd0_rgb888_pins");
	if (offset < 0)
		return offset;

	pins_phandle = fdt_create_phandle(blob, offset);
	if (!pins_phandle)
		return -1;

 	ret = fdt_setprop_string(blob, offset, "function" , "lcd0");

 	ret |= fdt_setprop_string(blob, offset, "pins" , "PD0");
 	ret |= fdt_appendprop_string(blob, offset, "pins", "PD1");
 	ret |= fdt_appendprop_string(blob, offset, "pins", "PD2");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD3");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD4");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD5");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD6");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD7");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD8");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD9");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD10");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD11");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD12");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD13");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD14");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD15");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD16");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD17");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD18");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD19");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD20");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD21");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD22");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD23");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD24");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD25");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD26");
	ret |= fdt_appendprop_string(blob, offset, "pins", "PD27");
 	if (ret < 0)
 		return ret;

	/**
	 * panel@50 {
	 * 	compatible = "olimex,lcd-olinuxino";
	 * 	#address-cells = <1>;
	 * 	#size-cells = <0>;
	 * 	reg = <0x50>;
	 *
	 * 	pinctrl-names = "default";
	 * 	pinctrl-0 = <&lcd0_rgb888_pins>;
	 *
	 * 	power-supply = <&reg_vcc5v0>;
	 *
	 *	enable-gpios = <&pio 7 8 GPIO_ACTIVE_HIGH>;
	 * 	backlight = <&backlight>;
	 * 	status = "okay";
	 *
	 * 	port@0 {
	 * 		#address-cells = <1>;
	 * 		#size-cells = <0>;
	 * 		reg = <0>;
	 *
	 * 		panel_in_tcon0: endpoint@0 {
	 * 			#address-cells = <1>;
	 * 			#size-cells = <0>;
	 * 			reg = <0>;
	 * 			remote-endpoint = <&tcon0_out_panel>;
	 * 			};
	 *		};
	 *	};
	 * };
	 */

	if (!lcd) {
		offset = fdt_path_offset(blob, "/soc/i2c@1c2b400");
	  	if (offset < 0)
	  		return offset;

		offset = fdt_add_subnode(blob, offset, "panel@50");
		if (offset < 0)
			return offset;
	} else {
		offset = fdt_path_offset(blob, "/");
		if (offset < 0)
			return offset;

		offset = fdt_add_subnode(blob, offset, "panel");
		if (offset < 0)
			return offset;
	}

	ret = fdt_setprop_string(blob, offset, "compatible", lcd_olinuxino_compatible());
	ret |= fdt_setprop_u32(blob, offset, "#address-cells", 1);
	ret |= fdt_setprop_u32(blob, offset, "#size-cells", 0);
	if (!lcd)
		ret |= fdt_setprop_u32(blob, offset, "reg", 0x50);
	ret |= fdt_setprop_string(blob, offset, "pinctrl-names", "default");
	ret |= fdt_setprop_u32(blob, offset, "pinctrl-0", pins_phandle);
	ret |= fdt_setprop_u32(blob, offset, "backlight", backlight_phandle);

	gpios[0] = cpu_to_fdt32(pinctrl_phandle);
	gpio = sunxi_name_to_gpio(olimex_get_lcd_pwr_pin());
	gpios[1] = cpu_to_fdt32(gpio >> 5);
	gpios[2] = cpu_to_fdt32(gpio & 0x1F);
	gpios[3] = cpu_to_fdt32(0);
	ret |= fdt_setprop(blob, offset, "enable-gpios", gpios, sizeof(gpios));
	ret |= fdt_set_node_status(blob, offset, FDT_STATUS_OKAY, 0);
	if (ret < 0)
 		return ret;

	offset = fdt_add_subnode(blob, offset, "port@0");
	if (offset < 0)
		return offset;

	ret = fdt_setprop_u32(blob, offset, "reg", 0);
	ret |= fdt_setprop_u32(blob, offset, "#size-cells", 0);
	ret |= fdt_setprop_u32(blob, offset, "#address-cells", 1);
	if (ret < 0)
 		return ret;

	offset = fdt_add_subnode(blob, offset, "endpoint@0");
	if (offset < 0)
		return offset;

	ret = fdt_setprop_u32(blob, offset, "reg", 0);
	ret |= fdt_setprop_u32(blob, offset, "#size-cells", 0);
	ret |= fdt_setprop_u32(blob, offset, "#address-cells", 1);
	if (ret < 0)
 		return ret;

	panel_endpoint_phandle = fdt_create_phandle(blob, offset);
	if (!panel_endpoint_phandle)
		return -1;

	/**
	* &tcon0_out {
	* 	#address-cells = <1>;
	* 	#size-cells = <0>;
	*
	* 	tcon0_out_panel: endpoint@0 {
	* 		#address-cells = <1>;
	* 		#size-cells = <0>;
	* 		reg = <0>;
	* 		remote-endpoint = <&panel_in_tcon0>;
	* 	};
	* };
	*/

	offset = fdt_path_offset(blob, "/soc/lcd-controller@1c0c000/ports/port@1");
  	if (offset < 0)
  		return offset;

	offset = fdt_add_subnode(blob, offset, "endpoint@0");
	if (offset < 0)
		return offset;

	ret = fdt_setprop_u32(blob, offset, "remote-endpoint", panel_endpoint_phandle);
	ret |= fdt_setprop_u32(blob, offset, "reg", 0);
	ret |= fdt_setprop_u32(blob, offset, "#size-cells", 0);
	ret |= fdt_setprop_u32(blob, offset, "#address-cells", 1);
	if (lcd) {
		if (lcd->id == 9284 || 				/* LCD-OLinuXino-10CTS */
		    lcd->id == 9278 ||				/* LCD-OLinuXino-7CTS */
		    lcd->id == 7862)				/* LCD-OLinuXino-10 */
			ret = fdt_setprop_empty(blob, offset, "allwinner,force-dithering");
	} else {
		if (lcd_olinuxino_eeprom.id == 9284 ||		/* LCD-OLinuXino-10CTS */
		    lcd_olinuxino_eeprom.id == 9278 ||		/* LCD-OLinuXino-7CTS */
		    lcd_olinuxino_eeprom.id == 9278)		/* LCD-OLinuXino-10 */
			ret = fdt_setprop_empty(blob, offset, "allwinner,force-dithering");
	}
	if (ret < 0)
 		return ret;

	tcon0_endpoint_phandle  = fdt_create_phandle(blob, offset);
	if (!tcon0_endpoint_phandle)
		return -1;

	if (!lcd)
		offset = fdt_path_offset(blob, "/soc/i2c@1c2b400/panel@50/port@0/endpoint@0");
	else
		offset = fdt_path_offset(blob, "/panel/port@0/endpoint@0");

	if (offset < 0)
		return offset;

	ret = fdt_setprop_u32(blob, offset, "remote-endpoint", tcon0_endpoint_phandle);
	if (ret < 0)
 		return ret;

	/* Enable TS */
	if ((!lcd && (lcd_olinuxino_eeprom.id == 9278 ||	/* LCD-OLinuXino-7CTS */
	    lcd_olinuxino_eeprom.id == 9284)) ||		/* LCD-OLinuXino-10CTS */
	    (lcd && (lcd->id == 8630 || 			/* LCD-OLinuXino-5 */
	    lcd->id == 9278 ||					/* LCD-OLinuXino-7CTS */
	    lcd->id == 9284))) {				/* LCD-OLinuXino-10CTS */

		offset = fdt_path_offset(blob, "/soc/i2c@1c2b400");
		if (offset < 0)
			return offset;

		if (lcd && lcd->id == 8630) {
			offset = fdt_add_subnode(blob, offset, "ft5x@38");
			if (offset < 0)
				return offset;

			ret = fdt_setprop_string(blob, offset, "compatible", "edt,edt-ft5306");
			ret |= fdt_setprop_u32(blob, offset, "reg", 0x38);
			ret |= fdt_setprop_u32(blob, offset, "touchscreen-size-x", 800);
			ret |= fdt_setprop_u32(blob, offset, "touchscreen-size-y", 480);
		} else {
			if ((!lcd && lcd_olinuxino_eeprom.id == 9278) ||
			    (lcd && lcd->id == 9278)) {
				offset = fdt_add_subnode(blob, offset, "gt911@14");
				if (offset < 0)
					return offset;

				ret = fdt_setprop_string(blob, offset, "compatible", "goodix,gt911");
			} else {
				offset = fdt_add_subnode(blob, offset, "gt928@14");
				if (offset < 0)
					return offset;

				ret = fdt_setprop_string(blob, offset, "compatible", "goodix,gt928");
			}
			ret |= fdt_setprop_u32(blob, offset, "reg", 0x14);
		}
		ret |= fdt_setprop_u32(blob, offset, "interrupt-parent", pinctrl_phandle);

		gpio = sunxi_name_to_gpio(olimex_get_lcd_irq_pin());
		irq[0] = cpu_to_fdt32(gpio >> 5);
		irq[1] = cpu_to_fdt32(gpio & 0x1F);
		irq[2] = cpu_to_fdt32(2);
		ret |= fdt_setprop(blob, offset, "interrupts", irq, sizeof(irq));

		gpios[0] = cpu_to_fdt32(pinctrl_phandle);
		gpios[1] = cpu_to_fdt32(gpio >> 5);
		gpios[2] = cpu_to_fdt32(gpio & 0x1F);
		gpios[3] = cpu_to_fdt32(0);
		ret |= fdt_setprop(blob, offset, "irq-gpios", gpios, sizeof(gpios));

		gpio = sunxi_name_to_gpio(olimex_get_lcd_rst_pin());
		gpios[0] = cpu_to_fdt32(pinctrl_phandle);
		gpios[1] = cpu_to_fdt32(gpio >> 5);
		gpios[2] = cpu_to_fdt32(gpio & 0x1F);
		if (lcd && lcd->id == 8630)
			gpios[3] = cpu_to_fdt32(1);
		else
			gpios[3] = cpu_to_fdt32(0);
		ret |= fdt_setprop(blob, offset, "reset-gpios", gpios, sizeof(gpios));

		if (lcd_olinuxino_eeprom.id == 9278 || (lcd && lcd->id == 9278))
			ret |= fdt_setprop_empty(blob, offset, "touchscreen-swapped-x-y");

	} else {
		/* Enable SUN4I-TS */
		offset = fdt_path_offset(blob, "/soc/rtp@1c25000");
		if (offset < 0)
			return offset;

		ret = fdt_setprop_empty(blob, offset, "allwinner,ts-attached");

		/* Some board comes with inverted x axis */
		if (lcd && (
			lcd->id == 7862 ||	/*  LCD-OLinuXino-10 */
			lcd->id == 7864 ||	/*  LCD-OLinuXino-7 */
			lcd->id == 7859		/* LCD-OLinuXino-4.3TS */
		))
			ret |= fdt_setprop_empty(blob, offset, "touchscreen-inverted-x");
	}

	return ret;
}
#endif

#if defined(CONFIG_OF_BOARD_FIXUP)
int board_fix_fdt(void *blob)
{
	return olinuxino_fdt_fixup(blob);
}
#endif

#if defined(CONFIG_OF_SYSTEM_SETUP)
int ft_system_setup(void *blob, bd_t *bd)
{
	size_t blob_size = gd->fdt_size;
	void *recovery;
	int ret = 0;

#if CONFIG_FDT_FIXUP_PARTITIONS
	static struct node_info nodes[] = {
		{ "jedec,spi-nor", MTD_DEV_TYPE_NOR, },
		{ "fixed-partitions", MTD_DEV_TYPE_NAND },
	};
#endif

	/* If OLinuXino configuration is not valid exit */
	if (!olimex_eeprom_is_valid())
		return 0;

	/* First make copy of the current ftd blob */
	recovery = malloc(blob_size);
	memcpy(recovery, blob, blob_size);

	/* Execute fixups */
	ret = olinuxino_fdt_fixup(blob);
	if (ret < 0)
		goto exit_recover;

#ifdef LCD_OLINUXINO
	/* Check if lcd is the default monitor */
	if (lcd_olinuxino_is_present()) {

		/* Check RGB or LVDS mode should be enabled */
		uint32_t id = env_get_ulong("lcd_olinuxino", 10, 0);
		if (id == 7894 || id == 7891)
			ret = board_fix_lcd_olinuxino_lvds(blob);
		else
			ret = board_fix_lcd_olinuxino_rgb(blob);

		if (ret < 0)
			goto exit_recover;

	}
#endif

#if CONFIG_FDT_FIXUP_PARTITIONS
	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
#endif
	return 0;

exit_recover:
	/* Copy back revocery blob */
	printf("Recovering the FDT blob...\n");
	memcpy(blob, recovery, blob_size);

	return 0;
}
#endif
