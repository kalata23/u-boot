# SPDX-License-Identifier: GPL-2.0+
#
# Makefile
#
# Copyright (C) 2018 Olimex Ltd.

obj-y	+= board.o
obj-y	+= ../common/board_detect.o
obj-y	+= ../common/boards.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_VIDEO_LCD_PANEL_OLINUXINO)	+= ../common/lcd_olinuxino.o
endif
