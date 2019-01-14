# Riru-LocationReportEnabler

A module of [Riru](https://github.com/RikkaApps/Riru). Enable Google's location report by hook `__system_property_get` in some packages.

## What does this module do

By default, `__system_property_get` (`android::base::GetProperty` on Pie+) will be hooked in these packages

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

  If `/data/misc/riru/location_report_enabler/packages` not exists, default package list will be used.

* Return value
  
  ```
  echo -n 310030 > /data/misc/riru/modules/location_report_enabler/gsm.sim.operator.numeric
  echo -n us > /data/misc/riru/modules/location_report_enabler/gsm.sim.operator.iso-country
  ```

  **Don't forget `-n`, or the return data will contain a newline**