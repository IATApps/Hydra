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
    let name: String
    weak var discovery: IATBTDiscovery?
    var centralManager: CBCentralManager?
    var peripheral : CBPeripheral?
    var peripheralUUID : UUID?
    var serviceManager : IATBTServiceManager?
    let serviceGroup : IATBTServiceGroup
    var advertisementData: [String:Any]?
    var disconnectActions : (() -> Void)?
    var scanTimer : IATFoundationUtilities.IATTimer?
    var lastSeenTimer : IATFoundationUtilities.IATTimer?
    var localName : String?
    var isScanning : Bool {
        get {
            return (self.centralManager?.isScanning) ?? false
        }
    }
    weak var eventQueue : DispatchQueue?
    let signalStrengthEvaluator : IATBTPeripheralSignalStrengthValidator
    let advertisementEvaluator : IATBTPeripheralDeviceAdvertisementValidator
    var foundUnpairables = IATFoundationUtilities.IATOneTimeTrigger(name: "Found Unpairable Devices")
    
    public init(name: String, discovery: IATBTDiscovery, serviceGroup: IATBTServiceGroup, eventQueue: DispatchQueue = DispatchQueue(label: "iat.bt.peripheral", qos: .background, attributes: DispatchQueue.Attributes(), autoreleaseFrequency: .inherit, target: nil)) {
        self.name = name
        self.discovery = discovery
        self.serviceGroup = serviceGroup
        self.signalStrengthEvaluator = IATBTPeripheralSignalStrengthValidator()
        self.advertisementEvaluator = IATBTPeripheralDeviceAdvertisementValidator()
        self.eventQueue = eventQueue
        self.centralManager = nil
        self.advertisementData = nil
        self.peripheral = nil
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
        eventQueue?.async {
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
        }
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
    private func setup() {
        guard self.centralManager == nil else {
            return
        }
        self.centralManager = CBCentralManager.init(delegate: self, queue: self.eventQueue, options: [CBCentralManagerOptionShowPowerAlertKey:NSNumber(value: true), CBCentralManagerOptionRestoreIdentifierKey: self.name])
    }
    

    //MARK: Static Methods
    static func sendBTServiceNotificationWithIsBluetoothConnected(isConnected: Bool) {
        let connectionDetails = ["isConnected": isConnected];
        NotificationCenter.default.post(name: kIATBT_Service_Changed_Status_Notification, object: nil, userInfo: connectionDetails)
        if isConnected == true {
            NotificationCenter.default.post(name: kIATBT_Service_Connected_Notification, object: nil, userInfo: nil)
        } else {
            NotificationCenter.default.post(name: kIATBT_Service_Disconnected_Notification, object: nil, userInfo: nil)
        }
    }
    //self.serviceGroup?.peripheral(peripheral, didDiscoverServices: error)

}

extension IATBTPeripheral : CBCentralManagerDelegate {
    // MARK: - CentralManager delegate
    private func postNotificationForEvent(event: BTDiscoveryEvent) {
        event.notifyFromMainQueue { (event) in
            self.discovery?.informDelegateOfEvent(event: event)
        }
    }
    
    open func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state {
        case .unsupported:
            self.postNotificationForEvent(event: .central_manager_state_unsupported)
        case .poweredOn:
            self.postNotificationForEvent(event: .central_manager_state_powered_on)
        case .poweredOff:
            self.postNotificationForEvent(event: .central_manager_state_powered_off)
        case .resetting:
            self.postNotificationForEvent(event: .central_manager_state_resetting)
        case .unauthorized:
            self.postNotificationForEvent(event: .central_manager_state_unauthorized)
        case .unknown:
            self.postNotificationForEvent(event: .central_manager_state_unknown)
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
            self.postNotificationForEvent(event: .device_connection_started)
            self.serviceManager = IATBTServiceManager(withPeripheral: peripheral, serviceGroup: self.serviceGroup)
            DispatchQueue.main.async {
                self.serviceGroup.startAquiringServices(forPeripheral: peripheral)
            }
        }
    }
    
    open func centralManager(_ central: CBCentralManager, willRestoreState dict: [String : Any]) {
        self.postNotificationForEvent(event: .central_manager_state_restoring)
    }
    
    open func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?) {
        
    }
    
    open func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        // from here follow a reconnection strategy
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
            return
        }
        
        if advertisementEvaluator.validate(peripheral: peripheral, advertisementData: advertisementData, rssi: RSSI) == false {
            self.foundUnpairables.doThis(once: {
                self.postNotificationForEvent(event: .scanning_found_pairable_devices)
            }, resetAfter: 3,
               onReset: {
                print("kBTDiscoveryFoundUnpairables has been reset")
            })
            return
        }
        
        if isConnectable && self.peripheral == nil {
            self.peripheral = peripheral
            self.peripheralUUID = peripheral.identifier
            self.advertisementData = advertisementData
            
            self.serviceManager = nil
            
            self.postNotificationForEvent(event: .device_connection_started)

            self.centralManager?.connect(peripheral)
        }
    }
}
