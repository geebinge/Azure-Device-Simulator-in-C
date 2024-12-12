// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// CAVEAT: This sample is to demonstrate azure IoT client concepts only and is not a guide design principles or style
// Checking of return codes and error values shall be omitted for brevity.  Please practice sound engineering practices
// when writing production code.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cjson/cJSON.h>

#include "iothub.h"
#include "iothub_device_client_ll.h"
#include "iothub_client_options.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/shared_util_options.h"


/* This sample uses the _LL APIs of iothub_client for example purposes.
Simply changing the using the convenience layer (functions not having _LL)
and removing calls to _DoWork will yield the same results. */

// The protocol you wish to use should be uncommented
//
#define SAMPLE_MQTT
//#define SAMPLE_MQTT_OVER_WEBSOCKETS
//#define SAMPLE_AMQP
//#define SAMPLE_AMQP_OVER_WEBSOCKETS
//#define SAMPLE_HTTP

// If using an OpenSSL ENGINE uncomment and modify the line below
//#define SAMPLE_OPENSSL_ENGINE "pkcs11"

#ifdef SAMPLE_MQTT
    #include "iothubtransportmqtt.h"
#endif // SAMPLE_MQTT
#ifdef SAMPLE_MQTT_OVER_WEBSOCKETS
    #include "iothubtransportmqtt_websockets.h"
#endif // SAMPLE_MQTT_OVER_WEBSOCKETS
#ifdef SAMPLE_AMQP
    #include "iothubtransportamqp.h"
#endif // SAMPLE_AMQP
#ifdef SAMPLE_AMQP_OVER_WEBSOCKETS
    #include "iothubtransportamqp_websockets.h"
#endif // SAMPLE_AMQP_OVER_WEBSOCKETS
#ifdef SAMPLE_HTTP
    #include "iothubtransporthttp.h"
#endif // SAMPLE_HTTP

#ifdef SET_TRUSTED_CERT_IN_SAMPLES
    #include "certs.h"
#endif // SET_TRUSTED_CERT_IN_SAMPLES

static const char* connectionStringPart1 = "HostName=";
static const char* connectionStringPart2 = ";DeviceId="; 
static const char* connectionStringPart3 = ";x509=true";
static const char* def_file = "/etc/aziot/sim_config.json";


#ifdef SAMPLE_OPENSSL_ENGINE
static const char* opensslEngine = SAMPLE_OPENSSL_ENGINE;
static const OPTION_OPENSSL_KEY_TYPE x509_key_from_engine = KEY_TYPE_ENGINE;
#endif

#define MESSAGE_COUNT        5
static bool g_continueRunning = true;
static size_t g_message_count_send_confirmations = 0;

typedef struct EVENT_INSTANCE_TAG
{
    IOTHUB_MESSAGE_HANDLE messageHandle;
    size_t messageTrackingId;  // For tracking the messages within the user callback.
} EVENT_INSTANCE;

static void send_confirm_callback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    (void)userContextCallback;
    // When a message is sent this callback will get invoked
    g_message_count_send_confirmations++;
    (void)printf("Confirmation callback received for message %zu with result %s\r\n", g_message_count_send_confirmations, MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
}


int main(void)
{
    IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol;
    IOTHUB_MESSAGE_HANDLE message_handle;
    size_t messages_sent = 0;
    const char* telemetry_msg = "test_message";

	long lSize;
    // open the file 
	FILE *fp = fopen(def_file, "r"); 
	if (fp == NULL) { 
		printf("Error: Unable to open the file %s\n", def_file); 
		return 1; 
	} else { 
		printf("Open %s\n", def_file); 
	}

	fseek( fp , 0L , SEEK_END);
	lSize = ftell( fp );
	rewind( fp );

	// read the file contents into a string 
	char *sim_config = malloc(lSize+1 ); 
	fread(sim_config, 1, lSize+1, fp); 
	fclose(fp); 

    // parse the JSON data 
    cJSON *json = cJSON_Parse(sim_config); 
    if (json == NULL) { 
        const char *error_ptr = cJSON_GetErrorPtr(); 
        if (error_ptr != NULL) { 
            printf("Error: %s\n", error_ptr); 
        } 
        cJSON_Delete(json); 
        return 1; 
    } 
  
    // access the JSON data 
    cJSON *HostName = cJSON_GetObjectItemCaseSensitive(json, "HostName"); 
    if (cJSON_IsString(HostName) && (HostName->valuestring != NULL)) { 
        printf("HostName: %s\n", HostName->valuestring); 
    } 

    cJSON *IDScope = cJSON_GetObjectItemCaseSensitive(json, "IDScope"); 
    if (cJSON_IsString(IDScope) && (IDScope->valuestring != NULL)) { 
        printf("IDScope: %s\n", IDScope->valuestring); 
    } 
    cJSON *DeviceID = cJSON_GetObjectItemCaseSensitive(json, "DeviceID"); 
    if (cJSON_IsString(DeviceID) && (DeviceID->valuestring != NULL)) { 
        printf("DeviceID: %s\n", DeviceID->valuestring); 
    } 

    cJSON *CERTFile = cJSON_GetObjectItemCaseSensitive(json, "CERTFile"); 
    if (cJSON_IsString(CERTFile) && (CERTFile->valuestring != NULL)) { 
        printf("CERTFile: %s\n", CERTFile->valuestring); 
    } 
    cJSON *KeyFile = cJSON_GetObjectItemCaseSensitive(json, "KeyFile"); 
    if (cJSON_IsString(KeyFile) && (KeyFile->valuestring != NULL)) { 
        printf("KeyFile: %s\n", KeyFile->valuestring); 
    } 

    cJSON *MSG_CNT = cJSON_GetObjectItemCaseSensitive(json, "MSG_CNT"); 
    if (cJSON_IsNumber(MSG_CNT)) { 
        printf("MSG_CNT: %d\n", MSG_CNT->valueint); 
    } 

    cJSON *MSG_SLP = cJSON_GetObjectItemCaseSensitive(json, "MSG_SLP"); 
    if (cJSON_IsNumber(MSG_SLP)) { 
        printf("MSG_SLP: %d\n", MSG_SLP->valueint); 
    } 

 
	char* connectionString =  malloc(strlen(connectionStringPart1)+strlen(connectionStringPart2)+strlen(connectionStringPart3)+strlen(DeviceID->valuestring)+strlen(HostName->valuestring)+1);
	strcpy(connectionString,connectionStringPart1);
	strcat(connectionString,HostName->valuestring);
	strcat(connectionString,connectionStringPart2);
	strcat(connectionString,DeviceID->valuestring);
	strcat(connectionString,connectionStringPart3);
 
    printf("connectionString: %s\n", connectionString); 
 
	/***** read public chain *****/
 
	fp = fopen(CERTFile->valuestring, "r"); 
	if (fp == NULL) { 
		printf("Error: Unable to open the file %s\n", CERTFile->valuestring); 
		return 1; 
	} else { 
		printf("Open %s\n", CERTFile->valuestring); 
	}

	fseek( fp , 0L , SEEK_END);
	lSize = ftell( fp );
	rewind( fp );

	char *x509certificate = malloc(lSize+1 ); 
	fread(x509certificate, 1, lSize+1, fp); 
	fclose(fp);

	/***** read private key *****/
	

	fp = fopen(KeyFile->valuestring, "r"); 
	if (fp == NULL) { 
		printf("Error: Unable to open the file %s\n", KeyFile->valuestring); 
		return 1; 
	} else { 
		printf("Open %s\n", KeyFile->valuestring); 
	}

	fseek( fp , 0L , SEEK_END);
	lSize = ftell( fp );
	rewind( fp );

	char *x509privatekey = malloc(lSize+1 ); 
	fread(x509privatekey, 1, lSize+1, fp); 
	fclose(fp);

    // Select the Protocol to use with the connection
#ifdef SAMPLE_MQTT
    protocol = MQTT_Protocol;
#endif // SAMPLE_MQTT
#ifdef SAMPLE_MQTT_OVER_WEBSOCKETS
    protocol = MQTT_WebSocket_Protocol;
#endif // SAMPLE_MQTT_OVER_WEBSOCKETS
#ifdef SAMPLE_AMQP
    protocol = AMQP_Protocol;
#endif // SAMPLE_AMQP
#ifdef SAMPLE_AMQP_OVER_WEBSOCKETS
    protocol = AMQP_Protocol_over_WebSocketsTls;
#endif // SAMPLE_AMQP_OVER_WEBSOCKETS
#ifdef SAMPLE_HTTP
    protocol = HTTP_Protocol;
#endif // SAMPLE_HTTP

    IOTHUB_DEVICE_CLIENT_LL_HANDLE device_ll_handle;

    // Used to initialize IoTHub SDK subsystem
    (void)IoTHub_Init();

    (void)printf("Creating IoTHub handle\r\n");
    // Create the iothub handle here
    device_ll_handle = IoTHubDeviceClient_LL_CreateFromConnectionString(connectionString, protocol);
    if (device_ll_handle == NULL)
    {
        (void)printf("Failure creating IotHub device. Hint: Check your connection string.\r\n");
    }
    else
    {
        // Set any option that are necessary.
        // For available options please see the iothub_sdk_options.md documentation
        //bool traceOn = true;
        //IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_LOG_TRACE, &traceOn);

        // Setting the Trusted Certificate. This is only necessary on systems without
        // built in certificate stores.
#ifdef SET_TRUSTED_CERT_IN_SAMPLES
        IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_TRUSTED_CERT, certificates);
#endif // SET_TRUSTED_CERT_IN_SAMPLES

#if defined SAMPLE_MQTT || defined SAMPLE_MQTT_OVER_WEBSOCKETS
        //Setting the auto URL Encoder (recommended for MQTT). Please use this option unless
        //you are URL Encoding inputs yourself.
        //ONLY valid for use with MQTT
        bool urlEncodeOn = true;
        (void)IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_AUTO_URL_ENCODE_DECODE, &urlEncodeOn);
#endif
        // Set the X509 certificates in the SDK
        if (
#ifdef SAMPLE_OPENSSL_ENGINE
            (IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_OPENSSL_ENGINE, opensslEngine) != IOTHUB_CLIENT_OK) ||
            (IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_OPENSSL_PRIVATE_KEY_TYPE, &x509_key_from_engine) != IOTHUB_CLIENT_OK) ||
#endif
            (IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_X509_CERT, x509certificate) != IOTHUB_CLIENT_OK) ||
            (IoTHubDeviceClient_LL_SetOption(device_ll_handle, OPTION_X509_PRIVATE_KEY, x509privatekey) != IOTHUB_CLIENT_OK)
            )
        {
            printf("failure to set options for x509, aborting\r\n");
        }
        else
        {
            do
            {
                if (messages_sent < MSG_CNT->valueint)
                {
                    // Construct the iothub message from a string or a byte array
                    message_handle = IoTHubMessage_CreateFromString(telemetry_msg);
                    //message_handle = IoTHubMessage_CreateFromByteArray((const unsigned char*)msgText, strlen(msgText)));

                    // Set Message property
                    (void)IoTHubMessage_SetMessageId(message_handle, "MSG_ID");
                    (void)IoTHubMessage_SetCorrelationId(message_handle, "CORE_ID");
                    (void)IoTHubMessage_SetContentTypeSystemProperty(message_handle, "application%2Fjson");
                    (void)IoTHubMessage_SetContentEncodingSystemProperty(message_handle, "utf-8");

                    // Add custom properties to message
                    (void)IoTHubMessage_SetProperty(message_handle, "property_key", "property_value");

                    (void)printf("Sending message %d to IoTHub\r\n", (int)(messages_sent + 1));
                    IoTHubDeviceClient_LL_SendEventAsync(device_ll_handle, message_handle, send_confirm_callback, NULL);

                    // The message is copied to the sdk so the we can destroy it
                    IoTHubMessage_Destroy(message_handle);

                    messages_sent++;
					sleep(MSG_SLP->valueint);
                }
                else if (g_message_count_send_confirmations >= MSG_CNT->valueint)
                {
                    // After all messages are all received stop running
                    g_continueRunning = false;
                }

                IoTHubDeviceClient_LL_DoWork(device_ll_handle);
                ThreadAPI_Sleep(1);

            } while (g_continueRunning);
        }
        // Clean up the iothub sdk handle
        IoTHubDeviceClient_LL_Destroy(device_ll_handle);
    }
    // Free all the sdk subsystem
    IoTHub_Deinit();

    cJSON_Delete(json); 	
	free(connectionString);
	free(sim_config);
    return 0;
}
