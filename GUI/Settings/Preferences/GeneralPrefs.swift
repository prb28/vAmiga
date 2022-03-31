// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension PreferencesController {
    
    func refreshGeneralTab() {
        
        // Initialize combo boxes
        if genFFmpegPath.tag == 0 {
            
            genFFmpegPath.tag = 1
            
            for i in 0...5 {
                if let path = amiga.recorder.findFFmpeg(i) {
                    genFFmpegPath.addItem(withObjectValue: path)
                } else {
                    break
                }
            }
        }
        
        // Snapshots
        genAutoSnapshots.state = pref.autoSnapshots ? .on : .off
        genSnapshotInterval.integerValue = pref.snapshotInterval
        genSnapshotInterval.isEnabled = pref.autoSnapshots

        // Screenshots
        genScreenshotSourcePopup.selectItem(withTag: pref.screenshotSource)
        genScreenshotTargetPopup.selectItem(withTag: pref.screenshotTargetIntValue)
                
        // Screen captures
        let hasFFmpeg = amiga.recorder.hasFFmpeg
        genFFmpegPath.stringValue = amiga.recorder.path
        genFFmpegPath.textColor = hasFFmpeg ? .textColor : .warningColor
        genFFmpegIcon.isHidden = !hasFFmpeg
        genSource.selectItem(withTag: pref.captureSource)
        genBitRate.stringValue = "\(pref.bitRate)"
        genAspectX.integerValue = pref.aspectX
        genAspectY.integerValue = pref.aspectY
        genSource.isEnabled = hasFFmpeg
        genBitRate.isEnabled = hasFFmpeg
        genAspectX.isEnabled = hasFFmpeg
        genAspectY.isEnabled = hasFFmpeg
        
        // Fullscreen
        genAspectRatioButton.state = pref.keepAspectRatio ? .on : .off
        genExitOnEscButton.state = pref.exitOnEsc ? .on : .off
                
        // Warp mode
        genWarpMode.selectItem(withTag: pref.warpModeIntValue)

        // Miscellaneous
        genEjectWithoutAskingButton.state = pref.ejectWithoutAsking ? .on : .off
        genPauseInBackground.state = pref.pauseInBackground ? .on : .off
        genCloseWithoutAskingButton.state = pref.closeWithoutAsking ? .on : .off
    }

    func selectGeneralTab() {

        refreshGeneralTab()
    }

    //
    // Action methods (Snapshots)
    //
    
    @IBAction func capAutoSnapshotAction(_ sender: NSButton!) {
        
        pref.autoSnapshots = sender.state == .on
        refresh()
    }
    
    @IBAction func capSnapshotIntervalAction(_ sender: NSTextField!) {
        
        if sender.integerValue > 0 {
            pref.snapshotInterval = sender.integerValue
        }
        refresh()
    }
    
    //
    // Action methods (Screenshots)
    //

    @IBAction func capScreenshotSourceAction(_ sender: NSPopUpButton!) {
        
        pref.screenshotSource = sender.selectedTag()
        refresh()
    }
    
    @IBAction func capScreenshotTargetAction(_ sender: NSPopUpButton!) {
        
        pref.screenshotTargetIntValue = sender.selectedTag()
        refresh()
    }

    //
    // Action methods (Screen captures)
    //
    
    @IBAction func genPathAction(_ sender: NSComboBox!) {

        pref.ffmpegPath = sender.stringValue
        refresh()
        
        // Display a warning if the recorder is inaccessible
        let fm = FileManager.default
        if fm.fileExists(atPath: sender.stringValue),
           !fm.isExecutableFile(atPath: sender.stringValue) {

            VAError.recorderSanboxed(name: sender.stringValue)
        }
    }
        
    @IBAction func capSourceAction(_ sender: NSPopUpButton!) {
        
        pref.captureSource = sender.selectedTag()
        refresh()
    }

    @IBAction func genBitRateAction(_ sender: NSComboBox!) {
        
        var input = sender.objectValueOfSelectedItem as? Int
        if input == nil { input = sender.integerValue }
        
        if let bitrate = input {
            pref.bitRate = bitrate
        }
        refresh()
    }

    @IBAction func genAspectXAction(_ sender: NSTextField!) {
        
        pref.aspectX = sender.integerValue
        refresh()
    }

    @IBAction func genAspectYAction(_ sender: NSTextField!) {
        
        pref.aspectY = sender.integerValue
        refresh()
    }
    
    //
    // Action methods (Fullscreen)
    //
    
    @IBAction func genAspectRatioAction(_ sender: NSButton!) {
        
        pref.keepAspectRatio = (sender.state == .on)
        refresh()
    }

    @IBAction func genExitOnEscAction(_ sender: NSButton!) {
        
        pref.exitOnEsc = (sender.state == .on)
        refresh()
    }

    //
    // Action methods (Warp mode)
    //

    @IBAction func genWarpModeAction(_ sender: NSPopUpButton!) {
        
        pref.warpMode = WarpMode(rawValue: sender.selectedTag())!
        parent.refreshStatusBar()
        refresh()
    }
    
    //
    // Action methods (Miscellaneous)
    //
    
    @IBAction func genEjectWithoutAskingAction(_ sender: NSButton!) {
        
        pref.ejectWithoutAsking = (sender.state == .on)
        refresh()
    }
    
    @IBAction func genPauseInBackgroundAction(_ sender: NSButton!) {
        
        pref.pauseInBackground = (sender.state == .on)
        refresh()
    }
    
    @IBAction func genCloseWithoutAskingAction(_ sender: NSButton!) {
        
        pref.closeWithoutAsking = (sender.state == .on)
        for c in myAppDelegate.controllers {
            c.needsSaving = c.amiga.running
        }
        refresh()
    }
    
    //
    // Action methods (Misc)
    //
    
    @IBAction func generalPresetAction(_ sender: NSPopUpButton!) {
        
        assert(sender.selectedTag() == 0)

        UserDefaults.resetGeneralUserDefaults()
        pref.loadGeneralUserDefaults()
        refresh()
    }
}
