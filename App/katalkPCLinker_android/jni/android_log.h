#ifndef	__ANDROID_LOG_H__
#define	__ANDROID_LOG_H__

#include <android/log.h>

#define	LOGV(...)	__android_log_print(ANDROID_LOG_VERBOSE, "libnav", __VA_ARGS__)
#define	LOGD(...)	__android_log_print(ANDROID_LOG_DEBUG, "libnav", __VA_ARGS__)
#define	LOGI(...)	__android_log_print(ANDROID_LOG_INFO, "libnav", __VA_ARGS__)
#define	LOGW(...)	__android_log_print(ANDROID_LOG_WARN, "libnav", __VA_ARGS__)
#define	LOGE(...)	__android_log_print(ANDROID_LOG_ERROR, "libnav", __VA_ARGS__)
 
#endif /* __ANDROID_LOG_H__ */