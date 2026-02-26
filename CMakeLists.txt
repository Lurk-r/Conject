cmake_minimum_required(VERSION 3.15)
project(Conject LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
add_executable(Conject src/main.cpp src/utils.cpp src/injector.cpp)
target_compile_definitions(Conject PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_compile_options(Conject PRIVATE /Os /O1 /GL /Gw /Gy /GF /Oy /fp:fast /GR- /EHs-c- /GS-) # /MT for fully portable
    target_link_options(Conject PRIVATE /LTCG /OPT:REF /OPT:ICF /MERGE:.rdata=.text /FILEALIGN:512 /SUBSYSTEM:CONSOLE) # /MAP  msvcrt.lib
else()
    target_compile_options(Conject PRIVATE -Os -flto -fomit-frame-pointer -ffunction-sections -fdata-sections -fmerge-all-constants -fno-exceptions -fno-rtti -fno-ident)
    target_link_options(Conject PRIVATE -flto -Wl,--gc-sections -s -Wl,--strip-all -Wl,--build-id=none -static) # "LINKER:-Map=$<TARGET_FILE:injector>.map"
endif()
