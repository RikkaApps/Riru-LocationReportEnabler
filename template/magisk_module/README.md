# Riru - Location Report Enabler

<https://github.com/RikkaApps/Riru-LocationReportEnabler>

## Changelog

### v10.0 (13) (2020-11-15)

- Works on Android 11
- Riru v22
- Configuration changes
  - Reinstall the module will not overwrite existing configurations
  - Follow Riru v22, move configuration path to `/data/adb/riru/modules/location_report_enabler` (old configuration will not be moved)
  - Reboot (or kill zygote) is required to let the new settings take effect
