//
//  IATBTService.swift
//  IATBluetooth
//
//  Created by Kurt Arnlund on 11/28/16.
//  Copyright © 2016 Ingenious Arts and Technologies LLC. All rights reserved.
//

import Foundation
import CoreBluetooth

@objc protocol IATBTServicePeripheralProtocol {
    func peripheral(_ peripheral: CBPeripheral, uuidToServiceMap:[String:CBService], didDiscoverServicesWithError: Error?)
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?)
}

@objc open class IATBTService : NSObject, IATBTServicePeripheralProtocol {
    let uuid : CBUUID!
    var characteristicUUIDs = Set<String>() // UUID string set
    open weak var service : CBService?
    open weak var serviceGroup: IATBTServiceGroup?

    fileprivate let required : Bool
    fileprivate var nameToCharacteristicUUID: [String:String]!  // Key : UUID string map
    fileprivate var characteristicUUIDtoCBCharacteristicMap: [String:CBCharacteristic]!  // Key UUID String : CBCharacteristic
    
    override convenience init() {
        self.init(serviceUUID: CBUUID(nsuuid: UUID()).uuidString, required:false, characteristicDescriptions: [:])
    }
    
    convenience init(serviceUUID: String!, required: Bool) {
        self.init(serviceUUID: serviceUUID, required: required, characteristicDescriptions: [:])
    }

    public init(serviceUUID: String!, required: Bool, characteristicDescriptions: [String:String]? = [:]) {
        self.uuid = CBUUID(string: serviceUUID)
        self.service = nil
        self.required = required
        let characteristics = (characteristicDescriptions ?? [:]).values
        let allUUIDs = Set(characteristics)
        self.nameToCharacteristicUUID = characteristicDescriptions ?? [:]
        self.characteristicUUIDs = allUUIDs
        self.characteristicUUIDtoCBCharacteristicMap = [:]
        super.init()
    }

    deinit {
        self.service = nil
    }
    
    open func resetForReconnection() {
        // If service is nill'd automatically because it is weak and the peripheral has disconnected
        self.characteristicUUIDtoCBCharacteristicMap = [:]
    }

    open func isRequired() -> Bool {
        return required
    }

    open func isMatch(forService otherService: CBService) -> Bool {
        return otherService.uuid.uuidString == self.uuid?.uuidString
    }
    
    open func isServiceCharacteristic(_ characteristic: CBCharacteristic) -> Bool {
        return self.characteristicUUIDs.contains(characteristic.uuid.uuidString)
    }
    
    open func validate() -> Bool {
        if uuid.uuidString != service?.uuid.uuidString {
            return false
        }
        
        var result = true
        service?.characteristics?.forEach({ (characteristic) in
            if characteristicUUIDs.contains(characteristic.uuid.uuidString) == false {
                result = false
            }
            let characteristicMapUUIDs = nameToCharacteristicUUID.values
            if characteristicMapUUIDs.contains(characteristic.uuid.uuidString) == false {
                result = false
            }
            if characteristicUUIDtoCBCharacteristicMap[characteristic.uuid.uuidString] == nil {
                result = false
            }
        })
        
        return result
    }
    
    open func alreadyAcquiredCharacteristics() -> Bool {
        //WARN: service is deallocated immediately when a peripheral disconnects.
        // We cannot rely on the service guard here because we want to use this 
        // method to decide at time of disconnect whether we are disconnecting 
        // from a fully discovered peripheral or if we should give up and just 
        // disconnect because the peripheral was never fully discovered.
        // fully discovered == (services and characteristics were found)
//        guard self.service != nil else {
//            print("service ", self.uuid.uuidString, " has no cbservice stored")
//            return false
//        }
        
        guard self.characteristicUUIDs.count > 0 else {
            return false
        }
        guard self.characteristicUUIDtoCBCharacteristicMap.count > 0 else {
            return false
        }
        return self.characteristicUUIDtoCBCharacteristicMap.count == self.characteristicUUIDs.count
    }
    
    fileprivate func characteristicUUIDArray() -> Array<CBUUID> {
        return characteristicUUIDs.flatMap{
            CBUUID.init(string: $0)
        }
    }
    
    open func peripheral(_ peripheral: CBPeripheral, uuidToServiceMap:[String:CBService], didDiscoverServicesWithError: Error?) {
        if let thisService = uuidToServiceMap[self.uuid.uuidString] {
            self.service = thisService
            if self.alreadyAcquiredCharacteristics() == false {
                peripheral.discoverCharacteristics(self.characteristicUUIDArray(), for:thisService)
            } else {
                self.serviceGroup?.serviceWasAcquired(self)
            }
        }
    }
    
    open func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        //print(peripheral, service.characteristics ?? "[none]")
        let alreadyAcquired = self.alreadyAcquiredCharacteristics()
        print("peripheral discovered characteristics for service ", service.uuid.uuidString, alreadyAcquired ? " [ALREADY ACQUIRED]" : "")
        if self.isMatch(forService: service) && alreadyAcquired == false {
            if let characteristics = service.characteristics {
                characteristics.forEach({ (characteristic) in
                    let uuidStr = characteristic.uuid.uuidString
                    if characteristicUUIDs.contains(characteristic.uuid.uuidString) == false {
                        print("Characterisic " + uuidStr + " was discovered, but was not setup to be named/recognized")
                        characteristicUUIDs.insert(uuidStr)
                    }
                    self.characteristicUUIDtoCBCharacteristicMap[uuidStr] = characteristic
                })
            }
            self.serviceGroup?.serviceWasAcquired(self)
        }
    }
    
    open func dumpCharacteristics() {
        self.service?.characteristics?.forEach({ (characteristic) in
            print(characteristic.debugDescription)
        })
    }
    
    open func characteristic(named name: String) -> CBCharacteristic? {
        if let uuidString = self.nameToCharacteristicUUID[name] {
            return self.characteristicUUIDtoCBCharacteristicMap[uuidString]
        }
        return nil
    }
    
}
