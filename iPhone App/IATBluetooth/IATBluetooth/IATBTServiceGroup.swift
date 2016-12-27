//
//  IATBTServiceGroup.swift
//  IATBluetooth
//
//  Created by Kurt Arnlund on 11/28/16.
//  Copyright © 2016 Ingenious Arts and Technologies LLC. All rights reserved.
//
//  This object managed the acquisition of a array of services.
//  
//  1. It asks the services to acquire their characteristic
//  2. If the required characteristics for all services are not found in timely manner, then
//     a service acquisition timeout even occurs (device_connection_failed)

import Foundation
import CoreBluetooth
import IATFoundationUtilities

@objc open class IATBTServiceGroup : NSObject {
    var services : [IATBTService]?
    var acquired = [IATBTService : Bool]()
    var serviceAcquisitionTimer : IATFoundationUtilities.IATTimer?
    fileprivate weak var discovery: IATBTDiscovery?
    fileprivate var notifyOnlyOncePerConnection = IATOneTimeTrigger(name: "service group success or failure")
//        IATFoundationUtilities.IATOneTimeTrigger(name: "service group success or fail")
    
    fileprivate override init() {
        super.init()
    }
    
    @objc public convenience init(discovery: IATBTDiscovery) {
        self.init(discovery: discovery, services: [IATBTDeviceInformationService()])
    }
    
    @objc public init(discovery: IATBTDiscovery, services: [IATBTService]? = [IATBTDeviceInformationService()]) {
        self.discovery = discovery
        self.services = services
        super.init()
        self.services?.forEach({ (service) in
            service.serviceGroup = self
        })
    }
    
    open func resetForDisconnect() {
        self.resetForReconnection()
        self.serviceAcquisitionTimer?.stop()
    }
    
    open func resetForReconnection() {
        guard services != nil else {
            return
        }
        notifyOnlyOncePerConnection.reset()
        self.serviceAcquisitionTimer?.stop()
        self.services?.forEach({ (service) in
            service.resetForReconnection()
            acquired[service] = false
        })
    }
    
    open func serviceUUIDs() -> [CBUUID]? {
        return self.services.flatMap{ array in
            return array.map({ (service) -> CBUUID in
                return service.uuid
            })
        }
    }

    open func requiredServiceUUIDs() -> [CBUUID]? {
        return self.services.flatMap{ array in
            if let service = array.first {
                if service.isRequired() == true {
                    return [service.uuid]
                } else {
                    return nil
                }
            } else {
                return nil
            }
        }
    }
    
    open func service(_ matchingClass: AnyClass) -> AnyObject? {
        return services?.first(where: { (element) -> Bool in
            return String(describing: type(of: element)) == String(describing: matchingClass)
        })
    }

    open func serviceWithUUID(_ UUID: CBUUID) -> CBService? {
        if let iatService = services?.first(where: { (service) -> Bool in
            return service.uuid.uuidString == UUID.uuidString
        }) {
            return iatService.service
        }
        return nil
    }
    
    open func validate() -> Bool {
        var result = true
        services?.forEach({ (service) in
            if service.uuid.uuidString != service.service?.uuid.uuidString {
                result = false
            }
            
            if service.validate() == false {
                result = false
            }
        })
        return result
    }
    
    open func allServicesAndCharacteristicsAcquired() -> Bool {
        guard self.acquired.count == self.services?.count else {
            return false
        }
        
        return self.services?.first(where: { (service) -> Bool in
            return !service.alreadyAcquiredCharacteristics()
        }) == nil
    }
    
    open func allRequiredServicesAndCharacteristicsAcquired() -> Bool {
        return self.services?.first(where: { (element) -> Bool in
            if element.isRequired() == false {
                return false
            }
            
            return !element.alreadyAcquiredCharacteristics()
        }) == nil
    }
    
    open func startAquiringServices(forPeripheral peripheral: CBPeripheral) {
        if self.startServiceAquisitionTimer() == true {
            let serviceUUIDs = self.serviceUUIDs()
            peripheral.discoverServices(serviceUUIDs)
        }
    }
    
    internal func serviceWasAcquired(_ service: IATBTService) {
        self.acquired[service] = true
        self.checkForServiceAcquisition()
    }
    
    open func startServiceAquisitionTimer() -> Bool {
        if serviceAcquisitionTimer != nil {
            serviceAcquisitionTimer?.stop()
        }
        serviceAcquisitionTimer = IATFoundationUtilities.IATTimer(withDuration: 8, whenExpired: {
            #if !DEBUG
            self.notifyOnce(event: .device_connection_failed_to_find_all_required_services_and_characteristics)
            //WARNING:
            NotificationCenter.default.post(name: kBTRequestDisconnect, object: nil)
            #endif
        })
        return true
    }
    
    public func stopServiceAquisitionTimer() {
        serviceAcquisitionTimer?.stop()
        serviceAcquisitionTimer = nil
    }
    
    public func checkForServiceAcquisition() {
        if self.allServicesAndCharacteristicsAcquired() == true {
            self.stopServiceAquisitionTimer()
            self.notifyOnce(event: .device_connection_found_all_required_services_and_characteristics, doOnce: {
                DispatchQueue.main.async {
                    self.notify(event: .device_connected)
                    if self.validate() == false {
                        print("Peripheral Core Bluetooth validation FAILED")
                    } else {
                        print("Peripheral Core Bluetooth validation PASSED")
                    }
                }
            })
        }
    }
    
    public func notifyOnce(event: BTDiscoveryEvent, doOnce: (()->Void)? = nil) {
        notifyOnlyOncePerConnection.doThis {
            self.notify(event: event)
            doOnce?()
        }
    }
    
    public func notify(event: BTDiscoveryEvent) {
        event.notify { (event) in
            self.discovery?.informDelegateOfEvent(event: event)
        }
    }
    
    //TODO: pass in matching properties and do not assume its a read property we are looking for
    public func characteristic(named name: String, matchingProperties: CBCharacteristicProperties) -> CBCharacteristic? {
        if let services = self.services {
            for service in services {
                if let characteristic = service.characteristic(named: name, matchingProperties: matchingProperties) {
                    return characteristic
                }
            }
        }
        return nil
    }
}

// CBPeripheral friend methods called by IATBTServiceManager
extension IATBTServiceGroup {
    public func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        guard let ourDefinedServices = self.services else {
            return
        }
        guard let peripheralServices = peripheral.services else {
            return
        }
        
        var uuidToServiceMap = [String:CBService]()
        
        peripheralServices.forEach({ (service) in
            print("discovered service ", service.uuid.uuidString)
            uuidToServiceMap[service.uuid.uuidString] = service
        })
        
        for service in ourDefinedServices {
            service.peripheral(peripheral, uuidToServiceMap: uuidToServiceMap, didDiscoverServicesWithError: error)
        }
    }
    
    public func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor discoveredService: CBService, error: Error?) {
        guard let ourDefinedServices = self.services else {
            return
        }
        //        print("discovered characteristics for service ", discoveredService.uuid.uuidString)
        if let matchingService = ourDefinedServices.first(where: { (service) -> Bool in
            return service.isMatch(forService: discoveredService)
        }) {
            print("matched with our service ", matchingService.uuid.uuidString)
            matchingService.peripheral(peripheral, didDiscoverCharacteristicsFor: discoveredService, error: error)
        }
    }
}
