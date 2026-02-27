# Conject - Console Injector
A simple Console-only DLL(s) Injector </br>
Feel free to use this project as base, credits if you can.

> This branch is the **universal** version of Conject.  
> Looking for the single-process version? See the **[main](https://github.com/Lurk-r/conject)** branch.
### Join our [server](https://dsc.gg/algea) for support

## Disclaimer

This project is for educational use and authorized security research only.  
Only test on software/systems you own or have explicit permission to assess.

## Overview

More flexible version of Conject.
It finds DLLs, builds a filtered process list, and lets you choose the target before injection.

## Features

- Console-only UI
- DLL discovery from injector folder (`*.dll`)
- Lightweight and easy to use
- Minimal dependencies (Windows API only, no `vcruntime`)
- Portable (with `/MT`)
- Selectable process list with windows (avoid services, child processes)
- Single or multi-DLL injection via QueueAPC
- Retry with elevation/crash checks
