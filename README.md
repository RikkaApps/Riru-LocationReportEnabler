# Riru - Location Report Enabler

A module of [Riru](https://github.com/RikkaApps/Riru). Enable Google location report and Google app timeline by hook `__system_property_get` in specific packages.

## What does this module do

By default, `__system_property_get` (and `android::base::GetProperty` on Pie+) will be hooked in these packages

* com.google.android.gsf
* com.google.android.gms
* com.google.android.apps.maps

and the return value will be changed

* `gsm.sim.operator.numeric` -> `310030`
* `gsm.sim.operator.iso-country` -> `us`

## Customize

* Add / remove enabled package

  ```
  touch /data/misc/riru/modules/location_report_enabler/packages/<package name>
  rm /data/misc/riru/modules/location_report_enabler/packages/<package name>
  ```

* Return value
  
  ```
  echo -n 310030 > /data/misc/riru/modules/location_report_enabler/gsm.sim.operator.numeric
  echo -n us > /data/misc/riru/modules/location_report_enabler/gsm.sim.operator.iso-country
  ```

  **Don't forget `-n`, or the return data will contain a newline**