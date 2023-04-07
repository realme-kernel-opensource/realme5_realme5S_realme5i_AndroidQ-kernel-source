#
## Makefile for the STMicroelectronics touchscreen driver.
#

obj-$(CONFIG_TOUCHSCREEN_ST) += fts.o fts_gui.o fts_driver_test.o fts_lib/
ifeq ($(SHIPPING_API_LEVEL), 28)
ccflags-y += -DSHIPPING_API_LEVEL=28
else
ccflags-y += -DSHIPPING_API_LEVEL=29
endif
