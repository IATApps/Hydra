//
//  CBPeripheral+iGateHelpers.m
//  hydra
//
//  Created by Kurt Arnlund on 11/12/15.
//
//

#import "CBPeripheral+iGateHelpers.h"

@implementation CBPeripheral (iGateHelpers)

- (BOOL)isConnected {
    return self.state == CBPeripheralStateConnected;
}

- (NSUUID*)UUID {
    return self.identifier;
}

//- (void)discoverServices:(nullable NSArray<CBUUID *> *)serviceUUIDs {
//    NSLog(@"discoverServices: %@", serviceUUIDs);
//}

//- (void)discoverIncludedServices:(nullable NSArray<CBUUID *> *)includedServiceUUIDs forService:(CBService *)service {
//    NSLog(@"discoverIncludedServices: %@ for %@", includedServiceUUIDs, service);
//}

//- (void)discoverCharacteristics:(nullable NSArray<CBUUID *> *)characteristicUUIDs forService:(CBService *)service {
//    NSLog(@"discoverCharacteristics: %@ for %@", characteristicUUIDs, service);
//}

//- (void)readValueForCharacteristic:(CBCharacteristic *)characteristic {
//    NSLog(@"readValueForCharacteristic: %@", characteristic);
//}

//- (NSUInteger)maximumWriteValueLengthForType:(CBCharacteristicWriteType)type {
//    NSLog(@"maximumWriteValueLengthForType: %d", type);
//}

//- (void)writeValue:(NSData *)data forCharacteristic:(CBCharacteristic *)characteristic type:(CBCharacteristicWriteType)type {
//    
//}

@end
