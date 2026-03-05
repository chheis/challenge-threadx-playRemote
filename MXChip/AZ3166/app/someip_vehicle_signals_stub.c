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

VOID someip_vehicle_signals_poll_receive(void)
{
}

UINT someip_vehicle_signals_get_remote_state(UINT* has_data,
                                             UINT* left_on,
                                             UINT* right_on,
                                             UINT* brake_on,
                                             UINT* button_a_pressed,
                                             UINT* button_b_pressed)
{
    if (has_data != NX_NULL)
    {
        *has_data = 0U;
    }
    if (left_on != NX_NULL)
    {
        *left_on = 0U;
    }
    if (right_on != NX_NULL)
    {
        *right_on = 0U;
    }
    if (brake_on != NX_NULL)
    {
        *brake_on = 0U;
    }
    if (button_a_pressed != NX_NULL)
    {
        *button_a_pressed = 0U;
    }
    if (button_b_pressed != NX_NULL)
    {
        *button_b_pressed = 0U;
    }

    return NX_SUCCESS;
}
