// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _FILE_TYPES_H
#define _FILE_TYPES_H

#include "Aliases.h"

VAMIGA_ENUM(long, AmigaFileType)
{
    FILETYPE_UKNOWN = 0,
    FILETYPE_SNAPSHOT,
    FILETYPE_ADF,
    FILETYPE_HDF,
    FILETYPE_EXT,
    FILETYPE_IMG,
    FILETYPE_DMS,
    FILETYPE_EXE,
    FILETYPE_DIR,
    FILETYPE_ROM,
    FILETYPE_ENCRYPTED_ROM,
    FILETYPE_EXTENDED_ROM,
    FILETYPE_COUNT
};

inline bool isAmigaFileType(long value)
{
    return value >= 0 && value < FILETYPE_COUNT;
}

VAMIGA_ENUM(long, RomIdentifier)
{
    ROM_MISSING,
    ROM_UNKNOWN,

    // Boot Roms (A1000)
    ROM_BOOT_A1000_8K,
    ROM_BOOT_A1000_64K,

    // Kickstart V1.x
    ROM_KICK11_31_034,
    ROM_KICK12_33_166,
    ROM_KICK12_33_180,
    ROM_KICK121_34_004,
    ROM_KICK13_34_005,
    ROM_KICK13_34_005_SK,

    // Kickstart V2.x
    ROM_KICK20_36_028,
    ROM_KICK202_36_207,
    ROM_KICK204_37_175,
    ROM_KICK205_37_299,
    ROM_KICK205_37_300,
    ROM_KICK205_37_350,

    // Kickstart V3.x
    ROM_KICK30_39_106,
    ROM_KICK31_40_063,

    // Hyperion
    ROM_HYP314_46_143,

    // Free Kickstart Rom replacements
    ROM_AROS_55696,
    ROM_AROS_55696_EXT,

    // Diagnostic cartridges
    ROM_DIAG11,
    ROM_DIAG12,
    ROM_DIAG121,
    ROM_LOGICA20,

    ROM_CNT
};

static inline bool isRomRevision(long value) { return value >= 0 && value <= ROM_CNT; }

VAMIGA_ENUM(long, DecryptionError)
{
    DECRYPT_NOT_AN_ENCRYPTED_ROM,
    DECRYPT_MISSING_ROM_KEY_FILE,
    DECRYPT_INVALID_ROM_KEY_FILE
};

VAMIGA_ENUM(long, BootBlockIdentifier)
{
    BB_NONE,
    BB_UNKNOWN,
    
    // Standard (DEPRECATED)
    // BB_KICK_1_3,
    // BB_KICK_2_0,
    
    // Standard boot blocks
    BB_OFS,
    BB_FFS,
    BB_OFS_INTL,
    BB_FFS_INTL,
    BB_OFS_DC,
    BB_FFS_DC,
    BB_OFS_LNFS,
    BB_FFS_LNFS,

    // REMOVE ASAP
    BB_BEERMON1,
    BB_BEERMON2,
    BB_20_FFS,
    BB_20_FFS_INTL,
    BB_20_OFS,
    BB_20_OFS_INTL,
    BB_30_FFS_DIRCACHE,
    BB_30_FFS_INTL_DIRCACHE,
    BB_30_FFS_INTL,
    BB_FILLED,
    BB_1x,
    
    // Viruses
    BB_SCA_VIRUS,
    BB_BYTE_BANDIT_VIRUS
};

VAMIGA_ENUM(long, BootBlockType)
{
    BB_STANDARD,
    BB_VIRUS,
    BB_UTILITY,
    BB_GAME
};

#endif
