ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)
	dtbo-$(CONFIG_ARCH_TRINKET) += trinket-idp-overlay.dtbo
	trinket-idp-overlay.dtbo-base := trinket.dtb
	
	dtbo-$(CONFIG_ARCH_TRINKET) += trinket-idp-overlay-19743.dtbo
	trinket-idp-overlay-19743.dtbo-base := trinket-19743.dtb
	dtbo-$(CONFIG_ARCH_TRINKET) += trinket-idp-overlay-19632.dtbo
	trinket-idp-overlay-19632.dtbo-base := trinket-19632.dtb
	dtbo-$(CONFIG_ARCH_TRINKET) += trinket-idp-overlay-19631.dtbo
	trinket-idp-overlay-19631.dtbo-base := trinket-19631.dtb
	
else
	dtb-$(CONFIG_ARCH_TRINKET)	+= trinket-idp.dtb
	dtb-$(CONFIG_ARCH_TRINKET)	+= trinket-idp-19632.dtb
	dtb-$(CONFIG_ARCH_TRINKET)	+= trinket-idp-19743.dtb
endif
always		:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo
