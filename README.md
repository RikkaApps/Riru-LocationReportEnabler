# Location Report Enabler

A module of [Riru](https://github.com/RikkaApps/Riru). Enable Google Feed & Timeline & Location report in unsupported regions by changing system properties in related packages.

## User guide

### What does this module do

By default, `__system_property_get` and `__system_property_read_callback` (API 26+) will be hooked in these packages

* com.google.android.gsf
* com.google.android.gms
* com.google.android.apps.maps

and the return value will be changed

* `gsm.sim.operator.numeric` -> `310030`
* `gsm.sim.operator.iso-country` -> `us`

### Configuration

Note, reboot (or kill zygote) is required to let the new settings take effect.

#### Packages

`/data/adb/modules/riru_location_report_enabler/config/packages/<package name>`

#### Properties

`/data/adb/modules/riru_location_report_enabler/config/properties/<key>` (file content is value)

## Build

Gradle tasks:

* `:riru:assembleDebug/Release`
   
   Generate Magisk module zip to `out`.

* `:riru:pushDebug/Release`
   
   Push the zip with adb to `/data/local/tmp`.

* `:riru:flashDebug/Release`
   
   Flash the zip with `adb shell su -c magisk --install-module`.

* `:riru:flashAndRebootDebug/Release`

   Flash the zip and reboot the device.
