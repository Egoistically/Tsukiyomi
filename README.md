# Tsukiyomi ([月読](https://ja.wikipedia.org/wiki/%E3%83%84%E3%82%AF%E3%83%A8%E3%83%9F))

Pretty barebones DLL (and injector) to fix Final Fantasy XIV's multi-monitor fullscreen annoyances. Can be added to [XIVLauncher](https://goatcorp.github.io/)'s autolaunch.

> Fix completely based on [FFXIV-FixFullScreen](https://github.com/amasover/FFXIV-FixFullScreen).

## Technicalities

For whatever reason, `ChangeDisplaySettingsExW` gets called every time the game comes back to foreground, causing the monitor to refresh its input and thus an annoying black screen. 

To fix this behavior, the DLL uses MinHook to detour the aforementioned API function to an empty one.

## Compiling

- Visual Studio 2022.
- MinHook included through [vcpkg](https://vcpkg.io/en/) (`minhook:x64-windows-static`).

## Questions

> Is it safe?  
I don't know. Don't think SQEX cares a lot about having DLL's injected into their game and win32 functions detoured, but I do not take any responsibility for what may happen.

> Why no binaries?  
Because providing binaries for an injector and a DLL is extremely shady, even if the project is open source.

## Credits

https://github.com/amasover/FFXIV-FixFullScreen  
https://github.com/TsudaKageyu/minhook