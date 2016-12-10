//
//  PaddedLabel.swift
//  hydra
//
//  Created by Kurt Arnlund on 11/21/16.
//  Ingenious Arts and Technologies LLC
//

import UIKit

@IBDesignable class PaddedLabel : UILabel {
    
    @IBInspectable var topInset = CGFloat(0.0) {
        didSet {
            self.invalidateIntrinsicContentSize()
        }
    }
    @IBInspectable var bottomInset = CGFloat(0.0)  {
        didSet {
            self.invalidateIntrinsicContentSize()
        }
    }
    @IBInspectable var leftInset = CGFloat(0.0)  {
        didSet {
            self.invalidateIntrinsicContentSize()
        }
    }
    @IBInspectable var rightInset = CGFloat(0.0)  {
        didSet {
            self.invalidateIntrinsicContentSize()
        }
    }
    
    required init(coder: NSCoder) {
        super.init(coder: coder)!
    }
    
    override func didMoveToSuperview() {
        super.didMoveToSuperview()
        invalidateIntrinsicContentSize()
    }

    override var intrinsicContentSize: CGSize {
        let intrinsicSuperViewContentSize = super.intrinsicContentSize;
        let myHeight = intrinsicSuperViewContentSize.height + topInset + bottomInset;
        let myWidth = intrinsicSuperViewContentSize.width + leftInset + rightInset;
        return CGSize(width: myWidth, height: myHeight);
    }
}
