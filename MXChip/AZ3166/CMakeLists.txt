#  Copyright (c) Microsoft
#  Copyright (c) 2024 Eclipse Foundation
# 
#  This program and the accompanying materials are made available 
#  under the terms of the MIT license which is available at
#  https://opensource.org/license/mit.
# 
#  SPDX-License-Identifier: MIT
# 
#  Contributors: 
#     Microsoft         - Initial version
#     Frédéric Desbiens - 2024 version.

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
set(CMAKE_C_STANDARD 99)

set(GSG_BASE_DIR ${CMAKE_SOURCE_DIR}/../..)
set(SHARED_SRC_DIR ${GSG_BASE_DIR}/shared/src)
set(SHARED_LIB_DIR ${GSG_BASE_DIR}/shared/lib)

# Set the toolchain if not defined
if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${GSG_BASE_DIR}/cmake/arm-gcc-cortex-m4.cmake")
endif()

list(APPEND CMAKE_MODULE_PATH ${GSG_BASE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(utilities)

# Define the Project
# CXX enables IntelliSense only. Sources are still compiled as C.
project(mxchip_threadx C CXX ASM)

# ---------------------------------------------------------------------------
# Optional OpenSOME/IP integration (disabled by default).
# ---------------------------------------------------------------------------
option(ENABLE_OPENSOMEIP "Enable OpenSOME/IP-based signal publishing" OFF)
set(OPENSOMEIP_SOURCE_DIR "${GSG_BASE_DIR}/third_party/opensomeip"
    CACHE PATH "Path to the OpenSOME/IP source tree")

add_subdirectory(${SHARED_SRC_DIR} shared_src)
add_subdirectory(lib)
add_subdirectory(app)

