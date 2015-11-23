//
//  iGate.h
//  iGate
//
//  Created by Zhan on 12-6-18.
//  Copyright (c) 2012年 Novacomm. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "CiGateDelegate.h"


@interface CiGate : NSObject<CiGateDelegate>

/*!
 *  @property delegate
 *
 *  @discussion The delegate object you want to receive iGate events.
 *
 */
@property(assign, nonatomic) id<CiGateDelegate> delegate;

/*!
 *  @property state
 *
 *  @discussion The current state of the iGate.
 *      Initially set to CiGateInit.
 *		It can be updated at any moment, upon which the relevant delegate callback will be invoked.
 *      Handle the state changes in iGateDidUpdateState.
 *
 */
@property(readonly) CiGateState state;

/*!
 *  @method initWithDelegate:queue:
 *
 *  @param delegate	The delegate to receive the iGate events such as state changes, received data indications.
 *  @param aFlag set TRUE to automatically connect to the founded device, currently please
                 always set it TRUE.
 *  @discussion The initialization call of iGate.
 *
 */
- (CiGate *)initWithDelegate:(id<CiGateDelegate>)aDelegate autoConnectFlag:(BOOL)aFlag serviceUuidStr:(NSString*)uuidStr;

/*!
 *  @method initWithDelegate:queue:
 *
 *  @param delegate	The delegate to receive the iGate events such as state changes, received data indications.
 *  @param aFlag set TRUE to automatically connect to the founded device, currently please
 always set it TRUE.
 *  @param bondDevUUID The bonded device UUID. When iGate is bonded to a device,
 *         it only connects to the bonded device when several BTLE devices is nearby.
 *  @discussion The initialization call of iGate.
 *
 */
- (CiGate *)initWithDelegate:(id<CiGateDelegate>)aDelegate autoConnectFlag:(BOOL)aFlag BondDevUUID:(CFUUIDRef)bondDevUUID serviceUuidStr:(NSString*)uuidStr;

/*!
 *  @method startSearch:
 *
 *  @discussion Start search for device. If no bond UUID is set before startSearch,
 *              iGate will try to connect/bond to a new device.
 *
 */
- (void)startSearch;

/*!
 *  @method stopSearch:
 *
 *  @discussion Stop search for device.
 *
 */
- (void)stopSearch;

/*!
 *  @method getConnectDevName:
 *
 *  @discussion get the connected device name.
 *               
 *
 */
- (NSString *) getConnectDevName;

/*!
 *  @method setConnectDevName:
 *
 *  @discussion set the connected device name. The connected device's name is only writable
 *              after it is bonded
 *
 */

- (void) setConnectDevName:(NSString *)name;

/*!
 *  @method getConnectDevRSSI:
 *
 *  @discussion While connected, fetch the current RSSI of the link/connected device.
 *  @see iGateDidUpdateConnectDevRSSI:error: of CiGateDelegate protocol
 *
 */
- (void) getConnectDevRSSI;

/*!
 *  @method getConnectDevUUID:
 *
 *  @discussion get the connected device UUID. If the current iGate state is bonded,
 *              the connected device UUID is also the bonded UUID. The user of iGate
 *              may save the UUID and reload it by setBondDevUUID so the app only
 *              connect to known devices.
 */
- (CFUUIDRef) getConnectDevUUID;

/*!
 *  @method getConnectDevAddr:
 *
 *  @discussion get the connected device's Bluetooth Address.
 *  See iGateDidUpdateConnectDevAddr:(CBluetoothAddr *)addr; of CiGateDelegate protocol
 */
- (void) readConnectDevAddr;

/*!
 *  @method readConnectDevAioLevel:
 *
 *  @discussion get the connected device's AIO level.
 *  aioSelector: 0~2
 */
- (void) readConnectDevAioLevel:(integer_t)aioSelector;

/*!
 *  @method setBondDevUUID:
 *
 *  @param bondDevUUID	The bonded device UUID. When iGate is bonded to a device,
 *         it only connects to the bonded device when several BTLE devices is nearby.
 *  @discussion set the Bonded device UUID before start search so only known device
 *              can be connected.
 *              The iGate will copy UUID in setBondDevUUID, so the user can release
 *              the passed in CFUUIDRef after it returns.
 *
 */
- (BOOL) setBondDevUUID:(CFUUIDRef)bondDevUUID;

/*!
 *  @method send data to the connected device:
 *
 *  @discussion Limit the data length <=20.
 *
 */
- (void)sendData:(NSData *)data;

@end

