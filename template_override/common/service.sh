#!/system/bin/sh
# Please don't hardcode /magisk/modname/... ; instead, please use $MODDIR/...
# This will make your scripts compatible even if Magisk change its mount point in the future
MODDIR=${0%/*}

# This script will be executed in late_start service mode
# More info in the main Magisk thread
resetprop -n ro.miui.ui.version.name V9
resetprop -n ro.miui.ui.version.code 7
resetprop -n ro.miui.version.code_time 1505408400
resetprop -n ro.miui.internal.storage /sdcard/