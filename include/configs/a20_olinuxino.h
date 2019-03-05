/*
 *
 * Copyright (C) 2018 Olimex Ltd.
 *   Author: Stefan Mavrodiev <stefan@olimex.com>
 *
 * SPDX-License-Identifier: (GPL-2.0+ OR MIT)
 */
#ifndef __A20_OLINUXINO_CONFIG_H
#define __A20_OLINUXINO_CONFIG_H

#include <asm/arch/cpu.h>
#include <linux/stringify.h>

/*
 * A20 specific configuration
 */
#ifdef CONFIG_USB_EHCI_HCD
#define CONFIG_USB_EHCI_SUNXI
#endif

#define CONFIG_ARMV7_SECURE_BASE        SUNXI_SRAM_B_BASE
#define CONFIG_ARMV7_SECURE_MAX_SIZE    (64 * 1024) /* 64 KB */


#ifdef CONFIG_OLD_SUNXI_KERNEL_COMPAT
/*
 * The U-Boot workarounds bugs in the outdated buggy sunxi-3.4 kernels at the
 * expense of restricting some features, so the regular machine id values can
 * be used.
 */
# define CONFIG_MACH_TYPE_COMPAT_REV	0
#else
/*
 * A compatibility guard to prevent loading outdated buggy sunxi-3.4 kernels.
 * Only sunxi-3.4 kernels with appropriate fixes applied are able to pass
 * beyond the machine id check.
 */
# define CONFIG_MACH_TYPE_COMPAT_REV	1
#endif

/* Serial & console */
#define CONFIG_SYS_NS16550_SERIAL
/* ns16550 reg in the low bits of cpu reg */
#define CONFIG_SYS_NS16550_CLK		24000000
#ifndef CONFIG_DM_SERIAL
# define CONFIG_SYS_NS16550_REG_SIZE	-4
# define CONFIG_SYS_NS16550_COM1		SUNXI_UART0_BASE
# define CONFIG_SYS_NS16550_COM2		SUNXI_UART1_BASE
# define CONFIG_SYS_NS16550_COM3		SUNXI_UART2_BASE
# define CONFIG_SYS_NS16550_COM4		SUNXI_UART3_BASE
# define CONFIG_SYS_NS16550_COM5		SUNXI_R_UART_BASE
#endif

/* CPU */
#define COUNTER_FREQUENCY		24000000

#define SDRAM_OFFSET(x) 0x4##x
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_LOAD_ADDR		0x42000000 /* default load address */
/* Note SPL_STACK_R_ADDR is set through Kconfig, we include it here
 * since it needs to fit in with the other values. By also #defining it
 * we get warnings if the Kconfig value mismatches. */
#define CONFIG_SPL_STACK_R_ADDR		0x4fe00000
#define CONFIG_SPL_BSS_START_ADDR	0x4ff80000

#define CONFIG_SPL_BSS_MAX_SIZE		0x00080000 /* 512 KiB */

#ifdef CONFIG_SUNXI_HIGH_SRAM
/*
 * The A80's A1 sram starts at 0x00010000 rather then at 0x00000000 and is
 * slightly bigger. Note that it is possible to map the first 32 KiB of the
 * A1 at 0x00000000 like with older SoCs by writing 0x16aa0001 to the
 * undocumented 0x008000e0 SYS_CTRL register. Where the 16aa is a key and
 * the 1 actually activates the mapping of the first 32 KiB to 0x00000000.
 */
#define CONFIG_SYS_INIT_RAM_ADDR	0x10000
#define CONFIG_SYS_INIT_RAM_SIZE	0x08000	/* FIXME: 40 KiB ? */
#else
#define CONFIG_SYS_INIT_RAM_ADDR	0x0
#define CONFIG_SYS_INIT_RAM_SIZE	0x8000	/* 32 KiB */
#endif

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define PHYS_SDRAM_0			CONFIG_SYS_SDRAM_BASE
#define PHYS_SDRAM_0_SIZE		0x80000000 /* 2 GiB */

#ifdef CONFIG_AHCI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SUNXI_AHCI
#define CONFIG_SYS_64BIT_LBA
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					 CONFIG_SYS_SCSI_MAX_LUN)
#endif

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG
#define CONFIG_SERIAL_TAG

#ifdef CONFIG_SPL_SPI_SUNXI
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x8000
#endif

#if defined(CONFIG_ENV_IS_IN_SPI_FLASH)
#define CONFIG_ENV_OFFSET_REDUND	( CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SECT_SIZE		(128 << 10)
#endif

#if defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SYS_MMC_MAX_DEVICE	4
#endif

#if defined(CONFIG_ENV_IS_NOWHERE)
#define CONFIG_ENV_SIZE			(128 << 10)
#endif

#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (64 << 20))

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CBSIZE	1024	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE	1024	/* Print Buffer Size */

/* standalone support */
#define CONFIG_STANDALONE_LOAD_ADDR	CONFIG_SYS_LOAD_ADDR

/* FLASH and environment organization */

#define CONFIG_SYS_MONITOR_LEN		(768 << 10)	/* 768 KiB */

#define CONFIG_SPL_BOARD_LOAD_IMAGE

#ifdef CONFIG_SUNXI_HIGH_SRAM
#define CONFIG_SPL_TEXT_BASE		0x10060		/* sram start+header */
#define CONFIG_SPL_MAX_SIZE		0x7fa0		/* 32 KiB */
#define LOW_LEVEL_SRAM_STACK		0x00018000
#else
#define CONFIG_SPL_TEXT_BASE		0x60		/* sram start+header */
#define CONFIG_SPL_MAX_SIZE		0x5fa0		/* 24KB on sun4i/sun7i */
#define LOW_LEVEL_SRAM_STACK		0x00008000	/* End of sram */
#endif

#define CONFIG_SPL_STACK		LOW_LEVEL_SRAM_STACK

#define CONFIG_SPL_PAD_TO		32768		/* decimal for 'dd' */


/* I2C */
#define CONFIG_SYS_I2C_MVTWSI
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_SPEED		400000
#define CONFIG_SYS_I2C_SLAVE		0x7f


#define CONFIG_SYS_SPD_BUS_NUM		0 /* The axp209 i2c bus is bus 0 */
#define CONFIG_VIDEO_LCD_I2C_BUS	-1 /* NA, but necessary to compile */
#define LCD_OLINUXINO_EEPROM_BUS	I2C_2

#ifdef CONFIG_REQUIRE_SERIAL_CONSOLE
#define OF_STDOUT_PATH		"/soc@01c00000/serial@01c28000:115200"
#endif /* ifdef CONFIG_REQUIRE_SERIAL_CONSOLE */

/* GPIO */
#define CONFIG_SUNXI_GPIO

#ifdef CONFIG_VIDEO_SUNXI
/*
 * The amount of RAM to keep free at the top of RAM when relocating u-boot,
 * to use as framebuffer. This must be a multiple of 4096.
 */
#define CONFIG_SUNXI_MAX_FB_SIZE (16 << 20)

#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_STD_TIMINGS
#define CONFIG_I2C_EDID
#define VIDEO_LINE_LEN (pGD->plnSizeX)

/* allow both serial and cfb console. */
/* stop x86 thinking in cfbconsole from trying to init a pc keyboard */

#endif /* CONFIG_VIDEO_SUNXI */

/* Ethernet support */
#define CONFIG_MII			/* MII PHY management		*/

#ifdef CONFIG_USB_EHCI_HCD
#define CONFIG_USB_OHCI_NEW
#define CONFIG_USB_OHCI_SUNXI
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS 1
#endif

#ifdef CONFIG_NAND_SUNXI
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_SYS_MAX_NAND_DEVICE 1

#define NAND_MTDIDS "nand0=nand.0"
#define NAND_MTDPARTS "mtdparts=nand.0:4m(NAND.SPL),4m(NAND.SPL.backup),4m(NAND.u-boot),4m(NAND.u-boot.backup),4m(NAND.u-boot-env),4m(NAND.u-boot-env.backup),4m(NAND.dtb),16m(NAND.kernel),-(NAND.rootfs)"

#define	BOOTENV_DEV_NAND(devtypeu, devtypel, instance) \
	"bootcmd_nand=" \
		"echo 'NAND booting is temporary disabled.'\0"

#define BOOTENV_DEV_NAME_NAND(devtypeu, devtypel, instance) \
		"nand "

#define BOOT_TARGET_DEVICES_NAND(func) func(NAND, nand, 0)
#else
#define BOOT_TARGET_DEVICES_NAND(func)
#endif


#ifdef CONFIG_USB_KEYBOARD
#define CONFIG_PREBOOT
#endif

#ifndef CONFIG_SPL_BUILD

/*
 * 160M RAM (256M minimum minus 64MB heap + 32MB for u-boot, stack, fb, etc.
 * 32M uncompressed kernel, 16M compressed kernel, 1M fdt,
 * 1M script, 1M pxe and the ramdisk at the end.
 */
#define BOOTM_SIZE     __stringify(0xa000000)
#define KERNEL_ADDR_R  __stringify(SDRAM_OFFSET(2000000))
#define FDT_ADDR_R     __stringify(SDRAM_OFFSET(3000000))
#define SCRIPT_ADDR_R  __stringify(SDRAM_OFFSET(3100000))
#define RAMDISK_ADDR_R __stringify(SDRAM_OFFSET(3200000))

#define MEM_LAYOUT_ENV_SETTINGS \
	"bootm_size=" BOOTM_SIZE "\0" \
	"kernel_addr_r=" KERNEL_ADDR_R "\0" \
	"fdt_addr_r=" FDT_ADDR_R "\0" \
	"scriptaddr=" SCRIPT_ADDR_R "\0" \
	"ramdisk_addr_r=" RAMDISK_ADDR_R "\0" \
	"fit_addr_r=0x50000000\0"


#ifdef CONFIG_DFU
#define CONFIG_SET_DFU_ALT_INFO

#ifdef CONFIG_DFU_MMC
#define DFU_ALT_INFO_MMC0 ""
#define DFU_ALT_INFO_MMC2 ""
#else
#define DFU_ALT_INFO_MMC0 ""
#define DFU_ALT_INFO_MMC2 ""
#endif

#ifdef CONFIG_DFU_NAND
#define DFU_ALT_INFO_NAND \
	"dfu_alt_info_nand=" \
		"NAND.SPL raw 0x00000000 0x00400000;" \
		"NAND.SPL.backup part 0 2;" \
		"NAND.u-boot part 0 3;" \
		"NAND.u-boot.backup part 0 4;" \
		"NAND.u-boot-env part 0 5;" \
		"NAND.u-boot-env.backup part 0 6;" \
		"NAND.dtb part 0 7;" \
		"NAND.kernel part 0 8;" \
		"NAND.rootfs part 0 9\0"
#else
#define DFU_ALT_INFO_NAND ""
#endif

#ifdef CONFIG_DFU_RAM
#define DFU_ALT_INFO_RAM \
	"dfu_alt_info_ram=" \
	"kernel ram " KERNEL_ADDR_R " 0x1000000;" \
	"fdt ram " FDT_ADDR_R " 0x100000;" \
	"ramdisk ram " RAMDISK_ADDR_R " 0x4000000\0"
#else
#define DFU_ALT_INFO_RAM ""
#endif

#ifdef CONFIG_DFU_SF
#define DFU_ALT_INFO_SF \
	"dfu_alt_info_sf=" \
		"SPI.u-boot raw 0 0x200000;" \
		"SPI.u-boot-env raw 0x200000 0x20000;" \
		"SPI.u-boot-env.backup raw 0x220000 0x20000\0"
#else
#define DFU_ALT_INFO_SF ""
#endif

#define DFU_ALT_INFO \
	DFU_ALT_INFO_MMC0 \
	DFU_ALT_INFO_MMC2 \
	DFU_ALT_INFO_RAM \
	DFU_ALT_INFO_NAND \
	DFU_ALT_INFO_SF

#endif /* CONFIG_DFU */

#ifdef CONFIG_SPI_FLASH
#define SPI_MTDIDS "nor0=flash.0"
#define SPI_MTDPARTS "mtdparts=flash.0:2m(SPI.u-boot),128k(SPI.u-boot-env),128k(SPI.u-boot-env.backup),-(SPI.user)"
#endif

#ifdef CONFIG_MMC
#define BOOTENV_DEV_MMC_AUTO(devtypeu, devtypel, instance)		\
	BOOTENV_DEV_MMC(MMC, mmc, 0)					\
	BOOTENV_DEV_MMC(MMC, mmc, 1)					\
	"bootcmd_mmc_auto="						\
		"if test ${mmc_bootdev} -eq 1; then "			\
			"run bootcmd_mmc1; "				\
			"run bootcmd_mmc0; "				\
		"elif test ${mmc_bootdev} -eq 0; then "			\
			"run bootcmd_mmc0; "				\
			"run bootcmd_mmc1; "				\
		"fi\0"

#define BOOTENV_DEV_NAME_MMC_AUTO(devtypeu, devtypel, instance) \
	"mmc_auto "

#define BOOT_TARGET_DEVICES_MMC(func) func(MMC_AUTO, mmc_auto, na)
#else
#define BOOT_TARGET_DEVICES_MMC(func)
#endif

#ifdef CONFIG_AHCI
#define BOOT_TARGET_DEVICES_SCSI(func) func(SCSI, scsi, 0)
#else
#define BOOT_TARGET_DEVICES_SCSI(func)
#endif

#ifdef CONFIG_USB_STORAGE
#define BOOT_TARGET_DEVICES_USB(func) func(USB, usb, 0)
#else
#define BOOT_TARGET_DEVICES_USB(func)
#endif

/* FEL boot support, auto-execute boot.scr if a script address was provided */
#define BOOTENV_DEV_FEL(devtypeu, devtypel, instance) \
	"bootcmd_fel=" \
		"if test -n ${fel_booted} && test -n ${fel_scriptaddr}; then " \
			"echo '(FEL boot)'; " \
			"source ${fel_scriptaddr}; " \
		"fi\0"
#define BOOTENV_DEV_NAME_FEL(devtypeu, devtypel, instance) \
	"fel "

#define BOOT_TARGET_DEVICES(func) \
	func(FEL, fel, na) \
	BOOT_TARGET_DEVICES_MMC(func) \
	BOOT_TARGET_DEVICES_NAND(func) \
	BOOT_TARGET_DEVICES_SCSI(func) \
	BOOT_TARGET_DEVICES_USB(func) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#ifdef CONFIG_USB_KEYBOARD
#define CONSOLE_STDIN_SETTINGS \
	"preboot=usb start\0" \
	"stdin=serial,usbkbd\0"
#else
#define CONSOLE_STDIN_SETTINGS \
	"stdin=serial\0"
#endif

#ifdef CONFIG_VIDEO
#define CONSOLE_STDOUT_SETTINGS \
	"stdout=serial,vga\0" \
	"stderr=serial,vga\0"
#elif CONFIG_DM_VIDEO
#define CONFIG_SYS_WHITE_ON_BLACK
#define CONSOLE_STDOUT_SETTINGS \
	"stdout=serial,vidconsole\0" \
	"stderr=serial,vidconsole\0"
#else
#define CONSOLE_STDOUT_SETTINGS \
	"stdout=serial\0" \
	"stderr=serial\0"
#endif

#define CONSOLE_ENV_SETTINGS \
	CONSOLE_STDIN_SETTINGS \
	CONSOLE_STDOUT_SETTINGS

#define CONFIG_EXTRA_ENV_SETTINGS \
	CONSOLE_ENV_SETTINGS \
	MEM_LAYOUT_ENV_SETTINGS \
	DFU_ALT_INFO \
	"console=ttyS0,115200\0" \
	BOOTENV

#else /* ifndef CONFIG_SPL_BUILD */
#define CONFIG_EXTRA_ENV_SETTINGS
#endif

#define CONFIG_MACH_TYPE	(4283 | ((CONFIG_MACH_TYPE_COMPAT_REV) << 28))

#endif /* __A20_OLINUXINO_CONFIG_H */
