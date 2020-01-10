#!/system/bin/sh
MODDIR=${0%/*}
MODULE_ID="module_xfingerprint_pay_wechat"

# Reset context jsut in case
chcon -R u:object_r:system_file:s0 "$MODDIR"

# Rename module.prop.new
if [ -f "/data/misc/riru/modules/$MODULE_ID/module.prop.new" ]; then
    rm "/data/misc/riru/modules/$MODULE_ID/module.prop"
    mv "/data/misc/riru/modules/$MODULE_ID/module.prop.new" "/data/misc/riru/modules/$MODULE_ID/module.prop"
fi