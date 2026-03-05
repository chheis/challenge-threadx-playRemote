::  Copyright (c) Microsoft
::  Copyright (c) 2024 Eclipse Foundation
:: 
::  This program and the accompanying materials are made available 
::  under the terms of the MIT license which is available at
::  https://opensource.org/license/mit.
:: 
::  SPDX-License-Identifier: MIT
:: 
::  Contributors: 
::     Microsoft         - Initial version
::     Frederic Desbiens - 2024 version.

@echo off

setlocal
cd /d %~dp0\..

set SOMEIP_DEVICE_ROLE=%1
if "%SOMEIP_DEVICE_ROLE%"=="" set SOMEIP_DEVICE_ROLE=1
if not "%SOMEIP_DEVICE_ROLE%"=="1" if not "%SOMEIP_DEVICE_ROLE%"=="2" (
    echo ERROR: SOMEIP device role must be 1 or 2.
    echo Usage: %~nx0 [1^|2]
    exit /b 1
)

::IF EXIST build (rd /S /Q build)

echo Building SOME/IP profile role=%SOMEIP_DEVICE_ROLE%
cmake -Bbuild -GNinja -DCMAKE_TOOLCHAIN_FILE="../../../cmake/arm-gcc-cortex-m4.cmake" -DENABLE_OPENSOMEIP=ON -DSOMEIP_DEVICE_ROLE=%SOMEIP_DEVICE_ROLE% -DOPENSOMEIP_SOURCE_DIR="../../third_party/opensomeip"
cmake --build build

IF %0 == "%~0" pause
