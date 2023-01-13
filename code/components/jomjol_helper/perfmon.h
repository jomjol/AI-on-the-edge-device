
#include "../../include/defines.h"

#ifdef DEBUG_ENABLE_PERFMON

#ifndef COMPONENTS_PERFMON_INCLUDE_PERFMON_H_
#define COMPONENTS_PERFMON_INCLUDE_PERFMON_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

esp_err_t perfmon_start();

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_PERFMON_INCLUDE_PERFMON_H_ */

#endif //DEBUG_ENABLE_PERFMON 
