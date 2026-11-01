LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_DEVICE),a02q)

# Stock vendor layout that PRODUCT_COPY_FILES can't carry: empty mount points
# used by fstab.qcom and the symlinks shipped in the stock vendor image
# (Adreno libs, rfs tree for tftp_server/rmt_storage).
A02Q_VENDOR_DIRS := bt_firmware dsp firmware-modem firmware_mnt

A02Q_VENDOR_SYMLINKS := \
    app/CneApp/lib/arm/libvndfwk_detect_jni.qti.so:/vendor/lib/libvndfwk_detect_jni.qti.so \
    firmware/wlanmdsp.otaupdate:/data/vendor/firmware/wlanmdsp.mbn \
    lib/libEGL_adreno.so:egl/libEGL_adreno.so \
    lib/libGLESv2_adreno.so:egl/libGLESv2_adreno.so \
    rfs/apq/gnss/hlos:/mnt/vendor/persist/hlos_rfs/shared \
    rfs/apq/gnss/ramdumps:/data/vendor/tombstones/rfs/modem \
    rfs/apq/gnss/readonly/firmware:/vendor/firmware_mnt \
    rfs/apq/gnss/readonly/vendor/firmware:/vendor/firmware \
    rfs/apq/gnss/readwrite:/mnt/vendor/persist/rfs/apq/gnss \
    rfs/apq/gnss/shared:/mnt/vendor/persist/rfs/shared \
    rfs/mdm/adsp/hlos:/mnt/vendor/persist/hlos_rfs/shared \
    rfs/mdm/adsp/ramdumps:/data/vendor/tombstones/rfs/lpass \
    rfs/mdm/adsp/readonly/firmware:/vendor/firmware_mnt \
    rfs/mdm/adsp/readonly/vendor/firmware:/vendor/firmware \
    rfs/mdm/adsp/readwrite:/mnt/vendor/persist/rfs/mdm/adsp \
    rfs/mdm/adsp/shared:/mnt/vendor/persist/rfs/shared \
    rfs/mdm/cdsp/hlos:/mnt/vendor/persist/hlos_rfs/shared \
    rfs/mdm/cdsp/ramdumps:/data/vendor/tombstones/rfs/cdsp \
    rfs/mdm/cdsp/readonly/firmware:/vendor/firmware_mnt \
    rfs/mdm/cdsp/readonly/vendor/firmware:/vendor/firmware \
    rfs/mdm/cdsp/readwrite:/mnt/vendor/persist/rfs/mdm/cdsp \
    rfs/mdm/cdsp/shared:/mnt/vendor/persist/rfs/shared \
    rfs/mdm/mpss/hlos:/mnt/vendor/persist/hlos_rfs/shared \
    rfs/mdm/mpss/ramdumps:/data/vendor/tombstones/rfs/modem \
    rfs/mdm/mpss/readonly/vendor/firmware:/vendor/firmware \
    rfs/mdm/mpss/readwrite:/mnt/vendor/persist/rfs/mdm/mpss \
    rfs/mdm/mpss/shared:/mnt/vendor/persist/rfs/shared \
    rfs/mdm/slpi/hlos:/mnt/vendor/persist/hlos_rfs/shared \
    rfs/mdm/slpi/ramdumps:/data/vendor/tombstones/rfs/slpi \
    rfs/mdm/slpi/readonly/firmware:/vendor/firmware_mnt \
    rfs/mdm/slpi/readonly/vendor/firmware:/vendor/firmware \
    rfs/mdm/slpi/readwrite:/mnt/vendor/persist/rfs/mdm/slpi \
    rfs/mdm/slpi/shared:/mnt/vendor/persist/rfs/shared \
    rfs/mdm/tn/hlos:/mnt/vendor/persist/hlos_rfs/shared \
    rfs/mdm/tn/ramdumps:/data/vendor/tombstones/rfs/tn \
    rfs/mdm/tn/readonly/firmware:/vendor/firmware_mnt \
    rfs/mdm/tn/readonly/vendor/firmware:/vendor/firmware \
    rfs/mdm/tn/readwrite:/mnt/vendor/persist/rfs/mdm/tn \
    rfs/mdm/tn/shared:/mnt/vendor/persist/rfs/shared \
    rfs/mdm/wpss/hlos:/mnt/vendor/persist/hlos_rfs/shared \
    rfs/mdm/wpss/ramdumps:/data/vendor/tombstones/rfs/wpss \
    rfs/mdm/wpss/readonly/firmware:/vendor/firmware_mnt \
    rfs/mdm/wpss/readonly/vendor/firmware:/vendor/firmware \
    rfs/mdm/wpss/readwrite:/mnt/vendor/persist/rfs/mdm/wpss \
    rfs/mdm/wpss/shared:/mnt/vendor/persist/rfs/shared \
    rfs/msm/adsp/hlos:/mnt/vendor/persist/hlos_rfs/shared \
    rfs/msm/adsp/ramdumps:/data/vendor/tombstones/rfs/lpass \
    rfs/msm/adsp/readonly/firmware:/vendor/firmware_mnt \
    rfs/msm/adsp/readonly/vendor/firmware:/vendor/firmware \
    rfs/msm/adsp/readwrite:/mnt/vendor/persist/rfs/msm/adsp \
    rfs/msm/adsp/shared:/mnt/vendor/persist/rfs/shared \
    rfs/msm/cdsp/hlos:/mnt/vendor/persist/hlos_rfs/shared \
    rfs/msm/cdsp/ramdumps:/data/vendor/tombstones/rfs/cdsp \
    rfs/msm/cdsp/readonly/firmware:/vendor/firmware_mnt \
    rfs/msm/cdsp/readonly/vendor/firmware:/vendor/firmware \
    rfs/msm/cdsp/readwrite:/mnt/vendor/persist/rfs/msm/cdsp \
    rfs/msm/cdsp/shared:/mnt/vendor/persist/rfs/shared \
    rfs/msm/mpss/hlos:/mnt/vendor/persist/hlos_rfs/shared \
    rfs/msm/mpss/ramdumps:/data/vendor/tombstones/rfs/modem \
    rfs/msm/mpss/readonly/firmware:/vendor/firmware-modem \
    rfs/msm/mpss/readonly/vendor/firmware:/vendor/firmware \
    rfs/msm/mpss/readwrite:/mnt/vendor/persist/rfs/msm/mpss \
    rfs/msm/mpss/shared:/mnt/vendor/persist/rfs/shared \
    rfs/msm/slpi/hlos:/mnt/vendor/persist/hlos_rfs/shared \
    rfs/msm/slpi/ramdumps:/data/vendor/tombstones/rfs/slpi \
    rfs/msm/slpi/readonly/firmware:/vendor/firmware_mnt \
    rfs/msm/slpi/readonly/vendor/firmware:/vendor/firmware \
    rfs/msm/slpi/readwrite:/mnt/vendor/persist/rfs/msm/slpi \
    rfs/msm/slpi/shared:/mnt/vendor/persist/rfs/shared \
    rfs/msm/wpss/hlos:/mnt/vendor/persist/hlos_rfs/shared \
    rfs/msm/wpss/ramdumps:/data/vendor/tombstones/rfs/wpss \
    rfs/msm/wpss/readonly/firmware:/vendor/firmware_mnt \
    rfs/msm/wpss/readonly/vendor/firmware:/vendor/firmware \
    rfs/msm/wpss/readwrite:/mnt/vendor/persist/rfs/msm/wpss \
    rfs/msm/wpss/shared:/mnt/vendor/persist/rfs/shared

A02Q_VENDOR_DIR_TARGETS := $(addprefix $(TARGET_OUT_VENDOR)/,$(A02Q_VENDOR_DIRS))
$(A02Q_VENDOR_DIR_TARGETS):
	@mkdir -p $@

define a02q-vendor-symlink
$(TARGET_OUT_VENDOR)/$(1):
	@mkdir -p $$(dir $$@)
	@ln -sfn $(2) $$@
ALL_DEFAULT_INSTALLED_MODULES += $(TARGET_OUT_VENDOR)/$(1)
endef
$(foreach l,$(A02Q_VENDOR_SYMLINKS),\
    $(eval $(call a02q-vendor-symlink,$(word 1,$(subst :, ,$(l))),$(word 2,$(subst :, ,$(l))))))

ALL_DEFAULT_INSTALLED_MODULES += $(A02Q_VENDOR_DIR_TARGETS)

# QTI open-source power HAL (see power/Android.mk). Resets LOCAL_PATH too.
include $(LOCAL_PATH)/power/Android.mk

# CAF libbt-vendor (pronto SMD path). hardware/qcom-caf/bt/Android.mk only
# includes it with BOARD_USES_QCOM_HARDWARE, which stays off for the stock
# display/audio/media blobs. Kept last: it resets LOCAL_PATH.
include hardware/qcom-caf/bt/libbt-vendor/Android.mk

endif
