# Conject - Console Injector
A simple Console-only DLL(s) Injector

> This branch is the **universal** version of Conject.  
> Looking for the single process version? See the **main** branch.
### Join our [server](https://dsc.gg/algea) for support

## Disclaimer

This project is for educational use and authorized security research only.  
Only test on software/systems you own or have explicit permission to assess.

## Overview

More flexible version of Conject.
It finds DLLs, builds a filtered process list, and lets you choose the target before injection.

## Features

- Console-only UI (clean CLI flow)
- DLL discovery from injector folder (`*.dll`)
- Lightweight and easy to use
- Minimal dependencies (Windows API only, no `vcruntime`)
- Portable (with `/MT`)
- Selectable process list
- Single or multi-DLL injection
- Retry handling with elevation/crash checks
