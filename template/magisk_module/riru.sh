#!/sbin/sh
RIRU_PATH="/data/misc/riru"
RIRU_MODULE_ID="%%%RIRU_MODULE_ID%%%"
RIRU_MODULE_PATH="$RIRU_PATH/modules/$RIRU_MODULE_ID"
RIRU_MIN_API_VERSION=%%%RIRU_MIN_API_VERSION%%%

abort_clean() {
  rm -rf "$MODPATH"
  abort "$1"
}

check_riru_version() {
  if [ ! -f "$RIRU_PATH/api_version" ] && [ ! -f "$RIRU_PATH/api_version.new" ]; then
    ui_print    "*********************************************************"
    ui_print    "! Riru is not installed"
    ui_print    "! Please install Riru (Riru - Core) from Magisk Manager or https://github.com/RikkaApps/Riru/releases"
    abort_clean "*********************************************************"
  fi
  RIRU_API_VERSION=$(cat "$RIRU_PATH/api_version.new") || RIRU_API_VERSION=$(cat "$RIRU_PATH/api_version") || RIRU_API_VERSION=0
  [ "$RIRU_API_VERSION" -eq "$RIRU_API_VERSION" ] || RIRU_API_VERSION=0
  ui_print "- Riru API version: $RIRU_API_VERSION"
  if [ "$RIRU_API_VERSION" -lt $RIRU_MIN_API_VERSION ]; then
    ui_print    "*********************************************************"
    ui_print    "! The latest version of Riru is required"
    ui_print    "! Please install Riru (Riru - Core) from Magisk Manager or https://github.com/RikkaApps/Riru/releases"
    abort_clean "*********************************************************"
  fi
}

check_architecture() {
  if [ "$ARCH" != "arm" ] && [ "$ARCH" != "arm64" ] && [ "$ARCH" != "x86" ] && [ "$ARCH" != "x64" ]; then
    abort_clean "! Unsupported platform: $ARCH"
  else
    ui_print "- Device platform: $ARCH"
  fi
}