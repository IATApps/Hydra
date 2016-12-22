//
//  CiGateDelegate.h
//  iGate
//
//  Created by zhan on 12-6-18.
//  Copyright (c) 2012年 Novacomm. All rights reserved.
//

#import <Foundation/Foundation.h>

/*!
 *  @enum CiGateState
 *
 *  @discussion Represents the current state of a iGate.
 *
 */
enum {
	CiGateStateInit = 0,	// State unknown, update imminent.
    CiGateStatePoweredOff,  // Bluetooth is currently powered off.
    CiGateStateUnknown,     // State unknown, update imminent.
    CiGateStateResetting,   // The connection with the system service was momentarily lost, update imminent.
	CiGateStateUnsupported,	// Something wrong, iOS device not support BTLE or not power on.
    CiGateStateUnauthorized,// The app is not authorized to use Bluetooth Low Energy.
    CiGateStateIdle,        // Bluetooth is currently powered on and available to use.
	CiGateStateSearching,	// The iGate is searching to find a peripheral.
	CiGateStateConnecting,	// the iGate is connecting to a peripheral.
	CiGateStateConnected,	// The igate is connected with a peripheral.
    CiGateStateBonded,	    // The igate is bondeded (and the connection is encypted) with a peripheral.
};
typedef NSInteger CiGateState;

typedef struct {
    UInt8 type;
    UInt16 nap;
    unsigned uap:8;
    unsigned lap:24;
} CBluetoothAddr;

/*!
 *  @macro IGATE_MAX_SEND_DATE_LEN
 *
 *  @discussion There's a limitation in packet size in low level BTLE data packet,
 *              so for each time 
 *
 */
#define IGATE_MAX_SEND_DATE_LEN 20

/*!
 *  @protocol CiGateDelegate
 *
 *  @discussion Delegate protocol for CBiGate.
 *
 */
@protocol CiGateDelegate <NSObject>

@required

- (void)iGateDidUpdateState:(CiGateState)iGateState;
- (void)iGateDidReceivedData:(NSData *)data;

@optional
/*!
 *  @method iGateDidUpdateConnectDevRSSI:error:
 *
 *  @discussion Invoked upon completion of a -[getConnectDevRSSI:] request.
 *      If successful, "error" is nil and the "rssi" indicates the new RSSI value.
 *      If unsuccessful, "error" is set with the encountered failure.
 *
 */
- (void)iGateDidUpdateConnectDevRSSI:(NSNumber *)rssi error:(NSError *)error;

/*!
 *  @method iGateDidUpdateConnectDevAddr
 *
 *  @discussion Invoked when the Bluetooth address of the connect device is got.
 *
 */
- (void)iGateDidUpdateConnectDevAddr:(CBluetoothAddr *)addr;

@end

