//
//  AppDelegate.m
//  Hydra
//
//  Created by User on 3/12/13.
//  Copyright (c) 2013 CHRobotics. All rights reserved.
//

#import "AppDelegate.h"
#import "Utilities.h"
#import "HydraPacket.h"

@implementation AppDelegate
{
    // Member data for storing received characters over the Bluetooth connection
    NSMutableData* BTLE_data;
    int BTLE_length;
    int haveConfigData;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    
    // 00000000-0000-0000-0C86-003D067A170D  -> UUID for a sample of NVC_MDCS71
    // If the app want to connect to a known device only, set the bonded UUID when
    // init iGate, or use setBondDevUUID.
    // In the test program, the bond UUID is set according to the message printed
    // in last connection.
    // In an actual app, the bond UUID and the device name may be stored and reloaded
    // when the app restarts.
    self.logData = NO;
    self.ch1MaxCurrent = 0;
    self.ch1TargetVoltage = 0;
    self.ch2MaxCurrent = 0;
    self.ch2TargetVoltage = 0;
    self.ch3MaxCurrent = 0;
    self.ch3TargetVoltage = 0;
    self.inputVoltage = 0;
    self.inputCurrent = 0;
    
    // Create array for storing data
    BTLE_data = [[NSMutableData alloc] initWithCapacity: 50];
    BTLE_length = 0;
    haveConfigData = 0;
    
    [self pair];
    
    return YES;
}
							
- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resBources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
    [[NSNotificationCenter defaultCenter]
     postNotificationName:@"DidBecomeActive"
     object:self];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
    self.serviceUUID = nil;
    self.iGate = nil;
}

#pragma mark - 

- (void)pair {
    self.serviceUUID = [[NSUUID alloc] initWithUUIDString:@"B5161D82-AAB0-4E55-8D96-C59D816E6971"];
    
    [self.iGate setDelegate:nil];
    
    NSString *bonded = [[NSUserDefaults standardUserDefaults] objectForKey:@"bonded"];
    if (bonded || self.bondedUUID) {
        if (!self.bondedUUID) {
            self.bondedUUID = [[NSUUID alloc] initWithUUIDString:bonded];
        }
        self.iGate = [[CiGate alloc] initWithDelegate:self autoConnectFlag:YES BondDevUUID:(__bridge CFUUIDRef)(self.bondedUUID) serviceUuidStr:self.serviceUUID.UUIDString];
    } else {
        self.iGate = [[CiGate alloc] initWithDelegate:self autoConnectFlag:YES serviceUuidStr:self.serviceUUID.UUIDString];
    }
    
    [self.iGate startSearch];
}

#pragma mark - CiGateDelegate methods
/*
 Invoked whenever the central manager's state is updated.
 */
- (void)iGateDidUpdateState:(CiGateState)iGateState
{
    NSDictionary *dict = [[NSDictionary alloc] initWithObjectsAndKeys:[NSNumber numberWithInt: iGateState],@"state", nil];
    
    switch (iGateState) {
        case CiGateStateInit:
            NSLog(@"iGate Init");
            break;
        case CiGateStatePoweredOff:
            NSLog(@"iGate Powered Off");
            break;
        case CiGateStateUnknown:
            NSLog(@"iGate Unknown");
            break;
        case CiGateStateResetting:
            NSLog(@"iGate Resetting");
            break;
        case CiGateStateUnsupported:
            NSLog(@"iGate Unsupported");
            break;
        case CiGateStateUnauthorized:
            NSLog(@"iGate Unauthorized");
            break;
        case CiGateStateIdle:
            NSLog(@"iGate Idle");
            break;
        case CiGateStateSearching:
            NSLog(@"iGate Searching");
            break;
        case CiGateStateConnecting:
            NSLog(@"iGate Connecting");
            break;
        case CiGateStateConnected:
            NSLog(@"iGate Connected");
            CFUUIDRef uuid = [self.iGate getConnectDevUUID];
            self.bondedUUID = (__bridge NSUUID*)uuid;
            [self.iGate stopSearch];
            [self pair];
            break;
        case CiGateStateBonded:
            NSLog(@"iGate Bonded");
            NSLog(@"BONDED! %@ UUID:  %@", [self.iGate getConnectDevName], [self.iGate getConnectDevUUID]);
            [[NSUserDefaults standardUserDefaults] setObject:self.bondedUUID.UUIDString forKey:@"bonded"];
            [self.iGate readConnectDevAddr];
            break;
            
        default:
            break;
    }
    
    [[NSNotificationCenter defaultCenter]
     postNotificationName:@"stateUpdate"
     object:nil userInfo:dict];
}

- (void)iGateDidReceivedData:(NSData *)nsdata
{
    int index = 0;
    uint8_t* packet;
    uint8_t* data;
    
    // Take the data and append it to the stored data.
    // Make sure we don't overflow... if the length would be too large,
    // just discard what we have and read the new data.
    // This should never happen if the BTLE device is behaving properly,
    // but we check for it anyway.
    if( (nsdata.length + BTLE_data.length) < 50 )
    {
        [BTLE_data appendData: nsdata];
    }
    else
    {
        [BTLE_data setData: nsdata];
    }
    
//    NSLog(@"BTLE_data %@", BTLE_data);
    
    // Check to make sure that we have enough data for a packet to even possibly exist
    if( BTLE_data.length < 3 )
    {
        return;
    }
    
    // Extract the NSdata as an array of bytes
    packet = (uint8_t*)[BTLE_data bytes];
    
    // Search for beginning of packet
    while( packet[0] != 's' || packet[1] != 'n' || packet[2] != 'p' )
    {
        packet = &packet[1];
        index++;
        
        // Make sure that we don't go beyond the edge of the array
        if( (index+2) >= BTLE_data.length )
        {
            return;
        }
    }
    
    // If we get here, the 'snp' sequence was found.  If index != 0, erase all preceding bytes - these are junk.  We can't do anything
    // with them without the beginning of the packet, which was apparently truncated.
    if( index > 0 )
    {
//        NSLog(@"Resetting data scratch log");
        
        BTLE_data = [BTLE_data initWithBytes:(const void*)packet length: (BTLE_data.length - index)];
        
        packet = (uint8_t*)[BTLE_data bytes];
    }
    
    HydraPacket *hydra = [[HydraPacket alloc] initWithPacket:BTLE_data];
    
    // Now check for expected packet types
    // Address 85 is a the first data register address.  Packet type 0xd0 indicates a batch packet containing four registers.
    if ((hydra.ptByte == 0xD0) && (hydra.addressByte == 0x55))
    {
        // First make sure we have enough data to fill a full packet (4*4 + 7 == 23)
        // If not, return.  Hopefully we'll receive the rest of the data in the next packet.
        if( BTLE_data.length < 23 )
        {
            return;
        }
        
        // Compare to the checksum provided in the packet and return if they don't match
        if (![hydra isChecksumValid])
        {
            NSLog(@"iGateDidReceivedData 0xD0 Checksum rejected");
            NSLog(@"%@", hydra);
            [BTLE_data setLength: 0];
            return;
        }
        
        HydraChannel *ch1 = [hydra batchEntryAtIndex:0];
        HydraChannel *ch2 = [hydra batchEntryAtIndex:1];
        HydraChannel *ch3 = [hydra batchEntryAtIndex:2];
        HydraChannel *ch4 = [hydra batchEntryAtIndex:3];
        
//        NSLog(@"Recevied %@", BTLE_data);
        
        // Checksum matches!  Extract data.
        // Put data in a dictionary so that we can pass it out easily
        NSDictionary *dict = [[NSDictionary alloc] initWithObjectsAndKeys:
                              @(ch1.voltage), CH_1_VOLT,
                              @(ch1.current), CH_1_CURR,
                              @(ch1.fault), CH_1_FAULT,
                              @(ch1.mode), CH_1_MODE,
                              @(ch2.voltage), CH_2_VOLT,
                              @(ch2.current), CH_2_CURR,
                              @(ch2.fault), CH_2_FAULT,
                              @(ch2.mode), CH_2_MODE,
                              @(ch3.voltage), CH_3_VOLT,
                              @(ch3.current), CH_3_CURR,
                              @(ch3.fault), CH_3_FAULT,
                              @(ch3.mode), CH_3_MODE,
                              @(ch4.voltage), IN_VOLT,
                              @(ch4.current), IN_CURR,
                              nil];
        
        self.ch1Voltage = ch1.voltage;
        if (self.ch1TargetVoltage == 0) {
            self.ch1TargetVoltage = self.ch1Voltage;
        }
        self.ch2Voltage = ch2.voltage;
        if (self.ch2TargetVoltage == 0) {
            self.ch2TargetVoltage = self.ch2Voltage;
        }
        self.ch3Voltage = ch3.voltage;
        if (self.ch3TargetVoltage == 0) {
            self.ch3TargetVoltage = self.ch3Voltage;
        }
        
        self.ch1Current = ch1.current;
        self.ch2Current = ch2.current;
        self.ch3Current = ch3.current;
        
        self.inputVoltage = ch4.voltage;
        self.inputCurrent = ch4.current;
        
        [[NSNotificationCenter defaultCenter] postNotificationName:@"recievedData" object:nil userInfo:dict];
        [self writeToLogFile:dict];
        
        // Clear the persistent data storage
        [BTLE_data setLength:0];
    }
    // Configuration data packet
    else if ((hydra.ptByte == 0xCC) && (hydra.addressByte == 0x00))
    { //recieved config packet
        // Compare to the checksum provided in the packet and return if they don't match
        if (![hydra isChecksumValid])
        {
            NSLog(@"iGateDidReceivedData 0xCC Checksum rejected");
            [BTLE_data setLength: 0];
            return;
        }
        
        data = &packet[5];
        uint16_t ch1TargetVoltage = (data[2] << 8)|data[3];
        uint16_t ch1MaxCurrent = ((data[0] & 0x0F) << 8)|data[1];
        uint8_t ch1OE = (data[0]>>7)&0x01;
        uint8_t ch1LC = (data[0]>>6)&0x01;
        
        uint16_t ch2TargetVoltage = (data[6] << 8)|data[7];
        uint16_t ch2MaxCurrent = ((data[4] & 0x0F) << 8)|data[5];
        uint8_t ch2OE = (data[4]>>7)&0x01;
        uint8_t ch2LC = (data[4]>>6)&0x01;
        
        uint16_t ch3TargetVoltage = (data[10] << 8)|data[11];
        uint16_t ch3MaxCurrent = ((data[0] & 0x0F) << 8)|data[9];
        uint8_t ch3OE = (data[8]>>7)&0x01;
        uint8_t ch3LC = (data[8]>>6)&0x01;
        
        self.ch1TargetVoltage = [NSNumber numberWithUnsignedInt:ch1TargetVoltage].intValue;
        self.ch2TargetVoltage = [NSNumber numberWithUnsignedInt:ch2TargetVoltage].intValue;
        self.ch3TargetVoltage = [NSNumber numberWithUnsignedInt:ch3TargetVoltage].intValue;

        self.ch1MaxCurrent = [NSNumber numberWithUnsignedInt:ch1MaxCurrent].intValue;
        self.ch2MaxCurrent = [NSNumber numberWithUnsignedInt:ch2MaxCurrent].intValue;
        self.ch3MaxCurrent = [NSNumber numberWithUnsignedInt:ch3MaxCurrent].intValue;
        
        self.ch1OE = [NSNumber numberWithUnsignedInt:ch1OE].boolValue;
        self.ch2OE = [NSNumber numberWithUnsignedInt:ch2OE].boolValue;
        self.ch3OE = [NSNumber numberWithUnsignedInt:ch3OE].boolValue;

        self.ch1LC = [NSNumber numberWithUnsignedInt:ch1LC].boolValue;
        self.ch2LC = [NSNumber numberWithUnsignedInt:ch2LC].boolValue;
        self.ch3LC = [NSNumber numberWithUnsignedInt:ch3LC].boolValue;
        
        [[NSNotificationCenter defaultCenter] postNotificationName:@"recievedConfig" object:nil userInfo:nil];
        
        // Set flag indicating that we've received configuration data for the Hydra
        haveConfigData = 1;
        [BTLE_data setLength: 0];
    }
    // Success packet.  Indicates that a register write happened successfully
    else if ((hydra.ptByte == 0x00) && (hydra.addressByte == 0x00))
    {
        // Compare to the checksum provided in the packet and return if they don't match
//        if (![hydra isChecksumValid])
//        {
//            NSLog(@"iGateDidReceivedData 0x00 Checksum rejected");
//            [[NSNotificationCenter defaultCenter] postNotificationName:@"rejected" object:nil userInfo:nil];
//            [BTLE_data setLength: 0];
//            return;
//        }
        
        [[NSNotificationCenter defaultCenter] postNotificationName:@"success" object:nil userInfo:nil];
        [BTLE_data setLength: 0];
    }
    // Firmware Response Packet
    else if ((hydra.ptByte == 0x80) && (hydra.addressByte == 0xAA)) {
        // Compare to the checksum provided in the packet and return if they don't match
        if (![hydra isChecksumValid])
        {
            NSLog(@"iGateDidReceivedData 0x80 Checksum rejected");
            [BTLE_data setLength: 0];
            return;
        }
        
        UInt8 *bytes = [hydra.packet mutableBytes];
        
        NSString *version = [NSString stringWithFormat:@"%c%c%c%c", bytes[5], bytes[6], bytes[7], bytes[8]];
//        NSLog(@"Firmware Version: %@", version);
        
        NSDictionary *userInfo = @{ @"version" : version };
        [[NSNotificationCenter defaultCenter] postNotificationName:@"firmwareVersionReport" object:nil userInfo:userInfo];
        [BTLE_data setLength: 0];
    }
}

- (void)iGateDidUpdateConnectDevRSSI:(NSNumber *)rssi error:(NSError *)error
{
    if(error==nil)
    {
        NSLog(@"rssi updated %@",rssi);
       
    }
}

-(void)iGateDidUpdateConnectDevAddr:(CBluetoothAddr *)addr
{
  NSLog(@"iGateDidUpdateConnectDevAddr: connected device address is %04x%02x%06x",addr->nap,addr->uap,addr->lap);
//  self.recDataStr=[self.recDataStr stringByAppendingFormat:@"type:%02d,addr:%04x%02x%06x",addr->type,addr->nap,addr->uap,addr->lap];
//  self.recData.text=self.recDataStr;
}

- (void)sendCommand:(NSData*)valData {
    CiGateState state = self.iGate.state;
    if (state == CiGateStateBonded)
    {
        NSLog(@"sending command..");
        [self.iGate sendData:valData];
        [self.iGate getConnectDevRSSI];
    }
    else {
        NSDictionary *dict = [[NSDictionary alloc] initWithObjectsAndKeys:[NSNumber numberWithInt: state],@"state", nil];
        [[NSNotificationCenter defaultCenter]
         postNotificationName:@"stateUpdate"
         object:nil userInfo:dict];
        NSLog(@"Send fail, not bonded yet");
    }
}

- (void)sendData:(NSString*)dataString {
    CiGateState state = self.iGate.state;
    if (state == CiGateStateBonded)
    {
//        NSLog(@"sending data..");
        NSMutableData *valData;
        if (dataString.length > IGATE_MAX_SEND_DATE_LEN)
        {
            valData = [[NSData dataWithBytes:(void*)[dataString UTF8String] length:IGATE_MAX_SEND_DATE_LEN] mutableCopy];
        }
        else
        {
            valData = [[NSData dataWithBytes:(void*)[dataString UTF8String] length:dataString.length] mutableCopy];
        }
        
        [self.iGate sendData:valData];
        [self.iGate getConnectDevRSSI];
    }
    else {
        NSDictionary *dict = [[NSDictionary alloc] initWithObjectsAndKeys:[NSNumber numberWithInt: state],@"state", nil];
        [[NSNotificationCenter defaultCenter]
         postNotificationName:@"stateUpdate"
         object:nil userInfo:dict];
        NSLog(@"Send fail, not bonded yet");
    }
}

-(void) writeToLogFile:(NSDictionary*)dict{
    if (!self.logData) return;
    //channel 1
    NSString *content = [NSString stringWithFormat:@"%.02f",((NSNumber*)[dict objectForKey:CH_1_VOLT]).floatValue/MILLI_SCALER];
    
    content = [content stringByAppendingString:[NSString stringWithFormat:@", %.02f",((NSNumber*)[dict objectForKey:CH_1_CURR]).floatValue/MILLI_SCALER]];
    
    if(((NSNumber*)[dict objectForKey:CH_1_FAULT]).intValue == 0){
        switch (((NSNumber*)[dict objectForKey:CH_1_MODE]).intValue) {
            case 0:
                content = [content stringByAppendingString:@", OFF"]; break;
            case 1:
                content = [content stringByAppendingString:@", CC"]; break;
            case 2:
                content = [content stringByAppendingString:@", CV"]; break;
            default: break;
        }
    }
    else {
        [content = content stringByAppendingFormat:@", ERR%i", ((NSNumber*)[dict objectForKey:CH_1_FAULT]).intValue];
    }
    //channel 2
    content = [content stringByAppendingString:[NSString stringWithFormat:@", %.02f",((NSNumber*)[dict objectForKey:CH_2_VOLT]).floatValue/MILLI_SCALER]];
    
    content = [content stringByAppendingString:[NSString stringWithFormat:@", %.02f",((NSNumber*)[dict objectForKey:CH_2_CURR]).floatValue/MILLI_SCALER]];
    
    if(((NSNumber*)[dict objectForKey:CH_2_FAULT]).intValue == 0){
        switch (((NSNumber*)[dict objectForKey:CH_2_MODE]).intValue) {
            case 0:
                content = [content stringByAppendingString:@", OFF"]; break;
            case 1:
                content = [content stringByAppendingString:@", CC"]; break;
            case 2:
                content = [content stringByAppendingString:@", CV"]; break;
            default: break;
        }
    }
    else {
        [content = content stringByAppendingFormat:@", ERR%i", ((NSNumber*)[dict objectForKey:CH_2_FAULT]).intValue];
    }
    
    //channel 3
    content = [content stringByAppendingString:[NSString stringWithFormat:@", %.02f",((NSNumber*)[dict objectForKey:CH_3_VOLT]).floatValue/MILLI_SCALER]];
    
    content = [content stringByAppendingString:[NSString stringWithFormat:@", %.02f",((NSNumber*)[dict objectForKey:CH_3_CURR]).floatValue/MILLI_SCALER]];
    
    if(((NSNumber*)[dict objectForKey:CH_3_FAULT]).intValue == 0){
        switch (((NSNumber*)[dict objectForKey:CH_3_MODE]).intValue) {
            case 0:
                content = [content stringByAppendingString:@", OFF"]; break;
            case 1:
                content = [content stringByAppendingString:@", CC"]; break;
            case 2:
                content = [content stringByAppendingString:@", CV"]; break;
            default: break;
        }
    }
    else {
        [content = content stringByAppendingFormat:@", ERR%i", ((NSNumber*)[dict objectForKey:CH_3_FAULT]).intValue];
    }
    //get the documents directory:
    NSString *documentsDirectory = DOCUMENTS_FOLDER;
    content = [NSString stringWithFormat:@"%@\n",content];
    //make a file name to write the data to using the documents directory:
    NSString *fileName = [NSString stringWithFormat:@"%@/hydraLog.csv",
                          documentsDirectory];
    NSFileHandle *fileHandle = [NSFileHandle fileHandleForWritingAtPath:fileName];
    if (fileHandle){
        [fileHandle seekToEndOfFile];
        [fileHandle writeData:[content dataUsingEncoding:NSUTF8StringEncoding]];
        [fileHandle closeFile];
    }
    else{
        [content writeToFile:fileName
                  atomically:NO
                    encoding:NSStringEncodingConversionAllowLossy
                       error:nil];
    }
    
}

@end
