#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdio.h>
#define SYSLOG
#ifdef SYSLOG
#include <syslog.h>
#endif


#ifdef SYSLOG
#define LOG(level, str, args...) do { printf(str, ## args); \
                printf("\n"); \
                syslog(level, str, ## args); } while(0)
#define OPEN_LOG(name) openlog(name, 0, 0)
#define CLOSE_LOG() closelog()
#else
#define LOG_EMERG      "EMERGENCY!"
#define LOG_ALERT      "ALERT!"
#define LOG_CRIT       "CRITICAL!"
#define LOG_WARNING    "WARNING"
#define LOG_ERR        "ERROR"
#define LOG_INFO       "INFO"
#define LOG_DEBUG      "DEBUG"
#define LOG(level, str, args...) do { printf("%s, ", level); \
                printf(str, ## args); \
                printf("\n"); } while(0)
#define OPEN_LOG(name) do {;} while(0)
#define CLOSE_LOG() do {;} while(0)
#endif /* SYSLOG */

#undef DEBUG
#ifdef DEBUG_DHCP
#define DEBUG(level, str, args...) LOG(level, str, ## args)
#define DEBUGGING
#else
#define DEBUG(level, str, args...) do {;} while(0)
#endif /* DEBUG */

#endif
