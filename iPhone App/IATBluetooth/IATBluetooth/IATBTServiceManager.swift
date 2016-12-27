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

@objc open class IATBTServiceManager : NSObject {
    fileprivate weak var peripheral : CBPeripheral?
    fileprivate weak var serviceGroup : IATBTServiceGroup?
    fileprivate weak var peripheralDelegatePassthrough : CBPeripheralDelegate?
    
    fileprivate override init() {
        super.init()
    }
    
    public init(withPeripheral peripheral: CBPeripheral, serviceGroup: IATBTServiceGroup, peripheralDelegate passthroughDelegateCallsTo: CBPeripheralDelegate? = nil) {
        self.peripheral = peripheral
        self.serviceGroup = serviceGroup
        super.init()
        self.peripheral?.delegate = self
        self.peripheralDelegatePassthrough = passthroughDelegateCallsTo
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
}


// CBPeripheralDelegate
extension IATBTServiceManager : CBPeripheralDelegate {
    
    public func peripheralDidUpdateName(_ peripheral: CBPeripheral) {
        guard peripheral == self.peripheral else {
            return
        }
        peripheralDelegatePassthrough?.peripheralDidUpdateName?(peripheral)
    }

    public func peripheral(_ peripheral: CBPeripheral, didModifyServices invalidatedServices: [CBService]) {
        guard peripheral == self.peripheral else {
            return
        }
        peripheralDelegatePassthrough?.peripheral?(peripheral, didModifyServices: invalidatedServices)
    }
    
    public func peripheral(_ peripheral: CBPeripheral, didReadRSSI RSSI: NSNumber, error: Error?) {
        guard peripheral == self.peripheral else {
            return
        }
        guard error == nil else {
            print(error?.localizedDescription ?? "ERROR: peripheral:didReadRSSI:error")
            return
        }
        peripheralDelegatePassthrough?.peripheral?(peripheral, didReadRSSI: RSSI, error: error)
    }
    
    open func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        guard peripheral == self.peripheral else {
            return
        }
        guard error == nil else {
            print(error?.localizedDescription ?? "ERROR: peripheral:didDiscoverServices:error:")
            return
        }
        guard (peripheral.services?.count)! > 0 else {
            print("WARNING: peripheral:didDiscoverServices: No services.")
            return;
        }
        self.serviceGroup?.peripheral(peripheral, didDiscoverServices: error)
        
        self.peripheralDelegatePassthrough?.peripheral?(peripheral, didDiscoverServices: error)
    }
    
    open func peripheral(_ peripheral: CBPeripheral, didDiscoverIncludedServicesFor service: CBService, error: Error?) {
        guard peripheral == self.peripheral else {
            return
        }
        guard error == nil else {
            print(error?.localizedDescription ?? "ERROR: peripheral:didDiscoverIncludedServicesFor:error:")
            return
        }
        peripheralDelegatePassthrough?.peripheral?(peripheral, didDiscoverIncludedServicesFor: service, error: error)
    }
    
    open func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        guard peripheral == self.peripheral else {
            return
        }
        guard error == nil else {
            print(error?.localizedDescription ?? "ERROR: peripheral:didDiscoverCharacteristicsFor:error:")
            return
        }
        self.serviceGroup?.peripheral(peripheral, didDiscoverCharacteristicsFor: service, error: error)
        self.peripheralDelegatePassthrough?.peripheral?(peripheral, didDiscoverCharacteristicsFor: service, error: error)
    }
    
    open func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        if let data = characteristic.value {
            print("incoming ", data)
        }
        guard error == nil else {
            print(error?.localizedDescription ?? "ERROR: peripheral:didUpdateValueFor(characteristic):error:")
            return
        }
        self.peripheralDelegatePassthrough?.peripheral?(peripheral, didUpdateValueFor: characteristic, error: error)
    }
    
    open func peripheral(_ peripheral: CBPeripheral, didWriteValueFor characteristic: CBCharacteristic, error: Error?) {
        guard peripheral == self.peripheral else {
            return
        }
        guard error == nil else {
            print(error?.localizedDescription ?? "ERROR: peripheral:didWriteValueFor(characteristic):error:")
            return
        }
        self.peripheralDelegatePassthrough?.peripheral?(peripheral, didWriteValueFor: characteristic, error: error)
    }
    
    open func peripheral(_ peripheral: CBPeripheral, didUpdateNotificationStateFor characteristic: CBCharacteristic, error: Error?) {
        guard peripheral == self.peripheral else {
            return
        }
        guard error == nil else {
            print(error?.localizedDescription ?? "ERROR: peripheral:didUpdateNotificationStateFor:error:")
            return
        }
        self.peripheralDelegatePassthrough?.peripheral?(peripheral, didUpdateNotificationStateFor: characteristic, error: error)
    }
    
    public func peripheral(_ peripheral: CBPeripheral, didDiscoverDescriptorsFor characteristic: CBCharacteristic, error: Error?) {
        guard peripheral == self.peripheral else {
            return
        }
        guard error == nil else {
            print(error?.localizedDescription ?? "ERROR: peripheral:didDiscoverDescriptorsFor:error:")
            return
        }
        self.peripheralDelegatePassthrough?.peripheral?(peripheral, didDiscoverDescriptorsFor: characteristic, error: error)
    }
    
    public func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor descriptor: CBDescriptor, error: Error?) {
        guard peripheral == self.peripheral else {
            return
        }
        guard error == nil else {
            print(error?.localizedDescription ?? "ERROR: peripheral:didUpdateValueFor(descriptor):error:")
            return
        }
        self.peripheralDelegatePassthrough?.peripheral?(peripheral, didUpdateValueFor: descriptor, error: error)
    }
    
    public func peripheral(_ peripheral: CBPeripheral, didWriteValueFor descriptor: CBDescriptor, error: Error?) {
        guard peripheral == self.peripheral else {
            return
        }
        guard error == nil else {
            print(error?.localizedDescription ?? "ERROR: peripheral:didWriteValueFor(descriptor):error:")
            return
        }
        self.peripheralDelegatePassthrough?.peripheral?(peripheral, didWriteValueFor: descriptor, error: error)
    }
    
}
