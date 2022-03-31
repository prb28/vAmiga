// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "HdControllerTypes.h"
#include "ZorroBoard.h"
#include "HDFFile.h"

class HdController : public ZorroBoard {
    
    // Number of this controller
    isize nr;

    // The hard drive this controller is connected to
    HardDrive &drive;
    
    // Rom code
    Buffer<u8> rom;
    
    // Transmitted pointer
    u32 pointer = 0;
    
    
    //
    // Initializing
    //
    
public:
    
    HdController(Amiga& ref, HardDrive& hdr);


    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override;
    void _dump(Category category, std::ostream& os) const override;

    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _reset(bool hard) override;
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {

    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        if (hard) {
            
            worker
            
            << baseAddr
            << state
            << pointer;
        }
    }
    
    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Methods from ZorroBoard
    //
    
public:
    
    virtual bool pluggedIn() const override;
    virtual isize pages() const override         { return 1; }
    virtual u8 type() const override             { return ERT_ZORROII | ERTF_DIAGVALID; }
    virtual u8 product() const override          { return 0x88; }
    virtual u8 flags() const override            { return 0x00; }
    virtual u16 manufacturer() const override    { return 0x0539; }
    virtual u32 serialNumber() const override    { return 3141592 + u32(nr); }
    virtual u16 initDiagVec() const override     { return 0x40; }
    virtual string vendorName() const override   { return "RASTEC"; }
    virtual string productName() const override  { return "HD controller"; }
    virtual string revisionName() const override { return "0.3"; }

private:
    
    void updateMemSrcTables() override;
    

    //
    // Accessing the board
    //
      
public:
        
    u8 peek8(u32 addr) override;
    u16 peek16(u32 addr) override;
    u8 spypeek8(u32 addr) const override;
    u16 spypeek16(u32 addr) const override;
    void poke8(u32 addr, u8 value) override;
    void poke16(u32 addr, u16 value) override;
        
private:
    
    void processCmd();
    void processInit();
    void processResource();
    void processInfoReq();
    void processInitSeg();
};
