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

#include "tx_api.h"
#include "nx_api.h"
#include "nxd_mqtt_client.h"

#include "board_init.h"
#include "cmsis_utils.h"
#include "screen.h"
#include "sntp_client.h"
#include "wwd_networking.h"

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
#define  LOCAL_SERVER_ADDRESS (IP_ADDRESS(192, 168, 0, 118))

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
#define MESSAGE_STRING              "This is a message. "
/* Define the priority of the MQTT internal thread. */
#define MQTT_THREAD_PRIORITY 2
/* Define the MQTT keep alive timer for 5 minutes */
#define MQTT_KEEP_ALIVE_TIMER       300
#define QOS0                        0
#define QOS1                        1
/* Declare event flag, which is used in this demo. */
TX_EVENT_FLAGS_GROUP                mqtt_app_flag;
#define DEMO_MESSAGE_EVENT          1
#define DEMO_ALL_EVENTS             3
/* Declare buffers to hold message and topic. */
static UCHAR message_buffer[NXD_MQTT_MAX_MESSAGE_LENGTH];
static UCHAR topic_buffer[NXD_MQTT_MAX_TOPIC_NAME_LENGTH];
/* Declare the disconnect notify function. */
static VOID my_disconnect_func(NXD_MQTT_CLIENT *client_ptr)
{
    NX_PARAMETER_NOT_USED(client_ptr);
    printf("client disconnected from server\n");
}

static VOID my_notify_func(NXD_MQTT_CLIENT* client_ptr, UINT number_of_messages)
{
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(number_of_messages);
    tx_event_flags_set(&mqtt_app_flag, DEMO_MESSAGE_EVENT, TX_OR);
    return;
}

static ULONG    error_counter;

static void eclipsetx_thread_entry(ULONG parameter)
{
    UINT status;

    printf("Starting Eclipse ThreadX thread\r\n\r\n");

    // Initialize the network
    if ((status = wwd_network_init(WIFI_SSID, WIFI_PASSWORD, WIFI_MODE)))
    {
        printf("ERROR: Failed to initialize the network (0x%08x)\r\n", status);
    }
    
    status = wwd_network_connect();
    printf("network connect status (0x%08x)\r\n", status);

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

    //status = nxd_mqtt_client_create(&mqtt_client, "my_client", CLIENT_ID_STRING, STRLEN(CLIENT_ID_STRING),
    //                                /*ip_ptr, pool_ptr*/&nx_ip, &nx_pool[2], (VOID*)mqtt_client_stack, sizeof(mqtt_client_stack), 
    //                               MQTT_THREAD_PRIORTY, NX_NULL, 0);
    
    NXD_ADDRESS server_ip;
    ULONG events;
    UINT topic_length, message_length;

    nxd_mqtt_client_disconnect_notify_set(&mqtt_client, my_disconnect_func);

    status = tx_event_flags_create(&mqtt_app_flag, "my app event");
    server_ip.nxd_ip_version = 4;
    server_ip.nxd_ip_address.v4 = LOCAL_SERVER_ADDRESS;

    /* Start the connection to the server. */
    status = nxd_mqtt_client_connect(&mqtt_client, &server_ip, NXD_MQTT_PORT, MQTT_KEEP_ALIVE_TIMER, 0, NX_WAIT_FOREVER);

    if (status != TX_SUCCESS) {
        printf("nxd_mqtt_client_connect (0x%08x)\r\n", status);
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
        error_counter++;
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

    lsm6dsl_data = lsm6dsl_data_read();
    char screen_value_str[21];
        //sprintf(screen_value_str, "%6d", absolute_acceleration);

    sprintf(screen_value_str, "%4d; %4d; %4d ", (int)lsm6dsl_data.acceleration_mg[0], (int)lsm6dsl_data.acceleration_mg[1], (int)lsm6dsl_data.acceleration_mg[2]);

    nxd_mqtt_client_publish(&mqtt_client, TOPIC_NAME,
        STRLEN(TOPIC_NAME), (CHAR*)screen_value_str, 
        STRLEN(MESSAGE_STRING), 0, QOS1, NX_WAIT_FOREVER);
    printf("Publish a message with QoS level 1: %s\n", screen_value_str);
    printf("Go for Loop forever\r\n");
    while(1){
        //ScreenShowSensValue();
        //printf("Loop forever\r\n");
        lsm6dsl_data = lsm6dsl_data_read();
        //sprintf(screen_value_str, "%6d", absolute_acceleration);

        sprintf(screen_value_str, "%4d; %4d; %4d ", (int)lsm6dsl_data.acceleration_mg[0], (int)lsm6dsl_data.acceleration_mg[1], (int)lsm6dsl_data.acceleration_mg[2]);
        ssd1306_SetCursor(0, 20);
        ssd1306_WriteString(screen_value_str, Font_7x10, White);
        ssd1306_UpdateScreen();
        nxd_mqtt_client_publish(&mqtt_client, TOPIC_NAME,
                    STRLEN(TOPIC_NAME), (CHAR*)screen_value_str, 
                    STRLEN(MESSAGE_STRING), 0, QOS1, NX_WAIT_FOREVER);
        printf("Publish a message with QoS level 1: %s\n", screen_value_str);
        tx_thread_sleep(20);
    }

    
    /* Subscribe to the topic with QoS level 0. */
    status = nxd_mqtt_client_subscribe(&mqtt_client, TOPIC_NAME, STRLEN(TOPIC_NAME), QOS0);
    if(status)
        error_counter++;
    
    /* Set the receive notify function. */
    status = nxd_mqtt_client_receive_notify_set(&mqtt_client, my_notify_func);
    if(status)
        error_counter++;
    printf("Error: %lu",error_counter);
    /* Publish a message with QoS Level 1. */
    status = nxd_mqtt_client_publish(&mqtt_client, TOPIC_NAME, STRLEN(TOPIC_NAME),
                                     (CHAR*)MESSAGE_STRING, STRLEN(MESSAGE_STRING), 0, QOS1, NX_WAIT_FOREVER);


    /* Now wait for the broker to publish the message. */

    tx_event_flags_get(&mqtt_app_flag, DEMO_ALL_EVENTS, TX_OR_CLEAR, &events, TX_WAIT_FOREVER);
    if(events & DEMO_MESSAGE_EVENT)
    {
        status = nxd_mqtt_client_message_get(&mqtt_client, topic_buffer, sizeof(topic_buffer), &topic_length, 
                                             message_buffer, sizeof(message_buffer), &message_length);
        if(status == NXD_MQTT_SUCCESS)
        {
            topic_buffer[topic_length] = 0;
            message_buffer[message_length] = 0;
            printf("topic = %s, message = %s\n", topic_buffer, message_buffer);
        }
    }

    /* Now unsubscribe the topic. */
    nxd_mqtt_client_unsubscribe(&mqtt_client, TOPIC_NAME, STRLEN(TOPIC_NAME));

    /* Disconnect from the broker. */
    nxd_mqtt_client_disconnect(&mqtt_client);

    /* Delete the client instance, release all the resources. */
    nxd_mqtt_client_delete(&mqtt_client);
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