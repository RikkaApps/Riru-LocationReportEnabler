# Riru-LocationReportEnabler

A module of [Riru](https://github.com/RikkaApps/Riru). Enable Google's location report by hook `__system_property_get` in some packages.

## What does this module do

Hook `__system_property_get` in these packages

* com.google.android.gsf
* com.google.android.gms

and change the return value

* `gsm.sim.operator.numeric` -> `310030`
* `gsm.sim.operator.iso-country` -> `us`