//
//  IATBTPeripheral.swift
//  IATBluetooth
//
//  Created by Kurt Arnlund on 11/29/16.
//  Copyright © 2016 Ingenious Arts and Technologies LLC. All rights reserved.
//

import Foundation
import CoreBluetooth
import IATFoundationUtilities

@objc public class IATBTPeripheralSignalStrengthValidator : NSObject {
    public func validate(peripheral: CBPeripheral, advertisementData: [String : Any], rssi: NSNumber) -> Bool {
        // Exclused disconnected and undetermined signal strengths
        if rssi.intValue == 0 { return false }
        if rssi.intValue == 127 { return false }
        
        return true
    }
}

@objc public class IATBTPeripheralDeviceAdvertisementValidator : NSObject {
    public func validate(peripheral: CBPeripheral, advertisementData: [String : Any], rssi: NSNumber) -> Bool {
        return true
    }
}


@objc public class IATBTPeripheral : NSObject {
    let name: String?
    weak var discovery: IATBTDiscovery?
    var centralManager: CBCentralManager?
    public var peripheral : CBPeripheral?
    public var peripheralUUID : UUID?
    public var serviceManager : IATBTServiceManager?
    public let serviceGroup : IATBTServiceGroup
    public var advertisementData: [String:Any]?
    var disconnectActions : (() -> Void)?
    var scanTimer : IATFoundationUtilities.IATTimer?
    var lastSeenTimer : IATFoundationUtilities.IATTimer?
    public var localName : String?
    public var isScanning : Bool {
        get {
            return (self.centralManager?.isScanning) ?? false
        }
    }
    weak var eventQueue : DispatchQueue?
    let signalStrengthEvaluator : IATBTPeripheralSignalStrengthValidator
    let advertisementEvaluator : IATBTPeripheralDeviceAdvertisementValidator
    var foundPairableDevices = IATFoundationUtilities.IATOneTimeTrigger(name: "Found Pairable Devices")
    var restorable = false
    var showPowerAlert = true
    var reconnectOnDisconnect = true
    
    public init(name: String?, discovery: IATBTDiscovery, serviceGroup: IATBTServiceGroup, eventQueue: DispatchQueue = DispatchQueue(label: "iat.bt.peripheral", qos: .background, attributes: DispatchQueue.Attributes(), autoreleaseFrequency: .inherit, target: nil)) {
        self.name = name
        self.discovery = discovery
        self.serviceGroup = serviceGroup
        self.signalStrengthEvaluator = IATBTPeripheralSignalStrengthValidator()
        self.advertisementEvaluator = IATBTPeripheralDeviceAdvertisementValidator()
        self.eventQueue = eventQueue
        self.centralManager = nil
        self.advertisementData = nil
        self.peripheral = nil
        self.peripheralUUID = nil
        super.init()
    }
    
    deinit {
        self.centralManager = nil
        self.peripheral = nil
        self.advertisementData = nil
        //TODO: Call disconnect actions?
        self.disconnectActions = nil
    }
    
    private func retrieveConnectedPeripherals() -> [CBPeripheral]? {
        guard let serviceUUIDs = self.serviceGroup.requiredServiceUUIDs() else {
            return nil
        }
        return (self.centralManager?.retrieveConnectedPeripherals(withServices: serviceUUIDs))
    }
    
    private func reconnectConnectedPeripherals() -> Bool {
        let connected = self.retrieveConnectedPeripherals()
        var attemptedConnectWithOurPeripheral = false
        
        if (connected?.count)! > 0 {
            connected?.forEach({ (connectedPeripheral) in
                if connectedPeripheral.identifier.uuidString == peripheralUUID?.uuidString {
                    centralManager?.connect(connectedPeripheral, options: nil)
                    attemptedConnectWithOurPeripheral = true
                }
            })
        }
        return attemptedConnectWithOurPeripheral
    }
    
    private func scanForMatchingPeripherals() {
        guard let serviceUUIDs = self.serviceGroup.requiredServiceUUIDs() else {
            return
        }
        self.centralManager?.scanForPeripherals(withServices: serviceUUIDs, options: [CBCentralManagerScanOptionAllowDuplicatesKey : true])
    }

    open func startScan() {
//        eventQueue?.async {
            self.setup()
            guard self.centralManager?.isScanning == false else {
                return
            }
            guard self.centralManager?.state == .poweredOn else {
                return
            }
            if self.peripheralUUID == nil || self.reconnectConnectedPeripherals() == false {
                self.scanForMatchingPeripherals()
            }
//        }
    }
    
    open func stopScan() {
        eventQueue?.async {
            guard self.centralManager?.isScanning == true else {
                return
            }
            self.centralManager?.stopScan()
        }
    }

    open func disconnect() {
        guard let peripheral = self.peripheral else {
            return
        }
        self.centralManager?.cancelPeripheralConnection(peripheral)
    }
    
    //MARK: Private Methods
    internal func setup() {
        guard self.centralManager == nil else {
            return
        }
        var options : [String : Any]? = [:]
        if self.restorable == true {
            options?[CBCentralManagerOptionRestoreIdentifierKey] = self.name
        }
        let showPowerAlert = true
        if showPowerAlert == true {
            options?[CBCentralManagerOptionShowPowerAlertKey] = showPowerAlert
        }
        
        self.centralManager = CBCentralManager.init(delegate: self, queue: self.eventQueue, options: options)
    }

    //MARK: Static Methods
}

extension IATBTPeripheral : CBCentralManagerDelegate {
    private func notify(event: BTDiscoveryEvent) {
        event.notifyFromMainQueue { (event) in
            self.discovery?.informDelegateOfEvent(event: event)
        }
    }
    
    // MARK: - Internal CoreBluetooth Tomfollery to optionally support state restoration
    // F#$% You CoreBluetooth for checking if the state restoration method exists.
    // We should have more powerful options at our disposal.
    
    override public func responds(to aSelector: Selector!) -> Bool {
        if aSelector == #selector(IATBTPeripheral.centralManager(_:willRestoreState:)) {
            if self.restorable == false {
                return false
            }
        }
        return super.responds(to: aSelector)
    }

    // MARK: - CentralManager delegate
    
    open func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state {
        case .unsupported:
            self.notify(event: .central_manager_state_unsupported)
        case .poweredOn:
            self.notify(event: .central_manager_state_powered_on)
        case .poweredOff:
            self.notify(event: .central_manager_state_powered_off)
        case .resetting:
            self.notify(event: .central_manager_state_resetting)
        case .unauthorized:
            self.notify(event: .central_manager_state_unauthorized)
        case .unknown:
            self.notify(event: .central_manager_state_unknown)
        }
    }
    
    open func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        guard central == self.centralManager else {
            return
        }
        
        guard self.peripheral != nil else {
            return
        }

        // check peripheral to see if it conforms to all the services that we are expecting?
        if peripheral == self.peripheral {
            self.serviceManager = IATBTServiceManager(withPeripheral: peripheral, serviceGroup: self.serviceGroup)
            DispatchQueue.main.async {
                self.serviceGroup.startAquiringServices(forPeripheral: peripheral)
            }
        }
    }
    
    open func centralManager(_ central: CBCentralManager, willRestoreState dict: [String : Any]) {
        self.notify(event: .central_manager_state_restoring)
    }
    
    open func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?) {
        
    }
    
    open func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        // from here follow a reconnection strategy
        self.reset()
        self.notify(event: .device_disconnected)
        if self.reconnectOnDisconnect == false {
            self.centralManager?.stopScan()
        }
    }
    
    open func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        guard central == self.centralManager else {
            return
        }

        var isConnectable = false
        if let localName = advertisementData[CBAdvertisementDataLocalNameKey] as! String? {
            self.localName = localName
        }
        
        if let connectable = advertisementData[CBAdvertisementDataIsConnectable] as! NSNumber? {
            isConnectable = connectable.boolValue
        }
        
        if signalStrengthEvaluator.validate(peripheral: peripheral, advertisementData: advertisementData, rssi: RSSI) == false {
            self.notify(event: .scanning_rejected_device_due_to_signal_strength)
            return
        }
        
        if advertisementEvaluator.validate(peripheral: peripheral, advertisementData: advertisementData, rssi: RSSI) == false {
            self.notify(event: .scanning_rejected_device_due_to_advertisement_data)
            return
        }

        self.foundPairableDevices.doThis(once: {
            print("central manager " + (self.centralManager?.isScanning == true ? "scanning" : ""))
            self.notify(event: .scanning_found_pairable_devices)
        })
        
        if isConnectable && self.peripheral == nil {
            self.peripheral = peripheral
            self.peripheralUUID = peripheral.identifier
            self.advertisementData = advertisementData
            
            self.serviceManager = nil
            
            self.notify(event: .device_connection_started)

            self.centralManager?.connect(peripheral)
        }
    }
    
    private func reset() {
        self.foundPairableDevices.reset { 
            print("foundPairableDevices has been reset")
        }
        self.peripheral = nil
        self.serviceGroup.reset()
        self.serviceManager?.reset()
    }
}
