//source : https://github.com/Carbon225/esp32-perfmon

#include "../../include/defines.h"

/*
ESP32 CPU usage monitor
Gives you a rough idea of how the Xtensa cores are utilized.

Works by attaching idle hooks and measuring how often they get called. The core usage is calculated: usage% = idle ticks since last measurement / expected idle ticks if core were idle * 100%. The expected idle tick count was measured by running an empty program.

Limitations:
Should only be used for user information, not in logic that needs accurate values
New IDF versions could optimize performance and therefore introduce an error to usage estimation.
When one core is at 100% the other might report a negative value

Usage:
#include "perfmon.h"
Call perfmon_start() once

*/

#ifdef DEBUG_ENABLE_PERFMON

#include "perfmon.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "sdkconfig.h"

#include "esp_log.h"
static const char *TAG = "perfmon";

static uint64_t idle0Calls = 0;
static uint64_t idle1Calls = 0;

#if defined(CONFIG_ESP32_DEFAULT_CPU_FREQ_240)
static const uint64_t MaxIdleCalls = 1855000;
#elif defined(CONFIG_ESP32_DEFAULT_CPU_FREQ_160)
static const uint64_t MaxIdleCalls = 1233100;
#else
#error "Unsupported CPU frequency"
#endif

static bool idle_task_0()
{
	idle0Calls += 1;
	return false;
}

static bool idle_task_1()
{
	idle1Calls += 1;
	return false;
}

static void perfmon_task(void *args)
{
	while (1)
	{
		float idle0 = idle0Calls;
		float idle1 = idle1Calls;
		idle0Calls = 0;
		idle1Calls = 0;

		int cpu0 = 100.f -  idle0 / MaxIdleCalls * 100.f;
		int cpu1 = 100.f - idle1 / MaxIdleCalls * 100.f;

		ESP_LOGI(TAG, "Core 0 at %d%%", cpu0);
		ESP_LOGI(TAG, "Core 1 at %d%%", cpu1);
		// TODO configurable delay
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

esp_err_t perfmon_start()
{
	ESP_ERROR_CHECK(esp_register_freertos_idle_hook_for_cpu(idle_task_0, 0));
	ESP_ERROR_CHECK(esp_register_freertos_idle_hook_for_cpu(idle_task_1, 1));
	// TODO calculate optimal stack size
	xTaskCreate(perfmon_task, "perfmon", 2048, NULL, 1, NULL);
	return ESP_OK;
}


#endif // DEBUG_ENABLE_PERFMON
