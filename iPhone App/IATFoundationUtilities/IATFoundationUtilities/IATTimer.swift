//
//  IATTimer.swift
//  IATBluetooth
//
//  Created by Kurt Arnlund on 11/29/16.
//  Copyright © 2016 Ingenious Arts and Technologies LLC. All rights reserved.
//

import Foundation

@objc public class IATTimer : NSObject {
    private let timer : Timer
    
    public init(withDuration duration: TimeInterval, whenExpired: @escaping () -> Void ) {
        timer = Timer.scheduledTimer(withTimeInterval: duration, repeats: false, block: { (timer) in
            whenExpired()
        })
    }
    
    deinit {
        stop()
    }
    
    public func stop() {
        if timer.isValid == true {
            timer.invalidate()
        }
    }
    
}
