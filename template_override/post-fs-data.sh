#!/system/bin/sh
MODDIR=${0%/*}

# Reset context in case
chcon -R u:object_r:system_file:s0 $MODDIR