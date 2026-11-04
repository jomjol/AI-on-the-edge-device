#include "api_info.h"

#include <stdlib.h>
#include <string>

#include "esp_chip_info.h"
#include "esp_heap_caps.h"

#include "api_response.h"
#include "api_router.h"
#include "connect_wlan.h"
#include "Helper.h"
#include "time_sntp.h"

// Defined in version.h, which only server_main.cpp can include. Other files declare them the same way.
extern const char* BUILD_TIME_UTC; // Generated into version.cpp, with seconds, unlike build_time()
extern const char* libfive_git_version(void);
extern const char* libfive_git_revision(void);
extern const char* libfive_git_branch(void);
extern std::string getFwVersion(void);
extern std::string getHTMLversion(void);

namespace
{

// Adds value as a string, or null if it's empty: a build without a tag has no GIT_TAG.
void add_string_or_null(cJSON *object, const char *key, const char *value)
{
    if (value[0] == '\0')
    {
        cJSON_AddNullToObject(object, key);
    }
    else
    {
        cJSON_AddStringToObject(object, key, value);
    }
}

// The SD card getters return numbers as text for /info?type=... Converting them here keeps one source for both.
void add_number_from_text(cJSON *object, const char *key, const std::string &text)
{
    cJSON_AddNumberToObject(object, key, atof(text.c_str()));
}

// strftime writes the offset as +0200, but ISO 8601 needs +02:00 next to the dashes and colons of the rest.
std::string to_iso_8601(time_t time)
{
    std::string text = ConvertTimeToString(time, "%Y-%m-%dT%H:%M:%S%z");
    text.insert(text.length() - 2, ":");
    return text;
}

void add_firmware(cJSON *root)
{
    cJSON *firmware = cJSON_AddObjectToObject(root, "firmware");
    cJSON_AddStringToObject(firmware, "version", getFwVersion().c_str());
    cJSON_AddStringToObject(firmware, "branch", libfive_git_branch());
    add_string_or_null(firmware, "tag", libfive_git_version());
    cJSON_AddStringToObject(firmware, "revision", libfive_git_revision());
    cJSON_AddStringToObject(firmware, "build_time", BUILD_TIME_UTC);
}

void add_web_ui(cJSON *root)
{
    cJSON *web_ui = cJSON_AddObjectToObject(root, "web_ui");
    cJSON_AddStringToObject(web_ui, "version", getHTMLversion().c_str());
}

void add_system(cJSON *root)
{
    cJSON *system = cJSON_AddObjectToObject(root, "system");
    cJSON_AddStringToObject(system, "hostname", getHostname()->c_str());
    cJSON_AddNumberToObject(system, "uptime_s", getUpTime());
    cJSON_AddNumberToObject(system, "temperature_c", static_cast<int>(temperatureRead()));

    // Worked out from the uptime, so it's also right if the clock was set after the boot
    if (getTimeIsSet())
    {
        cJSON_AddStringToObject(system, "start_time", to_iso_8601(time(NULL) - getUpTime()).c_str());
    }
    else
    {
        cJSON_AddNullToObject(system, "start_time");
    }
}

void add_heap_region(cJSON *memory, const char *key, uint32_t capabilities)
{
    cJSON *region = cJSON_AddObjectToObject(memory, key);
    cJSON_AddNumberToObject(region, "free_bytes", heap_caps_get_free_size(capabilities));
    cJSON_AddNumberToObject(region, "largest_free_block_bytes", heap_caps_get_largest_free_block(capabilities));
    cJSON_AddNumberToObject(region, "min_free_bytes", heap_caps_get_minimum_free_size(capabilities));
}

void add_memory(cJSON *root)
{
    cJSON *memory = cJSON_AddObjectToObject(root, "memory");
    cJSON_AddNumberToObject(memory, "heap_free_bytes", getESPHeapSize());
    add_heap_region(memory, "internal", MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    add_heap_region(memory, "external", MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
}

void add_network(cJSON *root)
{
    cJSON *network = cJSON_AddObjectToObject(root, "network");
    cJSON_AddStringToObject(network, "ip", getIPAddress()->c_str());
    cJSON_AddStringToObject(network, "ssid", getSSID()->c_str());
    cJSON_AddStringToObject(network, "mac", getMac().c_str());
    cJSON_AddNumberToObject(network, "rssi_dbm", get_WIFI_RSSI());
}

void add_chip(cJSON *root)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    cJSON *chip = cJSON_AddObjectToObject(root, "chip");
    cJSON_AddNumberToObject(chip, "cores", chip_info.cores);
    cJSON_AddNumberToObject(chip, "revision", chip_info.revision);
    cJSON_AddNumberToObject(chip, "features", chip_info.features);
}

void add_sd_card(cJSON *root)
{
    cJSON *sd_card = cJSON_AddObjectToObject(root, "sd_card");
    cJSON_AddStringToObject(sd_card, "manufacturer", getSDCardManufacturer().c_str());
    cJSON_AddStringToObject(sd_card, "name", getSDCardName().c_str());
    add_number_from_text(sd_card, "capacity_mb", getSDCardCapacity());
    add_number_from_text(sd_card, "sector_size_bytes", getSDCardSectorSize());
    add_number_from_text(sd_card, "partition_size_mb", getSDCardPartitionSize());
    add_number_from_text(sd_card, "partition_free_mb", getSDCardFreePartitionSpace());
    add_number_from_text(sd_card, "partition_allocation_size_bytes", getSDCardPartitionAllocationSize());
}

} // namespace

esp_err_t api_info_get_handler(httpd_req_t *req)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "api_version", API_VERSION);
    add_firmware(root);
    add_web_ui(root);
    add_system(root);
    add_memory(root);
    add_network(root);
    add_chip(root);
    add_sd_card(root);

    return api_send_json(req, root);
}
