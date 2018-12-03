# Riru-MiPushFakeModule

A module of [Riru](https://github.com/RikkaApps/Riru). Fake as a Xiaomi Device by hook system_property_get.

## What does this module do

By default, `__system_property_get` (`android::base::GetProperty` on Pie+) will be hooked in packages selected in
MiPushFramework with value map below

* `ro.miui.ui.version.name` -> `V9`
* `ro.miui.ui.version.code` -> `7`
* `ro.miui.version.code_time` -> `1527550858`
* `ro.miui.internal.storage` -> `/sdcard/`
* `ro.product.manufacturer` -> `Xiaomi`
* `ro.product.brand` -> `Xiaomi`
* `ro.product.name` -> `Xiaomi`

also we add some prop inspired by [MiPushFake](https://github.com/Magisk-Modules-Repo/MiPushFake)

 