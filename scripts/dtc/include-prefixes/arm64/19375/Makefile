ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)
	# ORIGIN
	dtbo-y += trinket-idp-overlay.dtbo
	trinket-idp-overlay.dtbo-base := trinket.dtb

	# EVB1
	dtbo-y += trinket-idp-overlay-EVB1.dtbo
	trinket-idp-overlay-EVB1.dtbo-base := trinket-EVB1.dtb

	# T0
	dtbo-y += trinket-idp-overlay-T0.dtbo
	trinket-idp-overlay-T0.dtbo-base := trinket-T0.dtb

	# T1
	dtbo-y += trinket-idp-overlay-T1.dtbo
	trinket-idp-overlay-T1.dtbo-base := trinket-T1.dtb

	# EVT1
	dtbo-y += trinket-idp-overlay-EVT1.dtbo
	trinket-idp-overlay-EVT1.dtbo-base := trinket-EVT1.dtb
	
	# EVT2
	dtbo-y += trinket-idp-overlay-EVT2.dtbo
	trinket-idp-overlay-EVT2.dtbo-base := trinket-EVT2.dtb
	
	# DVT1
	dtbo-y += trinket-idp-overlay-DVT1.dtbo
	trinket-idp-overlay-DVT1.dtbo-base := trinket-DVT1.dtb

	# DVT2
	dtbo-y += trinket-idp-overlay-DVT2.dtbo
	trinket-idp-overlay-DVT2.dtbo-base := trinket-DVT2.dtb

	#PVT1
	dtbo-y += trinket-idp-overlay-PVT1.dtbo
	trinket-idp-overlay-PVT1.dtbo-base := trinket-PVT1.dtb

	#MP1
	dtbo-y += trinket-idp-overlay-MP1.dtbo
	trinket-idp-overlay-MP1.dtbo-base := trinket-MP1.dtb
	
	#MP2
	dtbo-y += trinket-idp-overlay-MP2.dtbo
	trinket-idp-overlay-MP2.dtbo-base := trinket-MP2.dtb
else
	dtb-y	+= trinket-idp.dtb
endif

always		:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo
