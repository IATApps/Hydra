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

public protocol IATBTServiceGroupProtocol : NSObjectProtocol {
    func acquired(serviceGroup: IATBTServiceGroup)
    func failedToAcquire(serviceGroup: IATBTServiceGroup)
}

@objc public class IATBTServiceGroup : NSObject, IATBTServiceGroupProtocol {
    weak var delegate : IATBTServiceGroupProtocol?
    var services : [IATBTService]?
    var serviceAcquisitionTimer : IATFoundationUtilities.IATTimer?
    private var notified : Bool = false
    private weak var discovery: IATBTDiscovery?
    private var notifyOnlyOncePerConnection = IATOneTimeTrigger(name: "service group success or failure")
        
//        IATFoundationUtilities.IATOneTimeTrigger(name: "service group success or fail")
    
    private override init() {
        super.init()
    }
    
    public convenience init(discovery: IATBTDiscovery, delegate: IATBTServiceGroupProtocol? = nil) {
        self.init(discovery: discovery, services: [IATBTDeviceInformationService()], delegate: delegate)
    }
    
    public init(discovery: IATBTDiscovery, services: [IATBTService]? = [IATBTDeviceInformationService()], delegate: IATBTServiceGroupProtocol? = nil) {
        self.discovery = discovery
        self.delegate = delegate
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
        notified = false
    }
    
    public func serviceUUIDs() -> [CBUUID]? {
        return self.services.flatMap{ array in
            if let service = array.first {
                return [service.uuid]
            } else {
                return nil
            }
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
            peripheral.discoverServices(self.serviceUUIDs())
        }
    }
    
    public func startServiceAquisitionTimer() -> Bool {
        if serviceAcquisitionTimer != nil {
            serviceAcquisitionTimer?.stop()
        }
        serviceAcquisitionTimer = IATFoundationUtilities.IATTimer(withDuration: 3, whenExpired: {
            if self.checkForServiceAcquisition() == false {
                self.notify(reason: .device_connection_failed_to_find_all_required_services_and_characteristics)
                //WARNING: 
                NotificationCenter.default.post(name: kBTRequestDisconnect, object: nil)
            }
        })
        return true
    }
    
    public func stopServiceAquisitionTimer() {
        serviceAcquisitionTimer?.stop()
        serviceAcquisitionTimer = nil
    }
    
    public func checkForServiceAcquisition() -> Bool {
        if self.allRequiredServicesAndCharacteristicsAcquired() == true {
            self.stopServiceAquisitionTimer()
            self.notify(reason: .device_connection_found_all_required_services_and_characteristics)
            return true
        }
        return false
    }
    
    public func notify(reason event: BTDiscoveryEvent) {
        notifyOnlyOncePerConnection.doThis { 
            event.notify { (event) in
                switch event {
                case .device_connection_failed_to_find_all_required_services_and_characteristics:
                    self.delegate?.failedToAcquire(serviceGroup: self)
                case .device_connection_found_all_required_services_and_characteristics:
                    self.delegate?.acquired(serviceGroup: self)
                default:
                    print("ServiceGroup should notify for event %@", event.notificationName.rawValue)
                }
            }
        }
    }
    
    public func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        guard let services = self.services else {
            return
        }
        if let uuidToServiceMap = peripheral.services.flatMap({ (cb_service) -> [String:CBService]? in
            [(cb_service.first?.uuid.uuidString)! : cb_service.first!]
            }) {
            for service in services {
                service.peripheral(peripheral, uuidToServiceMap: uuidToServiceMap, didDiscoverServicesWithError: error)
            }
        }
    }
    
    public func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor discoveredService: CBService, error: Error?) {
        guard let services = self.services else {
            return
        }
        for service in services {
            if service.isMatch(forService: discoveredService) {
                service.peripheral(peripheral, didDiscoverCharacteristicsFor: discoveredService, error: error)
            }
        }
        _ = self.checkForServiceAcquisition()
    }

    // Optional overrides in subclasses
    open func acquired(serviceGroup: IATBTServiceGroup) {
        print("IATBTServiceGroup %@ needs a delegate assigned to it", self)
    }
    
    open func failedToAcquire(serviceGroup: IATBTServiceGroup) {
        print("IATBTServiceGroup %@ needs a delegate assigned to it", self)
    }
}
