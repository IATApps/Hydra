//
//  HydraPacket.m
//  hydra
//
//  Created by Kurt Arnlund on 11/14/15.
//  Ingenious Arts and Technologies LLC
//

#import "HydraPacket.h"
#import "BitUtilities.h"


@implementation HydraChannel

- (instancetype)initWithCurrent:(int)current voltage:(int)voltage enabled:(BOOL)enabled lcMode:(BOOL)lcMode {
    self = [super init];
    if (self) {
        self.statusMode = NO;
        self.current = current;
        self.voltage = voltage;
        self.enabled = enabled;
        self.lowCurrentMode = lcMode;
        self.fault = 0x00;
        self.mode = (enabled ? BIT(8) : 0) | (lcMode ? BIT(7) : 0) >> 6;
        self.voltageControlled = NO;
        self.currentControlled = NO;
    }
    return self;
}

- (instancetype)initWithBytes:(UInt8 *)bytes {
    self = [super init];
    if (self) {
        self.statusMode = YES;
        self.current = ((bytes[0] & 0x0F) << 8) + bytes[1]; // 12 bits for current
        self.voltage = (bytes[2] << 8) + bytes[3];  // 16 bits for voltage
        self.enabled = NO;
        self.lowCurrentMode = NO;
        // fault and mode bit-pairs in upper 4 bits of current value
        self.fault = (bytes[0] >> 4) & 0x03;
        self.mode = (bytes[0] >> 6) & 0x03;
        
        self.voltageControlled = (self.mode & 0x02) > 0;
        self.currentControlled = (self.mode & 0x01) > 0;
    }
    return self;
}

- (NSString*)debugDescription {
    NSString *desc = [NSString stringWithFormat:@"channel entry volts:%d current:%d mode:%2X fault:%2X %@ %@", self.voltage, self.current, self.mode, self.fault, self.enabled ? @"Enabled" : @"", self.lowCurrentMode ? @"LC_MODE" : @""];
    return desc;
}

@end



@implementation HydraPacket

- (instancetype)init
{
    self = [super init];
    if (self) {
        [self reset];
    }
    return self;
}

- (instancetype)initWithPacket:(NSData*)packet {
    self = [self init];
    if (self) {
        self.packet = [packet mutableCopy];
    }
    return self;
}

- (void)reset {
    self.packet = [NSMutableData new];
    [self startPacket];
}

- (void)startPacket {
    if (self.packet.length >= 4) {
        return;
    }
    
    [self.packet appendBytes:"snp" length:3];
    
    UInt8 ptByte = [self ptByte:0x00 hasData:NO];
    ptByte = [self ptByte:ptByte isBatch:NO batchLen:0];
    [self.packet appendBytes:&ptByte  length:1];
}

- (void)addAddress:(UInt8)address {
    [self setAddressByte:address];
}

- (void)addChecksum {
    NSAssert(self.packet.length >= 5, @"Packet must contain address and PT before a checksum can be added");
    UInt8 *bytes = [self.packet mutableBytes];
    UInt16 checksum = 0;
    for (int i = 0; i < self.packet.length; i++){
        checksum += bytes[i];
    }
    
    UInt8 sumBytes[] = { ((checksum & 0xFF00) >> 8), (checksum & 0xFF) };
    
//    NSLog(@"calculated checksum %d - pre checksum packet length %d", checksum, self.packet.length);
    [self.packet appendBytes:sumBytes length:2];
//    NSLog(@"calculated checksum %d - post checksum packet length %d", checksum, self.packet.length);
}

- (BOOL)isChecksumValid {
    uint16_t checksum = 0;
    UInt8 *bytes = [self.packet mutableBytes];
    for(int i = 0; i < self.packet.length - 2; i++){
        uint16_t value = (uint16_t)bytes[i];
        checksum += value;
    }

    // Compare to the checksum provided in the packet and return if they don't match
    uint16_t checker = ((bytes[self.packet.length-2] << 8) | bytes[self.packet.length-1]);

//    NSLog(@"validating checksum %d against stored checksum %d - packet length %d", checksum, checker, self.packet.length);

    return (checker == checksum);
}

- (UInt8)ptByte:(UInt8)value hasData:(BOOL)hasData {
    if (hasData) {
        BIT_ON(value, 8);
    }
    return value;
}

- (UInt8)ptByte:(UInt8)value isBatch:(BOOL)isBatch batchLen:(UInt8)bl {
    if (isBatch) {
        BIT_ON(value, 7);
        value &= 0xC0; // clear out existing batch length
        bl &= 0x0F; // make sure batch len is 4 bits
        value |= (bl << 2);
    }
    return value;
}

- (UInt8)ptByteBatchLen:(UInt8)ptByte {
    UInt8 batchLen = (ptByte >> 2) & 0x0F;
    return batchLen;
}

- (UInt8)ptByte {
    if (self.packet.length < 4) {
        return 0x00;
    }
    UInt8 *bytes = [self.packet mutableBytes];
    return bytes[3];
}

- (void)setPtByte:(UInt8)ptByte {
    UInt8 *bytes = [self.packet mutableBytes];
    if (self.packet.length >= 4) {
        bytes[3] = ptByte;
    }
}

- (UInt8)addressByte {
    if (self.packet.length < 5) {
        return 0x00;
    }
    UInt8 *bytes = [self.packet mutableBytes];
    return bytes[4];
}

- (void)setAddressByte:(UInt8)address {
    NSAssert(self.packet.length >= 4, @"setAddressByte can only be called when packet length is 4 or greater");

    if (self.packet.length == 4) {
        UInt8 addressByte = address;
        [self.packet appendBytes:&addressByte length:1];
    } else {
        UInt8 *bytes = [self.packet mutableBytes];
        bytes[4] = address;
    }
}

- (void)addBatchForCurrent:(int)current voltage:(int)voltage enabled:(BOOL)enabled lcMode:(BOOL)lcMode {
    UInt8 bytes[] = {LSB_NIBBLE(HIGH_BYTE(current)), LOW_BYTE(current), HIGH_BYTE(voltage), LOW_BYTE(voltage)};
    
    if (enabled) {
        BIT_ON(bytes[0], 8);
    }
    if (lcMode) {
        BIT_ON(bytes[0], 7);
    }

    UInt8 ptByte = [self ptByte];
    ptByte = [self ptByte:ptByte hasData:YES];
    UInt8 batchLen = [self ptByteBatchLen:ptByte];
    batchLen++;
    ptByte = [self ptByte:ptByte isBatch:YES batchLen:batchLen];
    [self setPtByte:ptByte];
    
    NSData *batch = [[NSMutableData alloc] initWithBytes:bytes length:sizeof(bytes)];
    
    [self.packet appendData:batch];
}

- (HydraChannel*)batchEntryAtIndex:(UInt8)index {
    UInt8 len = [self ptByteBatchLen:[self ptByte]];
    if (index > len) {
        return nil;
    }
    
    index *= 4;
    
    index += 5;
    
    UInt8 *bytes = [self.packet mutableBytes];
    
    HydraChannel *entry = [[HydraChannel alloc] initWithBytes:&bytes[index]];
    return entry;
}

- (BOOL)isCommandFailure {
    UInt8 ptByte = [self ptByte];
    return [self ptByteIsCommandFailure:ptByte];
}

- (BOOL)ptByteIsCommandFailure:(UInt8)value {
    return ((value & 0x01) > 0);
}

- (NSString*)debugDescription {
    NSString *desc = [NSString stringWithFormat:@"Packet\n%@", self.packet];
    return desc;
}

//- (void)hydra {
//    packet[0] = 's';  // 0x73 's'
//    packet[1] = 'n';  // 0x6E 'n'
//    packet[2] = 'p';  // 0x70 'p'
//    packet[3] = 0xCC;  // 0xCC 1100 1100  HAS_DATA  IS_BATCH  BATCH_LEN_3
//    packet[4] = 0;
//    
//    packet[5] = HIGH_NIBBLE(ch1current);
//    packet[6] =  LOW_NIBBLE(ch1current);
//    packet[7] = (ch1voltage >> 8) & 0xFF;
//    packet[8] = ch1voltage & 0xFF;
//    if (self.ch1view.enabled) packet[5] = packet[5] | 0x80;
//    if (self.appDelegate.ch1LC) packet[5] = packet[5] | 0x40;
//    
//    packet[9] = (ch2current >> 8) & 0x0F;
//    packet[10] = ch2current & 0xFF;
//    packet[11] = (ch2voltage >> 8) & 0xFF;
//    packet[12] = ch2voltage & 0xFF;
//    if (self.ch2view.enabled) packet[9] = packet[9] | 0x80;
//    if (self.appDelegate.ch2LC) packet[9] = packet[9] | 0x40;
//    
//    packet[13] = (ch3current >> 8) & 0x0F;
//    packet[14] = ch3current & 0xFF;
//    packet[15] = (ch3voltage >> 8) & 0xFF;
//    packet[16] = ch3voltage & 0xFF;
//    if (self.ch3view.enabled) packet[13] = packet[13] | 0x80;
//    if (self.appDelegate.ch3LC) packet[13] = packet[13] | 0x40;
//    
//    //checksum
//    uint16_t checksum = 0;
//    for (int i = 0; i< 17; i++){
//        checksum += packet[i];
//    }
//    packet[17] = (checksum >> 8) & 0xFF;
//    packet[18] = checksum & 0xFF;
//}


@end
