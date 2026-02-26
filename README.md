# Conject - Console Injector
A simple Console-only DLL(s) Injector

> This branch is the **single-process** version of Conject.  
> Looking for the process-picker version? See the **[universal](https://github.com/Lurk-r/conject/tree/universal)** branch.
### Join our [server](https://dsc.gg/algea) for support

## Disclaimer

This project is for educational use and authorized security research only.  
Only test on software/systems you own or have explicit permission to assess.

## Overview

1. Find DLL files in the injector folder (`*.dll`)
2. Let you pick one or multiple DLLs (auto-pick if only one is found)
3. Find the default target process pattern
4. Inject with retry handling and basic crash/elevation checks

If no matching process is found, the injector can wait/launch (via Steam) and inject after a delay.

## Features

- Console-only UI
- Default target process pattern
- DLL discovery from injector folder (`*.dll`)
- Lightweight and easy to use
- Minimal dependencies (Windows API only, no `vcruntime`)
- Portable (with `/MT`)
- Single or multi-DLL injection
- Retry with elevation/crash checks
