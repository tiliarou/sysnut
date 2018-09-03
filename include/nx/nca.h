#pragma once
#include "nut.h"

typedef struct {
	u8 c[0x10];
} NcmNcaId;

class NcaId
{
public:

private:
	u8 m_buffer[0x10];
};