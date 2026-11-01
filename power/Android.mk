# QTI open-source AIDL power HAL, built from vendor/qcom/opensource/power.
# Its own Android.mk is guarded by is-vendor-board-platform,QCOM, which is
# false here because BOARD_USES_QCOM_HARDWARE stays off (stock display/audio/
# media blobs), so the module would never be defined. Same sources, msm8953 only.
LOCAL_PATH := vendor/qcom/opensource/power

include $(CLEAR_VARS)

LOCAL_MODULE := android.hardware.power-service-qti
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_VENDOR_MODULE := true
LOCAL_INIT_RC := android.hardware.power-service-qti.rc
LOCAL_VINTF_FRAGMENTS := power.xml

LOCAL_SRC_FILES := \
    power-common.c \
    metadata-parser.c \
    utils.c \
    list.c \
    hint-data.c \
    Power.cpp \
    main.cpp \
    power-8953.c \
    ../../../../device/samsung/a02q/power/power-mode.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libdl \
    libbase \
    libutils \
    android.hardware.power-V1-ndk_platform \
    libbinder_ndk

LOCAL_HEADER_LIBRARIES := libhardware_headers

# Boosts go through the stock perf HAL (libqti-perfd-client, dlopen'ed).
# MODE_EXT: power-mode.cpp (battery saver caps the CPU).
LOCAL_CFLAGS += -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable \
    -DINTERACTION_BOOST -DMODE_EXT

include $(BUILD_EXECUTABLE)
