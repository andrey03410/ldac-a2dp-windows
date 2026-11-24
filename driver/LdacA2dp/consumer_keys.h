#pragma once

#include "avrcp.h"

static inline uint8_t KeyUsage(uint8_t Op, int xm6)
{
    switch (Op) {
    /* XM6 wear detection needs separate HID Play and Pause usages. */
    case AVRCP_OP_PLAY:  return xm6 ? 0xB0 : 0xCD;
    case AVRCP_OP_PAUSE: return xm6 ? 0xB1 : 0xCD;
    case AVRCP_OP_STOP: return 0xB7;
    case AVRCP_OP_FORWARD: return 0xB5;
    case AVRCP_OP_BACKWARD: return 0xB6;
    case AVRCP_OP_FAST_FORWARD: return 0xB3;
    case AVRCP_OP_REWIND: return 0xB4;
    case AVRCP_OP_VOLUME_UP: return 0xE9;
    case AVRCP_OP_VOLUME_DOWN: return 0xEA;
    case AVRCP_OP_MUTE: return 0xE2;
    default: return 0;
    }
}
