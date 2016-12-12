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

@objc public class IATBTServiceGroup : NSObject {
    var services : [IATBTService]?
    var serviceAcquisitionTimer : IATFoundationUtilities.IATTimer?
    private weak var discovery: IATBTDiscovery?
    private var notifyOnlyOncePerConnection = IATOneTimeTrigger(name: "service group success or failure")
//        IATFoundationUtilities.IATOneTimeTrigger(name: "service group success or fail")
    
    private override init() {
        super.init()
    }
    
    @objc public convenience init(discovery: IATBTDiscovery) {
        self.init(discovery: discovery, services: [IATBTDeviceInformationService()])
    }
    
    @objc public init(discovery: IATBTDiscovery, services: [IATBTService]? = [IATBTDeviceInformationService()]) {
        self.discovery = discovery
        self.services = services
        super.init()
    }
    
    public func reset() {
        notifyOnlyOncePerConnection.reset(once: nil)
        guard let services = services else {
            return
        }
        for service in services {
            service.reset()
        }
    }
    
    public func serviceUUIDs() -> [CBUUID]? {
        return self.services.flatMap{ array in
            return array.map({ (service) -> CBUUID in
                return service.uuid
            })
        }
    }

    public func requiredServiceUUIDs() -> [CBUUID]? {
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
    
    public func service(matchingClass: AnyClass) -> AnyObject? {
        return services?.first(where: { (element) -> Bool in
            return String(describing: type(of: element)) == String(describing: matchingClass)
        })
    }
    
    public func allServicesAndCharacteristicsAcquired() -> Bool {
        return self.services?.first(where: { (element) -> Bool in
            return !element.alreadyAcquiredCharacteristics()
        }) == nil
    }
    
    public func allRequiredServicesAndCharacteristicsAcquired() -> Bool {
        return self.services?.first(where: { (element) -> Bool in
            if element.isRequired() == false {
                return false
            }
            
            return !element.alreadyAcquiredCharacteristics()
        }) == nil
    }
    
    public func startAquiringServices(forPeripheral peripheral: CBPeripheral) {
        if self.startServiceAquisitionTimer() == true {
            let serviceUUIDs = self.serviceUUIDs()
            peripheral.discoverServices(serviceUUIDs)
        }
    }
    
    public func startServiceAquisitionTimer() -> Bool {
        if serviceAcquisitionTimer != nil {
            serviceAcquisitionTimer?.stop()
        }
        serviceAcquisitionTimer = IATFoundationUtilities.IATTimer(withDuration: 8, whenExpired: {
            self.notifyOnce(event: .device_connection_failed_to_find_all_required_services_and_characteristics)
            //WARNING:
            NotificationCenter.default.post(name: kBTRequestDisconnect, object: nil)
        })
        return true
    }
    
    public func stopServiceAquisitionTimer() {
        serviceAcquisitionTimer?.stop()
        serviceAcquisitionTimer = nil
    }
    
    public func checkForServiceAcquisition() -> Bool {
        if self.allServicesAndCharacteristicsAcquired() == true {
            self.stopServiceAquisitionTimer()
            self.notifyOnce(event: .device_connection_found_all_required_services_and_characteristics)
            return true
        }
        return false
    }
    
    public func notifyOnce(event: BTDiscoveryEvent) {
        notifyOnlyOncePerConnection.doThis {
            self.notify(event: event)
        }
    }
    
    public func notify(event: BTDiscoveryEvent) {
        event.notify { (event) in
            self.discovery?.informDelegateOfEvent(event: event)
        }
    }
    
    public func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        guard let ourDefinedServices = self.services else {
            return
        }
        guard let discoveredServices = peripheral.services else {
            return
        }

        var uuidToServiceMap = [String:CBService]()
            
        discoveredServices.forEach({ (service) in
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
        print("discovered service ", discoveredService.uuid.uuidString)
        if let matchingService = ourDefinedServices.first(where: { (service) -> Bool in
            return service.isMatch(forService: discoveredService)
        }) {
            print("matched with our service ", matchingService.uuid.uuidString)
            matchingService.peripheral(peripheral, didDiscoverCharacteristicsFor: discoveredService, error: error)
        }
        let allDone = self.checkForServiceAcquisition()
        if allDone == true {
            self.notify(event: .device_connected)
        }
    }
}
