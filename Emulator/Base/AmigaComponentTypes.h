// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(OPT)
{
    // Agnus
    OPT_AGNUS_REVISION,
    OPT_SLOW_RAM_MIRROR,
    
    // Denise
    OPT_DENISE_REVISION,
    OPT_VIEWPORT_TRACKING,
    
    // Pixel engine
    OPT_PALETTE,
    OPT_BRIGHTNESS,
    OPT_CONTRAST,
    OPT_SATURATION,
    
    // DMA Debugger
    OPT_DMA_DEBUG_ENABLE,
    OPT_DMA_DEBUG_MODE,
    OPT_DMA_DEBUG_COLOR,
    OPT_DMA_DEBUG_OPACITY,
    
    // CPU
    OPT_REG_RESET_VAL,
    
    // Real-time clock
    OPT_RTC_MODEL,

    // Memory
    OPT_CHIP_RAM,
    OPT_SLOW_RAM,
    OPT_FAST_RAM,
    OPT_EXT_START,
    OPT_SAVE_ROMS,
    OPT_SLOW_RAM_DELAY,
    OPT_BANKMAP,
    OPT_UNMAPPING_TYPE,
    OPT_RAM_INIT_PATTERN,
    
    // Disk controller
    OPT_DRIVE_CONNECT,
    OPT_DRIVE_SPEED,
    OPT_LOCK_DSKSYNC,
    OPT_AUTO_DSKSYNC,

    // Floppy Drives
    OPT_DRIVE_TYPE,
    OPT_EMULATE_MECHANICS,
    OPT_START_DELAY,
    OPT_STOP_DELAY,
    OPT_STEP_DELAY,
    OPT_DISK_SWAP_DELAY,
    OPT_DRIVE_PAN,
    OPT_STEP_VOLUME,
    OPT_POLL_VOLUME,
    OPT_INSERT_VOLUME,
    OPT_EJECT_VOLUME,
    
    // Hard Drives
    OPT_HDR_TYPE,
    OPT_HDR_CONNECT,
    OPT_HDR_PAN,
    OPT_HDR_STEP_VOLUME,
    
    // Ports
    OPT_SERIAL_DEVICE,

    // Compatibility
    OPT_HIDDEN_BITPLANES,
    OPT_HIDDEN_SPRITES,
    OPT_HIDDEN_LAYERS,
    OPT_HIDDEN_LAYER_ALPHA,
    OPT_CLX_SPR_SPR,
    OPT_CLX_SPR_PLF,
    OPT_CLX_PLF_PLF,
        
    // Blitter
    OPT_BLITTER_ACCURACY,
    
    // CIAs
    OPT_CIA_REVISION,
    OPT_TODBUG,
    OPT_ECLOCK_SYNCING,
    
    // Keyboard
    OPT_ACCURATE_KEYBOARD,
    
    // Mouse
    OPT_PULLUP_RESISTORS,
    OPT_SHAKE_DETECTION,
    OPT_MOUSE_VELOCITY,
    
    // Joystick
    OPT_AUTOFIRE,
    OPT_AUTOFIRE_BULLETS,
    OPT_AUTOFIRE_DELAY,
    
    // Paula audio
    OPT_SAMPLING_METHOD,
    OPT_FILTER_TYPE,
    OPT_FILTER_ALWAYS_ON,
    OPT_AUDPAN,
    OPT_AUDVOL,
    OPT_AUDVOLL,
    OPT_AUDVOLR,
    
    // Expansion boards
    OPT_DIAG_BOARD,
    
    // Remote servers
    OPT_SRV_PORT,
    OPT_SRV_PROTOCOL,
    OPT_SRV_AUTORUN,
    OPT_SRV_VERBOSE
};
typedef OPT Option;

#ifdef __cplusplus
struct OptionEnum : util::Reflection<OptionEnum, Option>
{    
    static long minVal() { return 0; }
    static long maxVal() { return OPT_AUDVOLR; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }

    static const char *prefix() { return "OPT"; }
    static const char *key(Option value)
    {
        switch (value) {
                
            case OPT_AGNUS_REVISION:        return "AGNUS_REVISION";
            case OPT_SLOW_RAM_MIRROR:       return "SLOW_RAM_MIRROR";
                
            case OPT_DENISE_REVISION:       return "DENISE_REVISION";
            case OPT_VIEWPORT_TRACKING:     return "VIEWPORT_TRACKING";
                
            case OPT_PALETTE:               return "PALETTE";
            case OPT_BRIGHTNESS:            return "BRIGHTNESS";
            case OPT_CONTRAST:              return "CONTRAST";
            case OPT_SATURATION:            return "SATURATION";

            case OPT_DMA_DEBUG_ENABLE:      return "DMA_DEBUG_ENABLE";
            case OPT_DMA_DEBUG_MODE:        return "DMA_DEBUG_MODE";
            case OPT_DMA_DEBUG_COLOR:       return "DMA_DEBUG_COLOR";
            case OPT_DMA_DEBUG_OPACITY:     return "DMA_DEBUG_OPACITY";

            case OPT_REG_RESET_VAL:         return "REG_RESET_VAL";
                
            case OPT_RTC_MODEL:             return "RTC_MODEL";

            case OPT_CHIP_RAM:              return "CHIP_RAM";
            case OPT_SLOW_RAM:              return "SLOW_RAM";
            case OPT_FAST_RAM:              return "FAST_RAM";
            case OPT_EXT_START:             return "EXT_START";
            case OPT_SAVE_ROMS:             return "SAVE_ROMS";
            case OPT_SLOW_RAM_DELAY:        return "SLOW_RAM_DELAY";
            case OPT_BANKMAP:               return "BANKMAP";
            case OPT_UNMAPPING_TYPE:        return "UNMAPPING_TYPE";
            case OPT_RAM_INIT_PATTERN:      return "RAM_INIT_PATTERN";
                
            case OPT_DRIVE_CONNECT:         return "DRIVE_CONNECT";
            case OPT_DRIVE_SPEED:           return "DRIVE_SPEED";
            case OPT_LOCK_DSKSYNC:          return "LOCK_DSKSYNC";
            case OPT_AUTO_DSKSYNC:          return "AUTO_DSKSYNC";

            case OPT_DRIVE_TYPE:            return "DRIVE_TYPE";
            case OPT_EMULATE_MECHANICS:     return "EMULATE_MECHANICS";
            case OPT_START_DELAY:           return "START_DELAY";
            case OPT_STOP_DELAY:            return "STOP_DELAY";
            case OPT_STEP_DELAY:            return "STEP_DELAY";
            case OPT_DISK_SWAP_DELAY:       return "DISK_SWAP_DELAY";
            case OPT_DRIVE_PAN:             return "DRIVE_PAN";
            case OPT_STEP_VOLUME:           return "STEP_VOLUME";
            case OPT_POLL_VOLUME:           return "POLL_VOLUME";
            case OPT_INSERT_VOLUME:         return "INSERT_VOLUME";
            case OPT_EJECT_VOLUME:          return "EJECT_VOLUME";
                
            case OPT_HDR_TYPE:              return "HDR_TYPE";
            case OPT_HDR_CONNECT:           return "HDR_CONNECT";
            case OPT_HDR_PAN:               return "HDR_PAN";
            case OPT_HDR_STEP_VOLUME:       return "HDR_STEP_VOLUME";

            case OPT_SERIAL_DEVICE:         return "SERIAL_DEVICE";

            case OPT_HIDDEN_BITPLANES:      return "HIDDEN_BITPLANES";
            case OPT_HIDDEN_SPRITES:        return "HIDDEN_SPRITES";
            case OPT_HIDDEN_LAYERS:         return "HIDDEN_LAYERS";
            case OPT_HIDDEN_LAYER_ALPHA:    return "HIDDEN_LAYER_ALPHA";
            case OPT_CLX_SPR_SPR:           return "CLX_SPR_SPR";
            case OPT_CLX_SPR_PLF:           return "CLX_SPR_PLF";
            case OPT_CLX_PLF_PLF:           return "CLX_PLF_PLF";
                    
            case OPT_BLITTER_ACCURACY:      return "BLITTER_ACCURACY";
                
            case OPT_CIA_REVISION:          return "CIA_REVISION";
            case OPT_TODBUG:                return "TODBUG";
            case OPT_ECLOCK_SYNCING:        return "ECLOCK_SYNCING";
                
            case OPT_ACCURATE_KEYBOARD:     return "ACCURATE_KEYBOARD";

            case OPT_PULLUP_RESISTORS:      return "PULLUP_RESISTORS";
            case OPT_SHAKE_DETECTION:       return "SHAKE_DETECTION";
            case OPT_MOUSE_VELOCITY:        return "MOUSE_VELOCITY";

            case OPT_AUTOFIRE:              return "AUTOFIRE";
            case OPT_AUTOFIRE_BULLETS:      return "AUTOFIRE_BULLETS";
            case OPT_AUTOFIRE_DELAY:        return "AUTOFIRE_DELAY";

            case OPT_SAMPLING_METHOD:       return "SAMPLING_METHOD";
            case OPT_FILTER_TYPE:           return "FILTER_TYPE";
            case OPT_FILTER_ALWAYS_ON:      return "FILTER_ALWAYS_ON";
            case OPT_AUDPAN:                return "AUDPAN";
            case OPT_AUDVOL:                return "AUDVOL";
            case OPT_AUDVOLL:               return "AUDVOLL";
            case OPT_AUDVOLR:               return "AUDVOLR";

            case OPT_DIAG_BOARD:            return "DIAG_BOARD";

            case OPT_SRV_PORT:              return "SRV_PORT";
            case OPT_SRV_PROTOCOL:          return "SRV_PROTOCOL";
            case OPT_SRV_AUTORUN:           return "SRV_AUTORUN";
            case OPT_SRV_VERBOSE:           return "SRV_VERBOSE";
        }
        return "???";
    }
};
#endif

enum_long(EMULATOR_STATE)
{
    EMULATOR_STATE_OFF,
    EMULATOR_STATE_PAUSED,
    EMULATOR_STATE_RUNNING
};
typedef EMULATOR_STATE EmulatorState;

#ifdef __cplusplus
struct EmulatorStateEnum : util::Reflection<EmulatorStateEnum, EmulatorState>
{
    static long minVal() { return 0; }
    static long maxVal() { return EMULATOR_STATE_RUNNING; }
    static bool isValid(auto val) { return val >= minVal() && val <= maxVal(); }

    static const char *prefix() { return "EMULATOR_STATE"; }
    static const char *key(EmulatorState value)
    {
        switch (value) {
                
            case EMULATOR_STATE_OFF:      return "OFF";
            case EMULATOR_STATE_PAUSED:   return "PAUSED";
            case EMULATOR_STATE_RUNNING:  return "RUNNING";
        }
        return "???";
    }
};
#endif
