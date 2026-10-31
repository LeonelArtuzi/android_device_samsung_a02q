#!/bin/bash
# Extracts a02q proprietary blobs from a source dump into vendor/samsung/a02q.
# Usage: ./extract-files.sh <path-to-dump>   (dump has vendor/ odm/ at top)
set -e

DEVICE=a02q
VENDOR=samsung

MY_DIR="${BASH_SOURCE%/*}"
[ ! -d "$MY_DIR" ] && MY_DIR="$PWD"
ANDROID_ROOT="$MY_DIR/../../.."

HELPER="$ANDROID_ROOT/tools/extract-utils/extract_utils.sh"
[ -f "$HELPER" ] || { echo "Helper não encontrado em $HELPER"; exit 1; }
source "$HELPER"

SRC=
KANG=
SECTION=
CLEAN_VENDOR=true
while [ "$#" -gt 0 ]; do
    case "$1" in
        -n|--no-cleanup) CLEAN_VENDOR=false ;;
        -k|--kang) KANG="--kang" ;;
        -s|--section) SECTION="$2"; shift; CLEAN_VENDOR=false ;;
        *) SRC="$1" ;;
    esac
    shift
done
[ -z "$SRC" ] && SRC=adb

setup_vendor "$DEVICE" "$VENDOR" "$ANDROID_ROOT" false "$CLEAN_VENDOR"
extract "$MY_DIR/proprietary-files.txt" "$SRC" $KANG ${SECTION:+--section "$SECTION"}

"$MY_DIR/setup-makefiles.sh"
