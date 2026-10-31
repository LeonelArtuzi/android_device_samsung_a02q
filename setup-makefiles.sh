#!/bin/bash
# Regenerates Android.bp/Android.mk/a02q-vendor.mk for the a02q blobs.
set -e

DEVICE=a02q
VENDOR=samsung

MY_DIR="${BASH_SOURCE%/*}"
[ ! -d "$MY_DIR" ] && MY_DIR="$PWD"
ANDROID_ROOT="$MY_DIR/../../.."

HELPER="$ANDROID_ROOT/tools/extract-utils/extract_utils.sh"
[ -f "$HELPER" ] || { echo "Helper não encontrado em $HELPER"; exit 1; }
source "$HELPER"

setup_vendor "$DEVICE" "$VENDOR" "$ANDROID_ROOT" false

write_headers
write_makefiles "$MY_DIR/proprietary-files.txt" true
write_footers
