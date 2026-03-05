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

set(SOURCES
    startup/system_stm32f4xx.c
    startup/startup_stm32f412rx.s
    startup/tx_initialize_low_level.S
    stm32cubef4/stm32f4xx_hal_msp.c
    board_init.c
    cloud_config.h
    console.c
    screen.c
    sntp_client.c
    main.c
    someip_vehicle_signals_stub.c
    wwd_networking.c
)

if(ENABLE_OPENSOMEIP)
    if(NOT EXISTS "${OPENSOMEIP_SOURCE_DIR}/include/someip/message.h")
        message(FATAL_ERROR "ENABLE_OPENSOMEIP=ON but OPENSOMEIP_SOURCE_DIR is invalid: ${OPENSOMEIP_SOURCE_DIR}")
    endif()

    list(REMOVE_ITEM SOURCES someip_vehicle_signals_stub.c)
    list(APPEND SOURCES someip_vehicle_signals.cpp)
endif()

add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE SOMEIP_DEVICE_ROLE=${SOMEIP_DEVICE_ROLE})

target_link_libraries(${PROJECT_NAME}
    azrtos::threadx
    azrtos::netxduo

    stm32cubef4
    mxchip_bsp
    app_common
    jsmn
    wiced_sdk
    nanoprintf
)

if(ENABLE_OPENSOMEIP)
    add_library(opensomeip_embedded STATIC
        ${OPENSOMEIP_SOURCE_DIR}/src/common/result.cpp
        ${OPENSOMEIP_SOURCE_DIR}/src/someip/types.cpp
        ${OPENSOMEIP_SOURCE_DIR}/src/someip/message.cpp
        ${OPENSOMEIP_SOURCE_DIR}/src/e2e/e2e_header.cpp
        ${OPENSOMEIP_SOURCE_DIR}/src/transport/endpoint.cpp
        ${OPENSOMEIP_SOURCE_DIR}/src/transport/udp_transport.cpp
        ${OPENSOMEIP_SOURCE_DIR}/src/platform/threadx/memory.cpp
    )

    target_include_directories(opensomeip_embedded PUBLIC
        ${OPENSOMEIP_SOURCE_DIR}/include
        ${OPENSOMEIP_SOURCE_DIR}/include/platform/threadx
        ${OPENSOMEIP_SOURCE_DIR}/include/platform/lwip
        ${CMAKE_CURRENT_LIST_DIR}/opensomeip_compat
    )

    target_link_libraries(opensomeip_embedded
        azrtos::threadx
        azrtos::netxduo
    )

    target_compile_options(opensomeip_embedded PRIVATE -Wno-error)

    set_property(TARGET opensomeip_embedded PROPERTY CXX_STANDARD 17)
    set_property(TARGET opensomeip_embedded PROPERTY CXX_STANDARD_REQUIRED ON)

    target_link_libraries(${PROJECT_NAME} opensomeip_embedded)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_OPENSOMEIP=1)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wno-error>)
    set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
    set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
endif()

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        .
)

target_link_directories(${PROJECT_NAME} 
    PUBLIC 
        startup
)

if(CMAKE_C_COMPILER_ID STREQUAL "IAR")
    message(FATAL_ERROR, "IAR is not currently implemented for this device")
else()
    set_target_linker(${PROJECT_NAME} MXChip_AZ3166.ld)
endif()

post_build(${PROJECT_NAME})
