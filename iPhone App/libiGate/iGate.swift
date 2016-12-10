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

enum CiGateState : Int {
    case initialized    // State unknown, update imminent.
    case powered_off    // Bluetooth is currently powered off.
    case unknown        // State unknown, update imminent.
    case resetting      // The connection with the system service was momentarily lost, update imminent.
    case unsupported	// Something wrong, iOS device not support BTLE or not power on.
    case unauthorized   // The app is not authorized to use Bluetooth Low Energy.
    case idle           // Bluetooth is currently powered on and available to use.
    case searching      // The iGate is searching to a device.
    case connecting     // the iGate is connecting to a device.
    case connected      // The igate is connected with a device.
    case bonded         // The igate is bondeded (and the connection is encypted)
}

protocol CiGateComDelegate : class {
    func didUpdate(iGate: CiGate, state: CiGateState)
    func didReceiveData(iGate: CiGate, data: Data)
}

//protocol CiGateRssiDelegate {
//    func didUpdateConnected(iGate: CiGate, rssi: NSNumber, error: Error?)
//}
//
//protocol CiGateBtAddrDelegate {
//    func didUpdateConnected(iGate: CiGate, address: CBluetoothAddr)
//}

class CiGate {
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
        self.commonStartup()
    }

    init(delegate comDelegate: CiGateComDelegate, autoConnect:Bool, bondedDevUUID:UUID, serviceUUIDstr:String ) {
        self.comDelegate = comDelegate
        self.serviceUUIDstr = serviceUUIDstr
        self.bondedDevUUID = bondedDevUUID
        self.iGateDiscovery = IATBTDiscovery(name: "iGate", withServices: [iGateComService, iGateDIService])
        self.commonStartup()
    }
    
    func startSearch() {
        //TODO: This is a stub function right now.   Make it work.
        self.state = .searching
        self.iGateDiscovery.startScanning()
    }

    func stopSearch() {
        //TODO: This is a stub function right now.   Make it work.
        
    }
    
    func getConnectedDevRSSI(asyncCallback: @escaping (NSNumber) -> Void ) {
        //TODO: This is a stub function right now.   Make it work.
        DispatchQueue.main.async {
            asyncCallback(NSNumber(floatLiteral: 0.0))
        }
    }
    
    func sendData(toDevice: Data) {
        //TODO: This is a stub function right now.   Make it work.
        
    }

    //MARK: - Private
    
    private func commonStartup() {
        self.notifyNewState()
        
        NotificationCenter.default.addObserver(forName: BTDiscoveryEvents(central_manager_state_authorized).notificationName, object: nil, queue: OperationQueue.main, using: { notification in
            self.state = .unauthorized
        })

        NotificationCenter.default.addObserver(forName: kBTDiscoveryBluetoothTurnedOff, object: nil, queue: OperationQueue.main, using: { notification in
            self.state = .powered_off
        })

        NotificationCenter.default.addObserver(forName: kBTDiscoveryBluetoothTurnedOn, object: nil, queue: OperationQueue.main, using: { notification in
            if self.state == .searching {
                DispatchQueue.main.async {
                    self.iGateDiscovery.startScanning()
                }
            }
        })

        NotificationCenter.default.addObserver(forName: kBTDiscoveryBluetoothUnsupported, object: nil, queue: OperationQueue.main, using: { notification in
            self.state = .unsupported
        })

        NotificationCenter.default.addObserver(forName: kBTDiscoveryPairingStarted, object: nil, queue: OperationQueue.main, using: { notification in
            self.state = .connecting
        })

    }
    
    private func notifyNewState() {
        self.comDelegate?.didUpdate(iGate: self, state: self._state)
    }
}

    
