// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_SCREENSHOT_INC
#define _AMIGA_SCREENSHOT_INC

#include "Aliases.h"
#include "AmigaConstants.h"

class Amiga;

struct Screenshot {
    
    // Image size
    u16 width, height;
    
    // Raw texture data
    u32 screen[(HPIXELS / 2) * (VPIXELS / 1)];
    
    // Date and time of screenshot creation
    time_t timestamp;
    
    // Factory methods
    static Screenshot *makeWithAmiga(Amiga *amiga, int dx = 4, int dy = 2);
    
    // Takes a screenshot from a given Amiga
    void take(Amiga *amiga, int dx = 4, int dy = 2);
};

#endif