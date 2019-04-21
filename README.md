# Magisk 微信指纹支付 (Fingerprint pay for WeChat)

Magisk 版 [Xposed Fingerprint pay](https://github.com/eritpchy/Xposed-Fingerprint-pay)

## 最低要求

* Root 过的 Android 6.0+ 设备
* [Magisk](https://github.com/topjohnwu/Magisk)


## 他怎么工作呢？

1. 利用 [Magisk](https://github.com/topjohnwu/Magisk) 的 [Riru](https://github.com/RikkaApps/Riru) 注入 zygote 进程
2. 加载指纹支付代码
3. 利用 [SandHook](https://github.com/ganyao114/SandHook) 完成注入功能

## 为什么要做出这个呢?

* 设备支持指纹, 但不支持指纹支付
* 更加容易集成到定制 rom 内
* Xposed 版本指纹支付, 会因 Xposed 导致系统卡顿, 软件崩溃, 银行软件不正常工作


## 如何使用

1. 下载 [magisk-riru-core.zip](https://github.com/RikkaApps/Riru/releases) 进手机
2. 下载 [magisk-riru-module-xfingerprint-pay-wechat.zip](https://github.com/eritpchy/Fingerprint-pay-magisk-wechat/releases) 进手机
3. 进入 Magisk Manager, 模块, 安装这两个模块, 不要重启
4. 勾选启用两个模块, 重启手机
5. Enjoy

## 编译

1. 运行./gradlew :riru-module-xfingerprint-pay-wechat:zip
2. 编译文件位于./release

## 致谢

* [Riru](https://github.com/RikkaApps/Riru)
* [SandHook](https://github.com/ganyao114/SandHook)
* [EdXposed](https://github.com/ElderDrivers/EdXposed)
* [Magisk](https://github.com/topjohnwu/Magisk)
