#include "MainFlowControl.h"

#include <string>
#include <vector>
#include "string.h"
#include "esp_log.h"
#include <esp_timer.h>

#include <iomanip>
#include <sstream>

#include "../../include/defines.h"
#include "Helper.h"
#include "statusled.h"

#include "esp_camera.h"
#include "time_sntp.h"
#include "ClassControllCamera.h"

#include "ClassFlowControll.h"

#include "ClassLogFile.h"
#include "server_GPIO.h"

#include "server_file.h"

#include "read_wlanini.h"
#include "connect_wlan.h"
#include "psram.h"

// support IDF 5.x
#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

ClassFlowControll flowctrl;
camera_flow_config_temp_t CFstatus;

TaskHandle_t xHandletask_autodoFlow = NULL;

bool bTaskAutoFlowCreated = false;
bool flowisrunning = false;

long auto_interval = 0;
bool autostartIsEnabled = false;

int countRounds = 0;
bool isPlannedReboot = false;

static const char *TAG = "MAINCTRL";

// #define DEBUG_DETAIL_ON

void CheckIsPlannedReboot(void)
{
    FILE *pfile;

    if ((pfile = fopen("/sdcard/reboot.txt", "r")) == NULL)
    {
        // LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Initial boot or not a planned reboot");
        isPlannedReboot = false;
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Planned reboot");
        DeleteFile("/sdcard/reboot.txt"); // Prevent Boot Loop!!!
        isPlannedReboot = true;
    }
}

bool getIsPlannedReboot(void)
{
    return isPlannedReboot;
}

int getCountFlowRounds(void)
{
    return countRounds;
}

esp_err_t GetJPG(std::string _filename, httpd_req_t *req)
{
    return flowctrl.GetJPGStream(_filename, req);
}

esp_err_t GetRawJPG(httpd_req_t *req)
{
    return flowctrl.SendRawJPG(req);
}

bool isSetupModusActive(void)
{
    return flowctrl.getStatusSetupModus();
}

void DeleteMainFlowTask(void)
{
#ifdef DEBUG_DETAIL_ON
    ESP_LOGD(TAG, "DeleteMainFlowTask: xHandletask_autodoFlow: %ld", (long)xHandletask_autodoFlow);
#endif

    if (xHandletask_autodoFlow != NULL)
    {
        vTaskDelete(xHandletask_autodoFlow);
        xHandletask_autodoFlow = NULL;
    }

#ifdef DEBUG_DETAIL_ON
    ESP_LOGD(TAG, "Killed: xHandletask_autodoFlow");
#endif
}

void doInit(void)
{
#ifdef DEBUG_DETAIL_ON
    ESP_LOGD(TAG, "Start flowctrl.InitFlow(config);");
#endif
    flowctrl.InitFlow(CONFIG_FILE);
#ifdef DEBUG_DETAIL_ON
    ESP_LOGD(TAG, "Finished flowctrl.InitFlow(config);");
#endif

    /* GPIO handler has to be initialized before MQTT init to ensure proper topic subscription */
    gpio_handler_init();

#ifdef ENABLE_MQTT
    flowctrl.StartMQTTService();
#endif // ENABLE_MQTT
}

bool doflow(void)
{
    std::string zw_time = getCurrentTimeString(LOGFILE_TIME_FORMAT);
    ESP_LOGD(TAG, "doflow - start %s", zw_time.c_str());
    flowisrunning = true;
    flowctrl.doFlow(zw_time);
    flowisrunning = false;

#ifdef DEBUG_DETAIL_ON
    ESP_LOGD(TAG, "doflow - end %s", zw_time.c_str());
#endif

    return true;
}

esp_err_t setCCstatusToCFstatus(void)
{
    CFstatus.ImageFrameSize = CCstatus.ImageFrameSize;
    CFstatus.ImageGainceiling = CCstatus.ImageGainceiling;

    CFstatus.ImageQuality = CCstatus.ImageQuality;
    CFstatus.ImageBrightness = CCstatus.ImageBrightness;
    CFstatus.ImageContrast = CCstatus.ImageContrast;
    CFstatus.ImageSaturation = CCstatus.ImageSaturation;
    CFstatus.ImageSharpness = CCstatus.ImageSharpness;
    CFstatus.ImageAutoSharpness = CCstatus.ImageAutoSharpness;
    CFstatus.ImageWbMode = CCstatus.ImageWbMode;
    CFstatus.ImageAwb = CCstatus.ImageAwb;
    CFstatus.ImageAwbGain = CCstatus.ImageAwbGain;
    CFstatus.ImageAec = CCstatus.ImageAec;
    CFstatus.ImageAec2 = CCstatus.ImageAec2;
    CFstatus.ImageAeLevel = CCstatus.ImageAeLevel;
    CFstatus.ImageAecValue = CCstatus.ImageAecValue;
    CFstatus.ImageAgc = CCstatus.ImageAgc;
    CFstatus.ImageAgcGain = CCstatus.ImageAgcGain;
    CFstatus.ImageBpc = CCstatus.ImageBpc;
    CFstatus.ImageWpc = CCstatus.ImageWpc;
    CFstatus.ImageRawGma = CCstatus.ImageRawGma;
    CFstatus.ImageLenc = CCstatus.ImageLenc;
    CFstatus.ImageSpecialEffect = CCstatus.ImageSpecialEffect;
    CFstatus.ImageHmirror = CCstatus.ImageHmirror;
    CFstatus.ImageVflip = CCstatus.ImageVflip;
    CFstatus.ImageDcw = CCstatus.ImageDcw;

    CFstatus.ImageLedIntensity = CCstatus.ImageLedIntensity;

    CFstatus.ImageZoomEnabled = CCstatus.ImageZoomEnabled;
    CFstatus.ImageZoomMode = CCstatus.ImageZoomMode;
    CFstatus.ImageZoomOffsetX = CCstatus.ImageZoomOffsetX;
    CFstatus.ImageZoomOffsetY = CCstatus.ImageZoomOffsetY;
    CFstatus.ImageZoomSize = CCstatus.ImageZoomSize;

    CFstatus.WaitBeforePicture = CCstatus.WaitBeforePicture;

    return ESP_OK;
}

esp_err_t setCFstatusToCCstatus(void)
{
    CCstatus.ImageFrameSize = CFstatus.ImageFrameSize;
    CCstatus.ImageGainceiling = CFstatus.ImageGainceiling;

    CCstatus.ImageQuality = CFstatus.ImageQuality;
    CCstatus.ImageBrightness = CFstatus.ImageBrightness;
    CCstatus.ImageContrast = CFstatus.ImageContrast;
    CCstatus.ImageSaturation = CFstatus.ImageSaturation;
    CCstatus.ImageSharpness = CFstatus.ImageSharpness;
    CCstatus.ImageAutoSharpness = CFstatus.ImageAutoSharpness;
    CCstatus.ImageWbMode = CFstatus.ImageWbMode;
    CCstatus.ImageAwb = CFstatus.ImageAwb;
    CCstatus.ImageAwbGain = CFstatus.ImageAwbGain;
    CCstatus.ImageAec = CFstatus.ImageAec;
    CCstatus.ImageAec2 = CFstatus.ImageAec2;
    CCstatus.ImageAeLevel = CFstatus.ImageAeLevel;
    CCstatus.ImageAecValue = CFstatus.ImageAecValue;
    CCstatus.ImageAgc = CFstatus.ImageAgc;
    CCstatus.ImageAgcGain = CFstatus.ImageAgcGain;
    CCstatus.ImageBpc = CFstatus.ImageBpc;
    CCstatus.ImageWpc = CFstatus.ImageWpc;
    CCstatus.ImageRawGma = CFstatus.ImageRawGma;
    CCstatus.ImageLenc = CFstatus.ImageLenc;
    CCstatus.ImageSpecialEffect = CFstatus.ImageSpecialEffect;
    CCstatus.ImageHmirror = CFstatus.ImageHmirror;
    CCstatus.ImageVflip = CFstatus.ImageVflip;
    CCstatus.ImageDcw = CFstatus.ImageDcw;

    CCstatus.ImageLedIntensity = CFstatus.ImageLedIntensity;

    CCstatus.ImageZoomEnabled = CFstatus.ImageZoomEnabled;
    CCstatus.ImageZoomMode = CFstatus.ImageZoomMode;
    CCstatus.ImageZoomOffsetX = CFstatus.ImageZoomOffsetX;
    CCstatus.ImageZoomOffsetY = CFstatus.ImageZoomOffsetY;
    CCstatus.ImageZoomSize = CFstatus.ImageZoomSize;

    CCstatus.WaitBeforePicture = CFstatus.WaitBeforePicture;

    return ESP_OK;
}

esp_err_t setCFstatusToCam(void)
{
    sensor_t *s = esp_camera_sensor_get();

    if (s != NULL)
    {
        s->set_framesize(s, CFstatus.ImageFrameSize);
        s->set_gainceiling(s, CFstatus.ImageGainceiling);

        s->set_quality(s, CFstatus.ImageQuality); // 0 - 63

        s->set_brightness(s, CFstatus.ImageBrightness); // -2 to 2
        s->set_contrast(s, CFstatus.ImageContrast);     // -2 to 2
        s->set_saturation(s, CFstatus.ImageSaturation); // -2 to 2
        // s->set_sharpness(s, CFstatus.ImageSharpness);   // auto-sharpness is not officially supported, default to 0
        Camera.SetCamSharpness(CFstatus.ImageAutoSharpness, CFstatus.ImageSharpness);

        s->set_exposure_ctrl(s, CFstatus.ImageAec);  // 0 = disable , 1 = enable
        s->set_ae_level(s, CFstatus.ImageAeLevel);   // -2 to 2
        s->set_aec_value(s, CFstatus.ImageAecValue); // 0 to 1200
        s->set_aec2(s, CFstatus.ImageAec2);          // 0 = disable , 1 = enable

        s->set_gain_ctrl(s, CFstatus.ImageAgc);    // 0 = disable , 1 = enable
        s->set_agc_gain(s, CFstatus.ImageAgcGain); // 0 to 30

        s->set_bpc(s, CFstatus.ImageBpc); // 0 = disable , 1 = enable
        s->set_wpc(s, CFstatus.ImageWpc); // 0 = disable , 1 = enable

        s->set_raw_gma(s, CFstatus.ImageRawGma); // 0 = disable , 1 = enable
        s->set_lenc(s, CFstatus.ImageLenc);      // 0 = disable , 1 = enable

        s->set_hmirror(s, CFstatus.ImageHmirror); // 0 = disable , 1 = enable
        s->set_vflip(s, CFstatus.ImageVflip);     // 0 = disable , 1 = enable

        s->set_dcw(s, CFstatus.ImageDcw); // 0 = disable , 1 = enable

        s->set_wb_mode(s, CFstatus.ImageWbMode);   // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
        s->set_awb_gain(s, CFstatus.ImageAwbGain); // 0 = disable , 1 = enable
        s->set_whitebal(s, CFstatus.ImageAwb);     // 0 = disable , 1 = enable

        // special_effect muß als Letztes gesetzt werden, sonst geht es nicht
        s->set_special_effect(s, CFstatus.ImageSpecialEffect); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)

        TickType_t xDelay2 = 1000 / portTICK_PERIOD_MS;
        vTaskDelay(xDelay2);

        return ESP_OK;
    }
    else
    {
        return ESP_FAIL;
    }
}

esp_err_t handler_get_heap(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_get_heap - Start");
    ESP_LOGD(TAG, "handler_get_heap uri: %s", req->uri);
#endif

    std::string zw = "Heap info:<br>" + getESPHeapInfo();

#ifdef TASK_ANALYSIS_ON
    char *pcTaskList = (char *)calloc_psram_heap(std::string(TAG) + "->pcTaskList", 1, sizeof(char) * 768, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (pcTaskList)
    {
        vTaskList(pcTaskList);
        zw = zw + "<br><br>Task info:<br><pre>Name | State | Prio | Lowest stacksize | Creation order | CPU (-1=NoAffinity)<br>" + std::string(pcTaskList) + "</pre>";
        free_psram_heap(std::string(TAG) + "->pcTaskList", pcTaskList);
    }
    else
    {
        zw = zw + "<br><br>Task info:<br>ERROR - Allocation of TaskList buffer in PSRAM failed";
    }
#endif

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    if (zw.length() > 0)
    {
        httpd_resp_send(req, zw.c_str(), zw.length());
    }
    else
    {
        httpd_resp_send(req, NULL, 0);
    }

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_get_heap - Done");
#endif

    return ESP_OK;
}

esp_err_t handler_init(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_init - Start");
    ESP_LOGD(TAG, "handler_doinit uri: %s", req->uri);
#endif

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    const char *resp_str = "Init started<br>";
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    doInit();

    resp_str = "Init done<br>";
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_init - Done");
#endif

    return ESP_OK;
}

esp_err_t handler_stream(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_stream - Start");
    ESP_LOGD(TAG, "handler_stream uri: %s", req->uri);
#endif

    char _query[50];
    char _value[10];
    bool flashlightOn = false;

    if (httpd_req_get_url_query_str(req, _query, 50) == ESP_OK)
    {
        //        ESP_LOGD(TAG, "Query: %s", _query);
        if (httpd_query_key_value(_query, "flashlight", _value, 10) == ESP_OK)
        {
#ifdef DEBUG_DETAIL_ON
            ESP_LOGD(TAG, "flashlight is found%s", _value);
#endif
            if (strlen(_value) > 0)
            {
                flashlightOn = true;
            }
        }
    }

    Camera.CaptureToStream(req, flashlightOn);

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_stream - Done");
#endif

    return ESP_OK;
}

esp_err_t handler_flow_start(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_flow_start - Start");
#endif

    ESP_LOGD(TAG, "handler_flow_start uri: %s", req->uri);

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    if (autostartIsEnabled)
    {
        xTaskAbortDelay(xHandletask_autodoFlow); // Delay will be aborted if task is in blocked (waiting) state. If task is already running, no action
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Flow start triggered by REST API /flow_start");
        const char *resp_str = "The flow is going to be started immediately or is already running";
        httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Flow start triggered by REST API, but flow is not active!");
        const char *resp_str = "WARNING: Flow start triggered by REST API, but flow is not active";
        httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    }

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_flow_start - Done");
#endif

    return ESP_OK;
}

#ifdef ENABLE_MQTT
esp_err_t MQTTCtrlFlowStart(std::string _topic)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("MQTTCtrlFlowStart - Start");
#endif

    ESP_LOGD(TAG, "MQTTCtrlFlowStart: topic %s", _topic.c_str());

    if (autostartIsEnabled)
    {
        xTaskAbortDelay(xHandletask_autodoFlow); // Delay will be aborted if task is in blocked (waiting) state. If task is already running, no action
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Flow start triggered by MQTT topic " + _topic);
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Flow start triggered by MQTT topic " + _topic + ", but flow is not active!");
    }

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("MQTTCtrlFlowStart - Done");
#endif

    return ESP_OK;
}
#endif // ENABLE_MQTT

esp_err_t handler_json(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_json - Start");
#endif

    ESP_LOGD(TAG, "handler_JSON uri: %s", req->uri);

    if (bTaskAutoFlowCreated)
    {
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_set_type(req, "application/json");

        std::string zw = flowctrl.getJSON();
        if (zw.length() > 0)
        {
            httpd_resp_send(req, zw.c_str(), zw.length());
        }
        else
        {
            httpd_resp_send(req, NULL, 0);
        }
    }
    else
    {
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "Flow not (yet) started: REST API /json not yet available!");
        return ESP_ERR_NOT_FOUND;
    }

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_JSON - Done");
#endif

    return ESP_OK;
}

esp_err_t handler_wasserzaehler(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler water counter - Start");
#endif

    if (bTaskAutoFlowCreated)
    {
        bool _rawValue = false;
        bool _noerror = false;
        bool _all = false;
        std::string _type = "value";
        string zw;

        ESP_LOGD(TAG, "handler water counter uri: %s", req->uri);

        char _query[100];
        char _size[10];

        if (httpd_req_get_url_query_str(req, _query, 100) == ESP_OK)
        {
            //        ESP_LOGD(TAG, "Query: %s", _query);
            if (httpd_query_key_value(_query, "all", _size, 10) == ESP_OK)
            {
#ifdef DEBUG_DETAIL_ON
                ESP_LOGD(TAG, "all is found%s", _size);
#endif
                _all = true;
            }

            if (httpd_query_key_value(_query, "type", _size, 10) == ESP_OK)
            {
#ifdef DEBUG_DETAIL_ON
                ESP_LOGD(TAG, "all is found: %s", _size);
#endif
                _type = std::string(_size);
            }

            if (httpd_query_key_value(_query, "rawvalue", _size, 10) == ESP_OK)
            {
#ifdef DEBUG_DETAIL_ON
                ESP_LOGD(TAG, "rawvalue is found: %s", _size);
#endif
                _rawValue = true;
            }

            if (httpd_query_key_value(_query, "noerror", _size, 10) == ESP_OK)
            {
#ifdef DEBUG_DETAIL_ON
                ESP_LOGD(TAG, "noerror is found: %s", _size);
#endif
                _noerror = true;
            }
        }

        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

        if (_all)
        {
            httpd_resp_set_type(req, "text/plain");
            ESP_LOGD(TAG, "TYPE: %s", _type.c_str());
            int _intype = READOUT_TYPE_VALUE;

            if (_type == "prevalue")
            {
                _intype = READOUT_TYPE_PREVALUE;
            }

            if (_type == "raw")
            {
                _intype = READOUT_TYPE_RAWVALUE;
            }

            if (_type == "error")
            {
                _intype = READOUT_TYPE_ERROR;
            }

            zw = flowctrl.getReadoutAll(_intype);
            ESP_LOGD(TAG, "ZW: %s", zw.c_str());

            if (zw.length() > 0)
            {
                httpd_resp_send(req, zw.c_str(), zw.length());
            }

            return ESP_OK;
        }

        std::string *status = flowctrl.getActStatus();
        string query = std::string(_query);
        //    ESP_LOGD(TAG, "Query: %s, query.c_str());

        if (query.find("full") != std::string::npos)
        {
            string txt;
            txt = "<body style=\"font-family: arial\">";

            if ((countRounds <= 1) && (*status != std::string("Flow finished")))
            {
                // First round not completed yet
                txt += "<h3>Please wait for the first round to complete!</h3><h3>Current state: " + *status + "</h3>\n";
            }
            else
            {
                txt += "<h3>Value</h3>";
            }

            httpd_resp_sendstr_chunk(req, txt.c_str());
        }

        zw = flowctrl.getReadout(_rawValue, _noerror, 0);

        if (zw.length() > 0)
        {
            httpd_resp_sendstr_chunk(req, zw.c_str());
        }

        if (query.find("full") != std::string::npos)
        {
            string txt, zw;

            if ((countRounds <= 1) && (*status != std::string("Flow finished")))
            {
                // First round not completed yet
                // Nothing to do
            }
            else
            {
                /* Digital ROIs */
                txt = "<body style=\"font-family: arial\">";
                txt += "<hr><h3>Recognized Digit ROIs (previous round)</h3>\n";
                txt += "<table style=\"border-spacing: 5px\"><tr style=\"text-align: center; vertical-align: top;\">\n";

                std::vector<HTMLInfo *> htmlinfodig;
                htmlinfodig = flowctrl.GetAllDigital();

                for (int i = 0; i < htmlinfodig.size(); ++i)
                {
                    if (flowctrl.GetTypeDigital() == Digital)
                    {
                        if (htmlinfodig[i]->val >= 10)
                        {
                            zw = "NaN";
                        }
                        else
                        {
                            zw = to_string((int)htmlinfodig[i]->val);
                        }

                        txt += "<td style=\"width: 100px\"><h4>" + zw + "</h4><p><img src=\"/img_tmp/" + htmlinfodig[i]->filename + "\"></p></td>\n";
                    }
                    else
                    {
                        std::stringstream stream;
                        stream << std::fixed << std::setprecision(1) << htmlinfodig[i]->val;
                        zw = stream.str();

                        if (std::stod(zw) >= 10)
                        {
                            zw = "NaN";
                        }

                        txt += "<td style=\"width: 100px\"><h4>" + zw + "</h4><p><img src=\"/img_tmp/" + htmlinfodig[i]->filename + "\"></p></td>\n";
                    }
                    delete htmlinfodig[i];
                }

                htmlinfodig.clear();

                txt += "</tr></table>\n";
                httpd_resp_sendstr_chunk(req, txt.c_str());

                /* Analog ROIs */
                txt = "<hr><h3>Recognized Analog ROIs (previous round)</h3>\n";
                txt += "<table style=\"border-spacing: 5px\"><tr style=\"text-align: center; vertical-align: top;\">\n";

                std::vector<HTMLInfo *> htmlinfoana;
                htmlinfoana = flowctrl.GetAllAnalog();

                for (int i = 0; i < htmlinfoana.size(); ++i)
                {
                    std::stringstream stream;
                    stream << std::fixed << std::setprecision(1) << htmlinfoana[i]->val;
                    zw = stream.str();

                    if (std::stod(zw) >= 10)
                    {
                        zw = "NaN";
                    }

                    txt += "<td style=\"width: 150px;\"><h4>" + zw + "</h4><p><img src=\"/img_tmp/" + htmlinfoana[i]->filename + "\"></p></td>\n";
                    delete htmlinfoana[i];
                }

                htmlinfoana.clear();

                txt += "</tr>\n</table>\n";
                httpd_resp_sendstr_chunk(req, txt.c_str());

                /* Full Image
                 * Only show it after the image got taken and aligned */
                txt = "<hr><h3>Aligned Image (current round)</h3>\n";

                if ((*status == std::string("Initialization")) ||
                    (*status == std::string("Initialization (delayed)")) ||
                    (*status == std::string("Take Image")))
                {
                    txt += "<p>Current state: " + *status + "</p>\n";
                }
                else
                {
                    txt += "<img src=\"/img_tmp/alg_roi.jpg\">\n";
                }
                httpd_resp_sendstr_chunk(req, txt.c_str());
            }
        }

        /* Respond with an empty chunk to signal HTTP response completion */
        httpd_resp_sendstr_chunk(req, NULL);
    }
    else
    {
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "Flow not (yet) started: REST API /value not available!");
        return ESP_ERR_NOT_FOUND;
    }

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_wasserzaehler - Done");
#endif

    return ESP_OK;
}

esp_err_t handler_editflow(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_editflow - Start");
#endif

    ESP_LOGD(TAG, "handler_editflow uri: %s", req->uri);

    char _query[200];
    char _valuechar[30];
    string _task;

    if (httpd_req_get_url_query_str(req, _query, 200) == ESP_OK)
    {
        if (httpd_query_key_value(_query, "task", _valuechar, 30) == ESP_OK)
        {
#ifdef DEBUG_DETAIL_ON
            ESP_LOGD(TAG, "task is found: %s", _valuechar);
#endif
            _task = string(_valuechar);
        }
    }

    if (_task.compare("namenumbers") == 0)
    {
        ESP_LOGD(TAG, "Get NUMBER list");
        return get_numbers_file_handler(req);
    }

    if (_task.compare("data") == 0)
    {
        ESP_LOGD(TAG, "Get data list");
        return get_data_file_handler(req);
    }

    if (_task.compare("tflite") == 0)
    {
        ESP_LOGD(TAG, "Get tflite list");
        return get_tflite_file_handler(req);
    }

    if (_task.compare("copy") == 0)
    {
        string in, out, zw;

        httpd_query_key_value(_query, "in", _valuechar, 30);
        in = string(_valuechar);
        httpd_query_key_value(_query, "out", _valuechar, 30);
        out = string(_valuechar);

#ifdef DEBUG_DETAIL_ON
        ESP_LOGD(TAG, "in: %s", in.c_str());
        ESP_LOGD(TAG, "out: %s", out.c_str());
#endif

        in = "/sdcard" + in;
        out = "/sdcard" + out;

        CopyFile(in, out);
        zw = "Copy Done";
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_send(req, zw.c_str(), zw.length());
    }

    if (_task.compare("cutref") == 0)
    {
        string in, out, zw;
        int x, y, dx, dy;
        bool enhance = false;

        httpd_query_key_value(_query, "in", _valuechar, 30);
        in = string(_valuechar);

        httpd_query_key_value(_query, "out", _valuechar, 30);
        out = string(_valuechar);

        httpd_query_key_value(_query, "x", _valuechar, 30);
        string _x = string(_valuechar);
        x = stoi(_x);

        httpd_query_key_value(_query, "y", _valuechar, 30);
        string _y = string(_valuechar);
        y = stoi(_y);

        httpd_query_key_value(_query, "dx", _valuechar, 30);
        string _dx = string(_valuechar);
        dx = stoi(_dx);

        httpd_query_key_value(_query, "dy", _valuechar, 30);
        string _dy = string(_valuechar);
        dy = stoi(_dy);

#ifdef DEBUG_DETAIL_ON
        ESP_LOGD(TAG, "in: %s", in.c_str());
        ESP_LOGD(TAG, "out: %s", out.c_str());
        ESP_LOGD(TAG, "x: %s", _x.c_str());
        ESP_LOGD(TAG, "y: %s", _y.c_str());
        ESP_LOGD(TAG, "dx: %s", _dx.c_str());
        ESP_LOGD(TAG, "dy: %s", _dy.c_str());
#endif

        if (httpd_query_key_value(_query, "enhance", _valuechar, 10) == ESP_OK)
        {
            string _enhance = string(_valuechar);

            if (_enhance.compare("true") == 0)
            {
                enhance = true;
            }
        }

        in = "/sdcard" + in;
        out = "/sdcard" + out;

        string out2 = out.substr(0, out.length() - 4) + "_org.jpg";

        if ((flowctrl.SetupModeActive || (*flowctrl.getActStatus() == std::string("Flow finished"))) && psram_init_shared_memory_for_take_image_step())
        {
            LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Taking image for Alignment Mark Update...");

            CAlignAndCutImage *caic = new CAlignAndCutImage("cutref", in);
            caic->CutAndSave(out2, x, y, dx, dy);
            delete caic;

            CImageBasis *cim = new CImageBasis("cutref", out2);

            if (enhance)
            {
                cim->Contrast(90);
            }

            cim->SaveToFile(out);
            delete cim;

            psram_deinit_shared_memory_for_take_image_step();
            zw = "CutImage Done";
        }
        else
        {
            LogFile.WriteToFile(ESP_LOG_WARN, TAG, std::string("Taking image for Alignment Mark not possible while device") + " is busy with a round (Current State: '" + *flowctrl.getActStatus() + "')!");
            zw = "Device Busy";
        }

        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_send(req, zw.c_str(), zw.length());
    }

    // wird beim Erstellen eines neuen Referenzbildes aufgerufen
    std::string *sys_status = flowctrl.getActStatus();

    if ((sys_status->c_str() != std::string("Take Image")) && (sys_status->c_str() != std::string("Aligning")))
    {
        if ((_task.compare("test_take") == 0) || (_task.compare("cam_settings") == 0))
        {
            std::string _host = "";

            // laden der aktuellen Kameraeinstellungen(CCstatus) in den Zwischenspeicher(CFstatus)
            setCCstatusToCFstatus(); // CCstatus >>> CFstatus

            if (httpd_query_key_value(_query, "host", _valuechar, 30) == ESP_OK)
            {
                _host = std::string(_valuechar);
            }

            if (httpd_query_key_value(_query, "waitb", _valuechar, 30) == ESP_OK)
            {
                int _waitb = std::stoi(_valuechar);
                if (_waitb != 0)
                {
                    CFstatus.WaitBeforePicture = _waitb;
                }
            }

            if (httpd_query_key_value(_query, "qual", _valuechar, 30) == ESP_OK)
            {
                int _qual = std::stoi(_valuechar);
                if ((_qual >= 0) && (_qual <= 63))
                {
                    CFstatus.ImageQuality = _qual;
                }
            }

            if (httpd_query_key_value(_query, "bri", _valuechar, 30) == ESP_OK)
            {
                int _bri = std::stoi(_valuechar);
                if ((_bri >= -2) && (_bri <= 2))
                {
                    CFstatus.ImageBrightness = _bri;
                }
            }

            if (httpd_query_key_value(_query, "con", _valuechar, 30) == ESP_OK)
            {
                int _con = std::stoi(_valuechar);
                if ((_con >= -2) && (_con <= 2))
                {
                    CFstatus.ImageContrast = _con;
                }
            }

            if (httpd_query_key_value(_query, "sat", _valuechar, 30) == ESP_OK)
            {
                int _sat = std::stoi(_valuechar);
                if ((_sat >= -2) && (_sat <= 2))
                {
                    CFstatus.ImageSaturation = _sat;
                }
            }

            if (httpd_query_key_value(_query, "shp", _valuechar, 30) == ESP_OK)
            {
                int _shp = std::stoi(_valuechar);
                if ((_shp >= -2) && (_shp <= 2))
                {
                    CFstatus.ImageSharpness = _shp;
                }
            }

            if (httpd_query_key_value(_query, "ashp", _valuechar, 30) == ESP_OK)
            {
                if (std::stoi(_valuechar) != 0)
                {
                    CFstatus.ImageAutoSharpness = 1;
                }
                else
                {
                    CFstatus.ImageAutoSharpness = 0;
                }
            }

            if (httpd_query_key_value(_query, "spe", _valuechar, 30) == ESP_OK)
            {
                int _spe = std::stoi(_valuechar);
                if ((_spe >= 0) && (_spe <= 6))
                {
                    CFstatus.ImageSpecialEffect = _spe;
                }
            }

            if (httpd_query_key_value(_query, "wbm", _valuechar, 30) == ESP_OK)
            {
                int _wbm = std::stoi(_valuechar);
                if ((_wbm >= 0) && (_wbm <= 4))
                {
                    CFstatus.ImageWbMode = _wbm;
                }
            }

            if (httpd_query_key_value(_query, "awb", _valuechar, 30) == ESP_OK)
            {
                if (std::stoi(_valuechar) != 0)
                {
                    CFstatus.ImageAwb = 1;
                }
                else
                {
                    CFstatus.ImageAwb = 0;
                }
            }

            if (httpd_query_key_value(_query, "awbg", _valuechar, 30) == ESP_OK)
            {
                if (std::stoi(_valuechar) != 0)
                {
                    CFstatus.ImageAwbGain = 1;
                }
                else
                {
                    CFstatus.ImageAwbGain = 0;
                }
            }

            if (httpd_query_key_value(_query, "aec", _valuechar, 30) == ESP_OK)
            {
                if (std::stoi(_valuechar) != 0)
                {
                    CFstatus.ImageAec = 1;
                }
                else
                {
                    CFstatus.ImageAec = 0;
                }
            }

            if (httpd_query_key_value(_query, "aec2", _valuechar, 30) == ESP_OK)
            {
                if (std::stoi(_valuechar) != 0)
                {
                    CFstatus.ImageAec2 = 1;
                }
                else
                {
                    CFstatus.ImageAec2 = 0;
                }
            }

            if (httpd_query_key_value(_query, "ael", _valuechar, 30) == ESP_OK)
            {
                int _ael = std::stoi(_valuechar);
                if ((_ael >= -2) && (_ael <= 2))
                {
                    CFstatus.ImageAeLevel = _ael;
                }
            }

            if (httpd_query_key_value(_query, "aecv", _valuechar, 30) == ESP_OK)
            {
                int _aecv = std::stoi(_valuechar);
                if ((_aecv >= 0) && (_aecv <= 1200))
                {
                    CFstatus.ImageAecValue = _aecv;
                }
            }

            if (httpd_query_key_value(_query, "agc", _valuechar, 30) == ESP_OK)
            {
                if (std::stoi(_valuechar) != 0)
                {
                    CFstatus.ImageAgc = 1;
                }
                else
                {
                    CFstatus.ImageAgc = 0;
                }
            }

            if (httpd_query_key_value(_query, "agcg", _valuechar, 30) == ESP_OK)
            {
                int _agcg = std::stoi(_valuechar);
                if ((_agcg >= 0) && (_agcg <= 30))
                {
                    CFstatus.ImageAgcGain = _agcg;
                }
            }

            if (httpd_query_key_value(_query, "bpc", _valuechar, 30) == ESP_OK)
            {
                if (std::stoi(_valuechar) != 0)
                {
                    CFstatus.ImageBpc = 1;
                }
                else
                {
                    CFstatus.ImageBpc = 0;
                }
            }

            if (httpd_query_key_value(_query, "wpc", _valuechar, 30) == ESP_OK)
            {
                if (std::stoi(_valuechar) != 0)
                {
                    CFstatus.ImageWpc = 1;
                }
                else
                {
                    CFstatus.ImageWpc = 0;
                }
            }

            if (httpd_query_key_value(_query, "rgma", _valuechar, 30) == ESP_OK)
            {
                if (std::stoi(_valuechar) != 0)
                {
                    CFstatus.ImageRawGma = 1;
                }
                else
                {
                    CFstatus.ImageRawGma = 0;
                }
            }

            if (httpd_query_key_value(_query, "lenc", _valuechar, 30) == ESP_OK)
            {
                if (std::stoi(_valuechar) != 0)
                {
                    CFstatus.ImageLenc = 1;
                }
                else
                {
                    CFstatus.ImageLenc = 0;
                }
            }

            if (httpd_query_key_value(_query, "mirror", _valuechar, 30) == ESP_OK)
            {
                if (std::stoi(_valuechar) != 0)
                {
                    CFstatus.ImageHmirror = 1;
                }
                else
                {
                    CFstatus.ImageHmirror = 0;
                }
            }

            if (httpd_query_key_value(_query, "flip", _valuechar, 30) == ESP_OK)
            {
                if (std::stoi(_valuechar) != 0)
                {
                    CFstatus.ImageVflip = 1;
                }
                else
                {
                    CFstatus.ImageVflip = 0;
                }
            }

            if (httpd_query_key_value(_query, "dcw", _valuechar, 30) == ESP_OK)
            {
                if (std::stoi(_valuechar) != 0)
                {
                    CFstatus.ImageDcw = 1;
                }
                else
                {
                    CFstatus.ImageDcw = 0;
                }
            }

            if (httpd_query_key_value(_query, "zoom", _valuechar, 30) == ESP_OK)
            {
                if (std::stoi(_valuechar) != 0)
                {
                    CFstatus.ImageZoomEnabled = 1;
                }
                else
                {
                    CFstatus.ImageZoomEnabled = 0;
                }
            }

            if (httpd_query_key_value(_query, "zoomx", _valuechar, 30) == ESP_OK)
            {
                CFstatus.ImageZoomOffsetX = std::stoi(_valuechar);
            }

            if (httpd_query_key_value(_query, "zoomy", _valuechar, 30) == ESP_OK)
            {
                CFstatus.ImageZoomOffsetY = std::stoi(_valuechar);
            }

            if (httpd_query_key_value(_query, "zooms", _valuechar, 30) == ESP_OK)
            {
                int _ImageZoomSize = std::stoi(_valuechar);
                if (_ImageZoomSize >= 0)
                {
                    CFstatus.ImageZoomSize = _ImageZoomSize;
                }
            }

            if (httpd_query_key_value(_query, "ledi", _valuechar, 30) == ESP_OK)
            {
                float _ImageLedIntensity = std::stof(_valuechar);
                Camera.SetLEDIntensity(_ImageLedIntensity);
                CFstatus.ImageLedIntensity = CCstatus.ImageLedIntensity;
            }

            if (_task.compare("cam_settings") == 0)
            {
                // wird aufgerufen, wenn das Referenzbild + Kameraeinstellungen gespeichert wurden
                setCFstatusToCCstatus(); // CFstatus >>> CCstatus

                // Kameraeinstellungen wurden verädert
                CFstatus.changedCameraSettings = true;

                ESP_LOGD(TAG, "Cam Settings set");
                std::string _zw = "CamSettingsSet";
                httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
                httpd_resp_send(req, _zw.c_str(), _zw.length());
            }
            else
            {
                // wird aufgerufen, wenn ein neues Referenzbild erstellt oder aktualisiert wurde
                // CFstatus >>> Kamera
                setCFstatusToCam();

                Camera.SetQualityZoomSize(CFstatus.ImageQuality, CFstatus.ImageFrameSize, CFstatus.ImageZoomEnabled, CFstatus.ImageZoomOffsetX, CFstatus.ImageZoomOffsetY, CFstatus.ImageZoomSize);
                // Camera.SetZoomSize(CFstatus.ImageZoomEnabled, CFstatus.ImageZoomOffsetX, CFstatus.ImageZoomOffsetY, CFstatus.ImageZoomSize);

                // Kameraeinstellungen wurden verädert
                CFstatus.changedCameraSettings = true;

                ESP_LOGD(TAG, "test_take - vor TakeImage");
                std::string image_temp = flowctrl.doSingleStep("[TakeImage]", _host);
                httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
                httpd_resp_send(req, image_temp.c_str(), image_temp.length());
            }
        }

        if (_task.compare("test_align") == 0)
        {
            std::string _host = "";

            if (httpd_query_key_value(_query, "host", _valuechar, 30) == ESP_OK)
            {
                _host = std::string(_valuechar);
            }

            std::string zw = flowctrl.doSingleStep("[Alignment]", _host);
            httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
            httpd_resp_send(req, zw.c_str(), zw.length());
        }
    }
    else
    {
        std::string _zw = "DeviceIsBusy";
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_send(req, _zw.c_str(), _zw.length());
    }

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_editflow - Done");
#endif

    return ESP_OK;
}

esp_err_t handler_statusflow(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_statusflow - Start");
#endif

    const char *resp_str;
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    if (bTaskAutoFlowCreated)
    {
#ifdef DEBUG_DETAIL_ON
        ESP_LOGD(TAG, "handler_statusflow: %s", req->uri);
#endif

        string *zw = flowctrl.getActStatusWithTime();
        resp_str = zw->c_str();

        httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    }
    else
    {
        resp_str = "Flow task not yet created";
        httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    }

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_statusflow - Done");
#endif

    return ESP_OK;
}

esp_err_t handler_cputemp(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_cputemp - Start");
#endif

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, std::to_string((int)temperatureRead()).c_str(), HTTPD_RESP_USE_STRLEN);

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_cputemp - End");
#endif

    return ESP_OK;
}

esp_err_t handler_rssi(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_rssi - Start");
#endif

    if (getWIFIisConnected())
    {
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_send(req, std::to_string(get_WIFI_RSSI()).c_str(), HTTPD_RESP_USE_STRLEN);
    }
    else
    {
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "WIFI not (yet) connected: REST API /rssi not available!");
        return ESP_ERR_NOT_FOUND;
    }

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_rssi - End");
#endif

    return ESP_OK;
}

esp_err_t handler_uptime(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_uptime - Start");
#endif

    std::string formatedUptime = getFormatedUptime(false);

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, formatedUptime.c_str(), formatedUptime.length());

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_uptime - End");
#endif

    return ESP_OK;
}

esp_err_t handler_prevalue(httpd_req_t *req)
{
#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_prevalue - Start");
    ESP_LOGD(TAG, "handler_prevalue: %s", req->uri);
#endif

    // Default usage message when handler gets called without any parameter
    const std::string RESTUsageInfo =
        "00: Handler usage:<br>"
        "- To retrieve actual PreValue, please provide only a numbersname, e.g. /setPreValue?numbers=main<br>"
        "- To set PreValue to a new value, please provide a numbersname and a value, e.g. /setPreValue?numbers=main&value=1234.5678<br>"
        "NOTE:<br>"
        "value >= 0.0: Set PreValue to provided value<br>"
        "value <  0.0: Set PreValue to actual RAW value (as long RAW value is a valid number, without N)";

    // Default return error message when no return is programmed
    std::string sReturnMessage = "E90: Uninitialized";

    char _query[100];
    char _numbersname[50] = "default";
    char _value[20] = "";

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    if (httpd_req_get_url_query_str(req, _query, 100) == ESP_OK)
    {
#ifdef DEBUG_DETAIL_ON
        ESP_LOGD(TAG, "Query: %s", _query);
#endif

        if (httpd_query_key_value(_query, "numbers", _numbersname, 50) != ESP_OK)
        {
            // If request is incomplete
            sReturnMessage = "E91: Query parameter incomplete or not valid!<br> "
                             "Call /setPreValue to show REST API usage info and/or check documentation";
            httpd_resp_send(req, sReturnMessage.c_str(), sReturnMessage.length());
            return ESP_FAIL;
        }

        if (httpd_query_key_value(_query, "value", _value, 20) == ESP_OK)
        {
#ifdef DEBUG_DETAIL_ON
            ESP_LOGD(TAG, "Value: %s", _value);
#endif
        }
    }
    else
    {
        // if no parameter is provided, print handler usage
        httpd_resp_send(req, RESTUsageInfo.c_str(), RESTUsageInfo.length());
        return ESP_OK;
    }

    if (strlen(_value) == 0)
    {
        // If no value is povided --> return actual PreValue
        sReturnMessage = flowctrl.GetPrevalue(std::string(_numbersname));

        if (sReturnMessage.empty())
        {
            sReturnMessage = "E92: Numbers name not found";
            httpd_resp_send(req, sReturnMessage.c_str(), sReturnMessage.length());
            return ESP_FAIL;
        }
    }
    else
    {
        // New value is positive: Set PreValue to provided value and return value
        // New value is negative and actual RAW value is a valid number: Set PreValue to RAW value and return value
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "REST API handler_prevalue called: numbersname: " + std::string(_numbersname) + ", value: " + std::string(_value));

        if (!flowctrl.UpdatePrevalue(_value, _numbersname, true))
        {
            sReturnMessage = "E93: Update request rejected. Please check device logs for more details";
            httpd_resp_send(req, sReturnMessage.c_str(), sReturnMessage.length());
            return ESP_FAIL;
        }

        sReturnMessage = flowctrl.GetPrevalue(std::string(_numbersname));

        if (sReturnMessage.empty())
        {
            sReturnMessage = "E94: Numbers name not found";
            httpd_resp_send(req, sReturnMessage.c_str(), sReturnMessage.length());
            return ESP_FAIL;
        }
    }

    httpd_resp_send(req, sReturnMessage.c_str(), sReturnMessage.length());

#ifdef DEBUG_DETAIL_ON
    LogFile.WriteHeapInfo("handler_prevalue - End");
#endif

    return ESP_OK;
}

void task_autodoFlow(void *pvParameter)
{
    int64_t fr_start, fr_delta_ms;

    bTaskAutoFlowCreated = true;

    if (!isPlannedReboot && (esp_reset_reason() == ESP_RST_PANIC))
    {
        flowctrl.setActStatus("Initialization (delayed)");
        // #ifdef ENABLE_MQTT
        // MQTTPublish(mqttServer_getMainTopic() + "/" + "status", "Initialization (delayed)", false); // Right now, not possible -> MQTT Service is going to be started later
        // #endif //ENABLE_MQTT
        vTaskDelay(60 * 5000 / portTICK_PERIOD_MS); // Wait 5 minutes to give time to do an OTA update or fetch the log
    }

    ESP_LOGD(TAG, "task_autodoFlow: start");
    doInit();

    flowctrl.setAutoStartInterval(auto_interval);
    autostartIsEnabled = flowctrl.getIsAutoStart();

    if (isSetupModusActive())
    {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "We are in Setup Mode -> Not starting Auto Flow!");
        autostartIsEnabled = false;
        // 15.7.0 Setup Wizard cannot take a Reference Picture #2953
        // std::string zw_time = getCurrentTimeString(LOGFILE_TIME_FORMAT);
        // flowctrl.doFlowTakeImageOnly(zw_time);
    }

    if (autostartIsEnabled)
    {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Starting Flow...");
    }
    else
    {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Autostart is not enabled -> Not starting Flow");
    }

    while (autostartIsEnabled)
    {
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "----------------------------------------------------------------"); // Clear separation between runs
        time_t roundStartTime = getUpTime();

        std::string _zw = "Round #" + std::to_string(++countRounds) + " started";
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, _zw);

        fr_start = esp_timer_get_time();

        if (flowisrunning)
        {
#ifdef DEBUG_DETAIL_ON
            ESP_LOGD(TAG, "Autoflow: doFlow is already running!");
#endif
        }
        else
        {
#ifdef DEBUG_DETAIL_ON
            ESP_LOGD(TAG, "Autoflow: doFlow is started");
#endif
            flowisrunning = true;
            doflow();
#ifdef DEBUG_DETAIL_ON
            ESP_LOGD(TAG, "Remove older log files");
#endif
            LogFile.RemoveOldLogFile();
            LogFile.RemoveOldDataLog();
        }

        // Round finished -> Logfile
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Round #" + std::to_string(countRounds) + " completed (" + std::to_string(getUpTime() - roundStartTime) + " seconds)");

        // CPU Temp -> Logfile
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "CPU Temperature: " + std::to_string((int)temperatureRead()) + "°C");

        // WIFI Signal Strength (RSSI) -> Logfile
        LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "WIFI Signal (RSSI): " + std::to_string(get_WIFI_RSSI()) + "dBm");

        // Check if time is synchronized (if NTP is configured)
        if (getUseNtp() && !getTimeIsSet())
        {
            LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Time server is configured, but time is not yet set!");
            StatusLED(TIME_CHECK, 1, false);
        }

#if (defined WLAN_USE_MESH_ROAMING && defined WLAN_USE_MESH_ROAMING_ACTIVATE_CLIENT_TRIGGERED_QUERIES)
        wifiRoamingQuery();
#endif

// Scan channels and check if an AP with better RSSI is available, then disconnect and try to reconnect to AP with better RSSI
// NOTE: Keep this direct before the following task delay, because scan is done in blocking mode and this takes ca. 1,5 - 2s.
#ifdef WLAN_USE_ROAMING_BY_SCANNING
        wifiRoamByScanning();
#endif

        fr_delta_ms = (esp_timer_get_time() - fr_start) / 1000;

        if (auto_interval > fr_delta_ms)
        {
            const TickType_t xDelay = (auto_interval - fr_delta_ms) / portTICK_PERIOD_MS;
            ESP_LOGD(TAG, "Autoflow: sleep for: %ldms", (long)xDelay);
            vTaskDelay(xDelay);
        }
    }

    while (1)
    {
        // Keep flow task running to handle necessary sub tasks like reboot handler, etc..
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL); // Delete this task if it exits from the loop above
    xHandletask_autodoFlow = NULL;

    ESP_LOGD(TAG, "task_autodoFlow: end");
}

void InitializeFlowTask(void)
{
    BaseType_t xReturned;

    ESP_LOGD(TAG, "getESPHeapInfo: %s", getESPHeapInfo().c_str());

    uint32_t stackSize = 16 * 1024;
    xReturned = xTaskCreatePinnedToCore(&task_autodoFlow, "task_autodoFlow", stackSize, NULL, tskIDLE_PRIORITY + 2, &xHandletask_autodoFlow, 0);

    if (xReturned != pdPASS)
    {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Creation task_autodoFlow failed. Requested stack size:" + std::to_string(stackSize));
        LogFile.WriteHeapInfo("Creation task_autodoFlow failed");
    }

    ESP_LOGD(TAG, "getESPHeapInfo: %s", getESPHeapInfo().c_str());
}

void register_server_main_flow_task_uri(httpd_handle_t server)
{
    ESP_LOGI(TAG, "server_main_flow_task - Registering URI handlers");

    httpd_uri_t camuri = {};
    camuri.method = HTTP_GET;

    camuri.uri = "/doinit";
    camuri.handler = handler_init;
    camuri.user_ctx = (void *)"Light On";
    httpd_register_uri_handler(server, &camuri);

    // Legacy API => New: "/setPreValue"
    camuri.uri = "/setPreValue.html";
    camuri.handler = handler_prevalue;
    camuri.user_ctx = (void *)"Prevalue";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/setPreValue";
    camuri.handler = handler_prevalue;
    camuri.user_ctx = (void *)"Prevalue";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/flow_start";
    camuri.handler = handler_flow_start;
    camuri.user_ctx = (void *)"Flow Start";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/statusflow.html";
    camuri.handler = handler_statusflow;
    camuri.user_ctx = (void *)"Light Off";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/statusflow";
    camuri.handler = handler_statusflow;
    camuri.user_ctx = (void *)"Light Off";
    httpd_register_uri_handler(server, &camuri);

    // Legacy API => New: "/cpu_temperature"
    camuri.uri = "/cputemp.html";
    camuri.handler = handler_cputemp;
    camuri.user_ctx = (void *)"Light Off";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/cpu_temperature";
    camuri.handler = handler_cputemp;
    camuri.user_ctx = (void *)"Light Off";
    httpd_register_uri_handler(server, &camuri);

    // Legacy API => New: "/rssi"
    camuri.uri = "/rssi.html";
    camuri.handler = handler_rssi;
    camuri.user_ctx = (void *)"Light Off";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/rssi";
    camuri.handler = handler_rssi;
    camuri.user_ctx = (void *)"Light Off";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/uptime";
    camuri.handler = handler_uptime;
    camuri.user_ctx = (void *)"Light Off";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/editflow";
    camuri.handler = handler_editflow;
    camuri.user_ctx = (void *)"EditFlow";
    httpd_register_uri_handler(server, &camuri);

    // Legacy API => New: "/value"
    camuri.uri = "/value.html";
    camuri.handler = handler_wasserzaehler;
    camuri.user_ctx = (void *)"Value";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/value";
    camuri.handler = handler_wasserzaehler;
    camuri.user_ctx = (void *)"Value";
    httpd_register_uri_handler(server, &camuri);

    // Legacy API => New: "/value"
    camuri.uri = "/wasserzaehler.html";
    camuri.handler = handler_wasserzaehler;
    camuri.user_ctx = (void *)"Wasserzaehler";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/json";
    camuri.handler = handler_json;
    camuri.user_ctx = (void *)"JSON";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/heap";
    camuri.handler = handler_get_heap;
    camuri.user_ctx = (void *)"Heap";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/stream";
    camuri.handler = handler_stream;
    camuri.user_ctx = (void *)"stream";
    httpd_register_uri_handler(server, &camuri);
}
