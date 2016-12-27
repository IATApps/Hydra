//
//  VoltageInputDisplayView.swift
//  hydra
//
//  Created by Kurt Arnlund on 11/21/16.
//  Ingenious Arts and Technologies LLC
//

import UIKit

@IBDesignable class VoltageOutputDisplayView : UIView {
    
    @IBOutlet weak var title: UILabel?
    @IBOutlet weak var currentLabel: UILabel?
    @IBOutlet weak var currentTargetLabel: UILabel?
    @IBOutlet weak var voltageLabel: UILabel?
    @IBOutlet weak var voltageTargetLabel: UILabel?
    @IBInspectable var channel = Int(1)
    @IBOutlet weak var modeLabel: UILabel?
    @IBInspectable var highlightColor = UIColor.white
    
    var editing : Bool! = false {
        didSet {
            updateModeText()
        }
    }
    
    override func awakeFromNib() {
        super.awakeFromNib()
        let zero = (voltage: Float(0.0), targetVoltage: Float(0.0), current: Float(0.0), targetCurrent: Float(0.0))
        update(outputTuple: zero)
        updateModeText()
        observeNotifications()
        updateTitle()
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
        if let output = HydraState.sharedInstance.voltageAndCurrent(forChannel: Int(channel)) {
            update(outputTuple: output)
        }
        updateModeText()
    }
    
    func update(outputTuple: (voltage: Float, targetVoltage: Float, current: Float, targetCurrent: Float) ) {
        voltageLabel?.text = VoltageFormatter().string(from: NSNumber(value: outputTuple.voltage))
        voltageTargetLabel?.text = VoltageFormatter().string(from: NSNumber(value: outputTuple.targetCurrent) )
        
        let paragraphStyle = NSMutableParagraphStyle()
        paragraphStyle.alignment = .center

        let currentTitle = "Current"
        let currentTitleAttribs = [NSFontAttributeName : UIFont(name: "Helvetica", size: 14)!]
        
        let currentVoltageTitle = "\n".appending(CurrentFormatter().string(from: NSNumber(value: outputTuple.current))!)
        let currentVoltageAttribs = [NSFontAttributeName : UIFont(name: "Helvetica", size: 16)!]
        
        let currentTxt = NSMutableAttributedString(string: currentTitle, attributes: currentTitleAttribs)
        currentTxt.append(NSMutableAttributedString(string: currentVoltageTitle, attributes: currentVoltageAttribs))

        currentTxt.addAttributes([NSForegroundColorAttributeName : UIColor.white, NSParagraphStyleAttributeName : paragraphStyle], range: currentTxt.fullRange() )
        
        currentLabel?.attributedText = currentTxt
        
        currentTargetLabel?.text = CurrentFormatter().string(from: NSNumber(value: outputTuple.targetCurrent) )
    }
    
    func updateTitle() {
        let mainTitle = "V\(channel)"
        let mainTitleAttribs = [NSFontAttributeName : UIFont(name: "Helvetica", size: 50)!]
        let outTitle = "out"
        let outTitleAttribs = [NSFontAttributeName : UIFont(name: "Helvetica", size: 18)!]
        let newTitle = NSMutableAttributedString(string: mainTitle, attributes: mainTitleAttribs)
        newTitle.append(NSMutableAttributedString(string: outTitle, attributes: outTitleAttribs))
        title?.attributedText = newTitle
    }
    
    func updateModeText() {
        if let channelSettings = HydraState.sharedInstance.getChannel(number:Int(channel)) {
            modeLabel?.text = modeText(onState:channelSettings.enabled)
        } else {
            modeLabel?.text = modeText(onState:false)
        }
    }
    
    func modeText(onState: Bool) -> String {
        if editing == true {
            return "EDITING"
        }
        if onState == true {
            return "ON"
        }
        return "OFF"
    }

}


