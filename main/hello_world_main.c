
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_check.h"
#include "mqtt_client.h"

#include "pzem-driver.h"

#define TXD_PIN 17
#define RXD_PIN 16
#define PZEM_ADDR 0xF8

#define WIFI_SSID "Rojas_wgo_fibra"
#define WIFI_PASS "jr131415"

#define MQTT_BROKER_URI "mqtt://192.168.0.121:1883"

#define MQTT_DATA_TOPIC "energia"
#define MQTT_REGISTER_TOPIC "monitorEnergia"

static char device_mac[18];
static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_task_started = false;

static void mqtt_task(void *pvParameters)
{
    char msg[256];

    while (1)
    {
        if (mqtt_client != NULL)
        {
            if (updateValues(PZEM_ADDR))
            {
                snprintf(
                    msg,
                    sizeof(msg),
                    "{"
                    "\"tensao\": %.1f,"
                    "\"corrente\": %.3f,"
                    "\"potencia\": %.1f,"
                    "\"energiaKwh\": %.2f,"
                    "\"frequencia\": %.1f,"
                    "\"fatorPotencia\": %.2f,"
                    "\"esp32Mac\":\"%s\""
                    "}",
                    getVoltage(),
                    getCurrent(),
                    getPower(),
                    getEnergy(),
                    getFrequency(),
                    getPF(),
                    device_mac
                );

                esp_mqtt_client_publish(
                    mqtt_client,
                    MQTT_DATA_TOPIC,
                    msg,
                    0,
                    0,
                    0
                );
            }
            else
            {
                snprintf(
                    msg,
                    sizeof(msg),
                    "{\"erro\":\"falha no pzem\",\"esp32Id\":\"%s\"}",
                    device_mac
                );

                esp_mqtt_client_publish(
                    mqtt_client,
                    MQTT_DATA_TOPIC,
                    msg,
                    0,
                    0,
                    0
                );
            }
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

static void mqtt_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
)
{
    esp_mqtt_event_handle_t event = event_data;

    switch (event_id)
    {
        case MQTT_EVENT_CONNECTED:
        {
            mqtt_client = event->client;

            char registration_msg[128];

            snprintf(
                registration_msg,
                sizeof(registration_msg),
                "{\"nome\":\"esp_32_energia\",\"macAddress\":\"%s\"}",
                device_mac
            );

            esp_mqtt_client_publish(
                mqtt_client,
                MQTT_REGISTER_TOPIC,
                registration_msg,
                0,
                1,
                0
            );

            if (!mqtt_task_started)
            {
                if (xTaskCreate(
                    mqtt_task,
                    "mqtt_task",
                    4096,
                    NULL,
                    5,
                    NULL
                ) == pdPASS)
                {
                    mqtt_task_started = true;
                }
            }

            break;
        }

        default:
            break;
    }
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    ESP_ERROR_CHECK(
        esp_mqtt_client_register_event(
            client,
            ESP_EVENT_ANY_ID,
            mqtt_event_handler,
            NULL
        )
    );

    ESP_ERROR_CHECK(esp_mqtt_client_start(client));
}

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data
)
{
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_START ||
            event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            esp_wifi_connect();
        }
    }
    else if (
        event_base == IP_EVENT &&
        event_id == IP_EVENT_STA_GOT_IP
    )
    {
        mqtt_app_start();
    }
}

static void wifi_init(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL,
            NULL
        )
    );

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(
        esp_wifi_set_config(WIFI_IF_STA, &wifi_config)
    );

    ESP_ERROR_CHECK(esp_wifi_start());
}

static void get_device_mac(void)
{
    uint8_t mac[6];

    ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_STA));

    snprintf(
        device_mac,
        sizeof(device_mac),
        "%02X:%02X:%02X:%02X:%02X:%02X",
        mac[0],
        mac[1],
        mac[2],
        mac[3],
        mac[4],
        mac[5]
    );
}

void app_main(void)
{
    get_device_mac();

    initialize_pzem(TXD_PIN, RXD_PIN);

    wifi_init();
}
