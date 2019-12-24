#!/bin/bash
set -e
cd ${0%/*}
./gradlew clean
./gradlew :riru-module-xfingerprint-pay-wechat:zip
adb shell rm -fv "/data/local/tmp/libxfingerprint_pay_wechat.dex"
adb push ./release/* /sdcard/Download/
