//
//  PaddedLabel.swift
//  hydra
//
//  Created by Kurt Arnlund on 11/21/16.
//
//

import UIKit

@IBDesignable class PaddedLabel : UILabel {
    
    @IBInspectable var topInset: NSNumber! = 0
    @IBInspectable var bottomInset: NSNumber! = 0
    @IBInspectable var leftInset: NSNumber! = 0
    @IBInspectable var rightInset: NSNumber! = 0
    
    required init(coder: NSCoder) {
        super.init(coder: coder)!
    }

    override var intrinsicContentSize: CGSize {
        let intrinsicSuperViewContentSize = super.intrinsicContentSize;
        let newHeight = intrinsicSuperViewContentSize.height + CGFloat(topInset.doubleValue) + CGFloat(bottomInset.doubleValue);
        let newWidth = intrinsicSuperViewContentSize.width + CGFloat(leftInset.doubleValue) + CGFloat(rightInset.doubleValue);
        return CGSize(width: newWidth, height: newHeight);
    }
}
