//
//  HydraViewController.swift
//  hydra
//
//  Created by Kurt Arnlund on 11/21/16.
//  Ingenious Arts and Technologies LLC
//

import UIKit

class HydraViewController : UIViewController {
    
    @IBOutlet weak var voltageInputDisplay: VoltageInputDisplayView!
    
    @IBAction func disconnect(_ sender: Any) {
        let appdel = UIApplication.shared.delegate as! AppDelegate
        appdel.hydraComm.disconnect()
    }
}
