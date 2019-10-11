#!/bin/bash
set -e
cd ${0%/*}
./gradlew clean
./gradlew :riru-module-xfingerprint-pay-wechat:zip
adb push ./release/* /sdcard/Download/
