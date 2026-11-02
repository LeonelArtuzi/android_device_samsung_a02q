/*
 * a02q extension of the QTI power HAL (MODE_EXT hook in Power.cpp).
 *
 * Battery saver (Mode::LOW_POWER) is a no-op upstream. Here it caps the CPU
 * through the stock perf HAL, the same arbiter the launch/interaction boosts
 * use, so boosts and the cap never fight over scaling_max_freq directly.
 *
 * Double tap to wake (Mode::DOUBLE_TAP_TO_WAKE) goes through the Samsung
 * sec_cmd interface of the ILITEK touch driver: "aot_enable,1" keeps the panel
 * in gesture mode while suspended and a double tap reports KEY_WAKEUP.
 */

#define LOG_TAG "a02q-power"

#include <iterator>
#include <mutex>

#include <aidl/android/hardware/power/BnPower.h>
#include <android-base/file.h>
#include <log/log.h>

extern "C" {
#include "hint-data.h"
#include "performance.h"
#include "utils.h"
}

using ::aidl::android::hardware::power::Mode;

namespace {

constexpr char kTspCmd[] = "/sys/class/sec/tsp/cmd";

constexpr int kLowPowerHintId = 0x1100;  // after VR_MODE_SUSTAINED_PERF_HINT_ID

// msm8953 is a single cluster (cpu0-7 share one policy): 652/1036/1401/1689/1804 MHz.
// Cap at 1401 MHz, dropping the two top steps. The perf HAL takes MHz and
// addresses this cluster as both BIG and LITTLE (see stock perfboostsconfig.xml).
constexpr int kLowPowerMaxFreqMhz = 1401;

std::mutex gLock;
bool gLowPower = false;

void setLowPower(bool enabled) {
    std::lock_guard<std::mutex> lock(gLock);
    if (enabled == gLowPower) return;

    if (enabled) {
        int resources[] = {MAX_FREQ_BIG_CORE_0, kLowPowerMaxFreqMhz,
                           MAX_FREQ_LITTLE_CORE_0, kLowPowerMaxFreqMhz};
        if (perform_hint_action(kLowPowerHintId, resources, std::size(resources)) < 0) {
            ALOGE("low power: failed to cap CPU at %d MHz", kLowPowerMaxFreqMhz);
            return;
        }
        ALOGI("low power: CPU capped at %d MHz", kLowPowerMaxFreqMhz);
    } else {
        undo_hint_action(kLowPowerHintId);
        ALOGI("low power: CPU cap released");
    }
    gLowPower = enabled;
}

}  // namespace

namespace aidl {
namespace android {
namespace hardware {
namespace power {
namespace impl {

bool isDeviceSpecificModeSupported(Mode type, bool* _aidl_return) {
    if (type != Mode::LOW_POWER && type != Mode::DOUBLE_TAP_TO_WAKE) return false;
    *_aidl_return = true;
    return true;
}

bool setDeviceSpecificMode(Mode type, bool enabled) {
    switch (type) {
        case Mode::LOW_POWER:
            setLowPower(enabled);
            return true;
        case Mode::DOUBLE_TAP_TO_WAKE:
            if (!::android::base::WriteStringToFile(enabled ? "aot_enable,1" : "aot_enable,0",
                                                     kTspCmd)) {
                ALOGE("double tap to wake: failed to write %s", kTspCmd);
            }
            return true;
        default:
            return false;
    }
}

}  // namespace impl
}  // namespace power
}  // namespace hardware
}  // namespace android
}  // namespace aidl
