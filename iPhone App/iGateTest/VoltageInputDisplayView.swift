//
//  VoltageInputDisplayView.swift
//  hydra
//
//  Created by Kurt Arnlund on 11/21/16.
//  Ingenious Arts and Technologies LLC
//

import UIKit

class VoltageInputDisplayView : UIView {
    
    @IBOutlet weak var title: UILabel!
    @IBOutlet weak var cutoffLabel: UILabel!
    @IBOutlet weak var voltageLabel: UILabel!
    
    override func awakeFromNib() {
        super.awakeFromNib()
        update(voltage: 0.0, cutoff: 0.0)
        observeNotifications()
    }

    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        super.touchesBegan(touches, with: event)
    }
    
    func observeNotifications() {
        NotificationCenter.default.addObserver(self, selector: #selector(VoltageInputDisplayView.hydraStateUpdated), name: HydraState.uiNotifName(), object: nil)
    }
    
    deinit {
        NotificationCenter.default.removeObserver(self, name: HydraState.uiNotifName(), object: nil)
    }
    
    func hydraStateUpdated(withNotification notification: NSNotification) {
        let input = HydraState.sharedInstance.inputVoltageAndCurrent()
        update(voltage: input.voltage, cutoff: input.current)
    }
    
    func update(voltage: Float, cutoff: Float) {
        voltageLabel.text = VoltageFormatter().string(from: NSNumber(value: voltage))
        
        let paragraphStyle = NSMutableParagraphStyle()
        paragraphStyle.alignment = .center

        let cutoffTitle = "Cutoff"
        let cutoffTitleAttribs = [NSFontAttributeName : UIFont(name: "Helvetica", size: 14)!]
        
        let cutoffVoltageTitle = "\n".appending(VoltageFormatter().string(from: NSNumber(value: cutoff))!)
        let cutoffVoltageAttribs = [NSFontAttributeName : UIFont(name: "Helvetica", size: 16)!]
        
        let cutoffTxt = NSMutableAttributedString(string: cutoffTitle, attributes: cutoffTitleAttribs)
        cutoffTxt.append(NSMutableAttributedString(string: cutoffVoltageTitle, attributes: cutoffVoltageAttribs))

        cutoffTxt.addAttributes([NSForegroundColorAttributeName : UIColor.white, NSParagraphStyleAttributeName : paragraphStyle], range: cutoffTxt.fullRange() )
        
        cutoffLabel.attributedText = cutoffTxt
    }
    
}


