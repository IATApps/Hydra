//
//  iGatePeripheralDelegate.swift
//  hydra
//
//  Created by Kurt Arnlund on 12/25/16.
//  Copyright © 2016 Ingenious Arts and Technologies LLC. All rights reserved.
//

import Foundation
import CoreBluetooth

class CiGatePeripheralDelegate : NSObject, CBPeripheralDelegate {
    weak var comDelegate : CiGateComDelegate?
    weak var ciGate : CiGate?
    
    init(comDelegate: CiGateComDelegate) {
        self.comDelegate = comDelegate
        super.init()
    }
    
    public func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        guard self.ciGate != nil else {
            print("ERROR: iGate parent object not assigned.")
            return
        }
        guard error == nil else {
            print(error?.localizedDescription ?? "ERROR: peripheral:didUpdateValueFor(characteristic):error:")
            return
        }
        guard characteristic.value != nil else {
            print("ERROR: no value in characteristic.")
            return
        }
        self.comDelegate?.didReceiveData(iGate: self.ciGate!, data: characteristic.value!)
    }
}
