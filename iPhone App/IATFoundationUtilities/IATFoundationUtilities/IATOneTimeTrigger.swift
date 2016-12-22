//
//  IATOneTimeTrigger.swift
//  IATBluetooth
//
//  Created by Kurt Arnlund on 12/7/16.
//  Copyright © 2016 Ingenious Arts and Technologies LLC. All rights reserved.
//

import Foundation

@objc open class IATOneTimeTrigger : NSObject {
    fileprivate let name : String
    fileprivate var triggered = false
    fileprivate var resetTimer : IATTimer?
    
    public init(name: String) {
        self.name = name
        super.init()
    }
    
    open func doThis(_ once: (() -> Void)? ) {
        guard self.triggered == false else {
            return
        }
        self.triggered = true
        once?()
        return
    }
    
    open func reset(_ once: (() -> Void)? = nil ) {
        guard self.triggered == true else {
            return
        }
        self.resetTimer?.stop()
        once?()
        self.triggered = false
        return
    }

    open func doThis(_ once: (() -> Void)?, resetAfter: TimeInterval, onReset: (() -> Void)?) {
        let action = {
            once?()
            self.resetTimer = IATTimer(withDuration: resetAfter, whenExpired: {
                self.reset(onReset)
            })
        }
        
        doThis(action)
    }
}
