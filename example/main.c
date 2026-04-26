#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "zenoh-pico.h"

static const char *TAG = "RAW_ZENOH";

typedef struct {
    int64_t data;
} __attribute__((packed)) ping_payload_t;

z_owned_session_t session;
z_owned_publisher_t pub;

void ping_callback(z_loaned_sample_t *sample, void *arg) {
    z_owned_publisher_t *p = (z_owned_publisher_t *)arg;
    
    const z_loaned_bytes_t *payload = z_sample_payload(sample);

    z_owned_bytes_t bounced_payload;
    z_bytes_clone(&bounced_payload, payload);

    z_publisher_put(z_publisher_loan(p), z_bytes_move(&bounced_payload), NULL);
}

void raw_zenoh_task(void *pvParameters) {
    ESP_LOGI(TAG, "Initializing Zenoh Session...");
    z_owned_config_t config;
    z_config_default(&config);
    
    zp_config_insert(z_config_loan_mut(&config), Z_CONFIG_CONNECT_KEY, "udp/192.168.0.0:7447");
    
    ESP_LOGI(TAG, "Opening session...");
    if (z_open(&session, z_config_move(&config), NULL) < 0) {
        ESP_LOGE(TAG, "Failed to open Zenoh session!");
        vTaskDelete(NULL);
    }

    ESP_LOGI(TAG, "Session open. Declaring Publisher (/pong)...");
    
    z_view_keyexpr_t pub_key;
    z_view_keyexpr_from_str(&pub_key, "pong");
    if (z_declare_publisher(z_session_loan(&session), &pub, z_view_keyexpr_loan(&pub_key), NULL) < 0) {
        ESP_LOGE(TAG, "Failed to declare publisher.");
    }

    ESP_LOGI(TAG, "Declaring Subscriber (/ping)...");
    z_view_keyexpr_t sub_key;
    z_view_keyexpr_from_str(&sub_key, "ping");
    
    z_owned_closure_sample_t callback;
    z_closure_sample(&callback, ping_callback, NULL, &pub);
    
    z_owned_subscriber_t sub;
    if (z_declare_subscriber(z_session_loan(&session), &sub, z_view_keyexpr_loan(&sub_key), z_closure_sample_move(&callback), NULL) < 0) {
        ESP_LOGE(TAG, "Failed to declare subscriber.");
    }

    ESP_LOGI(TAG, "Reflector active. Entering RTOS loop.");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Connecting to Wi-Fi...");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());
    ESP_LOGI(TAG, "Wi-Fi Connected!");

    xTaskCreatePinnedToCore(raw_zenoh_task, "raw_zenoh_task", 8192, NULL, 5, NULL, 1);
}
