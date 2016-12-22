//
//  HydraComm.swift
//  hydra
//
//  Created by Kurt Arnlund on 11/21/16.
//  Ingenious Arts and Technologies LLC
//

import Foundation

@objc class HydraComm : NSObject, CiGateComDelegate {
    
    var bondedUUID : UUID?
    
    lazy var btleData = Data(capacity: 1024)
    
    var iGate : CiGate?
    
    override func finalize() {
        self.iGate?.comDelegate = nil
        self.iGate = nil
    }
    
    var unsubscribedFromDataTimer : Timer?
    
    private func serviceUUID() -> UUID {
        return UUID(uuidString: "B5161D82-AAB0-4E55-8D96-C59D816E6971")!
    }
    
    public func pair() {
        if let bonded = UserDefaults.standard.object(forKey: "bonded") {
            self.bondedUUID = UUID(uuidString: bonded as! String)
        }
        if self.iGate == nil {
            self.iGate = CiGate(delegate: self, autoConnect: true, serviceUUIDstr: self.serviceUUID().uuidString)
        }
        self.iGate?.startSearch()
    }
    
    public func stop() {
        self.iGate?.stopSearch()
    }
    
    public func disconnect() {
        self.iGate?.disconnect()
        displayOverlayState(state: .disconnecting)
    }
    
    func didUpdate(iGate: CiGate, state: CiGateState) {
        switch (state) {
        case .initialized:
            print("iGate Init")
            displayOverlayState(state: .initialized)

        case .powered_off:
            print("iGate Powered Off")
            displayOverlayState(state: .bt_off)
            self.pair()

        case .unknown:
            print("iGate Unknown")

        case .resetting:
            print("iGate Resetting")
            displayOverlayState(state: .resetting)
            
        case .unsupported:
            print("iGate Unsupported")
            displayOverlayState(state: .bt_unsupported)
            
        case .unauthorized:
            print("iGate Unauthorized")
            displayOverlayState(state: .bt_unauthorized)
            
        case .idle:
            print("iGate Idle")
            
        case .searching:
            print("iGate Searching")
            displayOverlayState(state: .searching)
            
        case .connecting:
            print("iGate Connecting")
            displayOverlayState(state: .connecting)
            
        case .reconnectionSearch:
            print("iGate Searching for reconnection")
            displayOverlayState(state: .reconnectionSearch)
            
        case .connected:
            print("iGate Connected")
            displayOverlayState(state: .connected)
            if #available(iOS 10.0, *) {
                //                    unsubscribedFromDataTimer = Timer.scheduledTimer(withTimeInterval: 1, repeats: false, block: { timer in
                //                        self.iGate?.startSearch()
                //                    })
            } else {
                // Fallback on earlier versions
            }
            self.bondedUUID = self.iGate!.bondedDevUUID
            
        case .disconnected:
            print("iGate Disconnected")
            displayOverlayState(state: .disconnected)
 
        case .bonded:
            print("iGate Bonded")
            let uuidStr = self.iGate?.connectedDevUUID?.uuidString ?? "unknown"
            let bondedUuidStr = (self.bondedUUID?.uuidString)! as String
            let peripheralName = self.iGate?.connectedDevName
            print("BONDED TO %@ [UUID:  %@  |  Bonded UUID: %@]",  peripheralName!, uuidStr, bondedUuidStr)
            UserDefaults.standard.set(bondedUuidStr, forKey:"bonded")
        }
        
        let dict = ["state" : NSNumber(value: state.rawValue)];
        NotificationCenter.default.post( Notification(name: Notification.Name(rawValue: "stateUpdate"), object: nil, userInfo: dict) )
    }
    
    private func displayOverlayState(state: HydraStateOverlay.iGateOverlayStates) {
        NotificationCenter.default.post(name: HydraStateOverlay.iGateEvent()!, object: nil, userInfo: [HydraStateOverlay.iGateEventUserInfoStateKey() : state.rawValue])
    }
    
    func didReceiveData(iGate: CiGate, data: Data) {
        if unsubscribedFromDataTimer != nil {
            unsubscribedFromDataTimer?.invalidate()
            unsubscribedFromDataTimer = nil
        }
        
        print("iGateDidReceivedData")
        var index : Int = 0;
        
        // Take the data and append it to the stored data.
        // Make sure we don't overflow... if the length would be too large,
        // just discard what we have and read the new data.
        // This should never happen if the BTLE peripheral is behaving properly,
        // but we check for it anyway.
        btleData.append(data)
        
        print("BTLE_data %@", btleData);
        
        // Check to make sure that we have enough data for a packet to even possibly exist
        if( btleData.count < 3 )
        {
            return
        }
        
        while (index < btleData.count) {
             // Search for beginning of packet "snp"
            while( btleData[index] != UInt8(ascii:"s") || btleData[index+1] != UInt8(ascii:"n") || btleData[index+2] != UInt8(ascii:"p"))
            {
                index += 1;
                
                // Make sure that we don't go beyond the edge of the array
                if ( (index+2) >= btleData.count )
                {
                    return
                }
            }
        
//          If we get here, the 'snp' sequence was found.  
//          If index != 0, erase all preceding bytes - these are junk.  We can't do anything
//          with them, without the beginning of the packet, which was apparently truncated.
            if( index > 0 )
            {
                print("Resetting data scratch log");
                
                if (btleData.count - index) > 0 {
                    btleData.removeSubrange(Range.init(uncheckedBounds: (lower: 0, upper: index)))
                } else {
                    btleData = Data()
                }
                
                index = 0
            }
            
            let hydra = HydraPacket(packet: btleData)!
            
            // Now check for expected packet types
            // Packet type 0xd0 indicates a batch packet containing four registers.
            // Address 85(0x55) is the first data register address.
            // received a voltage and current status packet
            if hydra.checkPtByte(0xD0, addressByte: 0x55) {
                if hydra.packet.length < 23 {
                    return;
                }
                
                if !hydra.isChecksumValid() {
                    print("iGateDidReceivedData 0xD0 Checksum rejected");
                    print("%@", hydra);
                    btleData = Data(capacity: 1014)
                    return;
                }
                
                NotificationCenter.default.post(name: HydraState.rxNotifName(), object: nil, userInfo: [HydraState.userInfoKey() : hydra])
                
                index += 23
            }
            // recieved config packet
            else if hydra.checkPtByte(0xCC, addressByte: 0x00) {
                print("[Hydra Config Packet]")
            }
            // recevied a acknowledgement success packet
            else if hydra.checkPtByte(0x00, addressByte: 0x00) {
                print("[Hydra ACK Packet]")
            }
            // recevied a firmware response packet
            else if hydra.checkPtByte(0x80, addressByte: 0xAA) {
                print("[Hydra Firmware Packet]")
            }
        }
        
        if (index >= btleData.count) {
            btleData = Data()
        }

//        else if ((hydra.ptByte == 0xCC) && (hydra.addressByte == 0x00))
//        { //recieved config packet
//            // Compare to the checksum provided in the packet and return if they don't match
//            if (![hydra isChecksumValid])
//            {
//                NSLog(@"iGateDidReceivedData 0xCC Checksum rejected");
//                [BTLE_data setLength: 0];
//                return;
//            }
//            
//            uint8_t* data;
//            data = &packet[5];
//            uint16_t ch1MaxCurrent = ((data[0] & 0x0F) << 8)|data[1];
//            uint8_t ch1OE = (data[0]>>7)&0x01;
//            uint8_t ch1LC = (data[0]>>6)&0x01;
//            uint16_t ch1TargetVoltage = (data[2] << 8)|data[3];
//            
//            uint16_t ch2MaxCurrent = ((data[4] & 0x0F) << 8)|data[5];
//            uint8_t ch2OE = (data[4]>>7)&0x01;
//            uint8_t ch2LC = (data[4]>>6)&0x01;
//            uint16_t ch2TargetVoltage = (data[6] << 8)|data[7];
//            
//            uint16_t ch3MaxCurrent = ((data[8] & 0x0F) << 8)|data[9];
//            uint8_t ch3OE = (data[8]>>7)&0x01;
//            uint8_t ch3LC = (data[8]>>6)&0x01;
//            uint16_t ch3TargetVoltage = (data[10] << 8)|data[11];
//            
//            __unused HydraChannel *ch1 = [hydra batchEntryAtIndex:0];
//            __unused HydraChannel *ch2 = [hydra batchEntryAtIndex:1];
//            __unused HydraChannel *ch3 = [hydra batchEntryAtIndex:2];
//            __unused HydraChannel *ch4 = [hydra batchEntryAtIndex:3];
//            
//            self.ch1TargetVoltage = [NSNumber numberWithUnsignedInt:ch1TargetVoltage].intValue;
//            self.ch2TargetVoltage = [NSNumber numberWithUnsignedInt:ch2TargetVoltage].intValue;
//            self.ch3TargetVoltage = [NSNumber numberWithUnsignedInt:ch3TargetVoltage].intValue;
//            
//            self.ch1MaxCurrent = [NSNumber numberWithUnsignedInt:ch1MaxCurrent].intValue;
//            self.ch2MaxCurrent = [NSNumber numberWithUnsignedInt:ch2MaxCurrent].intValue;
//            self.ch3MaxCurrent = [NSNumber numberWithUnsignedInt:ch3MaxCurrent].intValue;
//            
//            self.ch1OE = [NSNumber numberWithUnsignedInt:ch1OE].boolValue;
//            self.ch2OE = [NSNumber numberWithUnsignedInt:ch2OE].boolValue;
//            self.ch3OE = [NSNumber numberWithUnsignedInt:ch3OE].boolValue;
//            
//            self.ch1LC = [NSNumber numberWithUnsignedInt:ch1LC].boolValue;
//            self.ch2LC = [NSNumber numberWithUnsignedInt:ch2LC].boolValue;
//            self.ch3LC = [NSNumber numberWithUnsignedInt:ch3LC].boolValue;
//            
//            [[NSNotificationCenter defaultCenter] postNotificationName:@"recievedConfig" object:nil userInfo:nil];
//            
//            // Set flag indicating that we've received configuration data for the Hydra
//            haveConfigData = 1;
//            [BTLE_data setLength: 0];
//        }
//            // Success packet.  Indicates that a register write happened successfully
//        else if ((hydra.ptByte == 0x00) && (hydra.addressByte == 0x00))
//        {
//            // Compare to the checksum provided in the packet and return if they don't match
//            //        if (![hydra isChecksumValid])
//            //        {
//            //            NSLog(@"iGateDidReceivedData 0x00 Checksum rejected");
//            //            [[NSNotificationCenter defaultCenter] postNotificationName:@"rejected" object:nil userInfo:nil];
//            //            [BTLE_data setLength: 0];
//            //            return;
//            //        }
//            
//            [[NSNotificationCenter defaultCenter] postNotificationName:@"success" object:nil userInfo:nil];
//            [BTLE_data setLength: 0];
//        }
//            // Firmware Response Packet
//        else if ((hydra.ptByte == 0x80) && (hydra.addressByte == 0xAA)) {
//            // Compare to the checksum provided in the packet and return if they don't match
//            if (![hydra isChecksumValid])
//            {
//                NSLog(@"iGateDidReceivedData 0x80 Checksum rejected");
//                [BTLE_data setLength: 0];
//                return;
//            }
//            
//            UInt8 *bytes = [hydra.packet mutableBytes];
//            
//            NSString *version = [NSString stringWithFormat:@"%c%c%c%c", bytes[5], bytes[6], bytes[7], bytes[8]];
//            //        NSLog(@"Firmware Version: %@", version);
//            
//            NSDictionary *userInfo = @{ @"version" : version };
//            [[NSNotificationCenter defaultCenter] postNotificationName:@"firmwareVersionReport" object:nil userInfo:userInfo];
//            [BTLE_data setLength: 0];
//        }
    }
    
//    func iGateDidUpdateConnectDevRSSI(_ rssi: NSNumber!, error: Error!) {
//        print("iGateDidUpdateConnectDevRSSI")
//    }
    
//    func iGateDidUpdateConnectDevAddr(_ addr: UnsafeMutablePointer<CBluetoothAddr>!) {
//        print("iGateDidUpdateConnectDevAddr")
//    }
}
