/*
 * Copyright (C) 2017 The Android Open Source Project
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
 * libSecC2ComponentStore for a02q: an open-source replacement for the stock
 * Samsung store, which was built against Android 12.0 and needs two
 * C2PlatformStorePluginLoader symbols whose signatures changed in 12L.
 *
 * The component store below is AOSP 12L SecC2ComponentStore
 * (frameworks/av/media/codec2/vndk/C2Store.cpp) with Samsung's codec list.
 * Behaviour matched from the stock library: only libraries that dlopen are
 * listed, GetCodec2SecComponentStore() is a singleton that also becomes the
 * preferred store, and RegisterSECCodecServices() registers it as the
 * "default" IComponentStore@1.0. The stock codecs link against these two
 * entry points.
 */

#define LOG_TAG "SecC2ComponentStore"
#include <utils/Log.h>

#include <C2AllocatorIon.h>
#include <C2Component.h>
#include <C2Config.h>
#include <C2DmaBufAllocator.h>
#include <C2PlatformSupport.h>
#include <codec2/hidl/1.0/ComponentStore.h>
#include <util/C2InterfaceHelper.h>

#include <dlfcn.h>

#include <map>
#include <memory>
#include <mutex>

namespace android {

namespace {

class SecC2ComponentStore : public C2ComponentStore {
public:
    virtual std::vector<std::shared_ptr<const C2Component::Traits>> listComponents() override;
    virtual std::shared_ptr<C2ParamReflector> getParamReflector() const override;
    virtual C2String getName() const override;
    virtual c2_status_t querySupportedValues_sm(
            std::vector<C2FieldSupportedValuesQuery> &fields) const override;
    virtual c2_status_t querySupportedParams_nb(
            std::vector<std::shared_ptr<C2ParamDescriptor>> *const params) const override;
    virtual c2_status_t query_sm(
            const std::vector<C2Param*> &stackParams,
            const std::vector<C2Param::Index> &heapParamIndices,
            std::vector<std::unique_ptr<C2Param>> *const heapParams) const override;
    virtual c2_status_t createInterface(
            C2String name, std::shared_ptr<C2ComponentInterface> *const interface) override;
    virtual c2_status_t createComponent(
            C2String name, std::shared_ptr<C2Component> *const component) override;
    virtual c2_status_t copyBuffer(
            std::shared_ptr<C2GraphicBuffer> src, std::shared_ptr<C2GraphicBuffer> dst) override;
    virtual c2_status_t config_sm(
            const std::vector<C2Param*> &params,
            std::vector<std::unique_ptr<C2SettingResult>> *const failures) override;
    SecC2ComponentStore();

    // For testing only
    SecC2ComponentStore(
            std::vector<std::tuple<C2String,
                                   C2ComponentFactory::CreateCodec2FactoryFunc,
                                   C2ComponentFactory::DestroyCodec2FactoryFunc>>);

    virtual ~SecC2ComponentStore() override = default;

private:

    /**
     * An object encapsulating a loaded component module.
     *
     * \todo provide a way to add traits to known components here to avoid loading the .so-s
     * for listComponents
     */
    struct ComponentModule : public C2ComponentFactory,
            public std::enable_shared_from_this<ComponentModule> {
        virtual c2_status_t createComponent(
                c2_node_id_t id, std::shared_ptr<C2Component> *component,
                ComponentDeleter deleter = std::default_delete<C2Component>()) override;
        virtual c2_status_t createInterface(
                c2_node_id_t id, std::shared_ptr<C2ComponentInterface> *interface,
                InterfaceDeleter deleter = std::default_delete<C2ComponentInterface>()) override;

        /**
         * \returns the traits of the component in this module.
         */
        std::shared_ptr<const C2Component::Traits> getTraits();

        /**
         * Creates an uninitialized component module.
         *
         * \param name[in]  component name.
         *
         * \note Only used by ComponentLoader.
         */
        ComponentModule()
            : mInit(C2_NO_INIT),
              mLibHandle(nullptr),
              createFactory(nullptr),
              destroyFactory(nullptr),
              mComponentFactory(nullptr) {
        }

        /**
         * Creates an uninitialized component module.
         * NOTE: For testing only
         *
         * \param name[in]  component name.
         *
         * \note Only used by ComponentLoader.
         */
        ComponentModule(
                C2ComponentFactory::CreateCodec2FactoryFunc createFactory,
                C2ComponentFactory::DestroyCodec2FactoryFunc destroyFactory)
            : mInit(C2_NO_INIT),
              mLibHandle(nullptr),
              createFactory(createFactory),
              destroyFactory(destroyFactory),
              mComponentFactory(nullptr) {
        }

        /**
         * Initializes a component module with a given library path. Must be called exactly once.
         *
         * \note Only used by ComponentLoader.
         *
         * \param libPath[in] library path
         *
         * \retval C2_OK        the component module has been successfully loaded
         * \retval C2_NO_MEMORY not enough memory to loading the component module
         * \retval C2_NOT_FOUND could not locate the component module
         * \retval C2_CORRUPTED the component module could not be loaded (unexpected)
         * \retval C2_REFUSED   permission denied to load the component module (unexpected)
         * \retval C2_TIMED_OUT could not load the module within the time limit (unexpected)
         */
        c2_status_t init(std::string libPath);

        virtual ~ComponentModule() override;

    protected:
        std::recursive_mutex mLock; ///< lock protecting mTraits
        std::shared_ptr<C2Component::Traits> mTraits; ///< cached component traits

        c2_status_t mInit; ///< initialization result

        void *mLibHandle; ///< loaded library handle
        C2ComponentFactory::CreateCodec2FactoryFunc createFactory; ///< loaded create function
        C2ComponentFactory::DestroyCodec2FactoryFunc destroyFactory; ///< loaded destroy function
        C2ComponentFactory *mComponentFactory; ///< loaded/created component factory
    };

    /**
     * An object encapsulating a loadable component module.
     *
     * \todo make this also work for enumerations
     */
    struct ComponentLoader {
        /**
         * Load the component module.
         *
         * This method simply returns the component module if it is already currently loaded, or
         * attempts to load it if it is not.
         *
         * \param module[out] pointer to the shared pointer where the loaded module shall be stored.
         *                    This will be nullptr on error.
         *
         * \retval C2_OK        the component module has been successfully loaded
         * \retval C2_NO_MEMORY not enough memory to loading the component module
         * \retval C2_NOT_FOUND could not locate the component module
         * \retval C2_CORRUPTED the component module could not be loaded
         * \retval C2_REFUSED   permission denied to load the component module
         */
        c2_status_t fetchModule(std::shared_ptr<ComponentModule> *module) {
            c2_status_t res = C2_OK;
            std::lock_guard<std::mutex> lock(mMutex);
            std::shared_ptr<ComponentModule> localModule = mModule.lock();
            if (localModule == nullptr) {
                if(mCreateFactory) {
                    // For testing only
                    localModule = std::make_shared<ComponentModule>(mCreateFactory,
                                                                    mDestroyFactory);
                } else {
                    localModule = std::make_shared<ComponentModule>();
                }
                res = localModule->init(mLibPath);
                if (res == C2_OK) {
                    mModule = localModule;
                }
            }
            *module = localModule;
            return res;
        }

        /**
         * Creates a component loader for a specific library path (or name).
         */
        ComponentLoader(std::string libPath)
            : mLibPath(libPath) {}

        // For testing only
        ComponentLoader(std::tuple<C2String,
                          C2ComponentFactory::CreateCodec2FactoryFunc,
                          C2ComponentFactory::DestroyCodec2FactoryFunc> func)
            : mLibPath(std::get<0>(func)),
              mCreateFactory(std::get<1>(func)),
              mDestroyFactory(std::get<2>(func)) {}

    private:
        std::mutex mMutex; ///< mutex guarding the module
        std::weak_ptr<ComponentModule> mModule; ///< weak reference to the loaded module
        std::string mLibPath; ///< library path

        // For testing only
        C2ComponentFactory::CreateCodec2FactoryFunc mCreateFactory = nullptr;
        C2ComponentFactory::DestroyCodec2FactoryFunc mDestroyFactory = nullptr;
    };

    struct Interface : public C2InterfaceHelper {
        std::shared_ptr<C2StoreIonUsageInfo> mIonUsageInfo;
        std::shared_ptr<C2StoreDmaBufUsageInfo> mDmaBufUsageInfo;

        Interface(std::shared_ptr<C2ReflectorHelper> reflector)
            : C2InterfaceHelper(reflector) {
            setDerivedInstance(this);

            struct Setter {
                static C2R setIonUsage(bool /* mayBlock */, C2P<C2StoreIonUsageInfo> &me) {
#ifdef __ANDROID_APEX__
                    static int32_t defaultHeapMask = [] {
                        int32_t heapmask = base::GetIntProperty(
                                "ro.com.android.media.swcodec.ion.heapmask", int32_t(0xFFFFFFFF));
                        ALOGD("Default ION heapmask = %d", heapmask);
                        return heapmask;
                    }();
                    static int32_t defaultFlags = [] {
                        int32_t flags = base::GetIntProperty(
                                "ro.com.android.media.swcodec.ion.flags", 0);
                        ALOGD("Default ION flags = %d", flags);
                        return flags;
                    }();
                    static uint32_t defaultAlign = [] {
                        uint32_t align = base::GetUintProperty(
                                "ro.com.android.media.swcodec.ion.align", 0u);
                        ALOGD("Default ION align = %d", align);
                        return align;
                    }();
                    me.set().heapMask = defaultHeapMask;
                    me.set().allocFlags = defaultFlags;
                    me.set().minAlignment = defaultAlign;
#else
                    me.set().heapMask = ~0;
                    me.set().allocFlags = 0;
                    me.set().minAlignment = 0;
#endif
                    return C2R::Ok();
                };

                static C2R setDmaBufUsage(bool /* mayBlock */, C2P<C2StoreDmaBufUsageInfo> &me) {
                    long long usage = (long long)me.get().m.usage;
                    if (C2DmaBufAllocator::system_uncached_supported() &&
                        !(usage & (C2MemoryUsage::CPU_READ | C2MemoryUsage::CPU_WRITE))) {
                        strncpy(me.set().m.heapName, "system-uncached", me.v.flexCount());
                    } else {
                        strncpy(me.set().m.heapName, "system", me.v.flexCount());
                    }
                    me.set().m.allocFlags = 0;
                    return C2R::Ok();
                };
            };

            addParameter(
                DefineParam(mIonUsageInfo, "ion-usage")
                .withDefault(new C2StoreIonUsageInfo())
                .withFields({
                    C2F(mIonUsageInfo, usage).flags({C2MemoryUsage::CPU_READ | C2MemoryUsage::CPU_WRITE}),
                    C2F(mIonUsageInfo, capacity).inRange(0, UINT32_MAX, 1024),
                    C2F(mIonUsageInfo, heapMask).any(),
                    C2F(mIonUsageInfo, allocFlags).flags({}),
                    C2F(mIonUsageInfo, minAlignment).equalTo(0)
                })
                .withSetter(Setter::setIonUsage)
                .build());

            addParameter(
                DefineParam(mDmaBufUsageInfo, "dmabuf-usage")
                .withDefault(C2StoreDmaBufUsageInfo::AllocShared(0))
                .withFields({
                    C2F(mDmaBufUsageInfo, m.usage).flags({C2MemoryUsage::CPU_READ | C2MemoryUsage::CPU_WRITE}),
                    C2F(mDmaBufUsageInfo, m.capacity).inRange(0, UINT32_MAX, 1024),
                    C2F(mDmaBufUsageInfo, m.allocFlags).flags({}),
                    C2F(mDmaBufUsageInfo, m.heapName).any(),
                })
                .withSetter(Setter::setDmaBufUsage)
                .build());
        }
    };

    /**
     * Retrieves the component module for a component.
     *
     * \param module pointer to a shared_pointer where the component module will be stored on
     *               success.
     *
     * \retval C2_OK        the component loader has been successfully retrieved
     * \retval C2_NO_MEMORY not enough memory to locate the component loader
     * \retval C2_NOT_FOUND could not locate the component to be loaded
     * \retval C2_CORRUPTED the component loader could not be identified due to some modules being
     *                      corrupted (this can happen if the name does not refer to an already
     *                      identified component but some components could not be loaded due to
     *                      bad library)
     * \retval C2_REFUSED   permission denied to find the component loader for the named component
     *                      (this can happen if the name does not refer to an already identified
     *                      component but some components could not be loaded due to lack of
     *                      permissions)
     */
    c2_status_t findComponent(C2String name, std::shared_ptr<ComponentModule> *module);

    /**
     * Loads each component module and discover its contents.
     */
    void visitComponents();

    std::mutex mMutex; ///< mutex guarding the component lists during construction
    bool mVisited; ///< component modules visited
    std::map<C2String, ComponentLoader> mComponents; ///< path -> component module
    std::map<C2String, C2String> mComponentNameToPath; ///< name -> path
    std::vector<std::shared_ptr<const C2Component::Traits>> mComponentList;

    std::shared_ptr<C2ReflectorHelper> mReflector;
    Interface mInterface;

    // For testing only
    std::vector<std::tuple<C2String,
                          C2ComponentFactory::CreateCodec2FactoryFunc,
                          C2ComponentFactory::DestroyCodec2FactoryFunc>> mCodec2FactoryFuncs;
};

c2_status_t SecC2ComponentStore::ComponentModule::init(
        std::string libPath) {
    ALOGV("in %s", __func__);
    ALOGV("loading dll");

    if(!createFactory) {
        mLibHandle = dlopen(libPath.c_str(), RTLD_NOW|RTLD_NODELETE);
        LOG_ALWAYS_FATAL_IF(mLibHandle == nullptr,
                "could not dlopen %s: %s", libPath.c_str(), dlerror());

        createFactory =
            (C2ComponentFactory::CreateCodec2FactoryFunc)dlsym(mLibHandle, "CreateCodec2Factory");
        LOG_ALWAYS_FATAL_IF(createFactory == nullptr,
                "createFactory is null in %s", libPath.c_str());

        destroyFactory =
            (C2ComponentFactory::DestroyCodec2FactoryFunc)dlsym(mLibHandle, "DestroyCodec2Factory");
        LOG_ALWAYS_FATAL_IF(destroyFactory == nullptr,
                "destroyFactory is null in %s", libPath.c_str());
    }

    mComponentFactory = createFactory();
    if (mComponentFactory == nullptr) {
        ALOGD("could not create factory in %s", libPath.c_str());
        mInit = C2_NO_MEMORY;
    } else {
        mInit = C2_OK;
    }

    if (mInit != C2_OK) {
        return mInit;
    }

    std::shared_ptr<C2ComponentInterface> intf;
    c2_status_t res = createInterface(0, &intf);
    if (res != C2_OK) {
        ALOGD("failed to create interface: %d", res);
        return mInit;
    }

    std::shared_ptr<C2Component::Traits> traits(new (std::nothrow) C2Component::Traits);
    if (traits) {
        if (!C2InterfaceUtils::FillTraitsFromInterface(traits.get(), intf)) {
            ALOGD("Failed to fill traits from interface");
            return mInit;
        }

        // TODO: get this properly from the store during emplace
        switch (traits->domain) {
        case C2Component::DOMAIN_AUDIO:
            traits->rank = 8;
            break;
        default:
            traits->rank = 512;
        }
    }
    mTraits = traits;

    return mInit;
}

SecC2ComponentStore::ComponentModule::~ComponentModule() {
    ALOGV("in %s", __func__);
    if (destroyFactory && mComponentFactory) {
        destroyFactory(mComponentFactory);
    }
    if (mLibHandle) {
        ALOGV("unloading dll");
        dlclose(mLibHandle);
    }
}

c2_status_t SecC2ComponentStore::ComponentModule::createInterface(
        c2_node_id_t id, std::shared_ptr<C2ComponentInterface> *interface,
        std::function<void(::C2ComponentInterface*)> deleter) {
    interface->reset();
    if (mInit != C2_OK) {
        return mInit;
    }
    std::shared_ptr<ComponentModule> module = shared_from_this();
    c2_status_t res = mComponentFactory->createInterface(
            id, interface, [module, deleter](C2ComponentInterface *p) mutable {
                // capture module so that we ensure we still have it while deleting interface
                deleter(p); // delete interface first
                module.reset(); // remove module ref (not technically needed)
    });
    return res;
}

c2_status_t SecC2ComponentStore::ComponentModule::createComponent(
        c2_node_id_t id, std::shared_ptr<C2Component> *component,
        std::function<void(::C2Component*)> deleter) {
    component->reset();
    if (mInit != C2_OK) {
        return mInit;
    }
    std::shared_ptr<ComponentModule> module = shared_from_this();
    c2_status_t res = mComponentFactory->createComponent(
            id, component, [module, deleter](C2Component *p) mutable {
                // capture module so that we ensure we still have it while deleting component
                deleter(p); // delete component first
                module.reset(); // remove module ref (not technically needed)
    });
    return res;
}

std::shared_ptr<const C2Component::Traits> SecC2ComponentStore::ComponentModule::getTraits() {
    std::unique_lock<std::recursive_mutex> lock(mLock);
    return mTraits;
}

SecC2ComponentStore::SecC2ComponentStore()
    : mVisited(false),
      mReflector(std::make_shared<C2ReflectorHelper>()),
      mInterface(mReflector) {

    // Like the stock store, only list codec libraries that are present.
    auto emplace = [this](const char *libPath) {
        void *handle = dlopen(libPath, RTLD_NOW);
        if (handle == nullptr) {
            ALOGW("skipping %s: %s", libPath, dlerror());
            return;
        }
        dlclose(handle);
        mComponents.emplace(libPath, libPath);
    };

    emplace("libcodec2_sec_mp3dec.so");
    emplace("libcodec2_sec_flacdec.so");
    emplace("libcodec2_sec_amrnbdec.so");
    emplace("libcodec2_sec_amrwbdec.so");
    emplace("libcodec2_sec_imadec.so");
    emplace("libcodec2_sec_aacdec.so");
    emplace("libcodec2_sec_aacenc.so");
    emplace("libcodec2_sec_wmadec.so");
    emplace("libcodec2_sec_wmv7dec.so");
    emplace("libcodec2_sec_wmv8dec.so");
    emplace("libcodec2_sec_mp43dec.so");
#if __has_include("dolby_codecs.h")
    // Dolby codec list, kept out of version control (see .gitignore).
#include "dolby_codecs.h"
#endif
    emplace("libcodec2_sec_vc1dec.so");
    emplace("libcodec2_sec_mp4vdec.so");
}

c2_status_t SecC2ComponentStore::copyBuffer(
        std::shared_ptr<C2GraphicBuffer> src, std::shared_ptr<C2GraphicBuffer> dst) {
    (void)src;
    (void)dst;
    return C2_OMITTED;
}

c2_status_t SecC2ComponentStore::query_sm(
        const std::vector<C2Param*> &stackParams,
        const std::vector<C2Param::Index> &heapParamIndices,
        std::vector<std::unique_ptr<C2Param>> *const heapParams) const {
    return mInterface.query(stackParams, heapParamIndices, C2_MAY_BLOCK, heapParams);
}

c2_status_t SecC2ComponentStore::config_sm(
        const std::vector<C2Param*> &params,
        std::vector<std::unique_ptr<C2SettingResult>> *const failures) {
    return mInterface.config(params, C2_MAY_BLOCK, failures);
}

void SecC2ComponentStore::visitComponents() {
    std::lock_guard<std::mutex> lock(mMutex);
    if (mVisited) {
        return;
    }
    for (auto &pathAndLoader : mComponents) {
        const C2String &path = pathAndLoader.first;
        ComponentLoader &loader = pathAndLoader.second;
        std::shared_ptr<ComponentModule> module;
        if (loader.fetchModule(&module) == C2_OK) {
            std::shared_ptr<const C2Component::Traits> traits = module->getTraits();
            if (traits) {
                mComponentList.push_back(traits);
                mComponentNameToPath.emplace(traits->name, path);
                for (const C2String &alias : traits->aliases) {
                    mComponentNameToPath.emplace(alias, path);
                }
            }
        }
    }
    mVisited = true;
}

std::vector<std::shared_ptr<const C2Component::Traits>> SecC2ComponentStore::listComponents() {
    // This method SHALL return within 500ms.
    visitComponents();
    return mComponentList;
}

c2_status_t SecC2ComponentStore::findComponent(
        C2String name, std::shared_ptr<ComponentModule> *module) {
    (*module).reset();
    visitComponents();

    auto pos = mComponentNameToPath.find(name);
    if (pos != mComponentNameToPath.end()) {
        return mComponents.at(pos->second).fetchModule(module);
    }
    return C2_NOT_FOUND;
}

c2_status_t SecC2ComponentStore::createComponent(
        C2String name, std::shared_ptr<C2Component> *const component) {
    // This method SHALL return within 100ms.
    component->reset();
    std::shared_ptr<ComponentModule> module;
    c2_status_t res = findComponent(name, &module);
    if (res == C2_OK) {
        // TODO: get a unique node ID
        res = module->createComponent(0, component);
    }
    return res;
}

c2_status_t SecC2ComponentStore::createInterface(
        C2String name, std::shared_ptr<C2ComponentInterface> *const interface) {
    // This method SHALL return within 100ms.
    interface->reset();
    std::shared_ptr<ComponentModule> module;
    c2_status_t res = findComponent(name, &module);
    if (res == C2_OK) {
        // TODO: get a unique node ID
        res = module->createInterface(0, interface);
    }
    return res;
}

c2_status_t SecC2ComponentStore::querySupportedParams_nb(
        std::vector<std::shared_ptr<C2ParamDescriptor>> *const params) const {
    return mInterface.querySupportedParams(params);
}

c2_status_t SecC2ComponentStore::querySupportedValues_sm(
        std::vector<C2FieldSupportedValuesQuery> &fields) const {
    return mInterface.querySupportedValues(fields, C2_MAY_BLOCK);
}

C2String SecC2ComponentStore::getName() const {
    return "android.componentStore.sec";
}

std::shared_ptr<C2ParamReflector> SecC2ComponentStore::getParamReflector() const {
    return mReflector;
}

}  // namespace

std::shared_ptr<C2ComponentStore> GetCodec2SecComponentStore() {
    static std::mutex mutex;
    static std::weak_ptr<C2ComponentStore> secStore;
    std::lock_guard<std::mutex> lock(mutex);
    std::shared_ptr<C2ComponentStore> store = secStore.lock();
    if (store == nullptr) {
        store = std::make_shared<SecC2ComponentStore>();
        secStore = store;
        SetPreferredCodec2ComponentStore(store);
    }
    return store;
}

}  // namespace android

extern "C" void RegisterSECCodecServices() {
    using ::android::hardware::media::c2::V1_0::IComponentStore;
    using ::android::hardware::media::c2::V1_0::utils::ComponentStore;

    std::shared_ptr<C2ComponentStore> store = ::android::GetCodec2SecComponentStore();
    if (store == nullptr) {
        ALOGE("Cannot create Codec2's IComponentStore service.");
        return;
    }
    ::android::sp<IComponentStore> service = new ComponentStore(store);
    if (service->registerAsService("default") != ::android::OK) {
        ALOGE("Cannot register Codec2's IComponentStore service.");
        return;
    }
    ALOGI("Codec2's IComponentStore service created.");
}
