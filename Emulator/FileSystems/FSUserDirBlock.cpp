// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSUserDirBlock.h"

FSUserDirBlock::FSUserDirBlock(FSVolume &ref) : HashableBlock(ref)
{
    
}

FSUserDirBlock::FSUserDirBlock(FSVolume &ref, const char *name) : HashableBlock(ref)
{
    
}

void
FSUserDirBlock::dump()
{
    
}

void
FSUserDirBlock::write(u8 *p)
{
    // Start from scratch
    memset(p, 0, 512);
    
    // Type
    p[3] = 0x02;
    
    // Block pointer to itself
    write32(p + 4, nr);
    
    // Hashtable
    hashTable.write(p + 24);

    // Protection status bits
    u32 protection = 0;
    write32(p + 320, protection);
    
    // Comment as BCPL string
    comment.write(p + 328);
    
    // Creation date
    created.write(p + 420);
    
    // Directory name as BCPL string
    name.write(p + 432);
    
    // Next block with same hash
    if (next) write32(p + 496, next->nr);

    // Block pointer to parent directory
    assert(parent != NULL);
    write32(p + 500, parent->nr);
    
    // Subtype
    p[508] = 2;
        
    // Checksum
    write32(p + 20, FSBlock::checksum(p));
}