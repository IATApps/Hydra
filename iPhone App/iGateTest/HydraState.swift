//
//  HydraState.swift
//  hydra
//
//  Created by Kurt Arnlund on 11/23/16.
//  Ingenious Arts and Technologies LLC
//

import Foundation

class HydraOutputChannel : NSObject {
    var voltage : Float = 0
    var current : Float = 0
}

class HydraState : NSObject {
    var haveConfigData : Bool = false
    var hydra : HydraPacket?
    var channelTarget : Array<HydraOutputChannel> = Array()
    class func uiNotifName() -> Notification.Name { return Notification.Name("StateUpdated") }
    class func rxNotifName() -> Notification.Name { return Notification.Name("ReceviedData") }
    class func userInfoKey() -> String { return "hydraPacket" }
    
    private override init() {
        super.init()
        
        channelTarget.append(HydraOutputChannel())
        channelTarget.append(HydraOutputChannel())
        channelTarget.append(HydraOutputChannel())
        
        NotificationCenter.default.addObserver(self, selector: #selector(HydraState.onReceivedData), name: HydraState.rxNotifName(), object: nil)
    }
    
    deinit {
        NotificationCenter.default.removeObserver(self, name: HydraState.rxNotifName(), object: nil)
    }
    
    static let sharedInstance : HydraState = HydraState()
    
    @objc func onReceivedData(withNotification notification : NSNotification) {
        hydra = notification.userInfo?[HydraState.userInfoKey()] as? HydraPacket
        haveConfigData = true
        notifyUserInterface()
    }
    
    func notifyUserInterface() {
        NotificationCenter.default.post(name: HydraState.uiNotifName(), object: nil, userInfo: nil)
    }
    
    func inputVoltageAndCurrent() -> (voltage: Float, current: Float)  {
        let input = hydra?.batchEntry(at: 3)
        return (int32ToFloat(value: input!.voltage), int32ToFloat(value: input!.current))
    }

    func getChannel(number channel: Int) -> HydraChannel? {
        return hydra?.batchEntry(at: UInt8(channel))
    }
    
    func voltageAndCurrent(forChannel: Int) -> (voltage: Float, targetVoltage: Float,  current: Float, targetCurrent: Float)?  {
        if (forChannel > 2) {
            return nil
        }
        let target = channelTarget[forChannel]
        let output = getChannel(number: forChannel)
        return (int32ToFloat(value: output!.voltage), target.voltage, int32ToFloat(value: output!.current), target.current)
    }

    func int32ToFloat(value: Int32) -> Float {
        let MILLI_SCALER:Float = 1000
        return Float(value) / MILLI_SCALER
    }
}
