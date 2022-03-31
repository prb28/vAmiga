// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Thread.h"
#include "Chrono.h"
#include <iostream>

Thread::Thread()
{
    // Initialize the sync timer
    targetTime = util::Time::now();
    
    // Start the thread and enter the main function
    thread = std::thread(&Thread::main, this);
}

Thread::~Thread()
{
    // Wait until the thread has terminated
    join();
}

template <> void
Thread::execute <Thread::SyncMode::Periodic> ()
{
    // Call the execution function
    loadClock.go();
    execute();
    loadClock.stop();
}

template <> void
Thread::execute <Thread::SyncMode::Pulsed> ()
{
    // Call the execution function
    loadClock.go();
    execute();
    loadClock.stop();
    
}

template <> void
Thread::sleep <Thread::SyncMode::Periodic> ()
{
    auto now = util::Time::now();

    // Only proceed if we're not running in warp mode
    if (warpMode) return;
        
    // Check if we're running too slow...
    if (now > targetTime) {
        
        // Check if we're completely out of sync...
        if ((now - targetTime).asMilliseconds() > 200) {
                        
            warn("Emulation is way too slow: %f\n",(now - targetTime).asSeconds());

            // Restart the sync timer
            targetTime = util::Time::now();
        }
    }
    
    // Check if we're running too fast...
    if (now < targetTime) {
        
        // Check if we're completely out of sync...
        if ((targetTime - now).asMilliseconds() > 200) {
            
            warn("Emulation is way too slow: %f\n",(targetTime - now).asSeconds());

            // Restart the sync timer
            targetTime = util::Time::now();
        }
    }
        
    // Sleep for a while
    // std::cout << "Sleeping... " << targetTime.asMilliseconds() << std::endl;
    // std::cout << "Delay = " << delay.asNanoseconds() << std::endl;
    targetTime += delay;
    targetTime.sleepUntil();
}

template <> void
Thread::sleep <Thread::SyncMode::Pulsed> ()
{
    // Wait for the next pulse
    if (!warpMode) waitForWakeUp();
}

void
Thread::main()
{
    debug(RUN_DEBUG, "main()\n");
          
    while (++loopCounter) {
           
        if (isRunning()) {
                        
            switch (mode) {
                case SyncMode::Periodic: execute<SyncMode::Periodic>(); break;
                case SyncMode::Pulsed: execute<SyncMode::Pulsed>(); break;
            }
        }
                
        if (!warpMode || !isRunning()) {
            
            switch (mode) {
                case SyncMode::Periodic: sleep<SyncMode::Periodic>(); break;
                case SyncMode::Pulsed: sleep<SyncMode::Pulsed>(); break;
            }
        }
        
        // Are we requested to enter or exit warp mode?
        if (newWarpMode != warpMode) {
            
            AmigaComponent::warpOnOff(newWarpMode);
            warpMode = newWarpMode;
        }

        // Are we requested to enter or exit warp mode?
        if (newDebugMode != debugMode) {
            
            AmigaComponent::debugOnOff(newDebugMode);
            debugMode = newDebugMode;
        }

        // Are we requested to change state?
        while (newState != state) {
            
            if (state == EXEC_OFF && newState == EXEC_PAUSED) {
                
                AmigaComponent::powerOn();
                state = EXEC_PAUSED;

            } else if (state == EXEC_PAUSED && newState == EXEC_OFF) {
                
                AmigaComponent::powerOff();
                state = EXEC_OFF;
            
            } else if (state == EXEC_PAUSED && newState == EXEC_RUNNING) {
                
                AmigaComponent::run();
                state = EXEC_RUNNING;
            
            } else if (state == EXEC_RUNNING && newState == EXEC_OFF) {
                
                AmigaComponent::pause();
                state = EXEC_PAUSED;
            
            } else if (state == EXEC_RUNNING && newState == EXEC_PAUSED) {
                
                AmigaComponent::pause();
                state = EXEC_PAUSED;

            } else if (state == EXEC_RUNNING && newState == EXEC_SUSPENDED) {
                
                state = EXEC_SUSPENDED;

            } else if (state == EXEC_SUSPENDED && newState == EXEC_RUNNING) {
                
                state = EXEC_RUNNING;

            } else if (newState == EXEC_HALTED) {
                
                AmigaComponent::halt();
                state = EXEC_HALTED;
                return;

            } else {
                
                // Invalid state transition
                fatalError;
            }
            
            debug(RUN_DEBUG, "Changed state to %s\n", ExecutionStateEnum::key(state));
        }
        
        // Compute the CPU load once in a while
        if (loopCounter % 32 == 0) {
            
            auto used  = loadClock.getElapsedTime().asSeconds();
            auto total = nonstopClock.getElapsedTime().asSeconds();
            
            cpuLoad = used / total;
            
            loadClock.restart();
            loadClock.stop();
            nonstopClock.restart();
        }
    }
}

void
Thread::setSyncDelay(util::Time newDelay)
{
    delay = newDelay;
}

void
Thread::setMode(SyncMode newMode)
{
    mode = newMode;
}

void
Thread::setWarpLock(bool value)
{
    warpLock = value;
}

void
Thread::setDebugLock(bool value)
{
    debugLock = value;
}

void
Thread::powerOn(bool blocking)
{
    debug(RUN_DEBUG, "powerOn()\n");

    // Never call this function inside the emulator thread
    assert(!isEmulatorThread());
    
    if (isPoweredOff()) {
        
        // Request a state change and wait until the new state has been reached
        changeStateTo(EXEC_PAUSED, blocking);
    }
}

void
Thread::powerOff(bool blocking)
{
    debug(RUN_DEBUG, "powerOff()\n");

    // Never call this function inside the emulator thread
    assert(!isEmulatorThread());
    
    if (!isPoweredOff()) {
                
        // Request a state change and wait until the new state has been reached
        changeStateTo(EXEC_OFF, blocking);
    }
}

void
Thread::run(bool blocking)
{
    debug(RUN_DEBUG, "run()\n");

    // Never call this function inside the emulator thread
    assert(!isEmulatorThread());

    // The emulator is expected to be powered on
    if (isPoweredOff()) throw VAError(ERROR_POWERED_OFF);
        
    if (!isRunning()) {

        // Throw an exception if the emulator is not ready to run
        isReady();
        
        // Request a state change and wait until the new state has been reached
        changeStateTo(EXEC_RUNNING, blocking);
    }
}

void
Thread::pause(bool blocking)
{
    debug(RUN_DEBUG, "pause()\n");

    // Never call this function inside the emulator thread
    assert(!isEmulatorThread());
    
    if (isRunning()) {
                
        // Request a state change and wait until the new state has been reached
        changeStateTo(EXEC_PAUSED, blocking);
    }
}

void
Thread::halt(bool blocking)
{
    assert(!isEmulatorThread());
    
    changeStateTo(EXEC_HALTED, blocking);
    join();
}

void
Thread::warpOn(isize source)
{
    assert(source >= 0 && source < 8);
    
    if (!warpLock) changeWarpTo(warpMode | (u8)(1 << source));
}

void
Thread::warpOff(isize source)
{
    assert(source >= 0 && source < 8);
    
    if (!warpLock) changeWarpTo(warpMode & ~(u8)(1 << source));
}

void
Thread::debugOn(isize source)
{
    assert(source >= 0 && source < 8);
    
    if (!debugLock) changeDebugTo(debugMode | (u8)(1 << source));
}

void
Thread::debugOff(isize source)
{
    if (!debugLock) changeDebugTo(debugMode & ~(u8)(1 << source));
}

void
Thread::changeStateTo(ExecutionState requestedState, bool blocking)
{
    newState = requestedState;
    if (blocking) while (state != requestedState) { };
}

void
Thread::changeWarpTo(u8 value, bool blocking)
{
    newWarpMode = value;
    if (blocking) while (warpMode != newWarpMode) { };
}

void
Thread::changeDebugTo(u8 value, bool blocking)
{
    newDebugMode = value;
    if (blocking) while (debugMode != newDebugMode) { };
}

void
Thread::wakeUp()
{
    if (mode == SyncMode::Pulsed) util::Wakeable::wakeUp();
}

void
Thread::suspend()
{
    debug(RUN_DEBUG, "Suspending (%ld)...\n", suspendCounter);
    
    if (suspendCounter || isRunning()) {

        suspendCounter++;
        assert(state == EXEC_RUNNING || state == EXEC_SUSPENDED);
        changeStateTo(EXEC_SUSPENDED, true);
    }
}

void
Thread::resume()
{
    debug(RUN_DEBUG, "Resuming (%ld)...\n", suspendCounter);

    if (suspendCounter && --suspendCounter == 0) {
        
        assert(state == EXEC_SUSPENDED);
        changeStateTo(EXEC_RUNNING, true);
        run();
    }
}
