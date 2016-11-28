//
//  StringExtensions.swift
//  hydra
//
//  Created by Kurt Arnlund on 11/23/16.
//  Ingenious Arts and Technologies LLC
//

import Foundation

extension NSAttributedString {
    func fullRange() -> NSRange {
        return NSMakeRange(0, self.length)
    }
}
