# a02q device packages. Minimal skeleton — HALs, blobs, overlays and configs
# are added during bring-up.

# Proprietary blobs (populated later by vendor/samsung/a02q via extract-files.sh).
$(call inherit-product-if-exists, vendor/samsung/a02q/a02q-vendor.mk)

# Open-source HALs (replace the stock blobs of the same name).
PRODUCT_PACKAGES += \
    android.hardware.atrace@1.0-service \
    android.hardware.cas@1.2-service \
    android.hardware.drm@1.4-service.clearkey \
    android.hardware.wifi@1.0-service \
    android.hardware.vibrator-service.a02q

# Bluetooth: AOSP HIDL service + CAF libbt-vendor (SMD) + NVM tag loader (bluetooth/).
PRODUCT_PACKAGES += \
    a02q-bt-nvm \
    android.hardware.bluetooth@1.0-impl \
    android.hardware.bluetooth@1.0-service \
    libbt-vendor

# Overlays (frameworks/base from the stock RROs, Settings, lineage-sdk).
DEVICE_PACKAGE_OVERLAYS += device/samsung/a02q/overlay

# RROs for APEX modules, which static overlays cannot reach.
PRODUCT_PACKAGES += \
    TetheringResA02q \
    WifiResA02q

# Display: LiveDisplay SDM backend (picture adjustment through the stock
# libsdm-disp-vndapis).
PRODUCT_PACKAGES += \
    vendor.lineage.livedisplay@2.0-service-sdm

# Display: CAF SDM HWC2, gralloc1 and memtrack (hardware/qcom-caf/msm8953/display).
PRODUCT_PACKAGES += \
    gralloc.msm8953 \
    hwcomposer.msm8953 \
    libdisplayconfig.qti \
    libqdMetaData_a02q \
    memtrack.msm8953

# CAF modules replacing stock blobs (their sources are built with
# BOARD_USES_QCOM_HARDWARE): IPA/data, OMX media, display config AIDL, lights.
PRODUCT_PACKAGES += \
    IPACM_cfg.xml \
    ipacm \
    librmnetctl \
    libc2dcolorconvert \
    libOmxCore \
    libmm-omxcore \
    libOmxVdec \
    libOmxVenc \
    libstagefrighthw \
    lights.msm8953 \
    vendor.qti.hardware.display.config-V1-ndk_platform \
    vendor.qti.hardware.display.config-V2-ndk_platform \
    vendor.qti.hardware.display.config-V3-ndk_platform \
    vendor.qti.hardware.display.config-V4-ndk_platform \
    vendor.qti.hardware.display.config-V5-ndk_platform \
    vendor.qti.hardware.display.config-V6-ndk_platform

# Media codec lists: platform/Samsung configs from the device tree (stock
# content, the CAF tree has none for msm8953), Google ones from AOSP.
PRODUCT_COPY_FILES += \
    device/samsung/a02q/configs/media/media_codecs.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml \
    device/samsung/a02q/configs/media/media_codecs_performance.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml \
    device/samsung/a02q/configs/media/media_codecs_c2_sec.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_c2_sec.xml \
    device/samsung/a02q/configs/media/media_codecs_c2_sec_ape.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_c2_sec_ape.xml \
    device/samsung/a02q/configs/media/media_codecs_c2_sec_qcp.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_c2_sec_qcp.xml \
    device/samsung/a02q/configs/media/media_codecs_c2_sec_wmx.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_c2_sec_wmx.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_c2.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_c2.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_c2_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_c2_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_c2_video.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_c2_video.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_telephony.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_telephony.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video_le.xml

# Audio: CAF HAL with its plugins, effects and OMX audio encoders (they share
# the HAL's private structures, so they come from the same source).
PRODUCT_PACKAGES += \
    audio.primary.msm8953 \
    liba2dpoffload \
    libbatterylistener \
    libcirrusspkrprot \
    libcomprcapture \
    libexthwplugin \
    libhdmiedid \
    libhfp \
    libsndmonitor \
    libspkrprot \
    libqcompostprocbundle \
    libqcomvisualizer \
    libqcomvoiceprocessing \
    libOmxAacEnc \
    libOmxAmrEnc \
    libOmxEvrcEnc \
    libOmxG711Enc \
    libOmxQcelp13Enc

# Audio HAL configs (stock content): the mixer paths and platform info this
# unit's card (msm8953-snd-card-mtp, internal codec, GPIO speaker PA) uses.
PRODUCT_COPY_FILES += \
    device/samsung/a02q/configs/audio/mixer_paths_mtp.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths_mtp.xml \
    device/samsung/a02q/configs/audio/audio_platform_info_intcodec.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_platform_info_intcodec.xml \
    device/samsung/a02q/configs/audio/audio_platform_info.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_platform_info.xml \
    device/samsung/a02q/configs/audio/audio_tuning_mixer.txt:$(TARGET_COPY_OUT_VENDOR)/etc/audio_tuning_mixer.txt

# Camera HAL shim (see camera/camera_shim.c).
PRODUCT_PACKAGES += \
    camera.a02q

# Samsung software Codec2: open-source store and service (codec2/) serving the
# stock libcodec2_sec_* codecs. Dolby blobs come from an untracked makefile.
PRODUCT_PACKAGES += \
    libSecC2ComponentStore \
    samsung.software.media.c2@1.0-service
$(call inherit-product-if-exists, vendor/samsung/a02q/dolby.mk)

# Open-source builds of files the stock vendor ships prebuilt (HIDL interface
# libs, soft codecs, effects, generic HAL services). Blobs of the same path are
# dropped from proprietary-files.txt so these win.
PRODUCT_PACKAGES += \
    android.frameworks.cameraservice.common@2.0.vendor \
    android.frameworks.cameraservice.device@2.0.vendor \
    android.frameworks.cameraservice.device@2.1.vendor \
    android.frameworks.cameraservice.service@2.0.vendor \
    android.frameworks.cameraservice.service@2.1.vendor \
    android.frameworks.cameraservice.service@2.2.vendor \
    android.frameworks.displayservice@1.0.vendor \
    android.frameworks.sensorservice@1.0.vendor \
    android.hardware.atrace@1.0.vendor \
    libcld80211 \
    android.hardware.audio.common-util.vendor \
    android.hardware.audio.common@2.0-util.vendor \
    android.hardware.audio.common@4.0-util.vendor \
    android.hardware.audio.common@4.0.vendor \
    android.hardware.audio.common@5.0-util.vendor \
    android.hardware.audio.common@5.0.vendor \
    android.hardware.audio.common@6.0-util.vendor \
    android.hardware.audio.common@6.0.vendor \
    android.hardware.audio.effect@2.0-impl \
    android.hardware.audio.effect@2.0-util.vendor \
    android.hardware.audio.effect@2.0.vendor \
    android.hardware.audio.effect@4.0-impl \
    android.hardware.audio.effect@4.0-util.vendor \
    android.hardware.audio.effect@4.0.vendor \
    android.hardware.audio.effect@5.0-impl \
    android.hardware.audio.effect@5.0-util.vendor \
    android.hardware.audio.effect@5.0.vendor \
    android.hardware.audio.effect@6.0-impl \
    android.hardware.audio.effect@6.0-util.vendor \
    android.hardware.audio.effect@6.0.vendor \
    android.hardware.audio.service \
    android.hardware.audio@2.0-impl \
    android.hardware.audio@2.0-util.vendor \
    android.hardware.audio@2.0.vendor \
    android.hardware.audio@4.0-impl \
    android.hardware.audio@4.0-util.vendor \
    android.hardware.audio@4.0.vendor \
    android.hardware.audio@5.0-impl \
    android.hardware.audio@5.0-util.vendor \
    android.hardware.audio@5.0.vendor \
    android.hardware.audio@6.0-impl \
    android.hardware.audio@6.0-util.vendor \
    android.hardware.audio@6.0.vendor \
    android.hardware.biometrics.face@1.0.vendor \
    android.hardware.biometrics.fingerprint@2.1.vendor \
    android.hardware.bluetooth.audio@2.0-impl \
    android.hardware.bluetooth.audio@2.0.vendor \
    android.hardware.bluetooth.audio@2.1.vendor \
    android.hardware.bluetooth@1.0.vendor \
    android.hardware.camera.common@1.0.vendor \
    android.hardware.camera.device@1.0.vendor \
    android.hardware.camera.device@3.2.vendor \
    android.hardware.camera.device@3.3.vendor \
    android.hardware.camera.device@3.4.vendor \
    android.hardware.camera.device@3.5.vendor \
    android.hardware.camera.device@3.6.vendor \
    android.hardware.camera.provider@2.4-external \
    android.hardware.camera.provider@2.4-impl \
    android.hardware.camera.provider@2.4-legacy \
    android.hardware.camera.provider@2.4-service \
    android.hardware.camera.provider@2.4.vendor \
    android.hardware.cas.native@1.0.vendor \
    android.hardware.cas@1.0.vendor \
    android.hardware.cas@1.1.vendor \
    android.hardware.cas@1.2.vendor \
    android.hardware.configstore@1.1-service \
    android.hardware.drm@1.0.vendor \
    android.hardware.drm@1.1.vendor \
    android.hardware.drm@1.2.vendor \
    android.hardware.drm@1.3.vendor \
    android.hardware.drm@1.4.vendor \
    android.hardware.dumpstate@1.0.vendor \
    android.hardware.dumpstate@1.1.vendor \
    android.hardware.gatekeeper@1.0.vendor \
    android.hardware.gnss.measurement_corrections@1.0.vendor \
    android.hardware.gnss.measurement_corrections@1.1.vendor \
    android.hardware.gnss.visibility_control@1.0.vendor \
    android.hardware.gnss@1.0.vendor \
    android.hardware.gnss@1.1.vendor \
    android.hardware.gnss@2.0.vendor \
    android.hardware.gnss@2.1.vendor \
    android.hardware.graphics.allocator@2.0-impl \
    android.hardware.graphics.allocator@2.0-service \
    android.hardware.graphics.composer@2.1-resources.vendor \
    android.hardware.graphics.composer@2.1-service \
    android.hardware.graphics.composer@2.1.vendor \
    android.hardware.graphics.mapper@2.0-impl-2.1 \
    android.hardware.health@1.0.vendor \
    android.hardware.health@2.0.vendor \
    android.hardware.health@2.1.vendor \
    android.hardware.keymaster@3.0.vendor \
    android.hardware.keymaster@4.0.vendor \
    android.hardware.keymaster@4.1.vendor \
    android.hardware.light@2.0-impl \
    android.hardware.light@2.0.vendor \
    android.hardware.media.c2@1.0.vendor \
    android.hardware.media.c2@1.1.vendor \
    android.hardware.media.c2@1.2.vendor \
    android.hardware.media.omx@1.0-service \
    android.hardware.memtrack@1.0-impl \
    android.hardware.memtrack@1.0-service \
    android.hardware.power-V1-ndk_platform.vendor \
    android.hardware.power-service-qti \
    android.hardware.power@1.0.vendor \
    android.hardware.power@1.1.vendor \
    android.hardware.power@1.2.vendor \
    android.hardware.radio.config@1.0.vendor \
    android.hardware.radio.config@1.1.vendor \
    android.hardware.radio.config@1.2.vendor \
    android.hardware.radio.deprecated@1.0.vendor \
    android.hardware.radio@1.0.vendor \
    android.hardware.radio@1.1.vendor \
    android.hardware.radio@1.2.vendor \
    android.hardware.radio@1.3.vendor \
    android.hardware.radio@1.4.vendor \
    android.hardware.renderscript@1.0-impl \
    android.hardware.sensors@1.0-impl \
    android.hardware.sensors@1.0-service \
    android.hardware.sensors@1.0.vendor \
    android.hardware.soundtrigger@2.0-impl \
    android.hardware.soundtrigger@2.1-impl \
    android.hardware.soundtrigger@2.1.vendor \
    android.hardware.tetheroffload.config@1.0.vendor \
    android.hardware.tetheroffload.control@1.0.vendor \
    android.hardware.thermal@1.0.vendor \
    android.hardware.thermal@2.0.vendor \
    android.hardware.usb@1.0.vendor \
    android.hardware.usb@1.1.vendor \
    android.hardware.usb@1.2.vendor \
    android.hardware.usb@1.3.vendor \
    android.hardware.vibrator-V1-ndk_platform.vendor \
    android.hardware.wifi.hostapd@1.0.vendor \
    android.hardware.wifi.hostapd@1.1.vendor \
    android.hardware.wifi.hostapd@1.2.vendor \
    android.hardware.wifi.hostapd@1.3.vendor \
    android.hardware.wifi.supplicant@1.0.vendor \
    android.hardware.wifi.supplicant@1.1.vendor \
    android.hardware.wifi.supplicant@1.2.vendor \
    android.hardware.wifi.supplicant@1.3.vendor \
    android.hardware.wifi.supplicant@1.4.vendor \
    android.hardware.wifi@1.0.vendor \
    android.hardware.wifi@1.1.vendor \
    android.hardware.wifi@1.2.vendor \
    android.hardware.wifi@1.3.vendor \
    android.hardware.wifi@1.4.vendor \
    android.hardware.wifi@1.5.vendor \
    android.hidl.allocator@1.0.vendor \
    android.system.net.netd@1.0.vendor \
    android.system.net.netd@1.1.vendor \
    android.system.wifi.keystore@1.0.vendor \
    audio.bluetooth.default \
    audio.r_submix.default \
    audio.usb.default \
    camera.device@1.0-impl \
    camera.device@3.2-impl \
    camera.device@3.3-impl \
    camera.device@3.4-external-impl \
    camera.device@3.4-impl \
    camera.device@3.5-external-impl \
    camera.device@3.5-impl \
    camera.device@3.6-external-impl \
    libalsautils \
    libaudiopreprocessing \
    libavservices_minijail.vendor \
    libavservices_minijail_vendor \
    libbinderdebug.vendor \
    libbluetooth_audio_session \
    libbundlewrapper \
    libcamera2ndk_vendor \
    libcodec2_hidl@1.0.vendor \
    libcodec2_hidl@1.1.vendor \
    libcodec2_hidl@1.2.vendor \
    libcodec2_hidl_plugin \
    libcodec2_simple_component.vendor \
    libcodec2_soft_common.vendor \
    libcodec2_vndk.vendor \
    libdownmix \
    libdrm.vendor \
    libdynproc \
    libeffectproxy \
    libeffects \
    libeffectsconfig.vendor \
    libhapticgenerator \
    libhidltransport.vendor \
    libhwbinder.vendor \
    libhwc2on1adapter \
    libhwc2onfbadapter \
    libhwminijail \
    libjson \
    libkeystore-engine-wifi-hidl \
    libkeystore-wifi-hidl \
    libldnhncr \
    libnbaio_mono \
    libnetfilter_conntrack \
    libnfnetlink \
    libopus.vendor \
    libprotobuf-cpp-full.vendor \
    libprotobuf-cpp-lite.vendor \
    libqti_vndfwk_detect.vendor \
    libreverbwrapper \
    librilutils \
    libsfplugin_ccodec_utils.vendor \
    libstagefright_amrnb_common.vendor \
    libstagefright_bufferpool@2.0.1.vendor \
    libstagefright_enc_common.vendor \
    libstagefright_flacdec.vendor \
    libstagefright_soft_aacdec.vendor \
    libstagefright_soft_aacenc.vendor \
    libstagefright_soft_amrdec.vendor \
    libstagefright_soft_amrnbenc.vendor \
    libstagefright_soft_amrwbenc.vendor \
    libstagefright_soft_avcdec.vendor \
    libstagefright_soft_avcenc.vendor \
    libstagefright_soft_flacdec.vendor \
    libstagefright_soft_flacenc.vendor \
    libstagefright_soft_g711dec.vendor \
    libstagefright_soft_gsmdec.vendor \
    libstagefright_soft_hevcdec.vendor \
    libstagefright_soft_mp3dec.vendor \
    libstagefright_soft_mpeg2dec.vendor \
    libstagefright_soft_mpeg4dec.vendor \
    libstagefright_soft_mpeg4enc.vendor \
    libstagefright_soft_opusdec.vendor \
    libstagefright_soft_rawdec.vendor \
    libstagefright_soft_vorbisdec.vendor \
    libstagefright_soft_vpxdec.vendor \
    libstagefright_soft_vpxenc.vendor \
    libstagefright_softomx.vendor \
    libstagefright_softomx_plugin.vendor \
    libtinycompress \
    libvisualizer \
    libvndfwk_detect_jni.qti.vendor \
    libvorbisidec.vendor \
    libvpx.vendor \
    libwifi-hal \
    libz_stable.vendor \
    vendor.display.config@1.0.vendor \
    vendor.display.config@1.1.vendor \
    vendor.display.config@1.10.vendor \
    vendor.display.config@1.11.vendor \
    vendor.display.config@1.2.vendor \
    vendor.display.config@1.3.vendor \
    vendor.display.config@1.4.vendor \
    vendor.display.config@1.5.vendor \
    vendor.display.config@1.6.vendor \
    vendor.display.config@1.7.vendor \
    vendor.display.config@1.8.vendor \
    vendor.display.config@1.9.vendor \
    vendor.display.config@2.0.vendor \
    vendor.qti.hardware.btconfigstore@1.0.vendor \
    vendor.qti.hardware.btconfigstore@2.0.vendor \
    vendor.qti.hardware.camera.device@1.0.vendor \
    vendor.qti.hardware.capabilityconfigstore@1.0.vendor \
    vendor.qti.hardware.cryptfshw@1.0.vendor \
    vendor.qti.hardware.display.allocator@1.0.vendor \
    vendor.qti.hardware.display.allocator@3.0.vendor \
    vendor.qti.hardware.display.mapper@1.0.vendor \
    vendor.qti.hardware.display.mapper@2.0.vendor \
    vendor.qti.hardware.display.mapper@3.0.vendor \
    vendor.qti.hardware.display.mapper@4.0.vendor \
    vendor.qti.hardware.display.mapperextensions@1.0.vendor \
    vendor.qti.hardware.display.mapperextensions@1.1.vendor \
    vendor.qti.hardware.perf@1.0.vendor \
    vendor.qti.hardware.perf@2.0.vendor \
    vendor.qti.hardware.perf@2.1.vendor \
    vendor.qti.hardware.perf@2.2.vendor \
    vendor.qti.hardware.servicetracker@1.0.vendor \
    vendor.qti.hardware.servicetracker@1.1.vendor \
    vendor.qti.hardware.servicetracker@1.2.vendor

PRODUCT_USES_VENDORIMAGE := true

# Dynamic partitions (system/vendor/product/odm in super, launched with them).
PRODUCT_USE_DYNAMIC_PARTITIONS := true
PRODUCT_USE_DYNAMIC_PARTITION_SIZE := true

# fstab: vendor copy for mount_all, ramdisk copy for first-stage mount.
PRODUCT_COPY_FILES += \
    device/samsung/a02q/rootdir/etc/fstab.qcom:$(TARGET_COPY_OUT_VENDOR)/etc/fstab.qcom \
    device/samsung/a02q/rootdir/etc/fstab.qcom:$(TARGET_COPY_OUT_RAMDISK)/fstab.qcom \
    device/samsung/a02q/rootdir/etc/init.a02q.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/init.a02q.rc

# Charging control (Lineage health HAL, see BoardConfig.mk).
PRODUCT_PACKAGES += \
    vendor.lineage.health-service.default

# Lineage HALs from hardware/samsung (copies in fastcharge/ and touch/):
# AFC fast charge toggle and glove mode (high touch sensitivity).
PRODUCT_PACKAGES += \
    vendor.lineage.fastcharge@1.0-service.samsung \
    vendor.lineage.touch@1.0-service.samsung

# Thermal: CAF HAL with an SDM450 table (thermal/); the stock thermal-engine
# keeps doing the throttling.
PRODUCT_PACKAGES += \
    android.hardware.thermal@2.0-service.a02q

# Face unlock: the stock Huaqin/ArcSoft HAL (vendor.samsung...face@2.0-service)
# already serves android.hardware.biometrics.face@1.0.
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.biometrics.face.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.biometrics.face.xml

# Hardware features (AOSP frameworks/native copies of the stock list; no IMS).
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth_le.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.camera.full.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.full.xml \
    frameworks/native/data/etc/android.hardware.camera.raw.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.raw.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.opengles.aep.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.opengles.aep.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepcounter.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.stepcounter.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepdetector.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.stepdetector.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.vulkan.compute-0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.compute-0.xml \
    frameworks/native/data/etc/android.hardware.vulkan.level-0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.level-0.xml \
    frameworks/native/data/etc/android.hardware.vulkan.version-1_1.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.version-1_1.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.wifi.passpoint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.passpoint.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.software.ipsec_tunnels.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.ipsec_tunnels.xml \
    frameworks/native/data/etc/android.software.midi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.midi.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.software.verified_boot.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.verified_boot.xml \
    frameworks/native/data/etc/android.software.opengles.deqp.level-2020-03-01.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.opengles.deqp.level.xml \
    frameworks/native/data/etc/android.software.vulkan.deqp.level-2020-03-01.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.vulkan.deqp.level.xml \
    device/samsung/a02q/configs/permissions/unavailable_features_a02q.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/unavailable_features_a02q.xml

# Audio policy: stock config with the Bluetooth Audio HAL module instead of
# the legacy a2dp one (see configs/audio/audio_policy_configuration.xml).
PRODUCT_COPY_FILES += \
    device/samsung/a02q/configs/audio/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/bluetooth_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/bluetooth_audio_policy_configuration.xml

# Wi-Fi (configs in vendor/etc/wifi stay stock; services in stock init.qcom.rc).
PRODUCT_PACKAGES += \
    hostapd \
    hostapd_cli \
    libwpa_client \
    wpa_cli \
    wpa_supplicant \
    wpa_supplicant.conf

# TODO bring-up (still stock blobs/rc today):
#  - device sepolicy (sepolicy/vendor) once permissive boot logs show denials
#  - trim proprietary-files.txt toward open-source HALs (power, health, lights, usb)
