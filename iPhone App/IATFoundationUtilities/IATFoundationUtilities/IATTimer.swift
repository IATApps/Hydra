//
//  IATTimer.swift
//  IATBluetooth
//
//  Created by Kurt Arnlund on 11/29/16.
//  Copyright © 2016 Ingenious Arts and Technologies LLC. All rights reserved.
//

import Foundation

@objc open class IATTimer : NSObject {
    fileprivate let timer : Timer
    
    public init(withDuration duration: TimeInterval, whenExpired: @escaping () -> Void ) {
        timer = Timer.scheduledTimer(withTimeInterval: duration, repeats: false, block: { (timer) in
            whenExpired()
        })
    }

    public init(withInterval interval: TimeInterval, repeatingAction: @escaping () -> Void ) {
        timer = Timer.scheduledTimer(withTimeInterval: interval, repeats: true, block: { (timer) in
            repeatingAction()
        })
    }

    deinit {
        stop()
    }
    
    open func stop() {
        if timer.isValid == true {
            timer.invalidate()
        }
    }
    
}
