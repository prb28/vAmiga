// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "HdrController.h"
#include "HdrControllerRom.h"
#include "HardDrive.h"
#include "HDFFile.h"
#include "Memory.h"
#include "OSDebugger.h"

HdrController::HdrController(Amiga& ref, HardDrive& hdr) : ZorroBoard(ref), drive(hdr)
{
    nr = drive.getNr();
    
    // Copy Rom code
    rom = new u8[EXPROM_SIZE];
    std::memcpy((u8 *)rom, exprom, EXPROM_SIZE);
    
    // Setup the device and dos name for this device
    char devName[] = "DH?";             devName[2] = char('0' + nr);
    char dosName[] = "hdrv?.device";    dosName[4] = char('0' + nr);
    
    // Patch Rom
    /*
    util::replace((char *)rom, EXPROM_SIZE, "DH0", devName);
    util::replace((char *)rom, EXPROM_SIZE, "hello.device", dosName);
    */
}

const char *
HdrController::getDescription() const
{
    switch (nr) {
            
        case 0: return "HdrController0";
        case 1: return "HdrController1";
        case 2: return "HdrController2";
        case 3: return "HdrController3";

        default:
            fatalError;
    }
}

void
HdrController::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
        
    ZorroBoard::_dump(category, os);
    
    if (category & dump::State) {
    
    }
}

void
HdrController::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)

    if (hard) {

        if (pluggedIn())  {

            state = STATE_AUTOCONF;
            debug(true, "Hard drive emulation enabled.\n");

        } else {
            
            state = STATE_SHUTUP;
            debug(true, "Hard drive emulation disabled. No HDF.\n");
        }
    }
}

bool
HdrController::pluggedIn() const
{
    return drive.isConnected();
}

void
HdrController::updateMemSrcTables()
{
    // Only proceed if this board has been configured
    if (baseAddr == 0) return;
    
    // Map in this device
    mem.cpuMemSrc[firstPage()] = MEM_ZOR;
}

u8
HdrController::peek8(u32 addr)
{
    auto result = spypeek8(addr);

    trace(ZOR_DEBUG, "peek8(%06x) = %02x\n", addr, result);
    return result;
}

u16
HdrController::peek16(u32 addr)
{
    auto result = spypeek16(addr);

    trace(ZOR_DEBUG, "peek16(%06x) = %04x\n", addr, result);
    return result;
}

u8
HdrController::spypeek8(u32 addr) const
{
    isize offset = (isize)(addr & 0xFFFF) - (isize)initDiagVec();
    return offset < EXPROM_SIZE ? rom[offset] : 0;
}

u16
HdrController::spypeek16(u32 addr) const
{
    isize offset = (isize)(addr & 0xFFFF) - (isize)initDiagVec();
    
    switch (offset) {
            
        case EXPROM_SIZE:
            
            // Return the number of partitions
            debug(HDR_DEBUG, "This drive has %ld partitions\n", drive.numPartitions());
            // return u16(drive.numPartitions());
            return 1;
            
        case EXPROM_SIZE + 2:
            
            // Number of filesystem drivers to add
            return 0;
            
        default:
            
            // Return Rom code
            return offset < EXPROM_SIZE ? HI_LO(rom[offset], rom[offset + 1]) : 0;
    }
}

void
HdrController::poke8(u32 addr, u8 value)
{
    trace(ZOR_DEBUG, "poke8(%06x,%02x)\n", addr, value);
}

void
HdrController::poke16(u32 addr, u16 value)
{
    trace(ZOR_DEBUG, "poke16(%06x,%04x)\n", addr, value);
    
    isize offset = (isize)(addr & 0xFFFF) - (isize)initDiagVec();

    switch (offset) {
            
        case EXPROM_SIZE:
            
            pointer = REPLACE_HI_WORD(pointer, value);
            break;
            
        case EXPROM_SIZE + 2:

            pointer = REPLACE_LO_WORD(pointer, value);
            break;

        case EXPROM_SIZE + 4:
            
            switch (value) {
                    
                case 0xfede: processCmd(); break;
                case 0xfedf: processInit(); break;
                case 0xfee0: processResource(); break;
                case 0xfee1: processInfoReq(); break;
                case 0xfee2: processInitSeg(); break;
                    
                default:
                    warn("Invalid value: %x\n", value);
                    break;
            }
            break;

        default:

            warn("Invalid addr: %x\n", addr);
            break;
    }
}

void
HdrController::processCmd()
{
    os::IOStdReq stdReq;
    osDebugger.read(pointer, &stdReq);
    
    auto unit = mem.spypeek32 <ACCESSOR_CPU> (stdReq.io_Unit + 0x2A);
    auto cmd = IoCommand(stdReq.io_Command);
    auto offset = isize(stdReq.io_Offset);
    auto length = isize(stdReq.io_Length);
    auto addr = u32(stdReq.io_Data);

    debug(HDR_DEBUG, "Unit: %d Blck: %ld Cmd: %s\n", unit, offset / 512, IoCommandEnum::key(cmd));

    switch (cmd) {
            
        case CMD_READ:
        {            
            // Perform the operation
            // auto error = drive.read(offset, length, addr);
            auto error = drive.read(unit, offset / 512, length, addr);

            // Check for errors
            if (error) mem.patch(pointer + IO_ERROR, u8(error));
            break;
        }
        case CMD_WRITE:
        case CMD_TD_FORMAT:
        {
            // Perform the operation
            // auto error = drive.write(offset, length, addr);
            auto error = drive.write(unit, offset / 512, length, addr);

            // Check for errors
            if (error) mem.patch(pointer + IO_ERROR, u8(error));
            break;
        }
  
        case CMD_RESET:
        case CMD_UPDATE:
        case CMD_CLEAR:
        case CMD_STOP:
        case CMD_START:
        case CMD_FLUSH:
        case CMD_TD_MOTOR:
        case CMD_TD_SEEK:
        case CMD_TD_REMOVE:
        case CMD_TD_CHANGENUM:
        case CMD_TD_CHANGESTATE:
        case CMD_TD_PROTSTATUS:
        case CMD_TD_ADDCHANGEINT:
        case CMD_TD_REMCHANGEINT:
            
            mem.patch(pointer + IO_ACTUAL, u32(0));
            mem.patch(pointer + IO_ERROR, u8(0));
            break;
            
        default:
            
            warn("Unsupported command: %lx\n", cmd);
            mem.patch(pointer + IO_ERROR, u8(IOERR_NOCMD));
    }
}

void
HdrController::processInit()
{
    trace(HDR_DEBUG, "processInit()\n");

    // Keep in check with exprom.asm
    constexpr u16 devn_dosName      = 0x00;  // APTR  Pointer to DOS file handler name
    constexpr u16 devn_unit         = 0x08;  // ULONG Unit number
    constexpr u16 devn_flags        = 0x0C;  // ULONG OpenDevice flags
    constexpr u16 devn_sizeBlock    = 0x14;  // ULONG # longwords in a block
    constexpr u16 devn_secOrg       = 0x18;  // ULONG sector origin -- unused
    constexpr u16 devn_numHeads     = 0x1C;  // ULONG number of surfaces
    constexpr u16 devn_secsPerBlk   = 0x20;  // ULONG secs per logical block
    constexpr u16 devn_blkTrack     = 0x24;  // ULONG secs per track
    constexpr u16 devn_resBlks      = 0x28;  // ULONG reserved blocks -- MUST be at least 1!
    constexpr u16 devn_interleave   = 0x30;  // ULONG interleave
    constexpr u16 devn_lowCyl       = 0x34;  // ULONG lower cylinder
    constexpr u16 devn_upperCyl     = 0x38;  // ULONG upper cylinder
    constexpr u16 devn_numBuffers   = 0x3C;  // ULONG number of buffers
    constexpr u16 devn_memBufType   = 0x40;  // ULONG Type of memory for AmigaDOS buffers
    constexpr u16 devn_transferSize = 0x44;  // LONG  largest transfer size (largest signed #)
    constexpr u16 devn_addMask      = 0x48;  // ULONG address mask
    constexpr u16 devn_bootPrio     = 0x4c;  // ULONG boot priority
    constexpr u16 devn_dName        = 0x50;  // char[4] DOS file handler name
    constexpr u16 devn_bootflags    = 0x54;  // boot flags (not part of DOS packet)
    constexpr u16 devn_segList      = 0x58;  // filesystem segment list (not part of DOS packet)
    
    u32 unit = mem.spypeek32 <ACCESSOR_CPU> (pointer + devn_unit);
    debug(HDR_DEBUG, "Initializing partition %d\n", unit);
    
    // Collect hard drive information
    auto geometry = drive.getGeometry();
    u32 sizeBlock = (u32)(geometry.bsize / 4);
    u32 numHeads = (u32)(geometry.heads);
    u32 blkTrack = (u32)(geometry.sectors);
    u32 upperCyl = (u32)(geometry.cylinders - 1);

    debug(true, "sizeBlock = %d\n", sizeBlock);
    debug(true, "numHeads = %d\n", numHeads);
    debug(true, "blkTrack = %d\n", blkTrack);
    debug(true, "upperCyl = %d\n", upperCyl);
    
    char dosName[] = {'D', 'H', '0', 0 };
    
    u32 name_ptr = mem.spypeek32 <ACCESSOR_CPU> (pointer + devn_dosName);
    for (isize i = 0; i < isizeof(dosName); i++) {
        mem.patch(u32(name_ptr + i), u8(dosName[i]));
    }
    
    mem.patch(pointer + devn_flags,         u32(0));
    mem.patch(pointer + devn_sizeBlock,     u32(sizeBlock));
    mem.patch(pointer + devn_secOrg,        u32(0));
    mem.patch(pointer + devn_numHeads,      u32(numHeads));
    mem.patch(pointer + devn_secsPerBlk,    u32(1));
    mem.patch(pointer + devn_blkTrack,      u32(blkTrack));
    mem.patch(pointer + devn_interleave,    u32(0));
    mem.patch(pointer + devn_resBlks,       u32(2));
    mem.patch(pointer + devn_lowCyl,        u32(0)); // CHECK VALUE
    mem.patch(pointer + devn_upperCyl,      u32(upperCyl));
    mem.patch(pointer + devn_numBuffers,    u32(1));
    mem.patch(pointer + devn_memBufType,    u32(0));
    mem.patch(pointer + devn_transferSize,  u32(0x7FFFFFFF));
    mem.patch(pointer + devn_addMask,       u32(0xFFFFFFFE));
    mem.patch(pointer + devn_bootPrio,      u32(0));
    mem.patch(pointer + devn_dName,         u32(0x444f5300));
    mem.patch(pointer + devn_bootflags,     u32(1));
    mem.patch(pointer + devn_segList,       u32(0));

    debug(HDR_DEBUG, "Initialization done\n");
}

void
HdrController::processResource()
{
    trace(HDR_DEBUG, "processResource()\n");
}

void
HdrController::processInfoReq()
{
    trace(HDR_DEBUG, "processInfoReq()\n");
}

void
HdrController::processInitSeg()
{
    trace(HDR_DEBUG, "processInitSeg()\n");
}
