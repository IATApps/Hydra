//
//  PaddedLabel.swift
//  hydra
//
//  Created by Kurt Arnlund on 11/21/16.
//  Ingenious Arts and Technologies LLC
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
    
    override func didMoveToSuperview() {
        super.didMoveToSuperview()
        invalidateIntrinsicContentSize()
    }

    override var intrinsicContentSize: CGSize {
        let intrinsicSuperViewContentSize = super.intrinsicContentSize;
        let myHeight = intrinsicSuperViewContentSize.height + CGFloat(topInset.doubleValue) + CGFloat(bottomInset.doubleValue);
        let myWidth = intrinsicSuperViewContentSize.width + CGFloat(leftInset.doubleValue) + CGFloat(rightInset.doubleValue);
        return CGSize(width: myWidth, height: myHeight);
    }
}
