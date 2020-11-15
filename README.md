# Riru - Location Report Enabler

A module of [Riru](https://github.com/RikkaApps/Riru). Enable Google Feed & Timeline & Location reprot in unsupported regions by changing system properties in related packages.

## What does this module do

By default, `__system_property_get` and `__system_property_read_callback` (API 26+) will be hooked in these packages

* com.google.android.gsf
* com.google.android.gms
* com.google.android.apps.maps

and the return value will be changed

* `gsm.sim.operator.numeric` -> `310030`
* `gsm.sim.operator.iso-country` -> `us`

## Configuration

Note, reboot (or kill zygote) is required to let the new settings take effect.

### Packages

`/data/adb/riru/modules/location_report_enabler/packages/<package name>`

### Properties

`/data/misc/riru/modules/location_report_enabler/properties/<key>` (file content is value)