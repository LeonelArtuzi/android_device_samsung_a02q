/*
 * camera.a02q: presents the stock HS50 camera HAL (camera.msm8953.so) to the
 * AOSP camera provider. Selected with ro.hardware.camera=a02q.
 *
 * Two vendor differences are bridged here:
 *  - The HAL was built with a camera_module_t that has one extra field after
 *    get_number_of_cameras, so every later member sits 4 bytes off the AOSP
 *    layout (the AOSP provider's init() would land on set_torch_mode).
 *  - The HAL addresses cameras by Samsung camera IDs (0 rear, 1 front, and
 *    20/21/51/52 for the extra rear sensors), translated internally by
 *    QCamera2Factory::mapCameraIDForHS50 from the sensor position bits that
 *    libmmcamera_interface reports. The AOSP framework asks for 0..N-1, so the
 *    shim exposes contiguous IDs and translates them both ways. The extra rear
 *    sensors are only probed with persist.vendor.camera.expose.aux=1.
 *
 * Torch intensity: the flash driver's rear_torch_flash node takes 1001-1009
 * and writes the PMI632 torch current register in 5 steps; the stock HAL
 * lights the torch at step 4. The level picked in Settings (A02qParts) is
 * applied after every enable, since lighting the LED resets the register.
 */

#define LOG_TAG "camera.a02q"

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cutils/properties.h>
#include <hardware/camera_common.h>
#include <log/log.h>

#define VENDOR_HAL "/vendor/lib/hw/camera.msm8953.so"
#define VENDOR_MM_INTERFACE "libmmcamera_interface.so"
#define MAX_CAMERAS 8

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
static uint32_t (*vendor_get_camera_position)(void);

/*
 * Sensor position bit -> Samsung camera ID, as handled by the vendor's
 * QCamera2Factory::mapCameraIDForHS50 (decompiled from the stock HAL).
 */
static const struct {
    int bit;
    int samsung_id;
} kPositionIds[] = {
        {0, 0},    /* rear main */
        {2, 1},    /* front */
        {4, 0x14}, /* 20 */
        {6, 0x15}, /* 21 */
        {8, 0x34}, /* 52 */
        {9, 0x33}, /* 51 */
};

static pthread_once_t table_once = PTHREAD_ONCE_INIT;
static int num_cameras;
static int samsung_ids[MAX_CAMERAS]; /* AOSP id -> Samsung id */

static void build_table(void) {
    /* Constructs QCamera2Factory, which probes the sensors and sets the position bits. */
    int vendor_count = vendor->get_number_of_cameras();
    uint32_t position = vendor_get_camera_position ? vendor_get_camera_position() : 0;

    for (size_t i = 0; i < sizeof(kPositionIds) / sizeof(kPositionIds[0]); i++) {
        if (!(position & (1u << kPositionIds[i].bit)) || num_cameras == MAX_CAMERAS) continue;
        samsung_ids[num_cameras] = kPositionIds[i].samsung_id;
        ALOGI("camera %d -> vendor id %d (position bit %d)", num_cameras,
              kPositionIds[i].samsung_id, kPositionIds[i].bit);
        num_cameras++;
    }
    if (num_cameras == 0) {
        /* No position info: fall back to the vendor's own contiguous IDs. */
        ALOGW("no camera position bits (%#x), exposing %d vendor ids as-is", position,
              vendor_count);
        for (; num_cameras < vendor_count && num_cameras < MAX_CAMERAS; num_cameras++)
            samsung_ids[num_cameras] = num_cameras;
    }
    ALOGI("position bits %#x, vendor count %d, exposing %d cameras", position, vendor_count,
          num_cameras);
}

static int to_vendor_id(int id) {
    pthread_once(&table_once, build_table);
    return (id >= 0 && id < num_cameras) ? samsung_ids[id] : -1;
}

static int from_vendor_id(int vendor_id) {
    pthread_once(&table_once, build_table);
    for (int i = 0; i < num_cameras; i++)
        if (samsung_ids[i] == vendor_id) return i;
    return vendor_id;
}

/* String IDs ("0", "1", ...) are used by open, open_legacy and the torch calls. */
static const char* to_vendor_id_str(const char* id, char* buf, size_t len) {
    char* end;
    long v = strtol(id, &end, 10);
    if (*id == '\0' || *end != '\0') return NULL;
    int vendor_id = to_vendor_id((int)v);
    if (vendor_id < 0) return NULL;
    snprintf(buf, len, "%d", vendor_id);
    return buf;
}

static const camera_module_callbacks_t* framework_callbacks;

static void shim_camera_device_status_change(const struct camera_module_callbacks* cb __unused,
                                             int camera_id, int new_status) {
    framework_callbacks->camera_device_status_change(framework_callbacks,
                                                     from_vendor_id(camera_id), new_status);
}

static void shim_torch_mode_status_change(const struct camera_module_callbacks* cb __unused,
                                          const char* camera_id, int new_status) {
    char buf[16];
    char* end;
    long v = strtol(camera_id, &end, 10);
    if (*camera_id != '\0' && *end == '\0') {
        snprintf(buf, sizeof(buf), "%d", from_vendor_id((int)v));
        camera_id = buf;
    }
    framework_callbacks->torch_mode_status_change(framework_callbacks, camera_id, new_status);
}

static const camera_module_callbacks_t shim_callbacks = {
        .camera_device_status_change = shim_camera_device_status_change,
        .torch_mode_status_change = shim_torch_mode_status_change,
};

static int shim_get_number_of_cameras(void) {
    pthread_once(&table_once, build_table);
    return num_cameras;
}

static int shim_get_camera_info(int camera_id, struct camera_info* info) {
    int vendor_id = to_vendor_id(camera_id);
    if (vendor_id < 0) return -EINVAL;
    return vendor->get_camera_info(vendor_id, info);
}

static int shim_set_callbacks(const camera_module_callbacks_t* callbacks) {
    framework_callbacks = callbacks;
    return vendor->set_callbacks(callbacks ? &shim_callbacks : NULL);
}

static void shim_get_vendor_tag_ops(vendor_tag_ops_t* ops) {
    vendor->get_vendor_tag_ops(ops);
}

static int shim_open_legacy(const struct hw_module_t* module __unused, const char* id,
                            uint32_t halVersion, struct hw_device_t** device) {
    char buf[16];
    const char* vendor_id = to_vendor_id_str(id, buf, sizeof(buf));
    if (!vendor_id) return -EINVAL;
    return vendor->open_legacy(&vendor->common, vendor_id, halVersion, device);
}

#define TORCH_LEVEL_PROP "persist.sys.a02q.torch_level"
#define TORCH_LEVEL_NODE "/sys/class/camera/flash/rear_torch_flash"

/* Only right after the HAL lit the torch: with the register at 0 the node
 * turns the torch on by itself. Unset level = stock current. */
static void apply_torch_level(void) {
    static const char* const codes[] = {"1001", "1002", "1003", "1005", "1007"};
    int level = property_get_int32(TORCH_LEVEL_PROP, 0);
    if (level < 1 || level > 5) return;
    int fd = open(TORCH_LEVEL_NODE, O_WRONLY | O_CLOEXEC);
    if (fd < 0 || write(fd, codes[level - 1], strlen(codes[level - 1])) < 0)
        ALOGE("torch level %d: %s: %s", level, TORCH_LEVEL_NODE, strerror(errno));
    if (fd >= 0) close(fd);
}

static int shim_set_torch_mode(const char* camera_id, bool enabled) {
    char buf[16];
    const char* vendor_id = to_vendor_id_str(camera_id, buf, sizeof(buf));
    if (!vendor_id) return -EINVAL;
    int ret = vendor->set_torch_mode(vendor_id, enabled);
    if (ret == 0 && enabled && strcmp(vendor_id, "0") == 0) apply_torch_level();
    return ret;
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
    int vendor_id = to_vendor_id(camera_id);
    if (vendor_id < 0) return -EINVAL;
    return vendor->is_stream_combination_supported(vendor_id, streams);
}

static void shim_notify_device_state_change(uint64_t deviceState) {
    vendor->notify_device_state_change(deviceState);
}

static int shim_open(const struct hw_module_t* module __unused, const char* id,
                     struct hw_device_t** device) {
    char buf[16];
    if (!vendor) return -ENODEV;
    const char* vendor_id = to_vendor_id_str(id, buf, sizeof(buf));
    if (!vendor_id) return -EINVAL;
    return vendor->common.methods->open(&vendor->common, vendor_id, device);
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
    /* Already loaded by the vendor HAL; RTLD_NOLOAD just takes a handle to it. */
    void* mm = dlopen(VENDOR_MM_INTERFACE, RTLD_NOW | RTLD_NOLOAD);
    if (mm) vendor_get_camera_position = (uint32_t(*)(void))dlsym(mm, "get_camera_position");
    if (!vendor_get_camera_position) ALOGW("get_camera_position not found: %s", dlerror());

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
