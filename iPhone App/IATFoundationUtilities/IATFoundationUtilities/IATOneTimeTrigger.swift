//
//  IATOneTimeTrigger.swift
//  IATBluetooth
//
//  Created by Kurt Arnlund on 12/7/16.
//  Copyright © 2016 Ingenious Arts and Technologies LLC. All rights reserved.
//

import Foundation

@objc public class IATOneTimeTrigger : NSObject {
    private let name : String
    private var triggered = false
    private var resetTimer : IATTimer?
    
    public init(name: String) {
        self.name = name
        super.init()
    }
    
    public func doThis(once: (() -> Void)? ) {
        guard self.triggered == false else {
            return
        }
        self.triggered = true
        once?()
        return
    }
    
    public func reset(once: (() -> Void)? = nil ) {
        guard self.triggered == true else {
            return
        }
        self.resetTimer?.stop()
        once?()
        self.triggered = false
        return
    }

    public func doThis(once: (() -> Void)?, resetAfter: TimeInterval, onReset: (() -> Void)?) {
        let action = {
            once?()
            self.resetTimer = IATTimer(withDuration: resetAfter, whenExpired: {
                self.reset(once: onReset)
            })
        }
        
        doThis(once: action)
    }
}
