// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "OSDebugger.h"
#include "IOUtils.h"
#include "Memory.h"

void
OSDebugger::read(u32 addr, u8 *result) const
{
    *result = mem.spypeek8 <ACCESSOR_CPU> (addr);
}

void
OSDebugger::read(u32 addr, u16 *result) const
{
    *result = mem.spypeek16 <ACCESSOR_CPU> (addr);
}

void
OSDebugger::read(u32 addr, u32 *result) const
{
    *result = mem.spypeek32 <ACCESSOR_CPU> (addr);
}

void
OSDebugger::read(u32 addr, string &result) const
{
    read(addr, result, 256);
}

void
OSDebugger::read(u32 addr, string &result, isize limit) const
{
    for (isize i = 0; i < limit; i++, addr++) {

        auto c = (char)mem.spypeek8 <ACCESSOR_CPU> (addr);
        
        if (c != 0 && c != '\r' && c != '\n') {
            result += c;
        } else {
            break;
        }
    }
}

//
//
//

os::ExecBase
OSDebugger::getExecBase() const
{
    os::ExecBase result;
    read(mem.spypeek32 <ACCESSOR_CPU> (4), &result);
    return result;
}

void
OSDebugger::read(u32 addr, os::CommandLineInterface *result) const
{
    result->addr = addr;
    
    read(addr +  0, &result->cli_Result2);
    read(addr +  4, &result->cli_SetName);
    read(addr +  8, &result->cli_CommandDir);
    read(addr +  12, &result->cli_ReturnCode);
    read(addr +  16, &result->cli_CommandName);
    read(addr +  20, &result->cli_FailLevel);
    read(addr +  24, &result->cli_Prompt);
    read(addr +  28, &result->cli_StandardInput);
    read(addr +  32, &result->cli_CurrentInput);
    read(addr +  36, &result->cli_CommandFile);
    read(addr +  40, &result->cli_Interactive);
    read(addr +  44, &result->cli_Background);
    read(addr +  48, &result->cli_CurrentOutput);
    read(addr +  52, &result->cli_DefaultStack);
    read(addr +  56, &result->cli_StandardOutput);
    read(addr +  60, &result->cli_Module);
}

void
OSDebugger::read(u32 addr, os::Node *result) const
{
    result->addr = addr;
    
    read(addr +  0, &result->ln_Succ);
    read(addr +  4, &result->ln_Pred);
    read(addr +  8, &result->ln_Type);
    read(addr +  9, &result->ln_Pri);
    read(addr + 10, &result->ln_Name);
}

void
OSDebugger::read(u32 addr, os::Library *result) const
{
    result->addr = addr;

    read(addr + 0,  &result->lib_Node);
    read(addr + 14, &result->lib_Flags);
    read(addr + 15, &result->lib_pad);
    read(addr + 16, &result->lib_NegSize);
    read(addr + 18, &result->lib_PosSize);
    read(addr + 20, &result->lib_Version);
    read(addr + 22, &result->lib_Revision);
    read(addr + 24, &result->lib_IdString);
    read(addr + 28, &result->lib_Sum);
    read(addr + 32, &result->lib_OpenCnt);
}

void
OSDebugger::read(u32 addr, os::IntVector *result) const
{
    result->addr = addr;

    read(addr + 0,  &result->iv_Data);
    read(addr + 4,  &result->iv_Code);
    read(addr + 8,  &result->iv_Node);
}

void
OSDebugger::read(u32 addr, os::List *result) const
{
    result->addr = addr;

    read(addr + 0,  &result->lh_Head);
    read(addr + 4,  &result->lh_Tail);
    read(addr + 8,  &result->lh_TailPred);
    read(addr + 12, &result->lh_Type);
    read(addr + 13, &result->lh_pad);
}

void
OSDebugger::read(u32 addr, os::MinList *result) const
{
    result->addr = addr;

    read(addr + 0,  &result->mlh_Head);
    read(addr + 4,  &result->mlh_Tail);
    read(addr + 8,  &result->mlh_TailPred);
}

void
OSDebugger::read(u32 addr, os::SoftIntList *result) const
{
    result->addr = addr;

    read(addr + 0,  &result->sh_List);
    read(addr + 4,  &result->sh_Pad);
}

void
OSDebugger::read(u32 addr, os::Task *result) const
{
    result->addr = addr;

    read(addr + 0,  &result->tc_Node);
    read(addr + 14, &result->tc_Flags);
    read(addr + 15, &result->tc_State);
    read(addr + 16, &result->tc_IDNestCnt);
    read(addr + 17, &result->tc_TDNestCnt);
    read(addr + 18, &result->tc_SigAlloc);
    read(addr + 22, &result->tc_SigWait);
    read(addr + 26, &result->tc_SigRecvd);
    read(addr + 30, &result->tc_SigExcept);
    read(addr + 34, &result->tc_TrapAlloc);
    read(addr + 36, &result->tc_TrapAble);
    read(addr + 38, &result->tc_ExceptData);
    read(addr + 42, &result->tc_ExceptCode);
    read(addr + 46, &result->tc_TrapData);
    read(addr + 50, &result->tc_TrapCode);
    read(addr + 54, &result->tc_SPReg);
    read(addr + 58, &result->tc_SPLower);
    read(addr + 62, &result->tc_SPUpper);
    read(addr + 66, &result->tc_Switch);
    read(addr + 70, &result->tc_Launch);
    read(addr + 74, &result->tc_MemEntry);
    read(addr + 88, &result->tc_UserData);
}

void
OSDebugger::read(u32 addr, os::MsgPort *result) const
{
    result->addr = addr;

    read(addr + 0,  &result->mp_Node);
    read(addr + 14, &result->mp_Flags);
    read(addr + 15, &result->mp_SigBit);
    read(addr + 16, &result->mp_SigTask);
    read(addr + 20, &result->mp_MsgList);
}

void
OSDebugger::read(u32 addr, os::Process *result) const
{
    result->addr = addr;

    read(addr + 0,   &result->pr_Task);
    read(addr + 92,  &result->pr_MsgPort);
    read(addr + 126, &result->pr_Pad);
    read(addr + 128, &result->pr_SegList);
    read(addr + 132, &result->pr_StackSize);
    read(addr + 136, &result->pr_GlobVec);
    read(addr + 140, &result->pr_TaskNum);
    read(addr + 144, &result->pr_StackBase);
    read(addr + 148, &result->pr_Result2);
    read(addr + 152, &result->pr_CurrentDir);
    read(addr + 156, &result->pr_CIS);
    read(addr + 160, &result->pr_COS);
    read(addr + 164, &result->pr_ConsoleTask);
    read(addr + 168, &result->pr_FileSystemTask);
    read(addr + 172, &result->pr_CLI);
    read(addr + 176, &result->pr_ReturnAddr);
    read(addr + 180, &result->pr_PktWait);
    read(addr + 184, &result->pr_WindowPtr);
    read(addr + 188, &result->pr_HomeDir);
    read(addr + 192, &result->pr_Flags);
    read(addr + 196, &result->pr_ExitCode);
    read(addr + 200, &result->pr_ExitData);
    read(addr + 204, &result->pr_Arguments);
    read(addr + 208, &result->pr_LocalVars);
    read(addr + 220, &result->pr_ShellPrivate);
    read(addr + 224, &result->pr_CES);
}

void
OSDebugger::read(u32 addr, os::ExecBase *result) const
{
    result->addr = addr;

    read(addr + 0,   &result->LibNode);
    read(addr + 34,  &result->SoftVer);
    read(addr + 36,  &result->LowMemChkSum);
    read(addr + 38,  &result->ChkBase);
    read(addr + 42,  &result->ColdCapture);
    read(addr + 46,  &result->CoolCapture);
    read(addr + 50,  &result->WarmCapture);
    read(addr + 54,  &result->SysStkUpper);
    read(addr + 58,  &result->SysStkLower);
    read(addr + 62,  &result->MaxLocMem);
    read(addr + 66,  &result->DebugEntry);
    read(addr + 70,  &result->DebugData);
    read(addr + 74,  &result->AlertData);
    read(addr + 78,  &result->MaxExtMem);
    read(addr + 82,  &result->ChkSum);
    
    for (u32 i = 0; i < 16; i++) {
        read(addr + 84 + i*12,  &result->IntVects[i]);
    }
    
    read(addr + 276,  &result->ThisTask);
    read(addr + 280,  &result->IdleCount);
    read(addr + 284,  &result->DispCount);
    read(addr + 288,  &result->Quantum);
    read(addr + 290,  &result->Elapsed);
    read(addr + 292,  &result->SysFlags);
    read(addr + 294,  &result->IDNestCnt);
    read(addr + 295,  &result->TDNestCnt);
    read(addr + 296,  &result->AttnFlags);
    read(addr + 298,  &result->AttnResched);
    read(addr + 300,  &result->ResModules);
    read(addr + 304,  &result->TaskTrapCode);
    read(addr + 308,  &result->TaskExceptCode);
    read(addr + 312,  &result->TaskExitCode);
    read(addr + 316,  &result->TaskSigAlloc);
    read(addr + 320,  &result->TaskTrapAlloc);
    read(addr + 322,  &result->MemList);
    read(addr + 336,  &result->ResourceList);
    read(addr + 350,  &result->DeviceList);
    read(addr + 364,  &result->IntrList);
    read(addr + 378,  &result->LibList);
    read(addr + 392,  &result->PortList);
    read(addr + 406,  &result->TaskReady);
    read(addr + 420,  &result->TaskWait);
    
    for (u32 i = 0; i < 5; i++) {
        read(addr + 434 + i*16, &result->SoftInts[i]);
    }
    for (u32 i = 0; i < 5; i++) {
        read(addr + 514 + i*4, &result->LastAlert[i]);
    }

    read(addr + 530,  &result->VBlankFrequency);
    read(addr + 531,  &result->PowerSupplyFrequency);
    read(addr + 532,  &result->SemaphoreList);
    read(addr + 546,  &result->KickMemPtr);
    read(addr + 550,  &result->KickTagPtr);
    read(addr + 554,  &result->KickCheckSum);

    read(addr + 558,  &result->ex_Pad0);
    read(addr + 560,  &result->ex_LaunchPoint);
    read(addr + 564,  &result->ex_RamLibPrivate);
    read(addr + 568,  &result->ex_EClockFrequency);
    read(addr + 572,  &result->ex_CacheControl);
    read(addr + 576,  &result->ex_TaskID);
    read(addr + 580,  &result->ex_PuddleSize);
    read(addr + 584,  &result->ex_PoolThreshold);
    read(addr + 588,  &result->ex_PublicPool);
    read(addr + 600,  &result->ex_MMULock);
    
    for (u32 i = 0; i < 12; i++) {
        read(addr + 604 + i, &result->ex_Reserved[i]);
    }
}

void
OSDebugger::read(std::vector <os::Task> &result) const
{
    auto execBase = getExecBase();
        
    os::Task current;
    read(execBase.ThisTask, &current);
    
    result.push_back(current);
    read(execBase.TaskReady.lh_Head, result);
    read(execBase.TaskWait.lh_Head, result);
}

void
OSDebugger::read(std::vector <os::Process> &result) const
{
    std::vector <os::Task> tasks;
    read(tasks);
    
    for (auto &t : tasks) {
        
        if (t.tc_Node.ln_Type == os::NT_PROCESS) {
            
            os::Process process;
            read(t.addr, &process);
            result.push_back(process);
        }
    }
}

void
OSDebugger::read(u32 addr, std::vector <os::Task> &result) const
{
    for (isize i = 0; addr && i < 128; i++) {
        
        os::Task task;
        read(addr, &task);
        
        addr = task.tc_Node.ln_Succ;
        if (addr) result.push_back(task);
    }
}

void
OSDebugger::read(u32 addr, std::vector <os::Library> &result) const
{
    for (isize i = 0; addr && i < 128; i++) {
        
        os::Library library;
        read(addr, &library);
        
        addr = library.lib_Node.ln_Succ;
        if (addr) result.push_back(library);
    }
}

void
OSDebugger::read(const os::Process &pr, std::vector <os::SegList> &result) const
{
    if (pr.pr_CLI) {
        
        os::CommandLineInterface cli;
        read(BPTR(pr.pr_CLI), &cli);

        os::SegList segList;
        read(BPTR(cli.cli_Module), segList);
        
        result.push_back(segList);
        
    } else {
    
        read(BPTR(pr.pr_SegList), result);
    }
}

void
OSDebugger::read(u32 addr, std::vector <os::SegList> &result) const
{
    if (addr == 0) return;
    
    auto arraySize = mem.spypeek32 <ACCESSOR_CPU> (addr);
    for (u32 i = 1; i <= arraySize && i < 128; i++) {
        
        auto listAddr = BPTR(mem.spypeek32 <ACCESSOR_CPU> (addr + 4 * i));
        
        os::SegList list;
        read(listAddr, list);
        result.push_back(list);
    }
}

void
OSDebugger::read(u32 addr, os::SegList &result) const
{
    if (addr == 0) return;
    
    for (isize i = 0; addr && i < 128; i++) {
        
        auto size = mem.spypeek32 <ACCESSOR_CPU> (addr - 4);
        auto next = mem.spypeek32 <ACCESSOR_CPU> (addr);
        auto data = addr + 4;
        
        result.push_back(std::make_pair(size, data));
        addr = BPTR(next);
    }
}

