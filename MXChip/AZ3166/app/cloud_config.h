/* 
 * Copyright (c) Microsoft
 * Copyright (c) 2024 Eclipse Foundation
 * 
 *  This program and the accompanying materials are made available 
 *  under the terms of the MIT license which is available at
 *  https://opensource.org/license/mit.
 * 
 *  SPDX-License-Identifier: MIT
 * 
 *  Contributors: 
 *     Microsoft         - Initial version
 *     Frédéric Desbiens - 2024 version.
 */

#ifndef _CLOUD_CONFIG_H
#define _CLOUD_CONFIG_H

typedef enum
{
    None         = 0,
    WEP          = 1,
    WPA_PSK_TKIP = 2,
    WPA2_PSK_AES = 3
} WiFi_Mode;

// ----------------------------------------------------------------------------
// WiFi connection config
// ----------------------------------------------------------------------------

#define WIFI_SSID     "sdvdemo"          //Change to your WiFi SSID.
#define WIFI_PASSWORD "Demo1234"          //Change to your WiFi password.
#define WIFI_MODE     WPA2_PSK_AES

// ----------------------------------------------------------------------------
// MQTT Config
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// SOME/IP signal publisher config
// ----------------------------------------------------------------------------
// Build-time device role selector:
//   1 = Device 1 (local :30490, target :30500)
//   2 = Device 2 (local :30500, target :30490)
// Can be overridden from CMake with -DSOMEIP_DEVICE_ROLE=<1|2>.
#ifndef SOMEIP_DEVICE_ROLE
#define SOMEIP_DEVICE_ROLE 1
#endif

#if (SOMEIP_DEVICE_ROLE == 1)
#define HOSTNAME      "chheis-eclipse-threadx-someip-sender-IP91"  //Change to unique hostname.
#define SOMEIP_SIGNAL_TARGET_IP "192.168.88.92"
#define SOMEIP_SIGNAL_TARGET_PORT 30500
#define SOMEIP_SIGNAL_LOCAL_IP "0.0.0.0"
#define SOMEIP_SIGNAL_LOCAL_PORT 30490
#elif (SOMEIP_DEVICE_ROLE == 2)
#define HOSTNAME      "chheis-eclipse-threadx-someip-reciever-IP92"  //Change to unique hostname.
#define SOMEIP_SIGNAL_TARGET_IP "192.168.88.91"
#define SOMEIP_SIGNAL_TARGET_PORT 30490
#define SOMEIP_SIGNAL_LOCAL_IP "0.0.0.0"
#define SOMEIP_SIGNAL_LOCAL_PORT 30500
#else
#error "Unsupported SOMEIP_DEVICE_ROLE. Use 1 or 2."
#endif

// SOME/IP IDs used by this board for signal notification payload:
// [left, right, brake, button_a, button_b] (each 1 byte, 0/1).
#define SOMEIP_SIGNAL_SERVICE_ID 0x4300
#define SOMEIP_SIGNAL_EVENT_ID 0x8001
#define SOMEIP_SIGNAL_CLIENT_ID 0xA316
#define SOMEIP_SIGNAL_INTERFACE_VERSION 0x01

// ----------------------------------------------------------------------------
// Time sync config
// ----------------------------------------------------------------------------
// Set to 1 to require SNTP time sync at startup, or 0 to run without NTP.
#define ENABLE_SNTP_TIME_SYNC 0


#endif // _CLOUD_CONFIG_H
