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

#ifndef _SOMEIP_VEHICLE_SIGNALS_H
#define _SOMEIP_VEHICLE_SIGNALS_H

#include "tx_api.h"

#ifdef __cplusplus
extern "C" {
#endif

UINT someip_vehicle_signals_init(void);
VOID someip_vehicle_signals_deinit(void);
VOID someip_vehicle_signals_publish(UINT left_on,
                                    UINT right_on,
                                    UINT brake_on,
                                    UINT button_a_pressed,
                                    UINT button_b_pressed);

#ifdef __cplusplus
}
#endif

#endif // _SOMEIP_VEHICLE_SIGNALS_H
