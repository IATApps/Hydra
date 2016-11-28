//
//  VoltageFormatter.swift
//  hydra
//
//  Created by Kurt Arnlund on 11/21/16.
//  Ingenious Arts and Technologies LLC
//

import Foundation

class CurrentFormatter: NumberFormatter {
    
    lazy var aFormatter = { () -> NumberFormatter in
        let aFormatter = NumberFormatter()
        aFormatter.numberStyle = .decimal;
        aFormatter.minimumFractionDigits = 3
        aFormatter.maximumFractionDigits = 3
        return aFormatter
    }
    
    override func string(from number: NSNumber) -> String? {
        return aFormatter().string(from: number)?.appending(" mA")
    }
}
