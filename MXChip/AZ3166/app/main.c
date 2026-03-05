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

#include <stdio.h>
#include <math.h>
#include <string.h>

#include "tx_api.h"
#include "nx_api.h"
#include "nxd_mqtt_client.h"

#include "board_init.h"
#include "cmsis_utils.h"
#include "screen.h"
//#include "sntp_client.h"
#include "wwd_networking.h"
#include "someip_vehicle_signals.h"

#include "cloud_config.h"
//#include "nx_client.h"
#include "sensor.h"
#include "ssd1306.h"
#define ECLIPSETX_THREAD_STACK_SIZE 4096
#define ECLIPSETX_THREAD_PRIORITY   4

TX_THREAD eclipsetx_thread;
TX_THREAD eclipsetx_thread2;
ULONG eclipsetx_thread_stack[ECLIPSETX_THREAD_STACK_SIZE / sizeof(ULONG)];
ULONG eclipsetx_thread_stack2[ECLIPSETX_THREAD_STACK_SIZE / sizeof(ULONG)];

//declare functions
void ScreenShowSensValue();
void ScreenShowValue(ULONG value);
void ScreenShowText(char* text);

/*****************************************************************************************/
/* MQTT Local Server IoT Client example.                                                 */
/*****************************************************************************************/
/* MQTT Demo defines */

/* IP Address of the local server. */
#define  LOCAL_SERVER_ADDRESS (IP_ADDRESS(192, 168, 88, 100))

#define  DEMO_STACK_SIZE            2048
#define  CLIENT_ID_STRING           "mytestclient"
#define  MQTT_CLIENT_STACK_SIZE     4096
#define  STRLEN(p)                  (sizeof(p) - 1)
/* Declare the MQTT thread stack space. */
static ULONG                        mqtt_client_stack[MQTT_CLIENT_STACK_SIZE / sizeof(ULONG)];
/* Declare the MQTT client control block. */
static NXD_MQTT_CLIENT              mqtt_client;
/* Define the symbol for signaling a received message. */
/* Define the test threads.  */
#define TOPIC_NAME                  "mqtt_data"
#define SUBSCRIBE_TOPIC             "InVehicleTopics"
#define MESSAGE_STRING              "This is a message. "
/* Define the priority of the MQTT internal thread. */
#define MQTT_THREAD_PRIORITY 2
/* Define the MQTT keep alive timer for 5 minutes */
#define MQTT_KEEP_ALIVE_TIMER       300
#define MQTT_CLEAN_SESSION          1
#define MQTT_STOP_HOLD_TICKS        (2 * TX_TIMER_TICKS_PER_SECOND)
#define QOS0                        0
#define QOS1                        1
#define MQTT_TOPIC_BUFFER_SIZE      64
#define MQTT_MESSAGE_BUFFER_SIZE    512
#define INDICATOR_BLINK_TICKS       (TX_TIMER_TICKS_PER_SECOND / 3)
/* Declare event flag, which is used in this demo. */
TX_EVENT_FLAGS_GROUP                mqtt_app_flag;
#define DEMO_MESSAGE_EVENT          1
#define DEMO_ALL_EVENTS             3
/* Declare buffers to hold message and topic. */
static UCHAR message_buffer[MQTT_MESSAGE_BUFFER_SIZE];
static UCHAR topic_buffer[MQTT_TOPIC_BUFFER_SIZE];
/* Declare the disconnect notify function. */
static VOID my_disconnect_func(NXD_MQTT_CLIENT *client_ptr)
{
    NX_PARAMETER_NOT_USED(client_ptr);
    printf("client disconnected from server\n");
}

static VOID my_notify_func(NXD_MQTT_CLIENT* client_ptr, UINT number_of_messages)
{
    UINT flag_status;

    NX_PARAMETER_NOT_USED(client_ptr);
    flag_status = tx_event_flags_set(&mqtt_app_flag, DEMO_MESSAGE_EVENT, TX_OR);
    printf("[MQTT][NOTIFY] messages=%u tx_event_flags_set=0x%08x\r\n",
           number_of_messages, flag_status);
    return;
}

static UINT     left_signal_on;
static UINT     right_signal_on;
static UINT     brake_active;
static UINT     indicator_blink_state = 1;
static ULONG    indicator_last_toggle_ticks;

static UINT read_boolean_value(const CHAR* message, const CHAR* key, UINT* value)
{
    const CHAR* key_ptr;
    const CHAR* value_ptr;
    CHAR lhs;
    CHAR rhs;
    UINT i;

    static const CHAR token_true[] = "true";
    static const CHAR token_true_quoted[] = "\"true\"";
    static const CHAR token_active[] = "active";
    static const CHAR token_active_quoted[] = "\"active\"";
    static const CHAR token_false[] = "false";
    static const CHAR token_false_quoted[] = "\"false\"";
    static const CHAR token_inactive[] = "inactive";
    static const CHAR token_inactive_quoted[] = "\"inactive\"";

    if ((message == NX_NULL) || (key == NX_NULL) || (value == NX_NULL))
    {
        return NX_NOT_SUCCESSFUL;
    }

    key_ptr = strstr(message, key);
    if (key_ptr == NX_NULL)
    {
        return NX_NOT_SUCCESSFUL;
    }

    value_ptr = strchr(key_ptr, ':');
    if (value_ptr == NX_NULL)
    {
        return NX_NOT_SUCCESSFUL;
    }
    value_ptr++;

    while ((*value_ptr == ' ') || (*value_ptr == '\t') || (*value_ptr == '\r') || (*value_ptr == '\n'))
    {
        value_ptr++;
    }

    /* Case-insensitive token matching to accept ACTIVE/INACTIVE/TRUE/FALSE variants. */
    i = 0;
    while ((token_true[i] != 0) && (value_ptr[i] != 0))
    {
        lhs = value_ptr[i];
        rhs = token_true[i];
        if ((lhs >= 'A') && (lhs <= 'Z')) { lhs = (CHAR)(lhs + ('a' - 'A')); }
        if (lhs != rhs) { break; }
        i++;
    }
    if ((token_true[i] == 0) || (strncmp(value_ptr, "1", 1) == 0))
    {
        *value = 1;
        return NX_SUCCESS;
    }

    i = 0;
    while ((token_true_quoted[i] != 0) && (value_ptr[i] != 0))
    {
        lhs = value_ptr[i];
        rhs = token_true_quoted[i];
        if ((lhs >= 'A') && (lhs <= 'Z')) { lhs = (CHAR)(lhs + ('a' - 'A')); }
        if ((rhs >= 'A') && (rhs <= 'Z')) { rhs = (CHAR)(rhs + ('a' - 'A')); }
        if (lhs != rhs) { break; }
        i++;
    }
    if (token_true_quoted[i] == 0)
    {
        *value = 1;
        return NX_SUCCESS;
    }

    i = 0;
    while ((token_active[i] != 0) && (value_ptr[i] != 0))
    {
        lhs = value_ptr[i];
        rhs = token_active[i];
        if ((lhs >= 'A') && (lhs <= 'Z')) { lhs = (CHAR)(lhs + ('a' - 'A')); }
        if (lhs != rhs) { break; }
        i++;
    }
    if (token_active[i] == 0)
    {
        *value = 1;
        return NX_SUCCESS;
    }

    i = 0;
    while ((token_active_quoted[i] != 0) && (value_ptr[i] != 0))
    {
        lhs = value_ptr[i];
        rhs = token_active_quoted[i];
        if ((lhs >= 'A') && (lhs <= 'Z')) { lhs = (CHAR)(lhs + ('a' - 'A')); }
        if ((rhs >= 'A') && (rhs <= 'Z')) { rhs = (CHAR)(rhs + ('a' - 'A')); }
        if (lhs != rhs) { break; }
        i++;
    }
    if (token_active_quoted[i] == 0)
    {
        *value = 1;
        return NX_SUCCESS;
    }

    i = 0;
    while ((token_false[i] != 0) && (value_ptr[i] != 0))
    {
        lhs = value_ptr[i];
        rhs = token_false[i];
        if ((lhs >= 'A') && (lhs <= 'Z')) { lhs = (CHAR)(lhs + ('a' - 'A')); }
        if (lhs != rhs) { break; }
        i++;
    }
    if ((token_false[i] == 0) || (strncmp(value_ptr, "0", 1) == 0))
    {
        *value = 0;
        return NX_SUCCESS;
    }

    i = 0;
    while ((token_false_quoted[i] != 0) && (value_ptr[i] != 0))
    {
        lhs = value_ptr[i];
        rhs = token_false_quoted[i];
        if ((lhs >= 'A') && (lhs <= 'Z')) { lhs = (CHAR)(lhs + ('a' - 'A')); }
        if ((rhs >= 'A') && (rhs <= 'Z')) { rhs = (CHAR)(rhs + ('a' - 'A')); }
        if (lhs != rhs) { break; }
        i++;
    }
    if (token_false_quoted[i] == 0)
    {
        *value = 0;
        return NX_SUCCESS;
    }

    i = 0;
    while ((token_inactive[i] != 0) && (value_ptr[i] != 0))
    {
        lhs = value_ptr[i];
        rhs = token_inactive[i];
        if ((lhs >= 'A') && (lhs <= 'Z')) { lhs = (CHAR)(lhs + ('a' - 'A')); }
        if (lhs != rhs) { break; }
        i++;
    }
    if (token_inactive[i] == 0)
    {
        *value = 0;
        return NX_SUCCESS;
    }

    i = 0;
    while ((token_inactive_quoted[i] != 0) && (value_ptr[i] != 0))
    {
        lhs = value_ptr[i];
        rhs = token_inactive_quoted[i];
        if ((lhs >= 'A') && (lhs <= 'Z')) { lhs = (CHAR)(lhs + ('a' - 'A')); }
        if ((rhs >= 'A') && (rhs <= 'Z')) { rhs = (CHAR)(rhs + ('a' - 'A')); }
        if (lhs != rhs) { break; }
        i++;
    }
    if (token_inactive_quoted[i] == 0)
    {
        *value = 0;
        return NX_SUCCESS;
    }

    return NX_NOT_SUCCESSFUL;
}

static VOID apply_led_state(UINT left, UINT right, UINT brake)
{
    ULONG now_ticks = tx_time_get();

    if ((ULONG)(now_ticks - indicator_last_toggle_ticks) >= INDICATOR_BLINK_TICKS)
    {
        indicator_last_toggle_ticks = now_ticks;
        indicator_blink_state = (UINT)!indicator_blink_state;
    }

    if (left && indicator_blink_state)
    {
        CLOUD_LED_ON();
    }
    else
    {
        CLOUD_LED_OFF();
    }

    if (right && indicator_blink_state)
    {
        WIFI_LED_ON();
    }
    else
    {
        WIFI_LED_OFF();
    }

    if (brake)
    {
        USER_LED_ON();
    }
    else
    {
        USER_LED_OFF();
    }
}

static void eclipsetx_thread_entry(ULONG parameter)
{
    UINT status;
    UINT someip_status;
    UINT receive_status;
    UINT left_status;
    UINT right_status;
    UINT brake_status;
    UINT button_a_pressed;
    UINT button_b_pressed;
    UINT someip_remote_has_data;
    UINT someip_remote_left;
    UINT someip_remote_right;
    UINT someip_remote_brake;
    UINT someip_remote_button_a;
    UINT someip_remote_button_b;
    UINT keep_running;
    ULONG stop_hold_start_ticks;

    printf("Starting Eclipse ThreadX thread\r\n\r\n");
    NX_PARAMETER_NOT_USED(parameter);

    // Initialize the network
    if ((status = wwd_network_init(WIFI_SSID, WIFI_PASSWORD, WIFI_MODE)))
    {
        printf("ERROR: Failed to initialize the network (0x%08x)\r\n", status);
    }
    
    status = wwd_network_connect();
    printf("network connect status (0x%08x)\r\n", status);

    someip_status = someip_vehicle_signals_init();
    printf("[SOMEIP][INIT] status=0x%08x\r\n", someip_status);

    NX_IP *test = &nx_ip;
    if (test == NULL) {
        screen_print("E2", L3);
    }
//UINT status;
    //NXD_ADDRESS server_ip;
    //ULONG events;
    //UINT topic_length, message_length;


    /* Create MQTT client instance. */
    status = nxd_mqtt_client_create(&mqtt_client, "my_client",
        CLIENT_ID_STRING, STRLEN(CLIENT_ID_STRING), &nx_ip, &nx_pool[0],
        (VOID*)mqtt_client_stack, sizeof(mqtt_client_stack),
        MQTT_THREAD_PRIORITY, NX_NULL, 0);
    printf("[MQTT][INIT] nxd_mqtt_client_create status=0x%08x\r\n", status);

    //status = nxd_mqtt_client_create(&mqtt_client, "my_client", CLIENT_ID_STRING, STRLEN(CLIENT_ID_STRING),
    //                                /*ip_ptr, pool_ptr*/&nx_ip, &nx_pool[2], (VOID*)mqtt_client_stack, sizeof(mqtt_client_stack), 
    //                               MQTT_THREAD_PRIORTY, NX_NULL, 0);
    
    NXD_ADDRESS server_ip;
    ULONG events;
    UINT topic_length, message_length;

    nxd_mqtt_client_disconnect_notify_set(&mqtt_client, my_disconnect_func);

    status = tx_event_flags_create(&mqtt_app_flag, "my app event");
    printf("[MQTT][INIT] tx_event_flags_create status=0x%08x\r\n", status);
    server_ip.nxd_ip_version = 4;
    server_ip.nxd_ip_address.v4 = LOCAL_SERVER_ADDRESS;
    printf("[MQTT][INIT] broker=%lu.%lu.%lu.%lu:%u\r\n",
           (ULONG)((LOCAL_SERVER_ADDRESS >> 24) & 0xFF),
           (ULONG)((LOCAL_SERVER_ADDRESS >> 16) & 0xFF),
           (ULONG)((LOCAL_SERVER_ADDRESS >> 8) & 0xFF),
           (ULONG)(LOCAL_SERVER_ADDRESS & 0xFF),
           (UINT)NXD_MQTT_PORT);
    printf("[MQTT][INIT] buffers topic=%u message=%u\r\n",
           (UINT)sizeof(topic_buffer), (UINT)sizeof(message_buffer));

    /* Start the connection to the server. */
    status = nxd_mqtt_client_connect(&mqtt_client, &server_ip, NXD_MQTT_PORT, MQTT_KEEP_ALIVE_TIMER, 0, NX_WAIT_FOREVER);
    status = nxd_mqtt_client_connect(&mqtt_client, &server_ip, NXD_MQTT_PORT,
                                     MQTT_KEEP_ALIVE_TIMER, MQTT_CLEAN_SESSION, NX_WAIT_FOREVER);

    if (status != TX_SUCCESS) {
        printf("nxd_mqtt_client_connect (0x%08x)\r\n", status);
    }
    else
    {
        printf("[MQTT][INIT] connected\r\n");
    }

    // if (status)
    // {
    //     printf("Error in creating MQTT client: 0x%02x\n", status);
    //     error_counter++;
    // }
    //ScreenShowValue(error_counter);
    
    

#ifdef NXD_MQTT_OVER_WEBSOCKET
    status = nxd_mqtt_client_websocket_set(&mqtt_client, (UCHAR *)"test.mosquitto.org", sizeof("test.mosquitto.org") - 1,
                                           (UCHAR *)"/mqtt", sizeof("/mqtt") - 1);
    if (status)
    {
        printf("Error in setting MQTT over WebSocket: 0x%02x\r\n", status);
    }
#endif /* NXD_MQTT_OVER_WEBSOCKET */

    /* Register the disconnect notification function. */
    //nxd_mqtt_client_disconnect_notify_set(&mqtt_client, my_disconnect_func);
    
    /* Create an event flag for this demo. */
    // status = tx_event_flags_create(&mqtt_app_flag, "my app event");
    // if(status){
    //     error_counter++;
    //     //ScreenShowText("tx_event_flags_cr");
    // }
    
    //ScreenShowValue(error_counter);


    //server_ip.nxd_ip_version = 4;
    //server_ip.nxd_ip_address.v4 = LOCAL_SERVER_ADDRESS;

    /* Start the connection to the server. */
    //status = nxd_mqtt_client_connect(&mqtt_client, &server_ip, NXD_MQTT_PORT, MQTT_KEEP_ALIVE_TIMER, 0, NX_WAIT_FOREVER);

    for (int i=0; i<3; i++)
        ScreenShowSensValue();
    /* Start the connection to the server. */
    //status = nxd_mqtt_client_connect(&mqtt_client, &server_ip, NXD_MQTT_PORT, 
    //                                 MQTT_KEEP_ALIVE_TIMER, 0, NX_WAIT_FOREVER);

    // if(status){
    //     error_counter++;
    //     printf("Error in creating nxd_mqtt_client_connect.......");
    //     //ScreenShowText("nxd_mqtt_client_connect.......");
    // }
    // ScreenShowValue(error_counter);

    lsm6dsl_data_t lsm6dsl_data;
    CHAR screen_value_str[96];
    CHAR signal_state_str[21];
    CHAR someip_remote_state_str[22];
    CHAR* left_section;
    CHAR* right_section;
    CHAR* brake_section;

    /* Subscribe and register receive callback before entering the publish loop. */
    status = nxd_mqtt_client_subscribe(&mqtt_client, SUBSCRIBE_TOPIC, STRLEN(SUBSCRIBE_TOPIC), QOS0);
    if (status != NX_SUCCESS)
    {
        printf("nxd_mqtt_client_subscribe failed (0x%08x)\r\n", status);
    }
    else
    {
        printf("[MQTT][SUB] subscribed topic=%s qos=%u\r\n", SUBSCRIBE_TOPIC, QOS0);
    }

    status = nxd_mqtt_client_receive_notify_set(&mqtt_client, my_notify_func);
    if (status != NX_SUCCESS)
    {
        printf("nxd_mqtt_client_receive_notify_set failed (0x%08x)\r\n", status);
    }
    else
    {
        printf("[MQTT][SUB] receive notify registered\r\n");
    }

    printf("Go for Loop forever\r\n");
    keep_running = 1U;
    stop_hold_start_ticks = 0U;
    while (keep_running != 0U)
    {
        INT gyro_x;
        INT gyro_y;
        INT gyro_z;
        ULONG now_ticks;

        lsm6dsl_data = lsm6dsl_data_read();
        gyro_x = (INT)lsm6dsl_data.angular_rate_mdps[0];
        gyro_y = (INT)lsm6dsl_data.angular_rate_mdps[1];
        gyro_z = (INT)lsm6dsl_data.angular_rate_mdps[2];

        snprintf(screen_value_str, sizeof(screen_value_str),
                 "{\"X\":%d,\"Y\":%d,\"Z\":%d}",
                 gyro_x,
                 gyro_y,
                 gyro_z);

        ssd1306_SetCursor(0, 20);
        ssd1306_WriteString(screen_value_str, Font_7x10, White);
        ssd1306_UpdateScreen();

        status = nxd_mqtt_client_publish(&mqtt_client, TOPIC_NAME,
                                         STRLEN(TOPIC_NAME), screen_value_str,
                                         (UINT)strlen(screen_value_str), 0, QOS1, NX_WAIT_FOREVER);
        if (status != NX_SUCCESS)
        {
            printf("nxd_mqtt_client_publish failed (0x%08x)\r\n", status);
        }
        else
        {
            printf("Published: %s\r\n", screen_value_str);
        }

        status = tx_event_flags_get(&mqtt_app_flag, DEMO_MESSAGE_EVENT, TX_OR_CLEAR, &events, TX_NO_WAIT);
        if ((status == TX_SUCCESS) && (events & DEMO_MESSAGE_EVENT))
        {
            printf("[MQTT][RX] event flags=0x%08lx\r\n", events);
            while (1)
            {
                receive_status = nxd_mqtt_client_message_get(&mqtt_client, topic_buffer, sizeof(topic_buffer), &topic_length,
                                                             message_buffer, sizeof(message_buffer), &message_length);
                if (receive_status != NXD_MQTT_SUCCESS)
                {
                    printf("[MQTT][RX] message_get status=0x%08x\r\n", receive_status);
                    break;
                }

                if (topic_length >= sizeof(topic_buffer))
                {
                    topic_length = sizeof(topic_buffer) - 1;
                }
                if (message_length >= sizeof(message_buffer))
                {
                    message_length = sizeof(message_buffer) - 1;
                }
                topic_buffer[topic_length] = 0;
                message_buffer[message_length] = 0;
                printf("[MQTT][RX] topic=%s (len=%u) payload_len=%u\r\n",
                       topic_buffer, topic_length, message_length);
                printf("[MQTT][RX] payload=%s\r\n", message_buffer);

                if (strcmp((CHAR*)topic_buffer, SUBSCRIBE_TOPIC) == 0)
                {
                    left_status = read_boolean_value((CHAR*)message_buffer, "\"Vehicle.Body.Lights.DirectionIndicator.Left.IsSignaling\"", &left_signal_on);
                    if (left_status != NX_SUCCESS)
                    {
                        left_section = strstr((CHAR*)message_buffer, "\"Left\"");
                        if (left_section != NX_NULL)
                        {
                            left_status = read_boolean_value(left_section, "\"IsSignaling\"", &left_signal_on);
                        }
                    }

                    right_status = read_boolean_value((CHAR*)message_buffer, "\"Vehicle.Body.Lights.DirectionIndicator.Right.IsSignaling\"", &right_signal_on);
                    if (right_status != NX_SUCCESS)
                    {
                        right_section = strstr((CHAR*)message_buffer, "\"Right\"");
                        if (right_section != NX_NULL)
                        {
                            right_status = read_boolean_value(right_section, "\"IsSignaling\"", &right_signal_on);
                        }
                    }

                    brake_status = read_boolean_value((CHAR*)message_buffer, "\"Vehicle.Body.Lights.Brake.IsActive\"", &brake_active);
                    if (brake_status != NX_SUCCESS)
                    {
                        brake_section = strstr((CHAR*)message_buffer, "\"Brake\"");
                        if (brake_section != NX_NULL)
                        {
                            brake_status = read_boolean_value(brake_section, "\"IsActive\"", &brake_active);
                        }
                    }

                    printf("[MQTT][PARSE] left status=0x%08x value=%u\r\n", left_status, left_signal_on);
                    printf("[MQTT][PARSE] right status=0x%08x value=%u\r\n", right_status, right_signal_on);
                    printf("[MQTT][PARSE] brake status=0x%08x value=%u\r\n", brake_status, brake_active);
                    apply_led_state(left_signal_on, right_signal_on, brake_active);
                    printf("[MQTT][LED] applied L=%u R=%u B=%u\r\n", left_signal_on, right_signal_on, brake_active);
                    snprintf(signal_state_str, sizeof(signal_state_str), "L:%u R:%u B:%u", left_signal_on, right_signal_on, brake_active);
                    ssd1306_SetCursor(0, 40);
                    ssd1306_WriteString(signal_state_str, Font_7x10, White);
                    ssd1306_UpdateScreen();
                }
                else
                {
                    printf("[MQTT][RX] ignored topic (expected=%s)\r\n", SUBSCRIBE_TOPIC);
                }

                printf("topic = %s, message = %s\r\n", topic_buffer, message_buffer);
            }
        }
        else if (status != TX_NO_EVENTS)
        {
            printf("[MQTT][RX] tx_event_flags_get status=0x%08x\r\n", status);
        }

        button_a_pressed = BUTTON_A_IS_PRESSED ? 1U : 0U;
        button_b_pressed = BUTTON_B_IS_PRESSED ? 1U : 0U;

        someip_vehicle_signals_poll_receive();
        someip_vehicle_signals_get_remote_state(&someip_remote_has_data,
                                                &someip_remote_left,
                                                &someip_remote_right,
                                                &someip_remote_brake,
                                                &someip_remote_button_a,
                                                &someip_remote_button_b);
        if (someip_remote_has_data != 0U)
        {
            snprintf(someip_remote_state_str, sizeof(someip_remote_state_str),
                     "SIP L%uR%uB%u A%uB%u   ",
                     someip_remote_left,
                     someip_remote_right,
                     someip_remote_brake,
                     someip_remote_button_a,
                     someip_remote_button_b);
        }
        else
        {
            snprintf(someip_remote_state_str, sizeof(someip_remote_state_str),
                     "SIP waiting...       ");
        }
        ssd1306_SetCursor(0, 54);
        ssd1306_WriteString(someip_remote_state_str, Font_6x8, White);
        ssd1306_UpdateScreen();

        someip_vehicle_signals_publish(left_signal_on,
                                       right_signal_on,
                                       brake_active,
                                       button_a_pressed,
                                       button_b_pressed);

        /* Hold both buttons for 2s to gracefully unsubscribe/disconnect. */
        now_ticks = tx_time_get();
        if ((button_a_pressed != 0U) && (button_b_pressed != 0U))
        {
            if (stop_hold_start_ticks == 0U)
            {
                stop_hold_start_ticks = now_ticks;
            }
            else if ((ULONG)(now_ticks - stop_hold_start_ticks) >= MQTT_STOP_HOLD_TICKS)
            {
                printf("[MQTT][CTRL] stop requested, starting cleanup\r\n");
                keep_running = 0U;
                continue;
            }
        }
        else
        {
            stop_hold_start_ticks = 0U;
        }

        /* Keep indicator outputs blinking even when no new MQTT message arrives. */
        apply_led_state(left_signal_on, right_signal_on, brake_active);
        tx_thread_sleep(20);
    }

    /* Now unsubscribe the topic. */
    status = nxd_mqtt_client_unsubscribe(&mqtt_client, SUBSCRIBE_TOPIC, STRLEN(SUBSCRIBE_TOPIC));
    printf("[MQTT][CLEANUP] unsubscribe status=0x%08x\r\n", status);

    /* Disconnect from the broker. */
    status = nxd_mqtt_client_disconnect(&mqtt_client);
    printf("[MQTT][CLEANUP] disconnect status=0x%08x\r\n", status);

    /* Delete the client instance, release all the resources. */
    status = nxd_mqtt_client_delete(&mqtt_client);
    printf("[MQTT][CLEANUP] delete status=0x%08x\r\n", status);
    someip_vehicle_signals_deinit();
    tx_thread_sleep(1000);

    ssd1306_SetCursor(20, 30);
    ssd1306_WriteString("Bye", Font_11x18,White);
    ssd1306_UpdateScreen();

    return;

}

void tx_application_define(void* first_unused_memory)
{
    systick_interval_set(TX_TIMER_TICKS_PER_SECOND);

    // Create ThreadX thread
    UINT status = tx_thread_create(&eclipsetx_thread,
        "Eclipse ThreadX Thread",
        eclipsetx_thread_entry,
        0,
        eclipsetx_thread_stack,
        ECLIPSETX_THREAD_STACK_SIZE,
        ECLIPSETX_THREAD_PRIORITY,
        ECLIPSETX_THREAD_PRIORITY,
        TX_NO_TIME_SLICE,
        TX_AUTO_START);

    if (status != TX_SUCCESS)
    {
        printf("ERROR: Eclipse ThreadX thread creation failed\r\n");
    }

    printf("\r\nleaving tx_application_define\r\n");
}

int main(void)
{
    // Initialize the board
    board_init();
    Screen_On();

    for(int i = 0; i < 5; i++){
        
        ScreenShowSensValue();

    }

    lsm6dsl_data_t lsm6dsl_data = lsm6dsl_data_read();
    printf("Tempetature: %d\r\n", (int)lsm6dsl_data.temperature_degC);

    printf("\r\n before kernel enter");

    // Enter the ThreadX kernel
    tx_kernel_enter();

    return 0;
}

void ScreenShowSensValue(){

        lsm6dsl_data_t lsm6dsl_data = lsm6dsl_data_read();
        //printf("Accelerometer: %d, %d, %d\r\n", (int)lsm6dsl_data.acceleration_mg[0], (int)lsm6dsl_data.acceleration_mg[1], (int)lsm6dsl_data.acceleration_mg[2]);
        //printf("Accelerometer: %f, %f, %f\r\n", mysensor_data_t.accel_x, mysensor_data_t.accel_y, mysensor_data_t.accel_z);
        char screen_value_str[21];
        //sprintf(screen_value_str, "%6d", absolute_acceleration);

        sprintf(screen_value_str, "%4d; %4d; %4d ", (int)lsm6dsl_data.acceleration_mg[0], (int)lsm6dsl_data.acceleration_mg[1], (int)lsm6dsl_data.acceleration_mg[2]);

        ssd1306_SetCursor(0, 20);
        ssd1306_WriteString(screen_value_str, Font_7x10, White);
        ssd1306_UpdateScreen();
}

void ScreenShowValue(ULONG value){
    char value_str[12];
    sprintf(value_str, "%lu", value);
    ssd1306_SetCursor(20, 30);
    ssd1306_WriteString(value_str, Font_11x18, White);
    ssd1306_UpdateScreen();
}

void ScreenShowText(char* text){
    ssd1306_SetCursor(0, 40);
    ssd1306_WriteString(text, Font_11x18, White);
    ssd1306_UpdateScreen();
}
