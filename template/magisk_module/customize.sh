SKIPUNZIP=1
RIRU_PATH="/data/misc/riru"
RIRU_MODULES_PATH="$RIRU_PATH/modules"

# check architecture
if [[ "$ARCH" != "arm" && "$ARCH" != "arm64" && "$ARCH" != "x86" && "$ARCH" != "x64" ]]; then
  abort "! Unsupported platform: $ARCH"
else
  ui_print "- Device platform: $ARCH"
fi

# check Riru version
if [[ ! -f "$RIRU_PATH/api_version" ]]; then
  ui_print "*********************************************************"
  ui_print "! 'Riru - Core' is not installed"
  ui_print "! You can download from 'Magisk Manager' or https://github.com/RikkaApps/Riru/releases"
  abort    "*********************************************************"
fi
RIRU_API_VERSION=$(cat "$RIRU_PATH/api_version")
ui_print "- Riru API version: $RIRU_API_VERSION"
if [[ "$RIRU_API_VERSION" -lt 4 ]]; then
  ui_print "*********************************************************"
  ui_print "! The latest version of 'Riru - Core' is required"
  ui_print "! You can download from 'Magisk Manager' or https://github.com/RikkaApps/Riru/releases"
  abort    "*********************************************************"
fi

# extract verify.sh
unzip -o "$ZIPFILE" 'verify.sh' -d "$TMPDIR" >&2
if [[ ! -f "$TMPDIR/verify.sh" ]]; then
  ui_print "*********************************************************"
  ui_print "! Unable to extract verify.sh!"
  ui_print "! This zip may be corrupted, please try downloading again"
  abort    "*********************************************************"
fi
. $TMPDIR/verify.sh

# extract libs
ui_print "- Extracting module files"
vunzip -o "$ZIPFILE" 'module.prop' 'post-fs-data.sh' 'uninstall.sh' -d "$MODPATH"

if [[ "$ARCH" == "x86" || "$ARCH" == "x64" ]]; then
  ui_print "- Extracting x86/64 libraries"
  vunzip -o "$ZIPFILE" 'system_x86/*' -d "$MODPATH"
  mv "$MODPATH/system_x86/lib" "$MODPATH/system/lib"
  mv "$MODPATH/system_x86/lib64" "$MODPATH/system/lib64"
else
  ui_print "- Extracting arm/arm64 libraries"
  vunzip -o "$ZIPFILE" 'system/*' -d "$MODPATH"
fi

if [[ "$IS64BIT" == "false" ]]; then
  ui_print "- Removing 64-bit libraries"
  rm -rf "$MODPATH/system/lib64"
fi

# Riru files
ui_print "- Extracting extra files"
unzip -o "$ZIPFILE" 'data/*' -d "$TMPDIR" >&2
[[ -d "$RIRU_MODULES_PATH" ]] || mkdir -p "$RIRU_MODULES_PATH" || abort "! Can't mkdir -p $RIRU_MODULES_PATH"
cp -af "$TMPDIR$RIRU_MODULES_PATH/." "$RIRU_MODULES_PATH" || abort "! Can't cp -af $TMPDIR$RIRU_MODULES_PATH/. $RIRU_MODULES_PATH"

# set permissions
ui_print "- Setting permissions"
set_perm_recursive "$MODPATH" 0 0 0755 0644
