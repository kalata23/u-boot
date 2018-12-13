/*
 * Device Tree fixup for A33-OLinuXino
 *
 * Copyright (C) 2018 Olimex Ltd.
 *   Author: Stefan Mavrodiev <stefan@olimex.com>
 *
 * SPDX-License-Identifier: (GPL-2.0+ OR MIT)
 */

#include <common.h>
#include <linux/libfdt.h>
#include <linux/sizes.h>
#include <fdt_support.h>
#include <asm/arch/gpio.h>

#include "../common/boards.h"
#include "../common/lcd_olinuxino.h"

#define FDT_PATH_ROOT		"/"

#define FDT_COMP_AXP223		"x-powers,axp223"
#define FDT_COMP_DE		"allwinner,sun8i-a33-display-engine"
#define FDT_COMP_PWM0		"allwinner,sun7i-a20-pwm"
#define FDT_COMP_PINCTRL	"allwinner,sun8i-a33-pinctrl"
#define FDT_COMP_TCON		"allwinner,sun8i-a33-tcon"

enum devices {
	PATH_I2C0 = 0,
	PATH_TCON0,
};

#define FDT_PATH(__name, __addr) \
{ \
	.name = __name, \
	.addr = __addr, \
}

struct __path {
	char name[16];
	uint32_t addr;
} paths[] = {
	FDT_PATH("i2c",			0x01c2ac00),
	FDT_PATH("lcd-controller",	0x01c0c000),
};

static int get_path_offset(void *blob, enum devices dev, char *dpath)
{
	char path[64];
	int offset;

	sprintf(path, "/soc@1c00000/%s@%x", paths[dev].name, paths[dev].addr);
	offset = fdt_path_offset(blob, path);
	if (offset >= 0)
		goto success;

	sprintf(path, "/soc@01c00000/%s@%08x", paths[dev].name, paths[dev].addr);
	offset = fdt_path_offset(blob, path);
	if (offset >= 0)
		goto success;

	printf("Path \"%s\" not found: %s (%d)\n", path, fdt_strerror(offset), offset);
	return offset;

success:
	if (path != NULL)
		strcpy(dpath, path);
	return offset;
}

#ifdef CONFIG_VIDEO_LCD_PANEL_OLINUXINO
static int board_fix_lcd_olinuxino(void *blob)
{
	struct lcd_olinuxino_board *lcd = lcd_olinuxino_get_data();

	uint32_t power_supply_phandle;
	uint32_t backlight_phandle;
	uint32_t pinctrl_phandle;
	uint32_t pins_phandle;
	uint32_t pwm_phandle;
	uint32_t panel_endpoint_phandle;
	uint32_t tcon0_endpoint_phandle;

	fdt32_t gpios[4];
	fdt32_t irq[3];
	fdt32_t levels[11];
	char path[64];
	int offset;
	int ret = 0;
	int gpio;
	int i;


	/**
	 * &pwm {
	 * 	pinctrl-names = "default";
	 * 	pimctrl-0 = <&pwm0_pins>;
	 * 	status = "okay";
	 * };
	 */

	offset = fdt_node_offset_by_compatible(blob, -1, FDT_COMP_PINCTRL);
 	if (offset < 0)
 		return offset;

	pinctrl_phandle = fdt_get_phandle(blob, offset);
	if (pinctrl_phandle < 0)
 		return offset;

	offset = fdt_subnode_offset(blob, offset, "pwm0");
	if (offset < 0)
		return offset;

	pins_phandle = fdt_create_phandle(blob, offset);
	if (!pins_phandle)
		return -1;

	offset = fdt_node_offset_by_compatible(blob, -1, FDT_COMP_PWM0);
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
	 * reg_vcc5v0: vcc5v0 {
	 *	compatible = "regulator-fixed";
	 *	regulator-name = "vcc5v0";
	 *	regulator-min-microvolt = <5000000>;
	 *	regulator-max-microvolt = <5000000>;
	 * };
	 */
	offset = fdt_path_offset(blob, FDT_PATH_ROOT);
  	if (offset < 0)
  		return offset;

 	offset = fdt_add_subnode(blob, offset, "vcc5v0");
 	if (offset < 0)
 		return offset;

	ret |= fdt_setprop_u32(blob, offset, "regulator-max-microvolt", 5000000);
	ret |= fdt_setprop_u32(blob, offset, "regulator-min-microvolt", 5000000);
	ret |= fdt_setprop_string(blob, offset, "regulator-name", "vcc5v0");
	ret |= fdt_setprop_string(blob, offset, "compatible", "regulator-fixed");
	if (ret < 0)
		return ret;

	power_supply_phandle = fdt_create_phandle(blob, offset);
	if (!power_supply_phandle)
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

	offset = fdt_path_offset(blob, FDT_PATH_ROOT);
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
	ret |= fdt_setprop_u32(blob, offset, "power-supply", power_supply_phandle);
	ret |= fdt_setprop_string(blob, offset, "compatible", "pwm-backlight");
	if (ret < 0)
		return ret;

	backlight_phandle = fdt_create_phandle(blob, offset);
	if (!backlight_phandle)
		return -1;

	/**
	 * panel@50 {
	 * 	compatible = "olimex,lcd-olinuxino";
	 * 	#address-cells = <1>;
	 * 	#size-cells = <0>;
	 * 	reg = <0x50>;
	 *
	 * 	pinctrl-names = "default";
	 * 	pinctrl-0 = <&lcd-rgb666_pins>;
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
	 offset = fdt_node_offset_by_compatible(blob, -1, FDT_COMP_AXP223);
   	if (offset < 0)
   		return offset;

 	offset = fdt_subnode_offset(blob, offset, "regulators");
 	if (offset < 0)
 		return offset;

 	offset = fdt_subnode_offset(blob, offset, "dc1sw");
 	if (offset < 0)
 		return offset;

	ret = fdt_setprop_empty(blob, offset, "regulator-always-on");
	if (ret)
		return ret;

 	power_supply_phandle = fdt_get_phandle(blob, offset);
 	if (power_supply_phandle < 0)
  		return offset;


	offset = fdt_node_offset_by_compatible(blob, -1, FDT_COMP_PINCTRL);
	if (offset < 0)
		return offset;

	offset = fdt_subnode_offset(blob, offset, "lcd-rgb666@0");
	if (offset < 0)
		return offset;

	pins_phandle = fdt_create_phandle(blob, offset);
	if (!pins_phandle)
		return -1;

	if (!lcd) {
		offset = get_path_offset(blob, PATH_I2C0, path);
	  	if (offset < 0)
	  		return offset;

		ret = fdt_set_node_status(blob, offset, FDT_STATUS_OKAY, 0);
		if (ret < 0)
			return ret;

		offset = fdt_add_subnode(blob, offset, "panel@50");
		if (offset < 0)
			return offset;
	} else {
		path[0] = 0;
		offset = fdt_path_offset(blob, FDT_PATH_ROOT);
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
	 * &tcon0 {
	 * 	status = "okay";
	 * };
	 */
	offset = fdt_node_offset_by_compatible(blob, -1, FDT_COMP_TCON);
   	if (offset < 0)
   		return offset;

	ret = fdt_set_node_status(blob, offset, FDT_STATUS_OKAY, 0);
	if (ret < 0)
 		return ret;

	/**
	 * &de {
	 * 	status = "okay";
	 * };
	 */
	offset = fdt_node_offset_by_compatible(blob, -1, FDT_COMP_DE);
	if (offset < 0)
		return offset;

	ret = fdt_set_node_status(blob, offset, FDT_STATUS_OKAY, 0);
	if (ret < 0)
		return ret;


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

	offset = get_path_offset(blob, PATH_TCON0, NULL);
  	if (offset < 0)
  		return offset;

	offset = fdt_subnode_offset(blob, offset, "ports");
	if (offset < 0)
		return offset;

	offset = fdt_subnode_offset(blob, offset, "port@1");
	if (offset < 0)
		return offset;

	offset = fdt_add_subnode(blob, offset, "endpoint@0");
	if (offset < 0)
		return offset;

	ret |= fdt_setprop_u32(blob, offset, "remote-endpoint", panel_endpoint_phandle);
	ret |= fdt_setprop_u32(blob, offset, "reg", 0);
	if (ret < 0)
 		return ret;

	tcon0_endpoint_phandle  = fdt_create_phandle(blob, offset);
	if (!tcon0_endpoint_phandle)
		return -1;

	if (!lcd)
		strcat(path, "/panel@50/port@0/endpoint@0");
	else
		strcat(path, "/panel/port@0/endpoint@0");

	offset = fdt_path_offset(blob, path);
	if (offset < 0)
		return offset;

	ret = fdt_setprop_u32(blob, offset, "remote-endpoint", tcon0_endpoint_phandle);
	if (ret < 0)
 		return ret;


	/**
	 * Enable TS for the following boards:
	 * LCD-OLinuXino-5
	 * LCD-OLinuXino-7CTS
	 * LCD-OLinuXino-10CTS
	 */
	if (lcd && lcd->id != 8630 && lcd->id != 9278 && lcd->id != 9284)
		return 0;

	/* Enable TS */
	offset = get_path_offset(blob, PATH_I2C0, path);
	if (offset < 0)
		return offset;

	/* Make sure I2C0 is enabled */
	ret = fdt_set_node_status(blob, offset, FDT_STATUS_OKAY, 0);
	if (ret < 0)
		return ret;

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

	if ((lcd && lcd->id == 9278) || (!lcd && lcd_olinuxino_eeprom.id == 9278))
		ret |= fdt_setprop_empty(blob, offset, "touchscreen-swapped-x-y");

	return ret;
}
#endif

int ft_system_setup(void *blob, bd_t *bd)
{
	int ret = 0;

#ifdef CONFIG_VIDEO_LCD_PANEL_OLINUXINO
	/* Check if lcd is the default monitor */
	char *s = env_get("monitor");
	if (s != NULL && !strncmp(s, "lcd", 3)) {
		ret = board_fix_lcd_olinuxino(blob);
		if (ret < 0)
			return ret;
		}
#endif
	return ret;
}
