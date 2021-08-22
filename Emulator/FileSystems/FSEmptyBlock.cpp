// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSEmptyBlock.h"
#include <cstring>

FSItemType
FSEmptyBlock::itemType(isize byte) const
{
    return FSI_UNUSED;
}

void
FSEmptyBlock::importBlock(const u8 *p, isize size)
{
    assert(size = bsize());
}

void
FSEmptyBlock::exportBlock(u8 *p, isize size)
{
    assert(size == bsize());
    assert(p);
    std::memset(p, 0, size);
}
