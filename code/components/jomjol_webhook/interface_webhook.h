#ifdef ENABLE_WEBHOOK

#pragma once
#ifndef INTERFACE_WEBHOOK_H
#define INTERFACE_WEBHOOK_H

#include <string>
#include <map>
#include <functional>

void WebhookInit(std::string _webhookURI, std::string _apiKey);
void WebhookPublish(std::string _value,std::string _valueraw,std::string _error,std::string _rate,std::string _timestamp, long int _timeUTC);

#endif //INTERFACE_WEBHOOK_H
#endif //ENABLE_WEBHOOK