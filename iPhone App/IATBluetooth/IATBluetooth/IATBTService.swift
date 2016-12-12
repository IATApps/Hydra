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

@objc public class IATBTService : NSObject, IATBTServicePeripheralProtocol/*, CBPeripheralDelegate*/ {
    let uuid : CBUUID!
    var characteristicUUIDs = Set<String>() // UUID string set
    private let required : Bool
    public weak var service : CBService?
    private var nameToCharacteristicMap: [String:String]!  // Key : UUID string map
    private var characteristicUUIDtoCBCharacteristicMap: [String:CBCharacteristic]!
    
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
        self.characteristicUUIDs = allUUIDs
        self.nameToCharacteristicMap = characteristicDescriptions ?? [:]
        self.characteristicUUIDtoCBCharacteristicMap = [:]
        super.init()
    }
    
    open func reset() {
        self.nameToCharacteristicMap = [:]
        self.characteristicUUIDtoCBCharacteristicMap = [:]
        self.service = nil
    }

    open func isRequired() -> Bool {
        return required
    }

    public func isMatch(forService otherService: CBService) -> Bool {
        return otherService.uuid.uuidString == self.uuid?.uuidString
    }
    
    public func isServiceCharacteristic(characteristic: CBCharacteristic) -> Bool {
        return self.characteristicUUIDs.contains(characteristic.uuid.uuidString)
    }
    
    public func alreadyAcquiredCharacteristics() -> Bool {
        guard self.characteristicUUIDs.count > 0 else {
            print("service ", self.uuid.uuidString, " has no characteristicUUIDs stored")
            return false
        }
        guard self.characteristicUUIDtoCBCharacteristicMap.count > 0 else {
            print("service ", self.uuid.uuidString, " has no characteristicUUIDtoCBCharacteristicMap")
            return false
        }
        print("service ", self.uuid.uuidString, " characteristicUUIDtoCBCharacteristicMap.count=", self.characteristicUUIDtoCBCharacteristicMap.count, " self.characteristicUUIDs.count=", self.characteristicUUIDs.count)
        return self.characteristicUUIDtoCBCharacteristicMap.count == self.characteristicUUIDs.count
    }
    
    private func characteristicUUIDArray() -> Array<CBUUID> {
        return characteristicUUIDs.flatMap{
            CBUUID.init(string: $0)
        }
    }
    
    public func peripheral(_ peripheral: CBPeripheral, uuidToServiceMap:[String:CBService], didDiscoverServicesWithError: Error?) {
        if let thisService = uuidToServiceMap[self.uuid.uuidString] {
            self.service = thisService
            if self.alreadyAcquiredCharacteristics() == false {
                peripheral.discoverCharacteristics(self.characteristicUUIDArray(), for:thisService)
            }
        }
    }
    
    public func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        //print(peripheral, service.characteristics ?? "[none]")
        let alreadyAcquired = self.alreadyAcquiredCharacteristics()
        print("peripheral discovered characteristics for service ", service.uuid.uuidString, alreadyAcquired ? " [ALREADY ACQUIRED]" : "")
        if self.isMatch(forService: service) && alreadyAcquired == false {
            if let characteristics = service.characteristics {
                var characteristicUUIDs = Set<String>()
                for characteristic in characteristics {
                    let uuidStr = characteristic.uuid.uuidString
                    characteristicUUIDs.insert(uuidStr)
                    self.characteristicUUIDtoCBCharacteristicMap[uuidStr] = characteristic
                }
                if self.characteristicUUIDs.count == 0  {
                   self.characteristicUUIDs = characteristicUUIDs
                }
                print("characteristics ", characteristicUUIDs)
            }
        }
    }
    
    public func dumpCharacteristics() {
        self.service?.characteristics?.forEach({ (characteristic) in
            print(characteristic.debugDescription)
        })
    }
    
    public func characteristic(named name: String) -> CBCharacteristic? {
        if let uuidString = self.nameToCharacteristicMap[name] {
            return self.characteristicUUIDtoCBCharacteristicMap[uuidString]
        }
        return nil
    }
    
}
