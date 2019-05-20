// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _DENISE_T_INC
#define _DENISE_T_INC

typedef struct
{
    int32_t *data;
    bool longFrame;
    bool interlace;
}
ScreenBuffer;

typedef struct
{
    uint16_t pos;
    uint16_t ctl;
    uint32_t ptr;
    int16_t hstrt;
    int16_t vstrt;
    int16_t vstop;
    bool attach;
}
SpriteInfo;

typedef struct
{
    uint16_t bplcon0;
    uint16_t bplcon1;
    uint16_t bplcon2;
    int16_t bpu;
    uint16_t bpldat[6];

    uint16_t diwstrt;
    uint16_t diwstop;
    int16_t hstrt;
    int16_t hstop;
    int16_t vstrt;
    int16_t vstop;

    uint16_t joydat[2];
    uint16_t clxdat;

    uint16_t colorReg[32];
    uint32_t color[32];
    
    SpriteInfo sprite[8];
}
DeniseInfo;

#endif
