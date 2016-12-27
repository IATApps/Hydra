//
//  IATBTDeviceInformationService.swift
//  IATBluetooth
//
//  Created by Kurt Arnlund on 11/28/16.
//  Copyright © 2016 Ingenious Arts and Technologies LLC. All rights reserved.
//

import Foundation
import CoreBluetooth

let kIATBT_DEVICE_INFORMATION_SERVICE_UUID = "180A"
let kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_SYSTEMID_UUID          = CBUUID(string: "2A23")
let kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_MODEL_NUMBER_UUID      = CBUUID(string: "2A24")
let kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_SERIAL_NUMBER_UUID     = CBUUID(string: "2A25")
let kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_FIRMWARE_VERSION_UUID  = CBUUID(string: "2A26")
let kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_HARDWARE_VERSION_UUID  = CBUUID(string: "2A27")
let kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_SOFTWARE_VERSION_UUID  = CBUUID(string: "2A28")
let kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_MANUFACTURER_NAME_UUID = CBUUID(string: "2A29")
let kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_REGULATORY_CERT_DATA_LIST_UUID = CBUUID(string: "2A2A")
let kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_REGULATORY_PNP_ID_UUID = CBUUID(string: "2A50")

let kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_SYSTEMID          = "System ID"
let kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_MODEL_NUMBER      = "Model Number"
let kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_SERIAL_NUMBER     = "Serial Number"
let kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_FIRMWARE_VERSION  = "Firmware Version"
let kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_HARDWARE_VERSION  = "Hardware Verison"
let kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_SOFTWARE_VERSION  = "Software Verison"
let kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_MANUFACTURER_NAME = "Manufacturer Name"
let kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_REGULATORY_CERT_DATA_LIST = "Regulatory Certification Data List"
let kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_PNP_ID            = "PnP ID"

@objc open class IATBTDeviceInformationService : IATBTService {
    public convenience init() {
        self.init(serviceUUID: kIATBT_DEVICE_INFORMATION_SERVICE_UUID,
                  required: true,
                  characteristicDescriptions: [kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_SYSTEMID : kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_SYSTEMID_UUID.uuidString,kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_SERIAL_NUMBER : kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_SERIAL_NUMBER_UUID.uuidString, kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_FIRMWARE_VERSION : kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_FIRMWARE_VERSION_UUID.uuidString, kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_HARDWARE_VERSION : kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_HARDWARE_VERSION_UUID.uuidString, kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_SOFTWARE_VERSION : kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_SOFTWARE_VERSION_UUID.uuidString])
    }
    
    public convenience init(serviceUUID: String!) {
        self.init(serviceUUID: serviceUUID, required: true, characteristicDescriptions: [:])
    }
    
    public override init(serviceUUID: String!, required: Bool, characteristicDescriptions: [String:String]!) {
        super.init(serviceUUID: serviceUUID, required: required, characteristicDescriptions: characteristicDescriptions)
    }
    
    //MARK - Convenience methods for obtaining specific characteristics
    
    open func systemIdCharacteristic() -> CBCharacteristic? {
        return self.characteristic(named: kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_SYSTEMID, matchingProperties: .read)
    }

    open func modelNumberCharacteristic() -> CBCharacteristic? {
        return self.characteristic(named: kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_MODEL_NUMBER, matchingProperties: .read)
    }

    open func serialNumberCharacteristic() -> CBCharacteristic? {
        return self.characteristic(named: kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_SERIAL_NUMBER, matchingProperties: .read)
    }

    open func hardwareVersionCharacteristic() -> CBCharacteristic? {
        return self.characteristic(named: kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_HARDWARE_VERSION, matchingProperties: .read)
    }

    open func softwareVersionCharacteristic() -> CBCharacteristic? {
        return self.characteristic(named: kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_SOFTWARE_VERSION, matchingProperties: .read)
    }

    open func manufacturerNameCharacteristic() -> CBCharacteristic? {
        return self.characteristic(named: kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_MANUFACTURER_NAME, matchingProperties: .read)
    }

    open func regulatoryCertDataListCharacteristic() -> CBCharacteristic? {
        return self.characteristic(named: kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_REGULATORY_CERT_DATA_LIST, matchingProperties: .read)
    }

    open func pnpIdCharacteristic() -> CBCharacteristic? {
        return self.characteristic(named: kIATBT_DEVICE_INFORMATION_CHARACTERISTIC_NAME_PNP_ID, matchingProperties: .read)
    }
}
