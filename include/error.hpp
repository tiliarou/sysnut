#pragma once

#include <stdio.h>
#include <switch.h>

#define ASSERT_OK(rc_out, desc) if (R_FAILED(rc_out)) { char msg[256] = {0}; snprintf(msg, 256-1, "%s:%u: %s.  Error code: 0x%08x\n", __func__, __LINE__, desc, rc_out); }
#define THROW_FORMAT(format, ...) { char error_prefix[512] = {0}; snprintf(error_prefix, 256-1, "%s:%u: ", __func__, __LINE__);\
                                char formatted_msg[256] = {0}; snprintf(formatted_msg, 256-1, format, ##__VA_ARGS__);\
                                strncat(error_prefix, formatted_msg, 512-1); }

#ifdef NXLINK_DEBUG
#define LOG_DEBUG(format, ...) fprintf(nxlinkout, "%s:%u: ", __func__, __LINE__); fprintf(nxlinkout, format, ##__VA_ARGS__);
#else
#define LOG_DEBUG(format, ...) ;
#endif