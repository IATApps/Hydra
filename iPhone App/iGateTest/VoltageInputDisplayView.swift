//
//  VoltageInputDisplayView.swift
//  hydra
//
//  Created by Kurt Arnlund on 11/21/16.
//
//

import UIKit

extension NSAttributedString {
    func fullRange() -> NSRange {
        return NSMakeRange(0, self.length)
    }
}

class VoltageInputDisplayView : UIView {
    
    @IBOutlet weak var cutoffLabel: UILabel!
    @IBOutlet weak var voltageLabel: UILabel!
    
    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        super.touchesBegan(touches, with: event)
        
        self.update(voltage: 12.0, cutoff: 9)
    }
    
    func update(voltage: Float, cutoff: Float) {
        voltageLabel.text = VoltageFormatter().string(from: NSNumber(value: voltage))
        
        let paragraphStyle = NSMutableParagraphStyle()
        paragraphStyle.alignment = .center

        
        let cutoffTitle = "Cutoff"
        let cutoffTitleAttribs = [NSFontAttributeName : UIFont(name: "Helvetica", size: 14)!]
        
        let cutoffVoltageTitle = "\n".appending(VoltageFormatter().string(from: NSNumber(value: cutoff))!)
        let cutoffVoltageAttribs = [NSFontAttributeName : UIFont(name: "Helvetica", size: 20)!]
        
        let cutoffTxt = NSMutableAttributedString(string: cutoffTitle, attributes: cutoffTitleAttribs)
        cutoffTxt.append(NSMutableAttributedString(string: cutoffVoltageTitle, attributes: cutoffVoltageAttribs))

        cutoffTxt.addAttributes([NSForegroundColorAttributeName : UIColor.white, NSParagraphStyleAttributeName : paragraphStyle], range: cutoffTxt.fullRange() )
        
        cutoffLabel.attributedText = cutoffTxt
    }

}


