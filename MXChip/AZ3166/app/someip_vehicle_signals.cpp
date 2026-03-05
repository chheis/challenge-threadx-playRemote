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

#include <array>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>

#include "nx_api.h"
#include "cloud_config.h"

#include <someip/message.h>
#include <someip/types.h>
#include <transport/endpoint.h>
#include <transport/udp_transport.h>

#ifndef SOMEIP_SIGNAL_SERVICE_ID
#define SOMEIP_SIGNAL_SERVICE_ID 0x4300
#endif

#ifndef SOMEIP_SIGNAL_EVENT_ID
#define SOMEIP_SIGNAL_EVENT_ID 0x8001
#endif

#ifndef SOMEIP_SIGNAL_CLIENT_ID
#define SOMEIP_SIGNAL_CLIENT_ID 0xA316
#endif

#ifndef SOMEIP_SIGNAL_INTERFACE_VERSION
#define SOMEIP_SIGNAL_INTERFACE_VERSION 0x01
#endif

#ifndef SOMEIP_SIGNAL_LOCAL_IP
#define SOMEIP_SIGNAL_LOCAL_IP "0.0.0.0"
#endif

#ifndef SOMEIP_SIGNAL_LOCAL_PORT
#define SOMEIP_SIGNAL_LOCAL_PORT 30490
#endif

#ifndef SOMEIP_SIGNAL_TARGET_IP
#define SOMEIP_SIGNAL_TARGET_IP "127.0.0.1"
#endif

#ifndef SOMEIP_SIGNAL_TARGET_PORT
#define SOMEIP_SIGNAL_TARGET_PORT 30500
#endif

namespace {

class VehicleSignalSomeipPublisher {
public:
    UINT init() {
        if (initialized_) {
            return NX_SUCCESS;
        }

        someip::transport::UdpTransportConfig udp_cfg;
        udp_cfg.blocking = true;
        udp_cfg.reuse_address = true;
        udp_cfg.max_message_size = 1400;

        const someip::transport::Endpoint local_ep(
            SOMEIP_SIGNAL_LOCAL_IP,
            (uint16_t)SOMEIP_SIGNAL_LOCAL_PORT,
            someip::transport::TransportProtocol::UDP);
        target_endpoint_ = someip::transport::Endpoint(
            SOMEIP_SIGNAL_TARGET_IP,
            (uint16_t)SOMEIP_SIGNAL_TARGET_PORT,
            someip::transport::TransportProtocol::UDP);

        transport_ = std::make_shared<someip::transport::UdpTransport>(local_ep, udp_cfg);
        if (transport_->start() != someip::Result::SUCCESS) {
            transport_.reset();
            return NX_NOT_SUCCESSFUL;
        }

        initialized_ = true;
        return NX_SUCCESS;
    }

    VOID deinit() {
        if (!initialized_) {
            return;
        }

        if (transport_) {
            (void)transport_->stop();
            transport_.reset();
        }

        initialized_ = false;
        has_last_payload_ = false;
        session_id_ = 1;
    }

    VOID publish(UINT left_on,
                 UINT right_on,
                 UINT brake_on,
                 UINT button_a_pressed,
                 UINT button_b_pressed) {
        if (!initialized_ || !transport_) {
            return;
        }

        std::array<uint8_t, 5> payload = {
            (left_on != 0U) ? (uint8_t)1 : (uint8_t)0,
            (right_on != 0U) ? (uint8_t)1 : (uint8_t)0,
            (brake_on != 0U) ? (uint8_t)1 : (uint8_t)0,
            (button_a_pressed != 0U) ? (uint8_t)1 : (uint8_t)0,
            (button_b_pressed != 0U) ? (uint8_t)1 : (uint8_t)0};

        if (has_last_payload_ && (payload == last_payload_)) {
            return;
        }

        if (session_id_ == 0U) {
            session_id_ = 1U;
        }

        someip::Message msg(
            someip::MessageId((uint16_t)SOMEIP_SIGNAL_SERVICE_ID, (uint16_t)SOMEIP_SIGNAL_EVENT_ID),
            someip::RequestId((uint16_t)SOMEIP_SIGNAL_CLIENT_ID, session_id_),
            someip::MessageType::NOTIFICATION,
            someip::ReturnCode::E_OK);

        msg.set_interface_version((uint8_t)SOMEIP_SIGNAL_INTERFACE_VERSION);
        msg.set_payload(std::vector<uint8_t>(payload.begin(), payload.end()));

        if (transport_->send_message(msg, target_endpoint_) == someip::Result::SUCCESS) {
            last_payload_ = payload;
            has_last_payload_ = true;
            session_id_++;
        }
    }

private:
    bool initialized_{false};
    bool has_last_payload_{false};
    uint16_t session_id_{1};
    std::array<uint8_t, 5> last_payload_{};
    someip::transport::Endpoint target_endpoint_;
    std::shared_ptr<someip::transport::UdpTransport> transport_;
};

VehicleSignalSomeipPublisher g_someip_publisher;

} // namespace

extern "C" UINT someip_vehicle_signals_init(void)
{
    UINT status = g_someip_publisher.init();
    if (status != NX_SUCCESS) {
        printf("[SOMEIP][INIT] failed\r\n");
    }
    return status;
}

extern "C" VOID someip_vehicle_signals_deinit(void)
{
    g_someip_publisher.deinit();
}

extern "C" VOID someip_vehicle_signals_publish(UINT left_on,
                                               UINT right_on,
                                               UINT brake_on,
                                               UINT button_a_pressed,
                                               UINT button_b_pressed)
{
    g_someip_publisher.publish(left_on, right_on, brake_on, button_a_pressed, button_b_pressed);
}
