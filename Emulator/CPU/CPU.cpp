// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "CPU.h"
#include "Agnus.h"
#include "Amiga.h"
#include "IOUtils.h"
#include "Memory.h"
#include "MsgQueue.h"

//
// Moira
//

namespace moira {

void
Moira::sync(int cycles)
{
    // Advance the CPU clock
    clock += cycles;

    // Emulate Agnus up to the same cycle
    agnus.execute(CPU_AS_DMA_CYCLES(cycles));
}

u8
Moira::read8(u32 addr)
{
    return mem.peek8 <ACCESSOR_CPU> (addr);
}

u16
Moira::read16(u32 addr)
{
    return mem.peek16 <ACCESSOR_CPU> (addr); 
}

u16
Moira::read16Dasm(u32 addr)
{
    return mem.spypeek16 <ACCESSOR_CPU> (addr);
}

u16
Moira::read16OnReset(u32 addr)
{
    return mem.chip ? read16(addr) : 0;
}

void
Moira::write8(u32 addr, u8 val)
{
    if constexpr (XFILES) if (addr - reg.pc < 5) {
        trace(true, "XFILES: write8 close to PC %x\n", reg.pc);
    }
    mem.poke8 <ACCESSOR_CPU> (addr, val);
}

void
Moira::write16 (u32 addr, u16 val)
{
    if constexpr (XFILES) if (addr - reg.pc < 5) {
        trace(true, "XFILES: write16 close to PC %x\n", reg.pc);
    }
    mem.poke16 <ACCESSOR_CPU> (addr, val);
}

u16
Moira::readIrqUserVector(u8 level) const
{
    return 0;
}

void
Moira::signalResetInstr()
{
    trace(XFILES, "XFILES: RESET instruction\n");
    amiga.softReset();
}

void
Moira::signalStopInstr(u16 op)
{
    if constexpr (XFILES) {
        if (!(op & 0x2000)) trace(true, "XFILES: STOP instruction (%x)\n", op);
    }
}

void
Moira::signalTasInstr()
{
    trace(XFILES, "XFILES: TAS instruction\n");
}

void
Moira::signalHalt()
{
    msgQueue.put(MSG_CPU_HALT);
}

void
Moira::signalAddressError(moira::AEStackFrame &frame)
{
    trace(XFILES, "XFILES: Address error exception %x %x %x %x %x\n",
          frame.code, frame.addr, frame.ird, frame.sr, frame.pc);
}

void
Moira::signalLineAException(u16 opcode)
{
    trace(XFILES, "XFILES: lineAException(%x)\n", opcode);
}

void
Moira::signalLineFException(u16 opcode)
{
    trace(XFILES, "XFILES: lineFException(%x)\n", opcode);
}

void
Moira::signalIllegalOpcodeException(u16 opcode)
{
    trace(XFILES, "XFILES: illegalOpcodeException(%x)\n", opcode);
}

void
Moira::signalTraceException()
{

}

void
Moira::signalTrapException()
{
    trace(XFILES, "XFILES: trapException\n");
}

void
Moira::signalPrivilegeViolation()
{
    
}

void
Moira::signalInterrupt(u8 level)
{
    debug(INT_DEBUG, "Executing level %d IRQ\n", level);
}

void
Moira::signalJumpToVector(int nr, u32 addr)
{
    bool isIrqException = nr >= 24 && nr <= 31;

    if (isIrqException) {
        trace(INT_DEBUG, "Exception %d: Changing PC to %x\n", nr, addr);
    }
}

void
Moira::signalSoftwareTrap(u16 instr, SoftwareTrap trap)
{
 
}

void
Moira::addressErrorHandler()
{
    
}

void
Moira::softstopReached(u32 addr)
{
    amiga.setFlag(RL::SOFTSTOP_REACHED);
}

void
Moira::breakpointReached(u32 addr)
{
    amiga.setFlag(RL::BREAKPOINT_REACHED);
}

void
Moira::watchpointReached(u32 addr)
{
    amiga.setFlag(RL::WATCHPOINT_REACHED);
}

void
Moira::catchpointReached(u8 vector)
{
    amiga.setFlag(RL::CATCHPOINT_REACHED);
}

void
Moira::swTrapReached(u32 addr)
{
    amiga.setFlag(RL::SWTRAP_REACHED);
}

void
Moira::execDebug(const char *cmd)
{
    if (agnus.pos.v == 76 || agnus.pos.v == 77) {
        trace(true, "%s\n", cmd);
    }
}

}

//
// CPU
//

CPU::CPU(Amiga& ref) : moira::Moira(ref)
{
    
}

i64
CPU::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_REG_RESET_VAL:  return (long)config.regResetVal;
        
        default:
            fatalError;
    }
}

void
CPU::setConfigItem(Option option, i64 value)
{
    switch (option) {
            
        case OPT_REG_RESET_VAL:

            config.regResetVal = (u32)value;
            return;
                        
        default:
            fatalError;
    }
}

CPUConfig
CPU::getDefaultConfig()
{
    CPUConfig defaults;

    defaults.regResetVal = 0x00000000;
    
    return defaults;
}

void
CPU::resetConfig()
{
    auto defaults = getDefaultConfig();

    setConfigItem(OPT_REG_RESET_VAL, defaults.regResetVal);
}

void
CPU::_reset(bool hard)
{    
    RESET_SNAPSHOT_ITEMS(hard)

    if (hard) {
                
        // Reset the Moira core
        Moira::reset();
        
        // Initialize all data and address registers with the startup value
        for(int i = 0; i < 8; i++) reg.d[i] = reg.a[i] = config.regResetVal;
        
        // Remove all previously recorded instructions
        debugger.clearLog();
        
    } else {
        
        /* "The RESET instruction causes the processor to assert RESET for 124
         *  clock periods toreset the external devices of the system. The
         *  internal state of the processor is notaffected. Neither the status
         *  register nor any of the internal registers is affected by an
         *  internal reset operation. All external devices in the system should
         *  be reset at the completion of the RESET instruction."
         *  [Motorola M68000 User Manual]
         */            
    }
}

void
CPU::_inspect() const
{
    _inspect(getPC0());
}

void
CPU::_inspect(u32 dasmStart) const
{
    {   SYNCHRONIZED
        
        info.pc0 = getPC0() & 0xFFFFFF;
        
        for (isize i = 0; i < 8; i++) {
            info.d[i] = getD((int)i);
            info.a[i] = getA((int)i);
        }
        info.usp = getUSP();
        info.ssp = getSSP();
        info.sr = getSR();
    }
}

void
CPU::_dump(Category category, std::ostream& os) const
{
    if (category == Category::Config) {
        
        os << util::tab("Register reset value");
        os << util::hex(config.regResetVal) << std::endl;
    }
    
    if (category == Category::State) {
        
        os << util::tab("Clock");
        os << util::dec(clock) << std::endl;
        os << util::tab("Control flags");
        os << util::hex((u16)flags) << std::endl;
        os << util::tab("Last exception");
        os << util::dec(exception);
    }

    if (category == Category::Registers) {
        
        os << util::tab("PC");
        os << util::hex(reg.pc0) << std::endl;
        os << std::endl;
        
        os << util::tab("SSP");
        os << util::hex(reg.ssp) << std::endl;
        os << util::tab("USP");
        os << util::hex(reg.usp) << std::endl;
        os << util::tab("IRC");
        os << util::hex(queue.irc) << std::endl;
        os << util::tab("IRD");
        os << util::hex(queue.ird) << std::endl;
        os << std::endl;
        
        os << util::tab("D0 - D3");
        os << util::hex(reg.d[0]) << ' ' << util::hex(reg.d[1]) << ' ';
        os << util::hex(reg.d[2]) << ' ' << util::hex(reg.d[3]) << ' ' << std::endl;
        os << util::tab("D4 - D7");
        os << util::hex(reg.d[4]) << ' ' << util::hex(reg.d[5]) << ' ';
        os << util::hex(reg.d[6]) << ' ' << util::hex(reg.d[7]) << ' ' << std::endl;
        os << util::tab("A0 - A3");
        os << util::hex(reg.a[0]) << ' ' << util::hex(reg.a[1]) << ' ';
        os << util::hex(reg.a[2]) << ' ' << util::hex(reg.a[3]) << ' ' << std::endl;
        os << util::tab("A4 - A7");
        os << util::hex(reg.a[4]) << ' ' << util::hex(reg.a[5]) << ' ';
        os << util::hex(reg.a[6]) << ' ' << util::hex(reg.a[7]) << ' ' << std::endl;
        os << std::endl;
        
        os << util::tab("Flags");
        os << (reg.sr.t ? 'T' : 't');
        os << (reg.sr.s ? 'S' : 's') << "--";
        os << "<" << util::dec(reg.sr.ipl) << ">---";
        os << (reg.sr.x ? 'X' : 'x');
        os << (reg.sr.n ? 'N' : 'n');
        os << (reg.sr.z ? 'Z' : 'z');
        os << (reg.sr.v ? 'V' : 'v');
        os << (reg.sr.c ? 'C' : 'c') << std::endl;
    }

    if (category == Category::Breakpoints) {
        
        for (int i = 0; i < debugger.breakpoints.elements(); i++) {
            
            auto bp = debugger.breakpoints.guardNr(i);
            auto nr = "Breakpoint " + std::to_string(i);
            
            os << util::tab(nr);
            os << util::hex(bp->addr);

            if (!bp->enabled) os << " (Disabled)";
            else if (bp->ignore) os << " (Disabled for " << bp->ignore << " hits)";
            os << std::endl;
        }
    }
    
    if (category == Category::Watchpoints) {
        
        for (int i = 0; i < debugger.watchpoints.elements(); i++) {
            
            auto wp = debugger.watchpoints.guardNr(i);
            auto nr = "Watchpoint " + std::to_string(i);
            
            os << util::tab(nr);
            os << util::hex(wp->addr);
            if (!wp->enabled) os << " (Disabled)";
            else if (wp->ignore) os << " (Disabled for " << wp->ignore << " hits)";
            os << std::endl;
        }
    }
    
    if (category == Category::Catchpoints) {
        
        for (int i = 0; i < debugger.catchpoints.elements(); i++) {
            
            auto wp = debugger.catchpoints.guardNr(i);
            auto nr = "Catchpoint " + std::to_string(i);

            os << util::tab(nr);
            os << "Vector " << util::dec(wp->addr);
            os << " (" << cpu.debugger.vectorName(u8(wp->addr)) << ")";
            if (!wp->enabled) os << " (Disabled)";
            else if (wp->ignore) os << " (Disabled for " << wp->ignore << " hits)";
            os << std::endl;
        }
    }

    if (category == Category::SwTraps) {
                
        for (auto &trap : debugger.swTraps.traps) {

            os << util::tab("0x" + util::hexstr <4> (trap.first));
            os << "Replaced by 0x" << util::hexstr <4> (trap.second.instruction);
            os << std::endl;
        }
    }

    if (category == Category::Callstack) {
               
        isize nr = 0;
        for (isize i = callstack.begin(); i != callstack.end(); i = callstack.next(i)) {

            auto &entry = callstack.elements[i];
            string instr = HI_BYTE(entry.opcode) == 0b01100001 ? "BSR " : "JSR ";
            
            os << util::tab("#" + std::to_string(nr++));
            os << util::hex(entry.oldPC) << ": " << instr << util::hex(entry.newPC);
            os << std::endl;
        }
    }
}

void
CPU::_debugOn()
{
    debug(RUN_DEBUG, "Enabling debug mode\n");
    debugger.enableLogging();
}

void
CPU::_debugOff()
{
    debug(RUN_DEBUG, "Disabling debug mode\n");
    debugger.disableLogging();
}

isize
CPU::didLoadFromBuffer(const u8 *buffer)
{
    /* Because we don't save breakpoints and watchpoints in a snapshot, the
     * CPU flags for checking breakpoints and watchpoints can be in a corrupt
     * state after loading. These flags need to be updated according to the
     * current breakpoint and watchpoint list.
     */
    debugger.breakpoints.setNeedsCheck(debugger.breakpoints.elements() != 0);
    debugger.watchpoints.setNeedsCheck(debugger.watchpoints.elements() != 0);
    return 0;
}

const char *
CPU::disassembleRecordedInstr(isize i, isize *len)
{
    return disassembleInstr(debugger.logEntryAbs((int)i).pc0, len);
}
const char *
CPU::disassembleRecordedWords(isize i, isize len)
{
    return disassembleWords(debugger.logEntryAbs((int)i).pc0, len);
}

const char *
CPU::disassembleRecordedFlags(isize i)
{
    static char result[18];
    
    disassembleSR(debugger.logEntryAbs((int)i).sr, result);
    return result;
}

const char *
CPU::disassembleRecordedPC(isize i)
{
    static char result[16];
    
    Moira::disassemblePC(debugger.logEntryAbs((int)i).pc0, result);
    return result;
}

const char *
CPU::disassembleInstr(u32 addr, isize *len)
{
    static char result[128];

    int l = disassemble(addr, result);

    if (len) *len = (isize)l;
    return result;
}

const char *
CPU::disassembleWords(u32 addr, isize len)
{
    static char result[64];

    disassembleMemory(addr, (int)len, result);
    return result;
}

const char *
CPU::disassembleAddr(u32 addr)
{
    static char result[16];

    disassemblePC(addr, result);
    return result;
}

const char *
CPU::disassembleInstr(isize *len)
{
    return disassembleInstr(reg.pc0, len);
}

const char *
CPU::disassembleWords(isize len)
{
    return disassembleWords(reg.pc0, len);
}

void
CPU::jump(u32 addr)
{
    {   SUSPENDED
        
        debugger.jump(addr);
    }
}

void
CPU::signalJsrBsrInstr(u16 opcode, u32 oldPC, u32 newPC)
{
    if (amiga.inDebugMode()) {
        
        trace(CST_DEBUG, "JSR/BSR: %x -> %x [%ld]\n", oldPC, newPC, callstack.count());
        
        if (callstack.isFull()) {
            
            debug(CST_DEBUG, "JSR/BSR: Large stack\n");
            (void)callstack.read();
        }
        
        CallStackEntry entry;
        entry.opcode = opcode;
        entry.oldPC = oldPC;
        entry.newPC = newPC;
        for (isize i = 0; i < 8; i++) entry.d[i] = reg.d[i];
        for (isize i = 0; i < 8; i++) entry.a[i] = reg.a[i];
        
        callstack.write(entry);
    }
}

void
CPU::signalRtsInstr()
{
    if (amiga.inDebugMode()) {
        
        trace(CST_DEBUG, "RTS [%ld]\n", callstack.count());
        
        if (callstack.isEmpty()) {
            
            trace(CST_DEBUG, "RTS: Empty stack\n");
            return;
        }
        
        (void)callstack.read();
    }
}

void
CPU::setBreakpoint(u32 addr)
{
    if (debugger.breakpoints.isSetAt(addr)) throw VAError(ERROR_BP_ALREADY_SET, addr);

    debugger.breakpoints.setAt(addr);
    msgQueue.put(MSG_BREAKPOINT_UPDATED);
}

void
CPU::deleteBreakpoint(isize nr)
{
    if (!debugger.breakpoints.isSet(nr)) throw VAError(ERROR_BP_NOT_FOUND, nr);

    debugger.breakpoints.remove(nr);
    msgQueue.put(MSG_BREAKPOINT_UPDATED);
}

void
CPU::enableBreakpoint(isize nr)
{
    if (!debugger.breakpoints.isSet(nr)) throw VAError(ERROR_BP_NOT_FOUND, nr);

    debugger.breakpoints.enable(nr);
    msgQueue.put(MSG_BREAKPOINT_UPDATED);
}

void
CPU::disableBreakpoint(isize nr)
{
    if (!debugger.breakpoints.isSet(nr)) throw VAError(ERROR_BP_NOT_FOUND, nr);

    debugger.breakpoints.disable(nr);
    msgQueue.put(MSG_BREAKPOINT_UPDATED);
}

void
CPU::ignoreBreakpoint(isize nr, isize count)
{
    if (!debugger.breakpoints.isSet(nr)) throw VAError(ERROR_BP_NOT_FOUND, nr);

    debugger.breakpoints.ignore(nr, count);
    msgQueue.put(MSG_BREAKPOINT_UPDATED);
}

void
CPU::setWatchpoint(u32 addr)
{
    if (debugger.watchpoints.isSetAt(addr)) throw VAError(ERROR_WP_ALREADY_SET, addr);

    debugger.watchpoints.setAt(addr);
    msgQueue.put(MSG_WATCHPOINT_UPDATED);
}

void
CPU::deleteWatchpoint(isize nr)
{
    if (!debugger.watchpoints.isSet(nr)) throw VAError(ERROR_WP_NOT_FOUND, nr);

    debugger.watchpoints.remove(nr);
    msgQueue.put(MSG_WATCHPOINT_UPDATED);
}

void
CPU::enableWatchpoint(isize nr)
{
    if (!debugger.watchpoints.isSet(nr)) throw VAError(ERROR_WP_NOT_FOUND, nr);

    debugger.watchpoints.enable(nr);
    msgQueue.put(MSG_WATCHPOINT_UPDATED);
}

void
CPU::disableWatchpoint(isize nr)
{
    if (!debugger.watchpoints.isSet(nr)) throw VAError(ERROR_WP_NOT_FOUND, nr);

    debugger.watchpoints.disable(nr);
    msgQueue.put(MSG_WATCHPOINT_UPDATED);
}

void
CPU::ignoreWatchpoint(isize nr, isize count)
{
    if (!debugger.watchpoints.isSet(nr)) throw VAError(ERROR_WP_NOT_FOUND, nr);

    debugger.watchpoints.ignore(nr, count);
    msgQueue.put(MSG_WATCHPOINT_UPDATED);
}

void
CPU::setCatchpoint(u8 vector)
{
    if (debugger.catchpoints.isSetAt(vector)) throw VAError(ERROR_CP_ALREADY_SET, vector);

    debugger.catchpoints.setAt(vector);
    msgQueue.put(MSG_CATCHPOINT_UPDATED);
}

void
CPU::deleteCatchpoint(isize nr)
{
    if (!debugger.catchpoints.isSet(nr)) throw VAError(ERROR_CP_NOT_FOUND, nr);

    debugger.catchpoints.remove(nr);
    msgQueue.put(MSG_CATCHPOINT_UPDATED);
}

void
CPU::enableCatchpoint(isize nr)
{
    if (!debugger.catchpoints.isSet(nr)) throw VAError(ERROR_CP_NOT_FOUND, nr);

    debugger.catchpoints.enable(nr);
    msgQueue.put(MSG_CATCHPOINT_UPDATED);
}

void
CPU::disableCatchpoint(isize nr)
{
    if (!debugger.catchpoints.isSet(nr)) throw VAError(ERROR_CP_NOT_FOUND, nr);

    debugger.catchpoints.disable(nr);
    msgQueue.put(MSG_CATCHPOINT_UPDATED);
}

void
CPU::ignoreCatchpoint(isize nr, isize count)
{
    if (!debugger.catchpoints.isSet(nr)) throw VAError(ERROR_CP_NOT_FOUND, nr);

    debugger.catchpoints.ignore(nr, count);
    msgQueue.put(MSG_CATCHPOINT_UPDATED);
}
