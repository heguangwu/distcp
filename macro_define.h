/*
 * macro_define.h
 *
 *  Created on: 2014-10-24
 *      Author: hgw
 */

#ifndef MACRO_DEFINE_H_
#define MACRO_DEFINE_H_

#define LOG_ERROR(log_fmt, log_arg...) \
    do{ \
        WARN_W.log(LL_ERROR,   "[%s:%d][%s] " log_fmt "\n", \
                     __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)

#define LOG_WARN(log_fmt, log_arg...) \
    do{ \
        WARN_W.log(LL_WARNING,   "[%s:%d][%s] " log_fmt "\n", \
                     __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)

#define LOG_INFO(log_fmt, log_arg...) \
    do{ \
        INFO_W.log(LL_INFO,   "[%s:%d][%s] " log_fmt "\n", \
                     __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)

#define LOG_TRACE(log_fmt, log_arg...) \
    do{ \
        INFO_W.log(LL_TRACE,   "[%s:%d][%s] " log_fmt "\n", \
                     __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)

#define LOG_DEBUG(log_fmt, log_arg...) \
    do{ \
        INFO_W.log(LL_DEBUG,   "[%s:%d][%s] " log_fmt "\n", \
                     __FILE__, __LINE__, __FUNCTION__, ##log_arg); \
    } while (0)

#define MACRO_RET(condition, return_val) {\
    if (condition) {\
        return return_val;\
    }\
}

#define MACRO_WARN(condition, log_fmt, log_arg...) {\
    if (condition) {\
        LOG_WARN( log_fmt, ##log_arg);\
    }\
}

#define MACRO_WARN_RET(condition, return_val, log_fmt, log_arg...) {\
    if ((condition)) {\
        LOG_WARN( log_fmt, ##log_arg);\
		return return_val;\
    }\
}

#endif /* MACRO_DEFINE_H_ */
