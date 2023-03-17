#include "ClassLogFile.h"
#include "esp_heap_caps.h"

static const char* TAG = "PSRAM";

using namespace std;


void *malloc_psram_heap(std::string name, size_t size, uint32_t caps) {
	void *ptr;

	ptr = heap_caps_malloc(size, caps);
    if (ptr == NULL) {
	  //  LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Allocated " + to_string(size) + " bytes in PSRAM for " + name + " (end: " + to_string(*ptr + size) + ")");
	    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Allocated " + to_string(size) + " bytes in PSRAM for " + name);
    }
    else {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to allocate " + to_string(size) + " bytes in PSRAM for " + name + "!");
    }

	return ptr;
}


void *calloc_psram_heap(std::string name, size_t n, size_t size, uint32_t caps) {
	void *ptr;

	ptr = heap_caps_calloc(n, size, caps);
    if (ptr == NULL) {
	  //  LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Allocated " + to_string(size) + " bytes in PSRAM for " + name + " (end: " + to_string(*ptr + size) + ")");
	    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Allocated " + to_string(size) + " bytes in PSRAM for " + name);
    }
    else {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to allocate " + to_string(size) + " bytes in PSRAM for " + name + "!");
    }

	return ptr;
}


void free_psram_heap(std::string name, void *ptr) {
    heap_caps_free(ptr);
    LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Freed memory in PSRAM used for " + name);
}