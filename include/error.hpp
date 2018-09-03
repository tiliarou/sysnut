#pragma once

#include <stdio.h>
#include <switch.h>
#include <string.h>
#include "log.h"

#define ASSERT_OK(rc_out, desc) if (R_FAILED(rc_out)) { print("%s:%u: %s.  Error code: 0x%08x\n", __func__, __LINE__, desc, rc_out); return NULL; }
#define ASSERT_VOID(rc_out, desc) if (R_FAILED(rc_out)) { print("%s:%u: %s.  Error code: 0x%08x\n", __func__, __LINE__, desc, rc_out); return; }
#define ASSERT_STR(rc_out, desc) if (R_FAILED(rc_out)) { print("%s:%u: %s.  Error code: 0x%08x\n", __func__, __LINE__, desc, rc_out); return std::string(""); }

