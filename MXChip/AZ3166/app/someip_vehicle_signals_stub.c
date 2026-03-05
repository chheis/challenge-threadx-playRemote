/* 
 * Copyright (c) Microsoft
 * Copyright (c) 2024 Eclipse Foundation
 * 
 *  This program and the accompanying materials are made available 
 *  under the terms of the MIT license which is available at
 *  https://opensource.org/license/mit.
 * 
 *  SPDX-License-Identifier: MIT
 */

#include "someip_vehicle_signals.h"
#include "nx_api.h"

UINT someip_vehicle_signals_init(void)
{
    return NX_SUCCESS;
}

VOID someip_vehicle_signals_deinit(void)
{
}

VOID someip_vehicle_signals_publish(UINT left_on,
                                    UINT right_on,
                                    UINT brake_on,
                                    UINT button_a_pressed,
                                    UINT button_b_pressed)
{
    NX_PARAMETER_NOT_USED(left_on);
    NX_PARAMETER_NOT_USED(right_on);
    NX_PARAMETER_NOT_USED(brake_on);
    NX_PARAMETER_NOT_USED(button_a_pressed);
    NX_PARAMETER_NOT_USED(button_b_pressed);
}
