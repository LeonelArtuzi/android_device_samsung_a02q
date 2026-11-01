DEVICE_PATH := device/samsung/a02q

# Architecture (SD450/msm8953): arm64 kernel, 32-bit userspace like stock.
# The stock vendor is 32-bit only (no vendor/lib64; Adreno EGL/GLES are
# 32-bit), so a 64-bit system couldn't load the GPU driver.
TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv8-a
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_VARIANT := generic
TARGET_CPU_VARIANT_RUNTIME := cortex-a53
TARGET_KERNEL_ARCH := arm64
# Kernel has no CONFIG_ANDROID_BINDER_IPC_32BIT.
TARGET_USES_64_BIT_BINDER := true

TARGET_BOARD_PLATFORM := msm8953
QCOM_BOARD_PLATFORMS += msm8953
TARGET_BOOTLOADER_BOARD_NAME := SRPTH21A009
TARGET_NO_BOOTLOADER := true

# Kernel (built from source).
BOARD_KERNEL_IMAGE_NAME := Image.gz
BOARD_KERNEL_BASE := 0x80000000
BOARD_KERNEL_PAGESIZE := 2048
BOARD_KERNEL_OFFSET := 0x00008000
BOARD_RAMDISK_OFFSET := 0x02000000
BOARD_KERNEL_TAGS_OFFSET := 0x01e00000
BOARD_DTB_OFFSET := 0x01f00000
BOARD_BOOT_HEADER_VERSION := 2
BOARD_INCLUDE_DTB_IN_BOOTIMG := true
BOARD_KERNEL_SEPARATED_DTBO := true
BOARD_KERNEL_CMDLINE := console=ttyMSM0,115200,n8 androidboot.console=ttyMSM0 androidboot.hardware=qcom user_debug=30 msm_rtb.filter=0x237 ehci-hcd.park=3 androidboot.bootdevice=7824900.sdhci lpm_levels.sleep_disabled=1 earlycon=msm_hsl_uart,0x78af000 androidboot.usbconfigfs=true vmalloc=300M loop.max_part=7
# shortcut: permissive while the vendor sepolicy is brought up; drop once denials are fixed.
BOARD_KERNEL_CMDLINE += androidboot.selinux=permissive
BOARD_MKBOOTIMG_ARGS += --ramdisk_offset $(BOARD_RAMDISK_OFFSET)
BOARD_MKBOOTIMG_ARGS += --tags_offset $(BOARD_KERNEL_TAGS_OFFSET)
BOARD_MKBOOTIMG_ARGS += --kernel_offset $(BOARD_KERNEL_OFFSET)
BOARD_MKBOOTIMG_ARGS += --dtb_offset $(BOARD_DTB_OFFSET)
BOARD_MKBOOTIMG_ARGS += --header_version $(BOARD_BOOT_HEADER_VERSION)
BOARD_MKBOOTIMG_ARGS += --board $(TARGET_BOOTLOADER_BOARD_NAME)
TARGET_KERNEL_SOURCE := kernel/samsung/a02q
TARGET_KERNEL_CONFIG := a02q_ltn_open_defconfig a02q_lineage.config
# Stock modules' vermagic is 4.9.227-26302074; setting LOCALVERSION also stops the git "+" suffix.
TARGET_KERNEL_ADDITIONAL_FLAGS := LOCALVERSION=-26302074
# Defines HUAQIN_KERNEL_PROJECT_HS50 like Samsung's AndroidKernel.mk; without it the
# s5k3l6 sjc/ofilm module check is compiled out and the wrong OTP/actuator is used.
TARGET_KERNEL_ADDITIONAL_FLAGS += HUAQIN_PROJECT_NAME=HS50
# Samsung's dtc 1.4.4 (same as stock) + overlay build for the hs50 .dtbo files.
TARGET_KERNEL_ADDITIONAL_FLAGS += \
    DTC_EXT=$(abspath $(TARGET_KERNEL_SOURCE))/tools/dtc \
    CONFIG_BUILD_ARM64_DT_OVERLAY=y
# dtbo: hs50 overlays in stock order.
BOARD_DTBO_CFG := $(DEVICE_PATH)/dtbo.cfg
BOARD_INCLUDE_RECOVERY_DTBO := true

# Audio modules from techpack/audio (Samsung audio-kernel), renamed to the
# names stock init.target.rc modprobes.
TARGET_MODULE_ALIASES := \
    adsp_loader_dlkm.ko:audio_adsp_loader.ko \
    analog_cdc_dlkm.ko:audio_analog_cdc.ko \
    apr_dlkm.ko:audio_apr.ko \
    cpe_lsm_dlkm.ko:audio_cpe_lsm.ko \
    digital_cdc_dlkm.ko:audio_digital_cdc.ko \
    fs1894_dlkm.ko:audio_fs1894.ko \
    hdmi_dlkm.ko:audio_hdmi.ko \
    machine_dlkm.ko:audio_machine_sdm450.ko \
    machine_ext_dlkm.ko:audio_machine_ext_sdm450.ko \
    mbhc_dlkm.ko:audio_mbhc.ko \
    native_dlkm.ko:audio_native.ko \
    pinctrl_wcd_dlkm.ko:audio_pinctrl_wcd.ko \
    platform_dlkm.ko:audio_platform.ko \
    q6_dlkm.ko:audio_q6.ko \
    q6_notifier_dlkm.ko:audio_q6_notifier.ko \
    stub_dlkm.ko:audio_stub.ko \
    swr_ctrl_dlkm.ko:audio_swr_ctrl.ko \
    swr_dlkm.ko:audio_swr.ko \
    usf_dlkm.ko:audio_usf.ko \
    wcd9335_dlkm.ko:audio_wcd9335.ko \
    wcd9xxx_dlkm.ko:audio_wcd9xxx.ko \
    wcd_core_dlkm.ko:audio_wcd_core.ko \
    wcd_cpe_dlkm.ko:audio_wcd_cpe.ko \
    wsa881x_analog_dlkm.ko:audio_wsa881x_analog.ko \
    wsa881x_dlkm.ko:audio_wsa881x.ko
# shortcut: kernel builds from source; bring-up may switch to a prebuilt Image.gz if the inline build fights the Lineage toolchain. Upgrade when moving past Android 12.

# Partitions.
BOARD_FLASH_BLOCK_SIZE := 131072
BOARD_BOOTIMAGE_PARTITION_SIZE := 67108864
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 67108864
BOARD_DTBOIMG_PARTITION_SIZE := 8388608
BOARD_USES_METADATA_PARTITION := true
BOARD_HAS_LARGE_FILESYSTEM := true

# Dynamic partitions (super).
BOARD_SUPER_PARTITION_SIZE := 4848615424
BOARD_SUPER_PARTITION_GROUPS := qti_dynamic_partitions
BOARD_QTI_DYNAMIC_PARTITIONS_SIZE := 4844421120
BOARD_QTI_DYNAMIC_PARTITIONS_PARTITION_LIST := system vendor product odm
TARGET_COPY_OUT_VENDOR := vendor
TARGET_COPY_OUT_PRODUCT := product
TARGET_COPY_OUT_ODM := odm
BOARD_SYSTEMIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_PRODUCTIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_ODMIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_USERDATAIMAGE_FILE_SYSTEM_TYPE := f2fs
# Cache size from the stock PIT (A02Q_CIS_OPEN.pit); non-A/B OTAs stash in it.
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_CACHEIMAGE_PARTITION_SIZE := 167772160
TARGET_USERIMAGES_USE_EXT4 := true
TARGET_USERIMAGES_USE_F2FS := true

# AVB (recovery; test key — real signing is a bring-up decision).
BOARD_AVB_ENABLE := true
BOARD_AVB_RECOVERY_KEY_PATH := external/avb/test/data/testkey_rsa4096.pem
BOARD_AVB_RECOVERY_ALGORITHM := SHA256_RSA4096
BOARD_AVB_RECOVERY_ROLLBACK_INDEX := 0
BOARD_AVB_RECOVERY_ROLLBACK_INDEX_LOCATION := 2

# Treble / VNDK (stock vendor is VNDK 31 on A12 base).
BOARD_VNDK_VERSION := current

# Stock blobs ship as PRODUCT_COPY_FILES; as cc_prebuilt modules soong would
# check every .so's deps against ~1900 32-bit Samsung libs.
BUILD_BROKEN_ELF_PREBUILT_PRODUCT_COPY_FILES := true

# RIL: keep Samsung's stock rild (skips AOSP hardware/ril/rild).
ENABLE_VENDOR_RIL_SERVICE := true

# VINTF: stock vendor manifest + device compatibility matrix, minus the
# hard-coded vendor-ndk/system-sdk/sepolicy versions (stock is 31, this
# vendor is built as 32), which assemble_vintf fills in.
DEVICE_MANIFEST_FILE := $(DEVICE_PATH)/configs/vintf/manifest.xml
DEVICE_MATRIX_FILE := $(DEVICE_PATH)/configs/vintf/compatibility_matrix.xml

# Vendor AIDs (passwd/group) and file caps the stock vendor init/rc expect.
TARGET_FS_CONFIG_GEN := $(DEVICE_PATH)/config.fs

# Filesystem: /efs (sec_efs) holds Samsung RIL data (IMEI etc.).
BOARD_ROOT_EXTRA_FOLDERS := efs
TARGET_RECOVERY_FSTAB := $(DEVICE_PATH)/rootdir/etc/fstab.qcom

# Properties (stock vendor/build.prop is regenerated by the build).
TARGET_VENDOR_PROP := $(DEVICE_PATH)/vendor.prop

# Wi-Fi (pronto/WCN36xx): prima is built into the kernel and starts when
# libwifi-hal writes /sys/module/wlan/parameters/fwpath.
BOARD_WLAN_DEVICE := qcwcn
BOARD_HOSTAPD_DRIVER := NL80211
BOARD_HOSTAPD_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_$(BOARD_WLAN_DEVICE)
WPA_SUPPLICANT_VERSION := VER_0_8_X
WIFI_DRIVER_FW_PATH_STA := "sta"
WIFI_DRIVER_FW_PATH_AP := "ap"
WIFI_DRIVER_FW_PATH_P2P := "p2p"

# Bluetooth (pronto, SMD): CAF libbt-vendor under the AOSP HIDL service;
# the NVM tags are sent by a02q-bt-nvm (bluetooth/).
BOARD_HAVE_BLUETOOTH_QCOM := true

# SELinux: qcom legacy-um policy (lists msm8953). Device policy goes in sepolicy/vendor.
include device/qcom/sepolicy-legacy-um/SEPolicy.mk
BOARD_VENDOR_SEPOLICY_DIRS += $(DEVICE_PATH)/sepolicy/vendor

include vendor/lineage/config/BoardConfigLineage.mk
