//
//  HydraStateOverlay.swift
//  hydra
//
//  Created by Kurt Arnlund on 11/27/16.
//  Copyright © 2016 Ingenious Arts and Technologies LLC. All rights reserved.
//

import Foundation

class HydraStateOverlay : NSObject {
    
    var overlay : HydraStatusOverlayView?
    
    enum iGateOverlayStates : String {
        case searching = "Searching"
        case connecting = "Connecting"
        case connected = "Connected"
        case bt_off = "Bluetooth Off"
        case bt_on = "Bluetooth On"
        case bt_unsupported = "Bluetooth is not supported on this device"
        case bt_unauthorized = "This app is not authorized by iOS to use Bluetooth"
    }
    
    class func iGateEvent() -> Notification.Name? {
        return Notification.Name("iGateEvent")
    }
    class func iGateEventUserInfoStateKey() -> String {
        return "iGateState"
    }
    
    private override init() {
        super.init()
        
        let eventName = HydraStateOverlay.iGateEvent()
        NotificationCenter.default.addObserver(self, selector: #selector(HydraStateOverlay.iGateEvent(notification:)), name: eventName, object: nil)
    }
    
    deinit {
        let eventName = HydraStateOverlay.iGateEvent()
        NotificationCenter.default.removeObserver(self, name: eventName, object: nil)
    }
    
    static let sharedInstance : HydraStateOverlay = HydraStateOverlay()
    
    @objc func iGateEvent(notification: Notification) {
        guard let notifyState = notification.userInfo?[HydraStateOverlay.iGateEventUserInfoStateKey()] else {
            return
        }
        
        if let state = iGateOverlayStates(rawValue: notifyState as! String) {
            if state == .connected {
                overlay?.hide()
            } else {
                loadOverlay()
                overlay?.title?.text = state.rawValue
                
                if state == .bt_off || state == .searching {
                    overlay?.searching()
                }
                if state == .connecting {
                    overlay?.connecting()
                }
            }
        }
    }
    
    func loadOverlay() {
        if overlay == nil {
            overlay = HydraStatusOverlayView.loadFromXib()
        }
    }
}
