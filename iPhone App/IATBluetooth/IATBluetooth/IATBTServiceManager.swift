//
//  IATBTServiceManager.swift
//  IATBluetooth
//
//  Created by Kurt Arnlund on 11/28/16.
//  Copyright © 2016 Ingenious Arts and Technologies LLC. All rights reserved.
//
//  The main purpose of this IATBTServiceManager is to act as a peripheral delegate
//  and route events to serviceGroup and other objects that would like to act as a 
//  peripheral delegate
//

import Foundation
import CoreBluetooth

let kIATBT_Service_Changed_Status_Notification = Notification.Name("kIATBT_Service_Changed_Status_Notification")
let kIATBT_Service_Connected_Notification = Notification.Name("kIATBT_Service_Connected_Notification")
let kIATBT_Service_Disconnected_Notification = Notification.Name("kIATBT_Service_Disconnected_Notification")

@objc open class IATBTServiceManager : NSObject, CBPeripheralDelegate {
    fileprivate weak var peripheral : CBPeripheral?
    fileprivate weak var serviceGroup : IATBTServiceGroup?
    fileprivate weak var peripheralDelegatePassthrough : CBPeripheralDelegate?
    
    fileprivate override init() {
        super.init()
    }
    
    public init(withPeripheral peripheral: CBPeripheral, serviceGroup: IATBTServiceGroup) {
        self.peripheral = peripheral
        self.serviceGroup = serviceGroup
        super.init()
        self.peripheral?.delegate = self
    }
    
    deinit {
        self.resetForReconnection()
        serviceGroup = nil
    }

    open func resetForReconnection() {
        guard peripheral != nil else {
            return
        }
        
        peripheral?.delegate = nil
        peripheral = nil
        self.serviceGroup?.resetForReconnection()
    }
    
    open func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        guard peripheral == self.peripheral else {
            return
        }
        guard error == nil else {
            print(error?.localizedDescription ?? "ERROR: peripheral:didDiscoverServices:")
            return
        }
        guard (peripheral.services?.count)! > 0 else {
            print("WARNING: peripheral:didDiscoverServices: No services.")
            return;
        }
        self.serviceGroup?.peripheral(peripheral, didDiscoverServices: error)
        
        self.peripheralDelegatePassthrough?.peripheral!(peripheral, didDiscoverServices: error)
    }
    
    open func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        if let data = characteristic.value {
            print("incoming ", data)
        }
    }
    
    open func peripheral(_ peripheral: CBPeripheral, didReadRSSI RSSI: NSNumber, error: Error?) {
        print("RSSI ", RSSI.intValue)
    }
    
    open func peripheral(_ peripheral: CBPeripheral, didWriteValueFor characteristic: CBCharacteristic, error: Error?) {
        print("written")
    }

    open func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        guard peripheral == self.peripheral else {
            return
        }
        guard error == nil else {
            print(error?.localizedDescription ?? "ERROR: peripheral:didDiscoverCharacteristicsFor:")
            return
        }
        self.serviceGroup?.peripheral(peripheral, didDiscoverCharacteristicsFor: service, error: error)
    }
}
