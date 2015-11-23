//
//  AppDelegate.h
//  Hydra
//
//  Created by User on 3/12/13.
//  Copyright (c) 2013 CHRobotics. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "iGate.h"
#define DOCUMENTS_FOLDER [NSHomeDirectory() stringByAppendingPathComponent:@"Documents"]

#define MILLI_SCALER 1000

#define CH_1_VOLT @"ch1voltage"
#define CH_1_CURR @"ch1current"
#define CH_1_MODE @"ch1mode"
#define CH_1_FAULT @"ch1fault"

#define CH_2_VOLT @"ch2voltage"
#define CH_2_CURR @"ch2current"
#define CH_2_MODE @"ch2mode"
#define CH_2_FAULT @"ch2fault"

#define CH_3_VOLT @"ch3voltage"
#define CH_3_CURR @"ch3current"
#define CH_3_MODE @"ch3mode"
#define CH_3_FAULT @"ch3fault"

#define IN_VOLT @"inVoltage"
#define IN_CURR @"inCurrent"


@interface AppDelegate : UIResponder <UIApplicationDelegate,CiGateDelegate>
{
    CFUUIDRef tmpUUID;
}
@property (strong, nonatomic) UIWindow *window;
@property (nonatomic, strong) CiGate *iGate;
@property (nonatomic, strong) NSUUID *serviceUUID;
@property (nonatomic, strong) NSUUID *bondedUUID;
@property bool logData;
//@property (strong, nonatomic) NSString *recDataStr;
@property int ch1Voltage;
@property int ch1TargetVoltage;
@property int ch1MaxCurrent;
@property int ch1Current;
@property bool ch1OE;
@property bool ch1LC;

@property int ch2Voltage;
@property int ch2TargetVoltage;
@property int ch2MaxCurrent;
@property int ch2Current;
@property bool ch2OE;
@property bool ch2LC;

@property int ch3Voltage;
@property int ch3TargetVoltage;
@property int ch3MaxCurrent;
@property int ch3Current;
@property bool ch3OE;
@property bool ch3LC;

@property int inputVoltage;
@property int inputCurrent;

@property float lowVoltageCutoff;

- (void)iGateDidUpdateState:(CiGateState)iGateState;
- (void)iGateDidReceivedData:(NSData *)data;
- (void)iGateDidUpdateConnectDevRSSI:(NSNumber *)rssi error:(NSError *)error;
- (void)sendData:(NSString*)dataString;
- (void)sendCommand:(NSData*)valData;
@end
