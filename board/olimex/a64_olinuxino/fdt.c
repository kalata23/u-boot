#define DEBUG
#include <common.h>
#include <dm.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>

#include <dm/uclass-internal.h>
#include <dm/device-internal.h>

#ifdef CONFIG_FDT_FIXUP_PARTITIONS
#include <jffs2/load_kernel.h>
#include <mtd_node.h>
#endif

#include "../common/lcd_olinuxino.h"
#include "../common/board_detect.h"
#include "../common/boards.h"

DECLARE_GLOBAL_DATA_PTR;

/**
 * Check PHY_RSTn pin value
 */
static int phyrst_pin_value(void)
{
	u32 reg;

	/* Make sure PD24 is input */
	sunxi_gpio_set_cfgpin(SUNXI_GPD(24), SUNXI_GPIO_INPUT);
	reg = readl(0x01c2087c);

	return ((reg >> 24) & 0x01);
}

/**
 * Depending on PHY_RSTn pin value enable/disable
 * sun8i-emac driver.
 *
 * By default its enabled, so if PHY_RSTn pin in 1
 * do nothing and return
 */
static int board_fix_ethernet(void *blob)
{
	int offset;
	int ret;

	/* Do nothing */
	if (phyrst_pin_value() == 1)
		return 0;

	offset = fdt_path_offset(blob, "/soc/ethernet@1c30000");
	if (offset < 0) {
		debug("/soc/ethernet@1c30000: not found: %s (%d)\n",
		      fdt_strerror(offset), offset);
		return offset;
	}

	ret = fdt_set_node_status(blob, offset, FDT_STATUS_DISABLED, 0);
	if (ret)
		debug("/soc/ethernet@1c30000: failed to set status: %s (%d)",
		fdt_strerror(ret), ret);

	return ret;
}

#ifdef CONFIG_VIDEO_LCD_OLINUXINO_PANEL
static int board_fix_lcd_olinuxino_rgb(void *blob)
{
	struct lcd_olinuxino_board *lcd = lcd_olinuxino_get_data();

	uint32_t backlight_phandle;
	uint32_t lcd_pins_phandle;
	uint32_t panel_endpoint_phandle;
	uint32_t pinctrl_phandle;
	uint32_t pwm_phandle;
	uint32_t tcon_endpoint_phandle;

	fdt32_t prop[11];

	int offset;
	int ret;
	int i;

	/* Do nothing */
	if (phyrst_pin_value() == 1)
		return 0;

	/* If LCD is not present after relocation, skip FDT modifications */
	if (gd->flags & GD_FLG_RELOC)
		if (!lcd_olinuxino_is_present())
			return 0;
	/**
	 * &pwm {
	 * 	pinctrl-names = "default";
	 *	pinctrl-0 = <&pwm0_pins_a>;
	 *	status = "okay";
	 * };
	 */

	offset = fdt_path_offset(blob, "/soc/pwm@1c21400");
	if (offset < 0)
		return offset;

	ret = fdt_set_node_status(blob, offset, FDT_STATUS_OKAY, 0);
	if (ret)
		return ret;

	pwm_phandle = fdt_create_phandle(blob, offset);
	if (!pwm_phandle)
		return -1;

	/**
	 * backlight: backlight {
	 * 	compatible = "pwm-backlight";
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

	prop[0] = cpu_to_fdt32(pwm_phandle);
	prop[1] = cpu_to_fdt32(0);
	prop[2] = cpu_to_fdt32(50000);
	prop[3] = cpu_to_fdt32(1);
	ret = fdt_setprop(blob, offset, "pwms", prop, sizeof(*prop) * 4);
	if (ret)
		return ret;

	for (i = 0; i < 11; i++)
		prop[i] = cpu_to_fdt32(i * 10);
	ret = fdt_setprop(blob, offset, "brightness-levels", prop, sizeof(*prop) * 11);
	ret |= fdt_setprop_u32(blob, offset, "default-brightness-level", 10);
	ret |= fdt_setprop_string(blob, offset, "compatible", "pwm-backlight");
	if (ret)
		return ret;

	backlight_phandle = fdt_create_phandle(blob, offset);
	if (!backlight_phandle)
		return -1;

	offset = fdt_path_offset(blob, "/soc/pinctrl@1c20800");
	if (offset < 0)
		return offset;

	pinctrl_phandle = fdt_create_phandle(blob, offset);
	if (!pinctrl_phandle)
		return -1;


	/**
	 * The following code should be executed after relocation e.g. in
	 * ft_system_setup().
	 */
	if (lcd == NULL || gd->flags & GD_FLG_RELOC) {

		/**
		 * lcd_rgb666_pins: lcd_rgb666_pins {
		 * 	pins = "PD0", "PD1", "PD2", "PD3", "PD4", "PD5",
		 * 		"PD6", "PD7", "PD8", "PD9", "PD10",
		 * 		"PD11", "PD12", "PD13", "PD14", "PD15",
		 * 		"PD16", "PD17", "PD18", "PD19", "PD20",
		 * 		"PD21";
		 * 	function = "lcd0";
		 * };
		 */

		offset = fdt_add_subnode(blob, offset, "lcd-rgb666-pins");
		if (offset < 0)
			return offset;

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
		if (ret)
			return ret;

		lcd_pins_phandle = fdt_create_phandle(blob, offset);
		if (!lcd_pins_phandle)
			return -1;

		/**
		 * &i2c0 {
		 * 	pinctrl-names = "default";
		 * 	pinctrl-0 = <&i2c0_pins>;
		 * 	status = "okay";
		 * };
		 *
		 */

		if (!lcd) {
			offset = fdt_path_offset(blob, "/soc/i2c@1c2ac00");
			if (offset < 0)
				return offset;

			ret = fdt_set_node_status(blob, offset, FDT_STATUS_OKAY, 0);
			if (ret)
				return ret;
		} else {
			offset = fdt_path_offset(blob, "/");
			if (offset < 0)
				return offset;
		}

		/**
		 *
		 * 	panel@50 {
		 * 		pinctrl-names = "default";
		 * 		pinctrl-0 = <&lcd_rgb666_pins>;
		 *
		 * 		compatible = "olimex,lcd-olinuxino";
		 * 		reg = <0x50>;
		 * 		status = "okay";
		 *
		 * 		enable-gpios = <&pio 3 23 GPIO_ACTIVE_HIGH>;
		 * 		backlight = <&backlight>;
		 *
		 *		port {
		 *			panel_in_tcon0: endpoint {
		 *				remote-endpoint = <&tcon0_out_panel>;
		 *			};
		 *		};
		 *	};
		 *
		 */

		if (!lcd)
			offset = fdt_add_subnode(blob, offset, "panel@50");
		else
			offset = fdt_add_subnode(blob, offset, "panel");
		if (offset < 0)
			return offset;

		ret = fdt_setprop_string(blob, offset, "compatible", lcd_olinuxino_compatible());
		ret |= fdt_setprop_string(blob, offset, "pinctrl-names", "default");
		ret |= fdt_setprop_u32(blob, offset, "pinctrl-0", lcd_pins_phandle);
		ret |= fdt_setprop_u32(blob, offset, "#address-cells", 1);
		ret |= fdt_setprop_u32(blob, offset, "#size-cells", 0);
		if (!lcd)
			ret |= fdt_setprop_u32(blob, offset, "reg", 0x50);
		ret |= fdt_setprop_u32(blob, offset, "backlight", backlight_phandle);

		prop[0] = cpu_to_fdt32(pinctrl_phandle);
		prop[1] = cpu_to_fdt32(3);
		prop[2] = cpu_to_fdt32(23);
		prop[3] = cpu_to_fdt32(0);
		ret |= fdt_setprop(blob, offset, "enable-gpios", prop, sizeof(*prop) * 4);
		ret |= fdt_set_node_status(blob, offset, FDT_STATUS_OKAY, 0);
		if (ret)
			return ret;

		offset = fdt_add_subnode(blob, offset, "port");
		if (offset < 0)
			return offset;

		offset = fdt_add_subnode(blob, offset, "endpoint");
		if (offset < 0)
			return offset;

		panel_endpoint_phandle = fdt_create_phandle(blob, offset);
		if (!panel_endpoint_phandle)
			return -1;

		/**
		 *	&tcon0_out {
		 *		tcon0_out_panel: endpoint@0 {
		 *			reg = <0>;
		 *			remote-endpoint = <&panel_in_tcon0>;
		 *		};
		 * 	};
		 */

		offset = fdt_path_offset(blob, "/soc/lcd-controller@1c0c000/ports/port@1");
		if (offset < 0)
			return offset;

		offset = fdt_add_subnode(blob, offset, "endpoint@0");
		if (offset < 0)
			return offset;

		ret = fdt_setprop_u32(blob, offset, "remote-endpoint", panel_endpoint_phandle);
		ret |= fdt_setprop_u32(blob, offset, "reg", 0);
		if (ret)
			return ret;

		tcon_endpoint_phandle  = fdt_create_phandle(blob, offset);
		if (!tcon_endpoint_phandle)
			return -1;

		if (lcd == NULL)
			offset = fdt_path_offset(blob, "/soc/i2c@1c2ac00/panel@50/port/endpoint");
		else
			offset = fdt_path_offset(blob, "/panel/port/endpoint");
		if (offset < 0)
			return offset;

		return fdt_setprop_u32(blob, offset, "remote-endpoint", tcon_endpoint_phandle);
	} else {
		/**
		 *
		 * 	panel {
		 *
		 * 		compatible = "olimex,lcd-olinuxino";
		 * 		status = "okay";
		 *
		 * 		enable-gpios = <&pio 3 23 GPIO_ACTIVE_HIGH>;
		 * 		backlight = <&backlight>;
		 *	};
		 *
		 */

		offset = fdt_path_offset(blob, "/");
		if (offset < 0)
			return offset;

		offset = fdt_add_subnode(blob, offset, "panel");
		if (offset < 0)
			return offset;

		ret = fdt_setprop_string(blob, offset, "compatible", "olimex,lcd-olinuxino");
		ret |= fdt_setprop_u32(blob, offset, "backlight", backlight_phandle);
		prop[0] = cpu_to_fdt32(pinctrl_phandle);
		prop[1] = cpu_to_fdt32(3);
		prop[2] = cpu_to_fdt32(23);
		prop[3] = cpu_to_fdt32(0);
		ret |= fdt_setprop(blob, offset, "enable-gpios", prop, sizeof(*prop) * 4);
		ret |= fdt_set_node_status(blob, offset, FDT_STATUS_OKAY, 0);
		return ret;
	}

	return 0;
}


static int board_fix_lcd_olinuxino_ts(void *blob)
{
	u32 id;
	int ret;

	/* Do nothing */
	if (phyrst_pin_value() == 1)
		return 0;

	if (!lcd_olinuxino_is_present())
		return 0;

	id = lcd_olinuxino_id();

	/**
	 * If ID is not passed by lcd_olinuxino, try to get eeprom id.
	 * If LCD is not pressent, do nothing.
	 */
	// if (!id) {
	//
	// }
	printf("ID: %d\n", id);
	// printf("%s\n", lcd_olinuxino_eeprom.info.name);


	return 0;

#if 0


		if ((!lcd && (lcd_olinuxino_eeprom.id == 9278 ||	/* LCD-OLinuXino-7CTS */
		    lcd_olinuxino_eeprom.id == 9284)) ||		/* LCD-OLinuXino-10CTS */
		    (lcd && (lcd->id == 8630 || 			/* LCD-OLinuXino-5 */
		    lcd->id == 9278 ||					/* LCD-OLinuXino-7CTS */
		    lcd->id == 9284))) {				/* LCD-OLinuXino-10CTS */

			offset = get_path_offset(blob, PATH_I2C2, path);
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
			offset = get_path_offset(blob, PATH_RTP, NULL);
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

	return 0;
#endif
}
#endif




#if defined(CONFIG_OF_SYSTEM_SETUP)
/**
 * Make FDT modifications before passing
 * the blob to the OS.
 *
 * If there is an error, recover the original
 * blob.
 */
int ft_system_setup(void *blob, bd_t *bd)
{
#ifdef CONFIG_FDT_FIXUP_PARTITIONS
	struct node_info nodes[] = {
		{ "jedec,spi-nor", MTD_DEV_TYPE_NOR, },
	};
#endif
	void *recovery;
	int ret;
	working_fdt = blob;

#ifdef CONFIG_VIDEO_LCD_OLINUXINO_PANEL
	lcd_olinuxino_init();
#endif

	/* First make copy of the current ftd blob */
	recovery = malloc(gd->fdt_size);
	memcpy(recovery, blob, gd->fdt_size);

	/* Increase FDT blob size by 4KiB */
	ret = fdt_increase_size(blob, 4096);
	if (ret)
		return ret;

	/* Execute fixups */
	ret = board_fix_ethernet(blob);
#ifdef CONFIG_VIDEO_LCD_OLINUXINO_PANEL
	ret |= board_fix_lcd_olinuxino_rgb(blob);
	ret |= board_fix_lcd_olinuxino_ts(blob);
#endif
	if (ret)
		goto recover;

/**
 * If there is mounted SPI flash, modify partitions
 * by reading mtdpart environment variable.
 */
#ifdef CONFIG_FDT_FIXUP_PARTITIONS
	if (eeprom->config.storage == 's')
		fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
#endif
	return 0;

recover:
	/* Copy back revocery blob */
	printf("Recovering the FDT blob...\n");
	memcpy(blob, recovery, gd->fdt_size);

	return 0;
}
#endif

#if defined(CONFIG_OF_BOARD_FIXUP)
/**
 * get_timings() - Get display timings from panel.
 *
 * @dev:	Panel device containing the display timings
 * @tim:	Place to put timings
 * @return 0 if OK, -ve on error
 */
int board_fix_fdt(void *blob)
{
	int offset;
	int ret;

	/* Increase FDT blob size by 4KiB */
	ret = fdt_increase_size(blob, 4096);
	if (ret)
		return ret;

	/* Make sure mmc1 is disabled */
	offset = fdt_path_offset(blob, "/soc/mmc@1c10000");
	if (offset >= 0) {
		ret = fdt_setprop_string(blob, offset, "status", "disabled");
		if (ret)
			return ret;
	}

	/* Execute fixups */
	ret = board_fix_ethernet(blob);
#ifdef CONFIG_VIDEO_LCD_OLINUXINO_PANEL
	ret |= board_fix_lcd_olinuxino_rgb(blob);
#endif
	return ret;
}
#endif
