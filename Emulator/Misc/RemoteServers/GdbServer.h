// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "RemoteServer.h"
#include "OSDebugger.h"

enum class GdbCmd
{
    Attached,
    C,
    ContQ,
    Cont,
    MustReplyEmpty,
    CtrlC,
    Offset,
    StartNoAckMode,
    sThreadInfo,
    Supported,
    Symbol,
    TfV,
    TfP,
    TStatus,
    fThreadInfo,
};

enum regnames {
    D0, D1, D2, D3, D4, D5, D6, D7,
    A0, A1, A2, A3, A4, A5, A6, A7,
    SR, PC
};

class GdbServer : public RemoteServer {
    // Traceframe default index
    static const int DEFAULT_TRACEFRAME = -1; 

    // Id for the cpu thread
    static const int THREAD_ID_CPU      =  1; 

    // Id for the copper thread
    static const int THREAD_ID_COPPER   =  2; 

    // The name of the process to be debugged
    string processName;
    
    // The segment list of the debug process
    os::SegList segList;
    
    // The most recently processed command string
    string latestCmd;
    
    // Indicates whether received packets should be acknowledged
    bool ackMode = true;

    // The current trace frame index
    int currentTraceFrame = DEFAULT_TRACEFRAME;

    //
    // Initializing
    //
    
public:

    GdbServer(Amiga& ref);

    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "GdbServer"; }
    void _dump(dump::Category category, std::ostream& os) const override;
    
    
    //
    // Methods from RemoteServer
    //
    
public:
    
    ServerConfig getDefaultConfig() override;
    bool shouldRun() override;
    string doReceive() override throws;
    void doSend(const string &payload) override throws;
    void doProcess(const string &payload) override throws;
    void didStart() override;
    void didStop() override;
    void didConnect() override;

            
    //
    // Attaching and detaching processes
    //
    
    // Attach a process to the GDB server
    bool attach(const string &name);

    // Detaches the attached process
    void detach();
    
    // Tries to reads the segList for the attached process
    bool readSegList();
    
    // Queries segment information about the attached process
    u32 codeSeg() const;
    u32 dataSeg() const;
    u32 bssSeg() const;

    
    //
    // Managing checksums
    //

    // Computes a checksum for a given string
    string computeChecksum(const string &s);

    // Verifies the checksum for a given string
    bool verifyChecksum(const string &s, const string &chk);

      
    //
    // Handling packets
    //

public:
        
    // Processes a packet in the format used by GDB
    void process(string packet) throws;

    // Processes a checksum-free packet with the first letter stripped off
    void process(char letter, string packet) throws;

private:
        
    // Processes a single command (GdbServerCmds.cpp)
    template <char letter> void process(string arg) throws;
    template <char letter, GdbCmd cmd> void process(string arg) throws;
    
    // Sends a packet with control characters and a checksum attached
    void reply(const string &payload);

    
    // Gets the current trace frame index
    int getCurrentTraceFrame();
    
    // Sets the current trace frame index
    void setCurrentTraceFrame(int traceFrame);
    
    //
    // Reading the emulator state
    //
    
    // Reads a register value
    string readRegister(isize nr);

    // Reads a byte from memory
    string readMemory(isize addr);

    // Returns the current copper address
    string getCopperCurrentAddress();

    //
    // Delegation methods
    //
    
public:
    
    void breakpointReached();
};
