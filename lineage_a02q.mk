# Galaxy A02s (SM-A025M), codename a02q, msm8953. LineageOS 19.1 product.

# Most specific first.
# 32-bit userspace (see BoardConfig.mk): no core_64_bit.mk.
$(call inherit-product, $(SRC_TARGET_DIR)/product/aosp_base_telephony.mk)

# Device config.
$(call inherit-product, device/samsung/a02q/device.mk)

# LineageOS common.
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

PRODUCT_NAME := lineage_a02q
PRODUCT_DEVICE := a02q
PRODUCT_BRAND := samsung
PRODUCT_MODEL := SM-A025M
PRODUCT_MANUFACTURER := samsung

# Matches stock ro.product.first_api_level.
PRODUCT_SHIPPING_API_LEVEL := 29

PRODUCT_GMS_CLIENTID_BASE := android-samsung

# Match the stock build fingerprint so Treble/keys line up (adjust to the base firmware).
BUILD_FINGERPRINT := samsung/a02qub/a02q:12/SP1A.210812.016/A025MUBS9CXI1:user/release-keys
PRODUCT_BUILD_PROP_OVERRIDES += \
    PRIVATE_BUILD_DESC="a02qub-user 12 SP1A.210812.016 A025MUBS9CXI1 release-keys"
