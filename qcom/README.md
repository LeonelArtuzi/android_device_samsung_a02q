# Copies of Lineage/CAF sources used by a02q

Lineage's own repos are never modified. What needs a change is copied here
and the build is pointed at the copy through Lineage's device hooks
(BoardConfig.mk: QCOM_SOONG_NAMESPACE, USE_DEVICE_SPECIFIC_{DISPLAY,MEDIA,AUDIO}
+ DEVICE_SPECIFIC_*_PATH). The display namespaces Lineage exports
(vendor/qcom/opensource/{commonsys-intf/display,display}) are used as-is.

| Copy | Source (lineage-19.1 manifest revision) |
|---|---|
| caf-msm8953/ | hardware/qcom-caf/msm8953: display f4e9a62, media da80d54, audio d7e2d8e; root Android.bp/mk from hardware/qcom-caf/common (symlinks there). audio/configs not copied (no makefiles, sample configs). |
| caf-msm8953/display/libqdmetadata-a02q/ | vendor/qcom/opensource/display/libqdmetadata 69f7b05 (qdMetaData.cpp, qd_utils.cpp unchanged) |

Changes (marked "a02q:" in the files):
- caf-msm8953/display/Android.bp: display_headers exports
  -DGRALLOC_HANDLE_HAS_RESERVED_SIZE (stock gralloc ABI: private_handle_t with
  reserved_size, numInts 23, checked by the blobs' inlined validate()).
- caf-msm8953/display/libqdmetadata-a02q/Android.bp: libqdMetaData_a02q,
  stem libqdMetaData, installed to /odm/lib (searched before /vendor/lib),
  same define; Lineage's module stays in /vendor/lib unused.
- caf-msm8953/display/libqservice/Android.bp: -DDO_NOT_CHECK_MANUAL_BINDER_INTERFACES,
  which soong only adds automatically for modules under vendor/ and hardware/.
- caf-msm8953/audio/hal/audio_extn/utils.c: (void)snd_device, -Werror build fix
  with sound trigger/listen/device arbitration disabled.
