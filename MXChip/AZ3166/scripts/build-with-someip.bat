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

::IF EXIST build (rd /S /Q build)

cmake -Bbuild -GNinja -DCMAKE_TOOLCHAIN_FILE="../../../cmake/arm-gcc-cortex-m4.cmake" -DENABLE_OPENSOMEIP=ON -DOPENSOMEIP_SOURCE_DIR="../../third_party/opensomeip"
cmake --build build

IF %0 == "%~0" pause
