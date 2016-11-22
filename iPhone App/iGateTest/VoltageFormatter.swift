//
//  VoltageFormatter.swift
//  hydra
//
//  Created by Kurt Arnlund on 11/21/16.
//
//

import Foundation

class VoltageFormatter: NumberFormatter {
    
    lazy var vFormatter = { () -> NumberFormatter in 
        let vFormatter = NumberFormatter()
        vFormatter.numberStyle = .decimal;
        vFormatter.minimumFractionDigits = 3
        vFormatter.maximumFractionDigits = 3
        return vFormatter
    }

    override func string(from number: NSNumber) -> String? {
        return vFormatter().string(from: number)?.appending(" V")
    }    
}
