/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Open-source replacement for the stock samsung.software.media.c2@1.0-service:
 * the AOSP vendor Codec2 service main (frameworks/av/media/codec2/hidl/
 * services/vendor.cpp) serving libSecC2ComponentStore, with the stock
 * seccomp policies.
 */

#define LOG_TAG "samsung.software.media.c2@1.0-service"

#include <android-base/logging.h>
#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>
#include <minijail.h>

#include <signal.h>

extern "C" void RegisterSECCodecServices();

static constexpr char kBaseSeccompPolicyPath[] =
        "/vendor/etc/seccomp_policy/samsung.software.media.c2-base-policy";
static constexpr char kExtSeccompPolicyPath[] =
        "/vendor/etc/seccomp_policy/samsung.software.media.c2-ext-policy";

int main(int /* argc */, char** /* argv */) {
    using namespace ::android;
    LOG(DEBUG) << "samsung.software.media.c2@1.0-service starting...";

    signal(SIGPIPE, SIG_IGN);
    SetUpMinijail(kBaseSeccompPolicyPath, kExtSeccompPolicyPath);

    ProcessState::initWithDriver("/dev/vndbinder");
    ProcessState::self()->startThreadPool();
    hardware::configureRpcThreadpool(8, true /* callerWillJoin */);

    RegisterSECCodecServices();

    hardware::joinRpcThreadpool();
    return 0;
}
