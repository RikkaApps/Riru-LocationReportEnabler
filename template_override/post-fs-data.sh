#!/system/bin/sh
MODDIR=${0%/*}

# Reset context in case
chcon -R u:object_r:system_file:s0 $MODDIR

# Get ro.product.mod_device in build.prop
CONFIG_PATH="/data/misc/riru/modules/miui_global_localization"
sed -n '/ro.product.mod_device/{s/.*=//;p}' /system/build.prop > "$CONFIG_PATH/ro.product.mod_device"
