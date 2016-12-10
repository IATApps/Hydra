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

@objc public class IATBTServiceManager : NSObject, CBPeripheralDelegate {
    private weak var peripheral : CBPeripheral?
    private weak var serviceGroup : IATBTServiceGroup?
    private weak var peripheralDelegatePassthrough : CBPeripheralDelegate?
    
    private override init() {
        super.init()
    }
    
    public init(withPeripheral peripheral: CBPeripheral, serviceGroup: IATBTServiceGroup) {
        self.peripheral = peripheral
        self.serviceGroup = serviceGroup
        super.init()
        self.peripheral?.delegate = self
    }
    
    deinit {
        serviceGroup = nil
        self.reset()
    }
    
    public func reset() {
        guard peripheral != nil else {
            return
        }
        
        peripheral?.delegate = nil
        peripheral = nil
        self.serviceGroup?.reset()
    }
    
    static func sendBTServiceNotificationWithIsBluetoothConnected(isConnected: Bool) {
        let connectionDetails = ["isConnected": isConnected];
        NotificationCenter.default.post(name: kIATBT_Service_Changed_Status_Notification, object: nil, userInfo: connectionDetails)
        if isConnected == true {
            NotificationCenter.default.post(name: kIATBT_Service_Connected_Notification, object: nil, userInfo: nil)
        } else {
            NotificationCenter.default.post(name: kIATBT_Service_Disconnected_Notification, object: nil, userInfo: nil)
        }
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
