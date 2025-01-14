// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DriveTypes.h"

//
// Enumerations
//

enum_long(DRIVE_TYPE)
{
    DRIVE_DD_35,
    DRIVE_HD_35,
    DRIVE_DD_525
};
typedef DRIVE_TYPE FloppyDriveType;

#ifdef __cplusplus
struct FloppyDriveTypeEnum : util::Reflection<FloppyDriveTypeEnum, FloppyDriveType>
{
    static long minVal() { return 0; }
    static long maxVal() { return DRIVE_DD_525; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }
    
    static const char *prefix() { return "DRIVE"; }
    static const char *key(FloppyDriveType value)
    {
        switch (value) {
                
            case DRIVE_DD_35:   return "DD_35";
            case DRIVE_HD_35:   return "HD_35";
            case DRIVE_DD_525:  return "DD_525";
        }
        return "???";
    }
};
#endif


//
// Structures
//

typedef struct
{
    FloppyDriveType type;
    
    // Indicates whether mechanical delays should be emulated
    bool mechanicalDelays;

    /* Mechanical delays. The start and stop delays specify the number of
     * cycles that pass between switching the drive motor on or off until the
     * drive motor runs at full speed or came to rest, respectively. The step
     * delay specifies the number of cycle needed by the drive head to move to
     * another cylinder. During this time, the FIFO is filled with garbage data.
     */
    Cycle startDelay;
    Cycle stopDelay;
    Cycle stepDelay;
    
    // Delay between ejecting an old disk and inserting a new one
    Cycle diskSwapDelay;
    
    // Noise settings
    i16 pan;
    u8 stepVolume;
    u8 pollVolume;
    u8 insertVolume;
    u8 ejectVolume;
}
FloppyDriveConfig;

typedef struct
{
    DriveHead head;
    bool hasDisk;
    bool motor;
}
FloppyDriveInfo;

