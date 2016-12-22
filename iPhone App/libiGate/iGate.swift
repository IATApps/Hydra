//
//  iGate.swift
//  hydra
//
//  Created by Kurt Arnlund on 11/29/16.
//  Copyright © 2016 Ingenious Arts and Technologies LLC. All rights reserved.
//

import Foundation
import CoreBluetooth
import IATBluetooth
import IATFoundationUtilities

enum CiGateState : Int {
    case initialized    // State unknown, update imminent.
    case powered_off    // Bluetooth is currently powered off.
    case unknown        // State unknown, update imminent.
    case resetting      // The connection with the system service was momentarily lost, update imminent.
    case unsupported	// Something wrong, iOS device not support BTLE or not power on.
    case unauthorized   // The app is not authorized to use Bluetooth Low Energy.
    case idle           // Bluetooth is currently powered on and available to use.
    case reconnectionSearch // The iGate is searching to reconnect to the previous device connected.
    case searching      // The iGate is searching to a device.
    case connecting     // the iGate is connecting to a device.
    case connected      // The igate is connected with a device.
    case bonded         // The igate is bondeded (and the connection is encypted)
    case disconnected
}

protocol CiGateComDelegate : class {
    func didUpdate(iGate: CiGate, state: CiGateState)
    func didReceiveData(iGate: CiGate, data: Data)
}

class NearbySignalStrengthValidator : IATBTPeripheralSignalStrengthValidator {
    var strengthLimit : Int = -70
    func searchingLimit() {
        strengthLimit = -70
    }
    
    func reconnectionLimit() {
        strengthLimit = -126
    }
    
    override func validate(_ peripheral: CBPeripheral, advertisementData: [String : Any], rssi: NSNumber) -> Bool {
        if super.validate(peripheral, advertisementData: advertisementData, rssi: rssi) == false {
            print("signal strength ", rssi.intValue, " out of range")
            return false
        }
        
        if rssi.intValue < strengthLimit {
            print("signal strength ", rssi.intValue, " not in custom limit ", strengthLimit)
            return false
        }
        
        return true
    }
}


class CiGate : NSObject, BTDiscoveryEventDelegate {
    private var _state : CiGateState = .initialized
    var state : CiGateState {
        set {
            if (newValue != _state) {
                _state = newValue
                self.notifyNewState()
            }
        }
        
        get {
            return _state
        }
    }
    private var _connectedDevName : String = ""
    var connectedDevName : String {
        set {
            _connectedDevName = newValue
        }
        
        get {
            return _connectedDevName
        }
    }
    var serviceUUIDstr : String?
    var connectedDevUUID : UUID?
    var bondedDevUUID : UUID?
    weak var comDelegate : CiGateComDelegate?
    let iGateDiscovery : IATBTDiscovery
    let iGateComService = IATBTService(serviceUUID: "B5161D82-AAB0-4E55-8D96-C59D816E6971", required: true, characteristicDescriptions: [:])
    let iGateDIService = IATBTDeviceInformationService()
    
    init(delegate comDelegate: CiGateComDelegate, autoConnect:Bool, serviceUUIDstr:String ) {
        self.comDelegate = comDelegate
        self.bondedDevUUID = nil
        self.serviceUUIDstr = serviceUUIDstr
        self.iGateDiscovery = IATBTDiscovery(name: "iGate", withServices: [iGateComService, iGateDIService])
        self.iGateDiscovery.peripherals.first?.signalStrengthEvaluator = NearbySignalStrengthValidator()
        super.init()
        self.iGateDiscovery.delegate = self
        self.commonStartup()
    }

    init(delegate comDelegate: CiGateComDelegate, autoConnect:Bool, bondedDevUUID:UUID, serviceUUIDstr:String ) {
        self.comDelegate = comDelegate
        self.serviceUUIDstr = serviceUUIDstr
        self.bondedDevUUID = bondedDevUUID
        self.iGateDiscovery = IATBTDiscovery(name: "iGate", withServices: [iGateComService, iGateDIService])
        self.iGateDiscovery.peripherals.first?.signalStrengthEvaluator = NearbySignalStrengthValidator()
        super.init()
        self.iGateDiscovery.delegate = self
        self.commonStartup()
    }
    
    func startSearch() {
        self.state = .searching
        self.iGateDiscovery.startScanning()
    }

    func stopSearch() {
        self.iGateDiscovery.stopScanning()
    }
    
    func disconnect() {
        self.iGateDiscovery.disconnect()
    }
    
    func getConnectedDevRSSI(asyncCallback: @escaping (NSNumber) -> Void ) {
        //TODO: This is a stub function right now.   Make it work.
        DispatchQueue.main.async {
            asyncCallback(NSNumber(floatLiteral: 0.0))
        }
    }
    
    func sendData(toDevice: Data, characteristicNamed characteristicKey:String) {
        //TODO: This is setup for Thync right now
        self.iGateDiscovery.performOnCharacteristic(named: characteristicKey) { (peripheral, characteristic) in
            peripheral.writeValue(toDevice, for: characteristic, type: .withoutResponse)
        }
    }
    
    func notify(on: Bool, characteristicNamed characteristicKey:String) {
        self.iGateDiscovery.performOnCharacteristic(named: characteristicKey) { (peripheral, characteristic) in
            let isNotifying = characteristic.isNotifying
            
            if isNotifying != on {
                print("setting notify " + (on ? "ON" : "OFF") + " for characteristic "  + characteristicKey + " (" + characteristic.uuid.uuidString + ")")
                peripheral.setNotifyValue(true, for: characteristic)
            } else {
                print("notify is already " + (on ? "ON" : "OFF") + " for characteristic "  + characteristicKey + " (" + characteristic.uuid.uuidString + ")")
            }
        }
    }

    func observeThyncCharacteristic(on: Bool) {
    }
    
    func mainDevice() -> IATBTPeripheral? {
        return self.iGateDiscovery.peripherals.first
    }

    //MARK: - Private
    
    private func strengthEvaluator() -> NearbySignalStrengthValidator? {
        return self.iGateDiscovery.peripherals.first?.signalStrengthEvaluator as? NearbySignalStrengthValidator
    }
    
    private func commonStartup() {
        self.notifyNewState()
    }
    
    func bluetoothEvent(_ event: BTDiscoveryEvent) {
        var error = false
        switch event {
        case .central_manager_state_unauthorized:
            self.state = .unauthorized
        case .central_manager_state_powered_off:
            self.state = .powered_off
        case .central_manager_state_powered_on:
            if self.state == .searching {
                self.strengthEvaluator()?.searchingLimit()
                self.iGateDiscovery.startScanning()
            }
        case .central_manager_state_unsupported:
            self.state = .unsupported
        case .scanning_found_pairable_devices:
            break
        case .device_connection_started:
            self.state = .connecting
        case .device_connected:
            self.state = .connected
            //MARK: This does not stop scanning.  It doesn't work here.
//            self.iGateDiscovery.stopScanning()
        case .device_disconnected:
            self.state = .disconnected
        case .attempting_to_reconnect:
            self.state = .reconnectionSearch
            self.strengthEvaluator()?.reconnectionLimit()
        default:
            error = true
        }
        if error == false {
            print("CiGate bluetooth event \""+event.notificationName.rawValue+"\"")
        } else {
            print("CiGate not handling event \""+event.notificationName.rawValue+"\"")
        }
    }
    
    private func notifyNewState() {
        self.comDelegate?.didUpdate(iGate: self, state: self._state)
    }
}

    
