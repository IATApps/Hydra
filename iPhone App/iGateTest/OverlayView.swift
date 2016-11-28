//
//  OverlayView.swift
//  hydra
//
//  Created by Kurt Arnlund on 11/27/16.
//  Copyright © 2016 Ingenious Arts and Technologies LLC. All rights reserved.
//

import Foundation

class OverlayView : UIView {
    @IBOutlet weak var loadedOverlayView : OverlayView?
    @IBOutlet weak var spinner : UIActivityIndicatorView?
    @IBOutlet weak var title : UILabel?
    @IBOutlet weak var hydraImage : UIImageView?
    
    static func loadFromXib() -> OverlayView {
        let fullScreen = UIScreen.main.bounds
        let container = OverlayView.init(frame: fullScreen)
        Bundle.main.loadNibNamed("OverlayView", owner: container, options: nil)
        let overlayView = container.loadedOverlayView!
        overlayView.frame = fullScreen
        container.addSubview(overlayView)
        overlayView.spinner?.startAnimating()
        UIApplication.shared.keyWindow?.addSubview(overlayView)
        UIApplication.shared.keyWindow?.bringSubview(toFront: overlayView)
        return overlayView
    }

    func searching() {
//        UIView.animateKeyframesWithDuration(2, delay: 0, options: [.beginFromCurrentState, .repeat, .autoreverse], animations: {
//            UIView.addKeyframeWithRelativeStartTime(0.0, relativeDuration: 0.5) {
//                snapshot.frame = smallFrame
//            }
//            UIView.addKeyframeWithRelativeStartTime(0.5, relativeDuration: 0.5) {
//                snapshot.frame = finalFrame
//            }
//        }, completion: nil)
        
        self.hydraImage?.alpha = 1.0
        UIView.animate(withDuration: 1.0, delay: 0, options: [.beginFromCurrentState, .repeat, .autoreverse], animations: {
            self.hydraImage?.alpha = 0.2
        }, completion: { Bool in
        })
    }

    func connecting() {
        self.hydraImage?.alpha = 1.0
        UIView.animate(withDuration: 0.5, delay: 0, options: [.beginFromCurrentState, .repeat, .autoreverse], animations: {
            self.hydraImage?.alpha = 0.5
        }, completion: { Bool in
        })
    }

    func hide() {
        self.hydraImage?.layer.removeAllAnimations()
        self.spinner?.stopAnimating()
        self.hydraImage?.alpha = 1.0
        UIView.animate(withDuration: 0.25, delay: 1, options: .beginFromCurrentState, animations: {
            self.alpha = 0.0
            self.hydraImage?.alpha = 0.0
            self.title?.alpha = 0.0
            self.spinner?.alpha = 0.0
        }, completion: { Bool in
            self.removeFromSuperview()
        })
    }
}
