// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Agnus.h"
#include "Denise.h"
#include "Paula.h"

#include "Amiga.h"

u16
Agnus::peekDMACONR()
{
    u16 result = dmacon;
    
    assert((result & ((1 << 14) | (1 << 13))) == 0);
    
    if (blitter.isBusy()) result |= (1 << 14);
    if (blitter.isZero()) result |= (1 << 13);
    
    return result;
}

template <Accessor s> void
Agnus::pokeDMACON(u16 value)
{
    trace(DMA_DEBUG, "pokeDMACON(%X)\n", value);

    // Schedule the write cycle
    if constexpr (s == ACCESSOR_CPU) {
        recordRegisterChange(DMA_CYCLES(1), SET_DMACON, value);
    } else {
        recordRegisterChange(DMA_CYCLES(2), SET_DMACON, value);
    }
}

void
Agnus::setDMACON(u16 oldValue, u16 value)
{
    trace(DMA_DEBUG, "setDMACON(%x, %x)\n", oldValue, value);
    
    // Compute new value
    u16 newValue;
    if (value & 0x8000) {
        newValue = (dmacon | value) & 0x07FF;
    } else {
        newValue = (dmacon & ~value) & 0x07FF;
    }
    
    if (oldValue == newValue) return;
    
    dmacon = newValue;
        
    // Check the lowest 5 bits
    bool oldDMAEN = (oldValue & DMAEN);
    bool oldBPLEN = (oldValue & BPLEN) && oldDMAEN;
    bool oldCOPEN = (oldValue & COPEN) && oldDMAEN;
    bool oldBLTEN = (oldValue & BLTEN) && oldDMAEN;
    bool oldSPREN = (oldValue & SPREN) && oldDMAEN;
    bool oldDSKEN = (oldValue & DSKEN) && oldDMAEN;
    bool oldAUD0EN = (oldValue & AUD0EN) && oldDMAEN;
    bool oldAUD1EN = (oldValue & AUD1EN) && oldDMAEN;
    bool oldAUD2EN = (oldValue & AUD2EN) && oldDMAEN;
    bool oldAUD3EN = (oldValue & AUD3EN) && oldDMAEN;
    
    bool newDMAEN = (newValue & DMAEN);
    bool newBPLEN = (newValue & BPLEN) && newDMAEN;
    bool newCOPEN = (newValue & COPEN) && newDMAEN;
    bool newBLTEN = (newValue & BLTEN) && newDMAEN;
    bool newSPREN = (newValue & SPREN) && newDMAEN;
    bool newDSKEN = (newValue & DSKEN) && newDMAEN;
    bool newAUD0EN = (newValue & AUD0EN) && newDMAEN;
    bool newAUD1EN = (newValue & AUD1EN) && newDMAEN;
    bool newAUD2EN = (newValue & AUD2EN) && newDMAEN;
    bool newAUD3EN = (newValue & AUD3EN) && newDMAEN;
        
    // Inform the delegates
    blitter.pokeDMACON(oldValue, newValue);
    
    // Bitplane DMA
    if (oldBPLEN ^ newBPLEN) {
        
        // Update the bitplane event table
        if (newBPLEN) {
            sequencer.sigRecorder.insert(pos.h + 3, SIG_BMAPEN_SET);
        } else {
            sequencer.sigRecorder.insert(pos.h + 3, SIG_BMAPEN_CLR);
        }
        sequencer.computeBplEvents(sequencer.sigRecorder);
    }
            
    // Disk DMA and sprite DMA
    if ((oldDSKEN ^ newDSKEN) || (oldSPREN ^ newSPREN)) {
        
        // Note: We don't need to rebuild the table if audio DMA changes,
        // because audio events are always executed.
        
        if (oldSPREN ^ newSPREN) {
            trace(DMA_DEBUG, "Sprite DMA %s\n", newSPREN ? "on" : "off");
        }
        if (oldDSKEN ^ newDSKEN) {
            trace(DMA_DEBUG, "Disk DMA %s\n", newDSKEN ? "on" : "off");
        }
        
        u16 newDAS = newDMAEN ? (newValue & 0x3F) : 0;
        
        // Schedule the DAS DMA table to be rebuild
        sequencer.hsyncActions |= UPDATE_DAS_TABLE;
        
        // Make the effect visible in the current rasterline as well
        sequencer.updateDasEvents(newDAS);
        /*
        for (isize i = pos.h; i < HPOS_CNT; i++) {
            sequencer.dasEvent[i] = sequencer.dasDMA[newDAS][i];
        }
        sequencer.updateDasJumpTable();
        */
        
        // Rectify the currently scheduled DAS event
        scheduleDasEventForCycle(pos.h);
    }
    
    // Copper DMA
    if (oldCOPEN ^ newCOPEN) {
        trace(DMA_DEBUG, "Copper DMA %s\n", newCOPEN ? "on" : "off");
        if (newCOPEN) copper.activeInThisFrame = true;
    }
    
    // Blitter DMA
    if (oldBLTEN ^ newBLTEN) {
        trace(DMA_DEBUG, "Blitter DMA %s\n", newBLTEN ? "on" : "off");
    }
    
    // Audio DMA
    if (oldAUD0EN ^ newAUD0EN) {
        newAUD0EN ? paula.channel0.enableDMA() : paula.channel0.disableDMA();
    }
    if (oldAUD1EN ^ newAUD1EN) {
        newAUD1EN ? paula.channel1.enableDMA() : paula.channel1.disableDMA();
    }
    if (oldAUD2EN ^ newAUD2EN) {
        newAUD2EN ? paula.channel2.enableDMA() : paula.channel2.disableDMA();
    }
    if (oldAUD3EN ^ newAUD3EN) {
        newAUD3EN ? paula.channel3.enableDMA() : paula.channel3.disableDMA();
    }
}

u16
Agnus::peekVHPOSR()
{
    // 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // V7 V6 V5 V4 V3 V2 V1 V0 H8 H7 H6 H5 H4 H3 H2 H1
    
    // Return the latched position if the counters are frozen
    if (ersy()) return HI_LO(latchedPos.v & 0xFF, latchedPos.h);
                     
    // The returned position is four cycles ahead
    auto result = agnus.pos + Beam {0,4};
    
    // Rectify the vertical position if it has wrapped over
    if (result.v >= frame.numLines()) result.v = 0;
    
    // In cycle 0 and 1, we need to return the old value of posv
    if (result.h <= 1) {
        return HI_LO(agnus.pos.v & 0xFF, result.h);
    } else {
        return HI_LO(result.v & 0xFF, result.h);
    }
    
    /*
    auto posh = pos.h + 4;
    auto posv = pos.v;
    
    // Check if posh has wrapped over (we just added 4)
    if (posh > HPOS_MAX) {
        posh -= HPOS_CNT;
        if (++posv >= frame.numLines()) posv = 0;
    }
    
    // The value of posv only shows up in cycle 2 and later
    if (posh > 1) {
        return HI_LO(posv & 0xFF, posh);
    }
    
    // In cycle 0 and 1, we need to return the old value of posv
    if (posv > 0) {
        return HI_LO((posv - 1) & 0xFF, posh);
    } else {
        return HI_LO(frame.prevLastLine() & 0xFF, posh);
    }
    */
}

void
Agnus::pokeVHPOS(u16 value)
{
    trace(POSREG_DEBUG, "pokeVHPOS(%X)\n", value);
    
    setVHPOS(value);
}

void
Agnus::setVHPOS(u16 value)
{
    [[maybe_unused]] int v7v0 = HI_BYTE(value);
    [[maybe_unused]] int h8h1 = LO_BYTE(value);
    
    trace(XFILES, "XFILES (VHPOS): %x (%d,%d)\n", value, v7v0, h8h1);

    // Don't know what to do here ...
}

u16
Agnus::peekVPOSR()
{
    // 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // LF I6 I5 I4 I3 I2 I1 I0 -- -- -- -- -- -- -- V8
 
    // I5 I4 I3 I2 I1 I0 (Chip Identification)
    u16 result = idBits();

    // LF (Long frame bit)
    if (frame.isLongFrame()) result |= 0x8000;

    // V8 (Vertical position MSB)
    result |= (ersy() ? latchedPos.v : pos.v) >> 8;
    
    trace(POSREG_DEBUG, "peekVPOSR() = %X\n", result);
    return result;
}

void
Agnus::pokeVPOS(u16 value)
{
    trace(POSREG_DEBUG, "pokeVPOS(%x) (%ld,%d)\n", value, pos.v, frame.lof);
    
    setVPOS(value);
}

void
Agnus::setVPOS(u16 value)
{
    /* I don't really know what exactly we are supposed to do here.
     * For the time being, I only take care of the LOF bit.
     */
    bool newlof = value & 0x8000;
    if (frame.lof == newlof) return;
    
    trace(XFILES, "XFILES (VPOS): %x (%ld,%d)\n", value, pos.v, frame.lof);

    /* If a long frame gets changed to a short frame, we only proceed if
     * Agnus is not in the last rasterline. Otherwise, we would corrupt the
     * emulators internal state (we would be in a line that is unreachable).
     */
    if (!newlof && inLastRasterline()) return;

    trace(XFILES, "XFILES (VPOS): Making a %s frame\n", newlof ? "long" : "short");
    frame.lof = newlof;
    
    // if (!newlof) amiga.signalStop();
    
    /* Reschedule a pending VBL event with a trigger cycle that is consistent
     * with the new value of the LOF bit.
     */
    switch (scheduler.id[SLOT_VBL]) {

        case VBL_STROBE0: scheduleStrobe0Event(); break;
        case VBL_STROBE1: scheduleStrobe1Event(); break;
        case VBL_STROBE2: scheduleStrobe2Event(); break;
            
        default: break;
    }
}

template <Accessor s> void
Agnus::pokeBPLCON0(u16 value)
{
    trace(DMA_DEBUG, "pokeBPLCON0(%X)\n", value);

    recordRegisterChange(DMA_CYCLES(4), SET_BPLCON0_AGNUS, value);
}

void
Agnus::setBPLCON0(u16 oldValue, u16 newValue)
{
    trace(DMA_DEBUG, "setBPLCON0(%4x,%4x)\n", oldValue, newValue);
        
    // Check if the hires bit of one of the BPU bits have been modified
    if ((oldValue ^ newValue) & 0xF000) {
            
        // Recompute the bitplane event table
        sequencer.sigRecorder.insert(pos.h, SIG_CON | newValue >> 12);
        sequencer.computeBplEvents(sequencer.sigRecorder);
                
        // Since the table has changed, we also need to update the event slot
        scheduleBplEventForCycle(pos.h);
    }
    
    // Latch the position counters if the ERSY bit is set
    if ((newValue & 0b10) && !(oldValue & 0b10)) latchedPos = pos;

    bplcon0 = newValue;
}

void
Agnus::pokeBPLCON1(u16 value)
{
    trace(DMA_DEBUG, "pokeBPLCON1(%X)\n", value);
    
    if (bplcon1 != value) {
        recordRegisterChange(DMA_CYCLES(1), SET_BPLCON1_AGNUS, value);
    }
}

void
Agnus::setBPLCON1(u16 oldValue, u16 newValue)
{
    assert(oldValue != newValue);
    trace(DMA_DEBUG, "setBPLCON1(%X,%X)\n", oldValue, newValue);

    bplcon1 = newValue & 0xFF;
    
    // Compute comparision values for the hpos counter
    scrollOdd  = (bplcon1 & 0b00001110) >> 1;
    scrollEven = (bplcon1 & 0b11100000) >> 5;
    
    // Update the bitplane event table
    sequencer.computeBplEvents(sequencer.sigRecorder);
    
    // Update the scheduled bitplane event according to the new table
    scheduleBplEventForCycle(pos.h);
}

template <Accessor s> void
Agnus::pokeDIWSTRT(u16 value)
{
    trace(DIW_DEBUG, "pokeDIWSTRT<%s>(%x)\n", AccessorEnum::key(s), value);
    
    recordRegisterChange(DMA_CYCLES(4), SET_DIWSTRT_AGNUS, value);
    recordRegisterChange(DMA_CYCLES(3), SET_DIWSTRT_DENISE, value);
}

template <Accessor s> void
Agnus::pokeDIWSTOP(u16 value)
{
    trace(DIW_DEBUG, "pokeDIWSTOP<%s>(%x)\n", AccessorEnum::key(s), value);
    
    recordRegisterChange(DMA_CYCLES(4), SET_DIWSTOP_AGNUS, value);
    recordRegisterChange(DMA_CYCLES(3), SET_DIWSTOP_DENISE, value);
}

void
Agnus::pokeBPL1MOD(u16 value)
{
    trace(BPLMOD_DEBUG, "pokeBPL1MOD(%X)\n", value);
    recordRegisterChange(DMA_CYCLES(2), SET_BPL1MOD, value);
}

void
Agnus::setBPL1MOD(u16 value)
{
    trace(BPLMOD_DEBUG, "setBPL1MOD(%X)\n", value);
    bpl1mod = (i16)(value & 0xFFFE);
}

void
Agnus::pokeBPL2MOD(u16 value)
{
    trace(BPLMOD_DEBUG, "pokeBPL2MOD(%X)\n", value);
    recordRegisterChange(DMA_CYCLES(2), SET_BPL2MOD, value);
}

void
Agnus::setBPL2MOD(u16 value)
{
    trace(BPLMOD_DEBUG, "setBPL2MOD(%X)\n", value);
    bpl2mod = (i16)(value & 0xFFFE);
}

template <int x> void
Agnus::pokeSPRxPOS(u16 value)
{
    trace(SPRREG_DEBUG, "pokeSPR%dPOS(%X)\n", x, value);

    // Compute the value of the vertical counter that is seen here
    i16 v = (i16)(pos.h < 0xDF ? pos.v : (pos.v + 1));

    // Compute the new vertical start position
    sprVStrt[x] = ((value & 0xFF00) >> 8) | (sprVStrt[x] & 0x0100);

    // Update sprite DMA status
    if (sprVStrt[x] == v) sprDmaState[x] = SPR_DMA_ACTIVE;
    if (sprVStop[x] == v) sprDmaState[x] = SPR_DMA_IDLE;
}

template <int x> void
Agnus::pokeSPRxCTL(u16 value)
{
    trace(SPRREG_DEBUG, "pokeSPR%dCTL(%X)\n", x, value);

    // Compute the value of the vertical counter that is seen here
    i16 v = (i16)(pos.h < 0xDF ? pos.v : (pos.v + 1));

    // Compute the new vertical start and stop position
    sprVStrt[x] = (i16)((value & 0b100) << 6 | (sprVStrt[x] & 0x00FF));
    sprVStop[x] = (i16)((value & 0b010) << 7 | (value >> 8));

    // Update sprite DMA status
    if (sprVStrt[x] == v) sprDmaState[x] = SPR_DMA_ACTIVE;
    if (sprVStop[x] == v) sprDmaState[x] = SPR_DMA_IDLE;
}

template <Accessor s>
void Agnus::pokeDSKPTH(u16 value)
{
    trace(DSKREG_DEBUG, "pokeDSKPTH(%04x) [%s]\n", value, AccessorEnum::key(s));

    // Schedule the write cycle
    if constexpr (s == ACCESSOR_CPU) {
        recordRegisterChange(DMA_CYCLES(1), SET_DSKPTH, value, s);
    }
    if constexpr (s == ACCESSOR_AGNUS) {
        recordRegisterChange(DMA_CYCLES(2), SET_DSKPTH, value, s);
    }
}

void
Agnus::setDSKPTH(u16 value)
{
    trace(DSKREG_DEBUG, "setDSKPTH(%04x)\n", value);

    // Check if the register is blocked due to ongoing DMA
    if (dropWrite(BUS_DISK)) return;
    
    // Perform the write
    dskpt = REPLACE_HI_WORD(dskpt, value);
    
    if (dskpt & ~agnus.ptrMask) {
        trace(XFILES, "DSKPT %08x out of range\n", dskpt);
    }
}

template <Accessor s>
void Agnus::pokeDSKPTL(u16 value)
{
    trace(DSKREG_DEBUG, "pokeDSKPTL(%04x) [%s]\n", value, AccessorEnum::key(s));

    // Schedule the write cycle
    if constexpr (s == ACCESSOR_CPU) {
        recordRegisterChange(DMA_CYCLES(1), SET_DSKPTL, value, s);
    }
    if constexpr (s == ACCESSOR_AGNUS) {
        recordRegisterChange(DMA_CYCLES(2), SET_DSKPTL, value, s);
    }
}

void
Agnus::setDSKPTL(u16 value)
{
    trace(DSKREG_DEBUG, "setDSKPTL(%04x)\n", value);

    // Check if the register is blocked due to ongoing DMA
    if (dropWrite(BUS_DISK)) return;
    
    // Perform the write
    dskpt = REPLACE_LO_WORD(dskpt, value & 0xFFFE);
}

template <int x, Accessor s> void
Agnus::pokeAUDxLCH(u16 value)
{
    debug(AUDREG_DEBUG, "pokeAUD%dLCH(%X)\n", x, value);

     audlc[x] = REPLACE_HI_WORD(audlc[x], value);
}

template <int x, Accessor s> void
Agnus::pokeAUDxLCL(u16 value)
{
    trace(AUDREG_DEBUG, "pokeAUD%dLCL(%X)\n", x, value);

    audlc[x] = REPLACE_LO_WORD(audlc[x], value & 0xFFFE);
}

template <int x, Accessor s> void
Agnus::pokeBPLxPTH(u16 value)
{
    trace(BPLREG_DEBUG, "pokeBPL%dPTH(%04x) [%s]\n", x, value, AccessorEnum::key(s));
    
    // Schedule the write cycle
    if constexpr (s == ACCESSOR_CPU) {
        recordRegisterChange(DMA_CYCLES(1), SET_BPL1PTH + x - 1, value, s);
    }
    if constexpr (s == ACCESSOR_AGNUS) {
        recordRegisterChange(DMA_CYCLES(2), SET_BPL1PTH + x - 1, value, s);
    }
}

template <int x> void
Agnus::setBPLxPTH(u16 value)
{
    trace(BPLREG_DEBUG, "setBPL%dPTH(%X)\n", x, value);

    // Check if the register is blocked due to ongoing DMA
    if (dropWrite(BUS_BPL1 + x - 1)) return;
    
    // Perform the write
    bplpt[x - 1] = REPLACE_HI_WORD(bplpt[x - 1], value);
    
    if (bplpt[x - 1] & ~agnus.ptrMask) {
        trace(XFILES, "BPL%dPT %08x out of range\n", x, bplpt[x - 1]);
    }
}

template <int x, Accessor s> void
Agnus::pokeBPLxPTL(u16 value)
{
    trace(BPLREG_DEBUG, "pokeBPL%dPTL(%04x) [%s]\n", x, value, AccessorEnum::key(s));

    // Schedule the write cycle
    if constexpr (s == ACCESSOR_CPU) {
        recordRegisterChange(DMA_CYCLES(1), SET_BPL1PTL + x - 1, value, s);
    }
    if constexpr (s == ACCESSOR_AGNUS) {
        recordRegisterChange(DMA_CYCLES(2), SET_BPL1PTL + x - 1, value, s);
    }
}

template <int x> void
Agnus::setBPLxPTL(u16 value)
{
    trace(BPLREG_DEBUG, "setBPL%dPTL(%X)\n", x, value);

    // Check if the register is blocked due to ongoing DMA
    if (dropWrite(BUS_BPL1 + x - 1)) return;
    
    // Perform the write
    bplpt[x - 1] = REPLACE_LO_WORD(bplpt[x - 1], value & 0xFFFE);
}

template <int x, Accessor s> void
Agnus::pokeSPRxPTH(u16 value)
{
    trace(SPRREG_DEBUG, "pokeSPR%dPTH(%04x) [%s]\n", x, value, AccessorEnum::key(s));

    // Schedule the write cycle
    if constexpr (s == ACCESSOR_CPU) {
        recordRegisterChange(DMA_CYCLES(1), SET_SPR0PTH + x, value, s);
    }
    if constexpr (s == ACCESSOR_AGNUS) {
        recordRegisterChange(DMA_CYCLES(2), SET_SPR0PTH + x, value, s);
    }
}

template <int x> void
Agnus::setSPRxPTH(u16 value)
{
    trace(SPRREG_DEBUG, "setSPR%dPTH(%%04x)\n", x, value);
    
    // Check if the register is blocked due to ongoing DMA
    if (dropWrite(BUS_SPRITE0 + x)) return;
    
    // Perform the write
    sprpt[x] = REPLACE_HI_WORD(sprpt[x], value);
    
    if (sprpt[x] & ~agnus.ptrMask) {
        trace(XFILES, "SPR%dPT %08x out of range\n", x, sprpt[x]);
    }
}

template <int x, Accessor s> void
Agnus::pokeSPRxPTL(u16 value)
{
    trace(SPRREG_DEBUG, "pokeSPR%dPTL(%04x) [%s]\n", x, value, AccessorEnum::key(s));

    // Schedule the write cycle
    if constexpr (s == ACCESSOR_CPU) {
        recordRegisterChange(DMA_CYCLES(1), SET_SPR0PTL + x, value, s);
    }
    if constexpr (s == ACCESSOR_AGNUS) {
        recordRegisterChange(DMA_CYCLES(2), SET_SPR0PTL + x, value, s);
    }
}

template <int x> void
Agnus::setSPRxPTL(u16 value)
{
    trace(SPRREG_DEBUG, "setSPR%dPTH(%%04x)\n", x, value);
    
    // Check if the register is blocked due to ongoing DMA
    if (dropWrite(BUS_SPRITE0 + x)) return;
    
    // Perform the write
    sprpt[x] = REPLACE_LO_WORD(sprpt[x], value & 0xFFFE);
}

bool
Agnus::dropWrite(BusOwner owner)
{
    /* A write to a pointer register is dropped if the pointer was used one
     * cycle before the update would happen.
     */
    if (!NO_PTR_DROPS && pos.h >= 1 && busOwner[pos.h - 1] == owner) {
        
        trace(XFILES, "XFILES: Dropping pointer register write (%d)\n", owner);
        return true;
    }
    
    return false;
}

template void Agnus::pokeDSKPTH<ACCESSOR_CPU>(u16 value);
template void Agnus::pokeDSKPTH<ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeDSKPTL<ACCESSOR_CPU>(u16 value);
template void Agnus::pokeDSKPTL<ACCESSOR_AGNUS>(u16 value);

template void Agnus::pokeAUDxLCH<0,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeAUDxLCH<1,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeAUDxLCH<2,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeAUDxLCH<3,ACCESSOR_CPU>(u16 value);

template void Agnus::pokeAUDxLCH<0,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeAUDxLCH<1,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeAUDxLCH<2,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeAUDxLCH<3,ACCESSOR_AGNUS>(u16 value);

template void Agnus::pokeAUDxLCL<0,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeAUDxLCL<1,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeAUDxLCL<2,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeAUDxLCL<3,ACCESSOR_CPU>(u16 value);

template void Agnus::pokeAUDxLCL<0,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeAUDxLCL<1,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeAUDxLCL<2,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeAUDxLCL<3,ACCESSOR_AGNUS>(u16 value);

template void Agnus::pokeBPLxPTH<1,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeBPLxPTH<2,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeBPLxPTH<3,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeBPLxPTH<4,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeBPLxPTH<5,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeBPLxPTH<6,ACCESSOR_CPU>(u16 value);

template void Agnus::pokeBPLxPTH<1,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeBPLxPTH<2,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeBPLxPTH<3,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeBPLxPTH<4,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeBPLxPTH<5,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeBPLxPTH<6,ACCESSOR_AGNUS>(u16 value);

template void Agnus::setBPLxPTH<1>(u16 value);
template void Agnus::setBPLxPTH<2>(u16 value);
template void Agnus::setBPLxPTH<3>(u16 value);
template void Agnus::setBPLxPTH<4>(u16 value);
template void Agnus::setBPLxPTH<5>(u16 value);
template void Agnus::setBPLxPTH<6>(u16 value);

template void Agnus::pokeBPLxPTL<1,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeBPLxPTL<2,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeBPLxPTL<3,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeBPLxPTL<4,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeBPLxPTL<5,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeBPLxPTL<6,ACCESSOR_CPU>(u16 value);

template void Agnus::pokeBPLxPTL<1,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeBPLxPTL<2,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeBPLxPTL<3,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeBPLxPTL<4,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeBPLxPTL<5,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeBPLxPTL<6,ACCESSOR_AGNUS>(u16 value);

template void Agnus::setBPLxPTL<1>(u16 value);
template void Agnus::setBPLxPTL<2>(u16 value);
template void Agnus::setBPLxPTL<3>(u16 value);
template void Agnus::setBPLxPTL<4>(u16 value);
template void Agnus::setBPLxPTL<5>(u16 value);
template void Agnus::setBPLxPTL<6>(u16 value);

template void Agnus::pokeSPRxPTH<0,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeSPRxPTH<1,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeSPRxPTH<2,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeSPRxPTH<3,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeSPRxPTH<4,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeSPRxPTH<5,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeSPRxPTH<6,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeSPRxPTH<7,ACCESSOR_CPU>(u16 value);

template void Agnus::pokeSPRxPTH<0,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeSPRxPTH<1,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeSPRxPTH<2,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeSPRxPTH<3,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeSPRxPTH<4,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeSPRxPTH<5,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeSPRxPTH<6,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeSPRxPTH<7,ACCESSOR_AGNUS>(u16 value);

template void Agnus::setSPRxPTH<0>(u16 value);
template void Agnus::setSPRxPTH<1>(u16 value);
template void Agnus::setSPRxPTH<2>(u16 value);
template void Agnus::setSPRxPTH<3>(u16 value);
template void Agnus::setSPRxPTH<4>(u16 value);
template void Agnus::setSPRxPTH<5>(u16 value);
template void Agnus::setSPRxPTH<6>(u16 value);
template void Agnus::setSPRxPTH<7>(u16 value);

template void Agnus::pokeSPRxPTL<0,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeSPRxPTL<1,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeSPRxPTL<2,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeSPRxPTL<3,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeSPRxPTL<4,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeSPRxPTL<5,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeSPRxPTL<6,ACCESSOR_CPU>(u16 value);
template void Agnus::pokeSPRxPTL<7,ACCESSOR_CPU>(u16 value);

template void Agnus::pokeSPRxPTL<0,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeSPRxPTL<1,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeSPRxPTL<2,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeSPRxPTL<3,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeSPRxPTL<4,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeSPRxPTL<5,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeSPRxPTL<6,ACCESSOR_AGNUS>(u16 value);
template void Agnus::pokeSPRxPTL<7,ACCESSOR_AGNUS>(u16 value);

template void Agnus::setSPRxPTL<0>(u16 value);
template void Agnus::setSPRxPTL<1>(u16 value);
template void Agnus::setSPRxPTL<2>(u16 value);
template void Agnus::setSPRxPTL<3>(u16 value);
template void Agnus::setSPRxPTL<4>(u16 value);
template void Agnus::setSPRxPTL<5>(u16 value);
template void Agnus::setSPRxPTL<6>(u16 value);
template void Agnus::setSPRxPTL<7>(u16 value);

template void Agnus::pokeSPRxPOS<0>(u16 value);
template void Agnus::pokeSPRxPOS<1>(u16 value);
template void Agnus::pokeSPRxPOS<2>(u16 value);
template void Agnus::pokeSPRxPOS<3>(u16 value);
template void Agnus::pokeSPRxPOS<4>(u16 value);
template void Agnus::pokeSPRxPOS<5>(u16 value);
template void Agnus::pokeSPRxPOS<6>(u16 value);
template void Agnus::pokeSPRxPOS<7>(u16 value);

template void Agnus::pokeSPRxCTL<0>(u16 value);
template void Agnus::pokeSPRxCTL<1>(u16 value);
template void Agnus::pokeSPRxCTL<2>(u16 value);
template void Agnus::pokeSPRxCTL<3>(u16 value);
template void Agnus::pokeSPRxCTL<4>(u16 value);
template void Agnus::pokeSPRxCTL<5>(u16 value);
template void Agnus::pokeSPRxCTL<6>(u16 value);
template void Agnus::pokeSPRxCTL<7>(u16 value);

template void Agnus::pokeBPLCON0<ACCESSOR_CPU>(u16 value);
template void Agnus::pokeBPLCON0<ACCESSOR_AGNUS>(u16 value);

template void Agnus::pokeDMACON<ACCESSOR_CPU>(u16 value);
template void Agnus::pokeDMACON<ACCESSOR_AGNUS>(u16 value);

template void Agnus::pokeDIWSTRT<ACCESSOR_CPU>(u16 value);
template void Agnus::pokeDIWSTRT<ACCESSOR_AGNUS>(u16 value);

template void Agnus::pokeDIWSTOP<ACCESSOR_CPU>(u16 value);
template void Agnus::pokeDIWSTOP<ACCESSOR_AGNUS>(u16 value);
