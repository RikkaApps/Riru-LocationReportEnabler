# Location Report Enabler

Enable Google Feed & Timeline & Location report in unsupported regions by changing system properties in related packages.

**This module requires Riru v25.0.0 or above.**

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

`/data/adb/modules/riru_location_report_enabler/config/packages/<package name>`

### Properties

`/data/adb/modules/riru_location_report_enabler/config/properties/<key>` (file content is value)

## Source code

<https://github.com/RikkaApps/Riru-LocationReportEnabler>

## Changelog

### v11.0.0 (2021-03-22)

- Riru v25
  - Move configuration path to Magisk module path (`/data/adb/modules/riru_location_report_enabler`), old configuration will be moved on upgrade
  - Unload on unrelated processes
- Reduce file size

### v10.2 (15) (2021-01-07)

- Fix the return value of `__system_property_get` ([#33](https://github.com/RikkaApps/Riru-LocationReportEnabler/pull/33))

### v10.0 (13) (2020-11-15)

- Works on Android 11
- Riru v22
- Configuration changes
  - Reinstall the module will not overwrite existing configurations
  - Follow Riru v22, move configuration path to `/data/adb/riru/modules/location_report_enabler` (old configuration will not be moved)
  - Reboot (or kill zygote) is required to let the new settings take effect
