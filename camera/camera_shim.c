/*
 * camera.a02q: presents the stock HS50 camera HAL (camera.msm8953.so) to the
 * AOSP camera provider. The vendor built it with a camera_module_t that has
 * one extra field after get_number_of_cameras, so every later member sits 4
 * bytes off the AOSP layout (the AOSP provider's init() lands on
 * set_torch_mode). Selected with ro.hardware.camera=a02q.
 */

#define LOG_TAG "camera.a02q"

#include <dlfcn.h>
#include <errno.h>
#include <stddef.h>

#include <hardware/camera_common.h>
#include <log/log.h>

#define VENDOR_HAL "/vendor/lib/hw/camera.msm8953.so"

/* Layout the vendor HAL was compiled with (offsets from the stock provider). */
typedef struct {
    hw_module_t common;
    int (*get_number_of_cameras)(void);
    void* vendor_extension; /* unknown, not used by AOSP */
    int (*get_camera_info)(int camera_id, struct camera_info* info);
    int (*set_callbacks)(const camera_module_callbacks_t* callbacks);
    void (*get_vendor_tag_ops)(vendor_tag_ops_t* ops);
    int (*open_legacy)(const struct hw_module_t* module, const char* id, uint32_t halVersion,
                       struct hw_device_t** device);
    int (*set_torch_mode)(const char* camera_id, bool enabled);
    int (*init)();
    int (*get_physical_camera_info)(int physical_camera_id, camera_metadata_t** static_metadata);
    int (*is_stream_combination_supported)(int camera_id,
                                           const camera_stream_combination_t* streams);
    void (*notify_device_state_change)(uint64_t deviceState);
    void* reserved[2];
} vendor_camera_module_t;

_Static_assert(sizeof(void*) != 4 || offsetof(vendor_camera_module_t, get_camera_info) == 136,
               "get_camera_info must be at +136 like the stock provider expects");
_Static_assert(sizeof(void*) != 4 || offsetof(vendor_camera_module_t, init) == 156,
               "init must be at +156 like the stock provider expects");

static vendor_camera_module_t* vendor;

static int shim_get_number_of_cameras(void) {
    return vendor->get_number_of_cameras();
}

static int shim_get_camera_info(int camera_id, struct camera_info* info) {
    return vendor->get_camera_info(camera_id, info);
}

static int shim_set_callbacks(const camera_module_callbacks_t* callbacks) {
    return vendor->set_callbacks(callbacks);
}

static void shim_get_vendor_tag_ops(vendor_tag_ops_t* ops) {
    vendor->get_vendor_tag_ops(ops);
}

static int shim_open_legacy(const struct hw_module_t* module __unused, const char* id,
                            uint32_t halVersion, struct hw_device_t** device) {
    return vendor->open_legacy(&vendor->common, id, halVersion, device);
}

static int shim_set_torch_mode(const char* camera_id, bool enabled) {
    return vendor->set_torch_mode(camera_id, enabled);
}

static int shim_init(void) {
    return vendor->init();
}

static int shim_get_physical_camera_info(int physical_camera_id,
                                         camera_metadata_t** static_metadata) {
    return vendor->get_physical_camera_info(physical_camera_id, static_metadata);
}

static int shim_is_stream_combination_supported(int camera_id,
                                                const camera_stream_combination_t* streams) {
    return vendor->is_stream_combination_supported(camera_id, streams);
}

static void shim_notify_device_state_change(uint64_t deviceState) {
    vendor->notify_device_state_change(deviceState);
}

static int shim_open(const struct hw_module_t* module __unused, const char* id,
                     struct hw_device_t** device) {
    if (!vendor) return -ENODEV;
    return vendor->common.methods->open(&vendor->common, id, device);
}

static struct hw_module_methods_t shim_methods = {
        .open = shim_open,
};

camera_module_t HAL_MODULE_INFO_SYM = {
        .common =
                {
                        .tag = HARDWARE_MODULE_TAG,
                        .id = CAMERA_HARDWARE_MODULE_ID,
                        .name = "a02q camera HAL shim",
                        .author = "LineageOS",
                        .methods = &shim_methods,
                },
};

/* libhardware reads HAL_MODULE_INFO_SYM right after dlopen, so fill it here. */
__attribute__((constructor)) static void shim_load(void) {
    void* handle = dlopen(VENDOR_HAL, RTLD_NOW);
    if (!handle) {
        ALOGE("dlopen %s: %s", VENDOR_HAL, dlerror());
        return;
    }
    vendor = (vendor_camera_module_t*)dlsym(handle, HAL_MODULE_INFO_SYM_AS_STR);
    if (!vendor) {
        ALOGE("dlsym %s: %s", HAL_MODULE_INFO_SYM_AS_STR, dlerror());
        return;
    }

    camera_module_t* m = &HAL_MODULE_INFO_SYM;
    m->common.module_api_version = vendor->common.module_api_version;
    m->common.hal_api_version = vendor->common.hal_api_version;
    /* Only expose what the vendor HAL implements; the provider checks for NULL. */
#define FWD(f) m->f = vendor->f ? shim_##f : NULL
    FWD(get_number_of_cameras);
    FWD(get_camera_info);
    FWD(set_callbacks);
    FWD(get_vendor_tag_ops);
    FWD(open_legacy);
    FWD(set_torch_mode);
    FWD(init);
    FWD(get_physical_camera_info);
    FWD(is_stream_combination_supported);
    FWD(notify_device_state_change);
#undef FWD
    ALOGI("loaded %s (module api %#x)", VENDOR_HAL, m->common.module_api_version);
}
