//
//  HydraPacket.h
//  hydra
//
//  Created by Kurt Arnlund on 11/14/15.
//
//
// Usage:
//    init;
#import <Foundation/Foundation.h>

@interface HydraChannel : NSObject
@property (nonatomic, assign) BOOL statusMode;
@property (nonatomic, assign) int current;
@property (nonatomic, assign) int voltage;
@property (nonatomic, assign) UInt8 fault;
@property (nonatomic, assign) UInt8 mode;
@property (nonatomic, assign) BOOL voltageControlled;
@property (nonatomic, assign) BOOL currentControlled;
@property (nonatomic, assign) BOOL enabled;
@property (nonatomic, assign) BOOL lowCurrentMode;

- (instancetype)initWithCurrent:(int)current voltage:(int)voltage enabled:(BOOL)enabled lcMode:(BOOL)lcMode;
- (instancetype)initWithBytes:(UInt8 *)bytes;
@end

@interface HydraPacket : NSObject

@property (nonatomic, strong) NSMutableData *packet;

- (instancetype)init;
- (instancetype)initWithPacket:(NSData*)packet;

- (void)reset;
- (void)addAddress:(UInt8)address;
- (void)addBatchForCurrent:(int)current voltage:(int)voltage enabled:(BOOL)enabled lcMode:(BOOL)lcMode;
- (void)addChecksum;

- (UInt8)ptByte;
- (UInt8)addressByte;
- (HydraChannel*)batchEntryAtIndex:(UInt8)index;
- (BOOL)isChecksumValid;

- (BOOL)isCommandFailure;

@end
