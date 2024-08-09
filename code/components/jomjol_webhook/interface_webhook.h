#ifdef ENABLE_WEBHOOK

#pragma once
#ifndef INTERFACE_WEBHOOK_H
#define INTERFACE_WEBHOOK_H

#include <string>
#include <map>
#include <functional>
#include <ClassFlowDefineTypes.h>

void WebhookInit(std::string _webhookURI, std::string _apiKey);
bool WebhookPublish(std::vector<NumberPost*>* numbers);
void WebhookUploadPic(ImageData *Img);

#endif //INTERFACE_WEBHOOK_H
#endif //ENABLE_WEBHOOK