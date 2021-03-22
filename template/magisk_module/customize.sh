SKIPUNZIP=1

# Check architecture
if [ "$ARCH" != "arm" ] && [ "$ARCH" != "arm64" ] && [ "$ARCH" != "x86" ] && [ "$ARCH" != "x64" ]; then
  abort "! Unsupported platform: $ARCH"
else
  ui_print "- Device platform: $ARCH"
fi

# Extract verify.sh
ui_print "- Extracting verify.sh"
unzip -o "$ZIPFILE" 'verify.sh' -d "$TMPDIR" >&2
if [ ! -f "$TMPDIR/verify.sh" ]; then
  ui_print "*********************************************************"
  ui_print "! Unable to extract verify.sh!"
  ui_print "! This zip may be corrupted, please try downloading again"
  abort "*********************************************************"
fi
. $TMPDIR/verify.sh

# Extract riru.sh
extract "$ZIPFILE" 'riru.sh' "$MODPATH"
. $MODPATH/riru.sh

check_riru_version
enforce_install_from_magisk_app

# Extract libs
ui_print "- Extracting module files"

extract "$ZIPFILE" 'module.prop' "$MODPATH"

mkdir "$MODPATH/riru"
mkdir "$MODPATH/riru/lib"
mkdir "$MODPATH/riru/lib64"

if [ "$ARCH" = "x86" ] || [ "$ARCH" = "x64" ]; then
  ui_print "- Extracting x86 libraries"
  extract "$ZIPFILE" "lib/x86/lib$RIRU_MODULE_ID.so" "$MODPATH/riru/lib" true

  if [ "$IS64BIT" = true ]; then
    ui_print "- Extracting x64 libraries"
    extract "$ZIPFILE" "lib/x86_64/lib$RIRU_MODULE_ID.so" "$MODPATH/riru/lib64" true
  fi
fi

if [ "$ARCH" = "arm" ] || [ "$ARCH" = "arm64" ]; then
  ui_print "- Extracting arm libraries"
  extract "$ZIPFILE" "lib/armeabi-v7a/lib$RIRU_MODULE_ID.so" "$MODPATH/riru/lib" true

  if [ "$IS64BIT" = true ]; then
    ui_print "- Extracting arm64 libraries"
    extract "$ZIPFILE" "lib/arm64-v8a/lib$RIRU_MODULE_ID.so" "$MODPATH/riru/lib64" true
  fi
fi

set_perm_recursive "$MODPATH" 0 0 0755 0644

if [ "$MAGISK_VER_CODE" -ge 21000 ]; then
  CURRENT_MODPATH=$(magisk --path)/.magisk/modules/riru_location_report_enabler
else
  CURRENT_MODPATH=/sbin/.magisk/modules/riru_location_report_enabler
fi

if [ -d "$CURRENT_MODPATH/config" ]; then
  ui_print "- Use existing configuration"
  cp -r "$CURRENT_MODPATH/config" "$MODPATH"/config
fi

if [ ! -d "$MODPATH"/config ] && [ -d "/data/adb/riru/modules/location_report_enabler/config" ]; then
  ui_print "- Use configuration from old versions"
  cp -r "/data/adb/riru/modules/location_report_enabler/config" "$MODPATH"/config
fi

if [ ! -d "$MODPATH"/config ]; then
  ui_print "- Creating default configuration"

  mkdir -p "$MODPATH/config/properties"
  echo -n "310030" >"$MODPATH/config/properties/gsm.sim.operator.numeric"
  echo -n "us" >"$MODPATH/config/properties/gsm.sim.operator.iso-country"

  mkdir -p "$MODPATH/config/packages"
  touch "$MODPATH/config/packages/com.google.android.gsf"
  touch "$MODPATH/config/packages/com.google.android.gms"
  touch "$MODPATH/config/packages/com.google.android.apps.maps"
fi

set_perm_recursive "$MODPATH/config" 0 0 0700 0600
