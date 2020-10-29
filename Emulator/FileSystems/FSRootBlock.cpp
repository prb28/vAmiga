// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSRootBlock.h"

FSRootBlock::FSRootBlock(FSVolume &ref, u32 nr) : FSBlock(ref, nr)
{
    
}

FSRootBlock::FSRootBlock(FSVolume &ref, u32 nr, const char *name) : FSRootBlock(ref, nr)
{
    this->name = FSName(name);
}

void
FSRootBlock::printName()
{
    printf("%s", name.name);
}

void
FSRootBlock::printPath()
{
    printf("%s::", name.name);
}

void
FSRootBlock::dump()
{
    printf("        Name: "); name.dump(); printf("\n");
    printf("     Created: "); created.dump(); printf("\n");
    printf("    Modified: "); modified.dump(); printf("\n");
    printf("  Hash table: "); hashTable.dump(); printf("\n");
}

bool
FSRootBlock::check(bool verbose)
{
    bool result = FSBlock::check(verbose);
        
    for (int i = 0; i < hashTable.hashTableSize; i++) {
        
        u32 ref = hashTable.hashTable[i];
        if (ref == 0) continue;

        result &= assertInRange(ref, verbose);
        result &= assertHasType(ref, FS_USERDIR_BLOCK, FS_FILEHEADER_BLOCK);
    }
        
    return result;
}

void
FSRootBlock::write(u8 *p)
{
    // Start from scratch
    memset(p, 0, 512);

    // Type
    p[3] = 0x02;
    
    // Hashtable size
    p[15] = 0x48;
    
    // Hashtable
    hashTable.write(p + 24);
    
    // BM flag (true if bitmap on disk is valid)
    p[312] = p[313] = p[314] = p[315] = 0xFF;
    
    // BM pages (indicates the blocks containing the bitmap)
    p[318] = HI_BYTE(881);
    p[319] = LO_BYTE(881);
    
    // Last recent change of the root directory of this volume
    modified.write(p + 420);
    
    // Date and time when this volume was formatted
    created.write(p + 484);
    
    // Volume name
    name.write(p + 432);
    
    // Secondary block type
    write32(p + 508, 1);
    
    // Compute checksum
    write32(p + 20, FSBlock::checksum(p));
}
