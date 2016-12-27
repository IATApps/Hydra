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

@objc open class IATBTPeripheralSignalStrengthValidator : NSObject {
    open func validate(_ peripheral: CBPeripheral, advertisementData: [String : Any], rssi: NSNumber) -> Bool {
        // Exclused disconnected and undetermined signal strengths
        if rssi.int32Value == 0 { return false }
        if rssi.int32Value == 127 { return false }
        
        return true
    }
}

@objc open class IATBTPeripheralDeviceAdvertisementValidator : NSObject {
    open func validate(_ peripheral: CBPeripheral, advertisementData: [String : Any], rssi: NSNumber) -> Bool {
        return true
    }
}


@objc open class IATBTPeripheral : NSObject {
    let name: String?
    weak var discovery: IATBTDiscovery?
    var centralManager: CBCentralManager?
    open var peripheral : CBPeripheral?
    open var peripheralUUID : UUID?
    open var serviceManager : IATBTServiceManager?
    open let serviceGroup : IATBTServiceGroup
    open var advertisementData: [String:Any]?
    var scanTimer : IATTimer?
    var lastSeenTimer : IATTimer?
    var reconnectTimeoutTimer : IATTimer?
    open var localName : String?
    open var isScanning : Bool {
        get {
            return (self.centralManager?.isScanning) ?? false
        }
    }
    weak var eventQueue : DispatchQueue?
    open var signalStrengthEvaluator : IATBTPeripheralSignalStrengthValidator
    open var advertisementEvaluator : IATBTPeripheralDeviceAdvertisementValidator
    var foundPairableDevices = IATOneTimeTrigger(name: "Found Pairable Devices")
    var restorable = false
    var showPowerAlert = true
    var reconnectOnDisconnect = true
    internal var connectTime : Date?
    var peripheralDelegate : CBPeripheralDelegate?
    
    public init(name: String?, discovery: IATBTDiscovery, serviceGroup: IATBTServiceGroup, peripheralDelegate: CBPeripheralDelegate? = nil, eventQueue: DispatchQueue = DispatchQueue(label: "iat.bt.peripheral", qos: .background, attributes: DispatchQueue.Attributes(), autoreleaseFrequency: .inherit, target: nil)) {
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
        self.peripheralDelegate = peripheralDelegate
        super.init()
    }
    
    deinit {
        self.centralManager = nil
        self.peripheral = nil
        self.advertisementData = nil
        self.eventQueue = nil
        self.peripheralDelegate = nil
    }
    
    fileprivate func retrieveConnectedPeripherals() -> [CBPeripheral]? {
        guard let serviceUUIDs = self.serviceGroup.requiredServiceUUIDs() else {
            return nil
        }
        return (self.centralManager?.retrieveConnectedPeripherals(withServices: serviceUUIDs))
    }
    
    fileprivate func reconnectConnectedPeripherals() -> Bool {
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
    
    fileprivate func scanForMatchingPeripherals() {
        guard let serviceUUIDs = self.serviceGroup.requiredServiceUUIDs() else {
            return
        }
        self.centralManager?.scanForPeripherals(withServices: serviceUUIDs, options: [CBCentralManagerScanOptionAllowDuplicatesKey : true])
    }

    internal func serviceGroupAcquiredServices() {
        if let interval = self.connectTime?.timeIntervalSinceNow {
            print("time to acquire services and characteristics ", interval)
        }
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
    fileprivate func notify(_ event: BTDiscoveryEvent) {
        event.notifyFromMainQueue { (event) in
            self.discovery?.informDelegateOfEvent(event: event)
        }
    }
    
    // MARK: - Internal CoreBluetooth Tomfollery to optionally support state restoration
    // F#$% You CoreBluetooth for checking if the state restoration method exists.
    // We should have more powerful options at our disposal.
    
    override open func responds(to aSelector: Selector!) -> Bool {
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
            self.notify(.central_manager_state_unsupported)
        case .poweredOn:
            self.notify(.central_manager_state_powered_on)
        case .poweredOff:
            self.notify(.central_manager_state_powered_off)
        case .resetting:
            self.notify(.central_manager_state_resetting)
        case .unauthorized:
            self.notify(.central_manager_state_unauthorized)
        case .unknown:
            self.notify(.central_manager_state_unknown)
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
            self.connectTime = Date()
            self.serviceManager = IATBTServiceManager(withPeripheral: peripheral, serviceGroup: self.serviceGroup, peripheralDelegate: self.peripheralDelegate)
            DispatchQueue.main.async {
                print("starting to connect to peripheral " + peripheral.identifier.uuidString)
                self.serviceGroup.startAquiringServices(forPeripheral: peripheral)
                self.discovery?.stopScanning()
            }
        }
    }
    
    open func centralManager(_ central: CBCentralManager, willRestoreState dict: [String : Any]) {
        self.notify(.central_manager_state_restoring)
    }
    
    open func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?) {
        self.resetForDisconnect()
    }
    
    open func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        // from here follow a reconnection strategy
        print("disconnect from peripheral " + peripheral.identifier.uuidString)
        if let peripheralUUID = self.peripheralUUID?.uuidString, self.reconnectOnDisconnect == true , self.serviceGroup.allServicesAndCharacteristicsAcquired() == true {
            self.resetForReconnection()
            print("attempt to reconnect with " + peripheralUUID);
            self.notify(.attempting_to_reconnect)
            self.reconnectTimeoutTimer = IATTimer(withDuration: 10, whenExpired: {
                print("Expanding search to all devices");
                self.peripheralUUID = nil;
                self.notify(.device_disconnected)
            })
        } else {
            self.resetForDisconnect()
            self.centralManager?.stopScan()
            self.notify(.device_disconnected)
        }
    }
    
    open func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        guard central == self.centralManager else {
            return
        }
        
        if self.peripheral != nil {
            return
        }

        var isConnectable = false
        if let localName = advertisementData[CBAdvertisementDataLocalNameKey] as! String? {
            self.localName = localName
        }
        
        if let connectable = advertisementData[CBAdvertisementDataIsConnectable] as! NSNumber? {
            isConnectable = connectable.boolValue
        }
        
        if signalStrengthEvaluator.validate(peripheral, advertisementData: advertisementData, rssi: RSSI) == false {
            self.notify(.scanning_rejected_device_due_to_signal_strength)
            
            self.foundPairableDevices.doThis({
                print("central manager " + (self.centralManager?.isScanning == true ? "scanning" : ""))
                self.notify(.scanning_found_pairable_devices)
            })
            return
        }
        
        if advertisementEvaluator.validate(peripheral, advertisementData: advertisementData, rssi: RSSI) == false {
            self.notify(.scanning_rejected_device_due_to_advertisement_data)
            
            self.foundPairableDevices.doThis({
                print("central manager " + (self.centralManager?.isScanning == true ? "scanning" : ""))
                self.notify(.scanning_found_pairable_devices)
            })
            return
        }

        if isConnectable && self.peripheral == nil {
            if let uuidLen = self.peripheralUUID?.uuidString.lengthOfBytes(using: .ascii) {
                if uuidLen >= 4 {
                    if self.peripheralUUID?.uuidString != peripheral.identifier.uuidString {
                        return
                    }
                }
            }
            
            self.peripheral = peripheral
            self.peripheralUUID = peripheral.identifier
            self.advertisementData = advertisementData
            
            self.serviceManager = nil
            
            self.reconnectTimeoutTimer?.stop()
            
            self.notify(.device_connection_started)
            
            self.centralManager?.connect(peripheral)
            self.stopScan()
        }
    }
    
    fileprivate func resetForReconnection() {
        self.foundPairableDevices.reset { 
            print("foundPairableDevices has been reset")
        }
        self.peripheral = nil
        self.serviceManager?.resetForReconnection()
    }
    
    fileprivate func resetForDisconnect() {
        self.resetForReconnection()
        self.peripheralUUID = nil
    }

}
