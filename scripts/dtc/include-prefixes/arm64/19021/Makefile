ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)
	dtbo-$(CONFIG_ARCH_TRINKET) += trinket-idp-overlay.dtbo
	trinket-idp-overlay.dtbo-base := trinket.dtb
	
	dtbo-$(CONFIG_ARCH_TRINKET) += trinket-idp-overlay-19021.dtbo
	trinket-idp-overlay-19021.dtbo-base := trinket-19021.dtb
	dtbo-$(CONFIG_ARCH_TRINKET) += trinket-idp-overlay-19026.dtbo
	trinket-idp-overlay-19026.dtbo-base := trinket-19026.dtb
	
	dtbo-$(CONFIG_ARCH_TRINKET) += trinket-idp-overlay-19321.dtbo
	trinket-idp-overlay-19321.dtbo-base := trinket-19321.dtb
	
	dtbo-$(CONFIG_ARCH_TRINKET) += trinket-idp-overlay-19328.dtbo
	trinket-idp-overlay-19328.dtbo-base := trinket-19328.dtb
else
	dtb-$(CONFIG_ARCH_TRINKET)	+= trinket-idp.dtb
	dtb-$(CONFIG_ARCH_TRINKET)	+= trinket-idp-19026.dtb
	dtb-$(CONFIG_ARCH_TRINKET)	+= trinket-idp-19321.dtb
	dtb-$(CONFIG_ARCH_TRINKET)	+= trinket-idp-19328.dtb
endif
always		:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo
