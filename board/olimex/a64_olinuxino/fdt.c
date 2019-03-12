#define DEBUG
#include <common.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include <malloc.h>

#include <asm/io.h>
#include <asm/arch/gpio.h>

#ifdef CONFIG_FDT_FIXUP_PARTITIONS
#include <jffs2/load_kernel.h>
#include <mtd_node.h>
#endif

#include "../common/board_detect.h"
#include "../common/boards.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef DEBUG
#define MAX_LEVEL	32		/* how deeply nested we will go */
#define CMD_FDT_MAX_DUMP 64

#include <linux/ctype.h>
#include <linux/types.h>


static int is_printable_string(const void *data, int len)
{
	const char *s = data;

	/* zero length is not */
	if (len == 0)
		return 0;

	/* must terminate with zero or '\n' */
	if (s[len - 1] != '\0' && s[len - 1] != '\n')
		return 0;

	/* printable or a null byte (concatenated strings) */
	while (((*s == '\0') || isprint(*s) || isspace(*s)) && (len > 0)) {
		/*
		 * If we see a null, there are three possibilities:
		 * 1) If len == 1, it is the end of the string, printable
		 * 2) Next character also a null, not printable.
		 * 3) Next character not a null, continue to check.
		 */
		if (s[0] == '\0') {
			if (len == 1)
				return 1;
			if (s[1] == '\0')
				return 0;
		}
		s++;
		len--;
	}

	/* Not the null termination, or not done yet: not printable */
	if (*s != '\0' || (len != 0))
		return 0;

	return 1;
}

/*
 * Print the property in the best format, a heuristic guess.  Print as
 * a string, concatenated strings, a byte, word, double word, or (if all
 * else fails) it is printed as a stream of bytes.
 */
static void print_data(const void *data, int len)
{
	int j;

	/* no data, don't print */
	if (len == 0)
		return;

	/*
	 * It is a string, but it may have multiple strings (embedded '\0's).
	 */
	if (is_printable_string(data, len)) {
		puts("\"");
		j = 0;
		while (j < len) {
			if (j > 0)
				puts("\", \"");
			puts(data);
			j    += strlen(data) + 1;
			data += strlen(data) + 1;
		}
		puts("\"");
		return;
	}

	if ((len %4) == 0) {
		if (len > CMD_FDT_MAX_DUMP)
			printf("* 0x%p [0x%08x]", data, len);
		else {
			const __be32 *p;

			printf("<");
			for (j = 0, p = data; j < len/4; j++)
				printf("0x%08x%s", fdt32_to_cpu(p[j]),
					j < (len/4 - 1) ? " " : "");
			printf(">");
		}
	} else { /* anything else... hexdump */
		if (len > CMD_FDT_MAX_DUMP)
			printf("* 0x%p [0x%08x]", data, len);
		else {
			const u8 *s;

			printf("[");
			for (j = 0, s = data; j < len; j++)
				printf("%02x%s", s[j], j < len - 1 ? " " : "");
			printf("]");
		}
	}
}


/*
 * Recursively print (a portion of) the working_fdt.  The depth parameter
 * determines how deeply nested the fdt is printed.
 */
static int fdt_print(const char *pathp, char *prop, int depth)
{
	static char tabs[MAX_LEVEL+1] =
		"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
		"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	const void *nodep;	/* property node pointer */
	int  nodeoffset;	/* node offset from libfdt */
	int  nextoffset;	/* next node offset from libfdt */
	uint32_t tag;		/* tag */
	int  len;		/* length of the property */
	int  level = 0;		/* keep track of nesting level */
	const struct fdt_property *fdt_prop;

	nodeoffset = fdt_path_offset (working_fdt, pathp);
	if (nodeoffset < 0) {
		/*
		 * Not found or something else bad happened.
		 */
		printf ("libfdt fdt_path_offset() returned %s\n",
			fdt_strerror(nodeoffset));
		return 1;
	}
	/*
	 * The user passed in a property as well as node path.
	 * Print only the given property and then return.
	 */
	if (prop) {
		nodep = fdt_getprop (working_fdt, nodeoffset, prop, &len);
		if (len == 0) {
			/* no property value */
			printf("%s %s\n", pathp, prop);
			return 0;
		} else if (nodep && len > 0) {
			printf("%s = ", prop);
			print_data (nodep, len);
			printf("\n");
			return 0;
		} else {
			printf ("libfdt fdt_getprop(): %s\n",
				fdt_strerror(len));
			return 1;
		}
	}

	/*
	 * The user passed in a node path and no property,
	 * print the node and all subnodes.
	 */
	while(level >= 0) {
		tag = fdt_next_tag(working_fdt, nodeoffset, &nextoffset);
		switch(tag) {
		case FDT_BEGIN_NODE:
			pathp = fdt_get_name(working_fdt, nodeoffset, NULL);
			if (level <= depth) {
				if (pathp == NULL)
					pathp = "/* NULL pointer error */";
				if (*pathp == '\0')
					pathp = "/";	/* root is nameless */
				printf("%s%s {\n",
					&tabs[MAX_LEVEL - level], pathp);
			}
			level++;
			if (level >= MAX_LEVEL) {
				printf("Nested too deep, aborting.\n");
				return 1;
			}
			break;
		case FDT_END_NODE:
			level--;
			if (level <= depth)
				printf("%s};\n", &tabs[MAX_LEVEL - level]);
			if (level == 0) {
				level = -1;		/* exit the loop */
			}
			break;
		case FDT_PROP:
			fdt_prop = fdt_offset_ptr(working_fdt, nodeoffset,
					sizeof(*fdt_prop));
			pathp    = fdt_string(working_fdt,
					fdt32_to_cpu(fdt_prop->nameoff));
			len      = fdt32_to_cpu(fdt_prop->len);
			nodep    = fdt_prop->data;
			if (len < 0) {
				printf ("libfdt fdt_getprop(): %s\n",
					fdt_strerror(len));
				return 1;
			} else if (len == 0) {
				/* the property has no value */
				if (level <= depth)
					printf("%s%s;\n",
						&tabs[MAX_LEVEL - level],
						pathp);
			} else {
				if (level <= depth) {
					printf("%s%s = ",
						&tabs[MAX_LEVEL - level],
						pathp);
					print_data (nodep, len);
					printf(";\n");
				}
			}
			break;
		case FDT_NOP:
			printf("%s/* NOP */\n", &tabs[MAX_LEVEL - level]);
			break;
		case FDT_END:
			return 1;
		default:
			if (level <= depth)
				printf("Unknown tag 0x%08X\n", tag);
			return 1;
		}
		nodeoffset = nextoffset;
	}
	return 0;
}
#endif


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

#ifdef CONFIG_VIDEO_LCD_PANEL_OLINUXINO
static int board_fix_lcd_olinuxino_rgb(void *blob)
{
	uint32_t backlight_phandle;
	uint32_t i2c_pins_phandle;
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

	/**
	 * &pwm {
	 * 	pinctrl-names = "default";
	 *	pinctrl-0 = <&pwm0_pins_a>;
	 *	status = "okay";
	 * };
	 */

	offset = fdt_path_offset(blob, "/soc/pwm@1c21400");
	if (offset < 0) {
		debug("/soc/pwm@1c21400: find path: %s (%d)\n",
		      fdt_strerror(offset), offset);
		return offset;
	}
	ret = fdt_set_node_status(blob, offset, FDT_STATUS_OKAY, 0);
	if (ret < 0) {
		debug("/soc/pwm@1c21400: set status: %s (%d)\n",
		      fdt_strerror(ret), ret);
		return ret;
	}
	pwm_phandle = fdt_create_phandle(blob, offset);
	if (!pwm_phandle) {
		debug("/soc/pwm@1c21400: create phandle error\n");
		return -1;
	}

	/**
	 * backlight: backlight {
	 * 	compatible = "pwm-backlight";
	 * 	pwms = <&pwm 0 50000 1>;
	 * 	brightness-levels = <0 10 20 30 40 50 60 70 80 90 100>;
	 *	default-brightness-level = <10>;
	 * };
	 */

	offset = fdt_path_offset(blob, "/");
	if (offset < 0) {
		debug("/: find path: %s (%d)\n",
		      fdt_strerror(offset), offset);
		return offset;
	}

	offset = fdt_add_subnode(blob, offset, "backlight");
	if (offset < 0) {
		debug("/: add subnode: %s (%d)\n",
		      fdt_strerror(offset), offset);
		return offset;
	}

	prop[0] = cpu_to_fdt32(pwm_phandle);
	prop[1] = cpu_to_fdt32(0);
	prop[2] = cpu_to_fdt32(50000);
	prop[3] = cpu_to_fdt32(1);
	ret = fdt_setprop(blob, offset, "pwms", prop, sizeof(*prop) * 4);
	if (ret < 0) {
		debug("/backlight: failed to set pwm: %s (%d)\n",
		      fdt_strerror(offset), offset);
		return ret;
	}

	for (i = 0; i < 11; i++)
		prop[i] = cpu_to_fdt32(i * 10);
	ret = fdt_setprop(blob, offset, "brightness-levels", prop, sizeof(*prop) * 11);
	ret |= fdt_setprop_u32(blob, offset, "default-brightness-level", 10);
	ret |= fdt_setprop_string(blob, offset, "compatible", "pwm-backlight");
	if (ret < 0) {
		debug("/backlight: failed to set property: %s (%d)\n",
		      fdt_strerror(ret), ret);
		return ret;
	}

	backlight_phandle = fdt_create_phandle(blob, offset);
	if (!backlight_phandle) {
		debug("/backlight: create phandle error\n");
		return -1;
	}

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

	offset = fdt_path_offset(blob, "/soc/pinctrl@1c20800");
 	if (offset < 0) {
 		debug("/soc/pinctrl@1c20800: find path: %s (%d)\n",
 		      fdt_strerror(offset), offset);
 		return offset;
 	}

	pinctrl_phandle = fdt_create_phandle(blob, offset);
	if (!pinctrl_phandle) {
		debug("/soc/pinctrl@1c20800: create phandle error\n");
		return -1;
	}

	offset = fdt_add_subnode(blob, offset, "lcd-rgb666-pins");
	if (offset < 0) {
		debug("/soc/pinctrl@1c20800: add subnode: %s (%d)\n",
		      fdt_strerror(offset), offset);
		return offset;
	}
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
	if (ret < 0) {
		debug("/soc/pinctrl@1c20800/lcd-rgb666-pins: failed to set property: %s (%d)\n",
		      fdt_strerror(ret), ret);
		return ret;
	}

	lcd_pins_phandle = fdt_create_phandle(blob, offset);
	if (!lcd_pins_phandle) {
		debug("/soc/pinctrl@1c20800/lcd-rgb666-pins: create phandle error\n");
		return -1;
	}

	/**
	 * &i2c0 {
	 * 	pinctrl-names = "default";
	 * 	pinctrl-0 = <&i2c0_pins>;
	 * 	status = "okay";
	 * };
	 *
	 */

	offset = fdt_path_offset(blob, "/soc/pinctrl@1c20800/i2c0_pins");
	if (offset < 0) {
		debug("/soc/pinctrl@1c20800/i2c0_pins: find path: %s (%d)\n",
		       fdt_strerror(offset), offset);
		return offset;
	}

	i2c_pins_phandle = fdt_create_phandle(blob, offset);
	if (!i2c_pins_phandle) {
		debug("/soc/pinctrl@1c20800/i2c0_pins: create phandle error\n");
		return -1;
	}

	offset = fdt_path_offset(blob, "/soc/i2c@1c2ac00");
	if (offset < 0) {
		debug("/soc/i2c@1c2ac00: find path: %s (%d)\n",
 		      fdt_strerror(offset), offset);
		return offset;
	}

	ret = fdt_setprop_string(blob, offset, "pinctrl-names", "default");
	ret |= fdt_setprop_u32(blob, offset, "pinctrl-0", i2c_pins_phandle);
	ret |= fdt_set_node_status(blob, offset, FDT_STATUS_OKAY, 0);
	if (ret < 0) {
		debug("/soc/i2c@1c2ac00: failed to set status: %s (%d)\n",
		      fdt_strerror(offset), offset);
		return ret;
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

	offset = fdt_add_subnode(blob, offset, "panel@50");
	if (offset < 0) {
		debug("/soc/i2c@1c2ac00: add subnode: %s (%d)",
		      fdt_strerror(offset), offset);
		return offset;
	}

	ret = fdt_setprop_string(blob, offset, "compatible", "olimex,lcd-olinuxino");
	ret |= fdt_setprop_string(blob, offset, "pinctrl-names", "default");
	ret |= fdt_setprop_u32(blob, offset, "pinctrl-0", lcd_pins_phandle);
	ret |= fdt_setprop_u32(blob, offset, "#address-cells", 1);
	ret |= fdt_setprop_u32(blob, offset, "#size-cells", 0);
	ret |= fdt_setprop_u32(blob, offset, "reg", 0x50);
	ret |= fdt_setprop_u32(blob, offset, "backlight", backlight_phandle);

	prop[0] = cpu_to_fdt32(pinctrl_phandle);
	prop[1] = cpu_to_fdt32(3);
	prop[2] = cpu_to_fdt32(23);
	prop[3] = cpu_to_fdt32(0);
	ret |= fdt_setprop(blob, offset, "enable-gpios", prop, sizeof(*prop) * 4);
	ret |= fdt_set_node_status(blob, offset, FDT_STATUS_OKAY, 0);
	if (ret < 0) {
		debug("/soc/i2c@1c2ac00/panel@50: failed to set property: %s (%d)\n",
		      fdt_strerror(ret), ret);
		return ret;
	}

	offset = fdt_add_subnode(blob, offset, "port");
	if (offset < 0) {
		debug("/soc/i2c@1c2ac00/panel@50: add subnode: %s (%d)",
		      fdt_strerror(offset), offset);
		return offset;
	}

	offset = fdt_add_subnode(blob, offset, "endpoint");
	if (offset < 0) {
		debug("/soc/i2c@1c2ac00/panel@50/port: add subnode: %s (%d)",
		      fdt_strerror(offset), offset);
		return offset;
	}

	panel_endpoint_phandle = fdt_create_phandle(blob, offset);
	if (!panel_endpoint_phandle) {
		debug("/soc/i2c@1c2ac00/panel@50/port/endpoint: create phandle error\n");
		return -1;
	}

	/**
	 *	&tcon0_out {
	 *		tcon0_out_panel: endpoint@0 {
	 *			reg = <0>;
	 *			remote-endpoint = <&panel_in_tcon0>;
	 *		};
	 * 	};
	 */

	offset = fdt_path_offset(blob, "/soc/lcd-controller@1c0c000/ports/port@1");
	if (offset < 0) {
		debug("/soc/lcd-controller@1c0c000/ports/port@1: find path: %s (%d)\n",
		       fdt_strerror(offset), offset);
		return offset;
	}

	offset = fdt_add_subnode(blob, offset, "endpoint@0");
	if (offset < 0) {
		debug("/soc/lcd-controller@1c0c000/ports/port@1: add subnode: %s (%d)",
		      fdt_strerror(offset), offset);
		return offset;
	}

	ret = fdt_setprop_u32(blob, offset, "remote-endpoint", panel_endpoint_phandle);
	ret |= fdt_setprop_u32(blob, offset, "reg", 0);
	if (ret < 0) {
		debug("/soc/lcd-controller@1c0c000/ports/port@1: failed to set property: %s (%d)\n",
		      fdt_strerror(ret), ret);
		return ret;
	}

	tcon_endpoint_phandle  = fdt_create_phandle(blob, offset);
	if (!tcon_endpoint_phandle) {
		debug("/soc/lcd-controller@1c0c000/ports/port@1: create phandle error\n");
		return -1;
	}



	offset = fdt_path_offset(blob, "/soc/i2c@1c2ac00/panel@50/port/endpoint");
	if (offset < 0) {
		debug("/soc/i2c@1c2ac00/panel@50/port/endpoint: find path: %s (%d)\n",
		       fdt_strerror(offset), offset);
		fdt_print("/soc/i2c@1c2ac00", NULL, MAX_LEVEL);
		return offset;
	}

	ret = fdt_setprop_u32(blob, offset, "remote-endpoint", tcon_endpoint_phandle);
	if (ret < 0) {
		debug("/soc/lcd-controller@1c0c000/ports/port@1: failed to set property: %s (%d)\n",
		      fdt_strerror(ret), ret);
		return ret;
	}

	return 0;
}
#endif

/**
 * List all fixups
 */
static int (*olinuxino_fixes[]) (void *blob) = {
	board_fix_ethernet,
#ifdef CONFIG_VIDEO_LCD_PANEL_OLINUXINO
	board_fix_lcd_olinuxino_rgb,
#endif
};

int olinuxino_fdt_fixup(void *blob)
{
	uint8_t i;
	int ret;

	ret = fdt_increase_size(blob, 4096);
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


#if defined(CONFIG_OF_SYSTEM_SETUP)
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

	/* First make copy of the current ftd blob */
	recovery = malloc(gd->fdt_size);
	memcpy(recovery, blob, gd->fdt_size);

	/* Execute fixups */
	printf("Executing fdt system setup...\n");
	ret = olinuxino_fdt_fixup(blob);
	if (ret)
		goto recover;


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
int board_fix_fdt(void *blob)
{
	int offset;
	int ret;


	/* First increase size by 4K */
	ret = fdt_increase_size(blob, 4096);
	if (ret)
		goto error;

	/* Make sure mmc1 is disabled */
	offset = fdt_path_offset(blob, "/soc/mmc@1c10000");
	if (offset >= 0) {
		ret = fdt_setprop_string(blob, offset, "status", "disabled");
		if (ret)
			goto error;
	}

	/* TODO: Make other fixes */
	board_fix_ethernet(blob);

	return 0;

error:
	printf("%s: Failed to modify FDT blob: %d\n", __func__, ret);
	return 0;
}
#endif
