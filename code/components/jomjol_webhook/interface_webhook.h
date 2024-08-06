#ifdef ENABLE_WEBHOOK

#pragma once
#ifndef INTERFACE_WEBHOOK_H
#define INTERFACE_WEBHOOK_H

#include <string>
#include <map>
#include <functional>

void WebhookInit(std::string _webhookURI, std::string _apiKey);
void WebhookPublish(std::string _key, std::string _content, long int _timeUTC);

#endif //INTERFACE_WEBHOOK_H
#endif //ENABLE_WEBHOOK