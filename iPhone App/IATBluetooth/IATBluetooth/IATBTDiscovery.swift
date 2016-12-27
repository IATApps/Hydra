//
//  IATBTDiscovery.swift
//  IATBluetooth
//
//  Created by Kurt Arnlund on 11/28/16.
//  Copyright © 2016 Ingenious Arts and Technologies LLC. All rights reserved.
//

import Foundation
import CoreBluetooth
import IATFoundationUtilities

public let kBTDiscoveryDisconnectPeripheralUUIDKey = "cb_peripheral_uuid"

public let kBTRequestDisconnect = Notification.Name("kBTRequestDisconnect")

@objc public protocol BTDiscoveryEventDelegate : NSObjectProtocol {
    func bluetoothEvent(_ event: BTDiscoveryEvent)
}

@objc public enum BTDiscoveryEvent : Int {
    case scanning_started
    case scanning_stopped
    case scanning_timed_out
    case scanning_found_devices
    case scanning_found_pairable_devices
    case scanning_found_no_devices
    case scanning_rejected_device_due_to_signal_strength
    case scanning_rejected_device_due_to_advertisement_data
    case central_manager_state_restoring
    case central_manager_state_unknown
    case central_manager_state_unsupported
    case central_manager_state_unauthorized
    case central_manager_state_resetting
    case central_manager_state_powered_on
    case central_manager_state_powered_off
    case device_disconnected
    case attempting_to_reconnect
    case device_reconnected         // an unexpected disconnect will result in an automatic attempt to reconnect
    case device_connection_started
    case device_connection_found_all_required_services_and_characteristics
    case device_connection_failed_to_find_all_required_services_and_characteristics
    case device_connected
}

public extension BTDiscoveryEvent {
    var notificationName : Notification.Name {
        switch self {
        case .scanning_started: return NSNotification.Name("scanning_started")
        case .scanning_stopped: return NSNotification.Name("scanning_stopped")
        case .scanning_timed_out: return NSNotification.Name("scanning_timed_out")
        case .scanning_found_devices: return NSNotification.Name("scanning_found_devices")
        case .scanning_found_pairable_devices: return NSNotification.Name("scanning_found_pairable_devices")
        case .scanning_found_no_devices: return NSNotification.Name("scanning_found_no_devices")
        case .scanning_rejected_device_due_to_signal_strength: return NSNotification.Name("scanning_rejected_device_due_to_signal_strength")
        case .scanning_rejected_device_due_to_advertisement_data: return NSNotification.Name("scanning_rejected_device_due_to_advertisement_data")
        case .central_manager_state_restoring: return NSNotification.Name("central_manager_state_restoring")
        case .central_manager_state_unknown: return NSNotification.Name("central_manager_state_unknown")
        case .central_manager_state_unsupported: return NSNotification.Name("central_manager_state_unsupported")
        case .central_manager_state_unauthorized: return NSNotification.Name("central_manager_state_unauthorized")
        case .central_manager_state_resetting: return NSNotification.Name("central_manager_state_resetting")
        case .central_manager_state_powered_on: return NSNotification.Name("central_manager_state_powered_on")
        case .central_manager_state_powered_off: return NSNotification.Name("central_manager_state_powered_off")
        case .device_disconnected: return NSNotification.Name("device_disconnected")
        case .attempting_to_reconnect: return NSNotification.Name("attempting_to_reconnect")
        case .device_reconnected: return NSNotification.Name("device_reconnected")
        case .device_connection_started: return NSNotification.Name("device_connection_started")
        case .device_connection_found_all_required_services_and_characteristics: return Notification.Name("device_connection_found_all_required_services_and_characteristics")
        case .device_connection_failed_to_find_all_required_services_and_characteristics: return NSNotification.Name("device_connection_failed_to_find_all_required_services_and_characteristics")
        case .device_connected: return NSNotification.Name("device_connected")
        }
    }
    
    func notify(_ additionalActions: ((_: BTDiscoveryEvent) -> Void)? ) {
        NotificationCenter.default.post(name: self.notificationName, object: self)
        additionalActions?(self)
    }
    
    func notifyFromMainQueue(additionalActions: ((_: BTDiscoveryEvent) -> Void)?) {
        DispatchQueue.main.async {
            self.notify(additionalActions)
        }
    }
}


//TODO: Create an object that wraps up peripherals with the service group, the advertisementData, and the connect and disconnect dates so that this discovery object can manager multiple peripherals

@objc public class IATBTDiscovery : NSObject {
    let eventQueue = DispatchQueue.init(label: "bluetooth event queue")
    public var peripherals : [IATBTPeripheral] = []
    public var isScanning : Bool {
        get {
            var result = false
            for peripheral in peripherals {
                result = result || peripheral.isScanning
            }
            return result
        }
        set {
            return
        }
    }
    public var isBtOn : Bool = false
    var scanTimeoutTimer : IATTimer?
    var lastSeenTimer : IATTimer?
    public let scanTimeout : TimeInterval
    public var delegate :BTDiscoveryEventDelegate? = nil
    
    static public let sharedInstance : IATBTDiscovery = IATBTDiscovery()
    
    fileprivate override init() {
        self.scanTimeout = 0
        super.init()
    }
    
    public init(name: String, peripheralDelegate: CBPeripheralDelegate? = nil, withServices services: [IATBTService]? = nil, scanTimeout: TimeInterval = 0) {
        self.scanTimeout = scanTimeout
        super.init()
        let group = IATBTServiceGroup(discovery: self, services: services)
        let peripheral = IATBTPeripheral(name: name, discovery: self, serviceGroup: group, peripheralDelegate: peripheralDelegate)
        self.peripherals.append(peripheral)
        startObservingNotifications()
    }
    
    deinit {
        self.doneObservingNotifications()
        self.peripherals.removeAll(keepingCapacity: true)
    }
    
    public func startScanning() {
        guard self.isScanning == false else {
            return
        }
        guard peripherals.count > 0 else {
            return
        }

        if self.scanTimeout > 0 {
            self.scanTimeoutTimer = IATTimer(withDuration: self.scanTimeout, whenExpired: {
                self.informDelegateOfEvent(event: BTDiscoveryEvent.scanning_timed_out)
                self.stopScanning()
            })
        }
        
        eventQueue.async {
            for peripheral in self.peripherals {
                peripheral.startScan()
            }
        }
    }

    public func stopScanning() {
        guard self.isScanning == true else {
            return
        }
        
        self.eventQueue.async {
            self.peripherals.forEach({ (peripheral) in
                peripheral.stopScan()
            })
            self.scanTimeoutTimer?.stop()
            self.informDelegateOfEvent(event: BTDiscoveryEvent.scanning_stopped)
        }
    }

    public func resumeScanning() {
        self.eventQueue.async {
            self.peripherals.forEach({ (peripheral) in
                if peripheral.centralManager?.state == .poweredOn {
                    peripheral.startScan()
                } else {
                    peripheral.stopScan()
                }
            })
        }
    }
    
    fileprivate func startObservingNotifications() {
        NotificationCenter.default.addObserver(forName: kBTRequestDisconnect, object: nil, queue: nil, using:{ notification in
            // if a specific peripheral is requested then find it and disconnect
            if let peripheralUUID = notification.userInfo?[kBTDiscoveryDisconnectPeripheralUUIDKey] as? String {
                if let peripheral = self.peripherals.first(where: { peripheral in
                    return peripheral.peripheralUUID?.uuidString == peripheralUUID
                }) {
                    peripheral.disconnect()
                }
            } else {
                // disconnect all peripherals
                self.peripherals.forEach({ (peripheral) in
                    peripheral.disconnect()
                })
            }
        })
    }
    
    private func doneObservingNotifications() {
        DispatchQueue.main.async {
            NotificationCenter.default.removeObserver(self, name: kBTRequestDisconnect, object: nil)
        }
    }
    
    public func disconnect() {
        self.peripherals.forEach({ (peripheral) in
            peripheral.disconnect()
        })
    }
    
    private func disconnect(peripheral: CBPeripheral) {
        if let oneOfOurPeripherals = self.peripherals.first(where: { oneOfOurPeripherals in
            return oneOfOurPeripherals.peripheralUUID?.uuidString == peripheral.identifier.uuidString
        }) {
            oneOfOurPeripherals.disconnect()
        }
    }
    
    // MARK: - Utilities
    
    func informDelegateOfEvent(event: BTDiscoveryEvent) {
        event.notifyFromMainQueue { (event) in
            self.delegate?.bluetoothEvent(event)
        }
        if event == .device_connected {
            self.peripherals.forEach({ (peripheral) in
                peripheral.serviceGroupAcquiredServices()
            })
        }
    }
    
    public func isKnownPeripheral(peripheral: CBPeripheral) -> Bool {
        return self.peripherals.first(where: { oneOfOurPeripherals in
            return oneOfOurPeripherals.peripheralUUID?.uuidString == peripheral.identifier.uuidString
        }) != nil
    }
    
    public func characteristic(named name: String, matchingProperties: CBCharacteristicProperties, _ perform: ((CBPeripheral, CBCharacteristic) -> Void)) {
        for peripheral in peripherals {
            if let characteristic = peripheral.serviceGroup.characteristic(named: name, matchingProperties: matchingProperties) {
                if let peripheral = peripheral.peripheral {
                    perform(peripheral, characteristic)
                }
            }
        }
    }

    public func peripheral(named peripheralName: String, characteristicNamed: String, matchingProperties: CBCharacteristicProperties, _ perform: ((CBPeripheral, CBCharacteristic) -> Void)) {
        for peripheral in peripherals {
            if peripheral.name != peripheralName { continue }
            
            if let characteristic = peripheral.serviceGroup.characteristic(named: characteristicNamed, matchingProperties: matchingProperties) {
                if let peripheral = peripheral.peripheral {
                    perform(peripheral, characteristic)
                }
            }
        }
    }

}
