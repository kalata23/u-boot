# SPDX-License-Identifier: GPL-2.0+
#
# Makefile
#
# Copyright (C) 2018 Olimex Ltd.

obj-y	+= board.o
obj-y	+= ../common/board_detect.o
obj-y	+= ../../sunxi/dram_sun5i_auto.o

ifndef CONFIG_SPL_BUILD
obj-y	+= ../common/boards.o
obj-y	+= ../common/cmd_olinuxino.o
obj-$(CONFIG_VIDEO_LCD_PANEL_OLINUXINO)	+= ../common/lcd_olinuxino.o
obj-$(CONFIG_SUNXI_AHCI)	+= ../../sunxi/ahci.o
obj-$(CONFIG_OF_LIBFDT)		+= fdt.o
endif
