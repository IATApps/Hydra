//
//  IATBTSerivceObjCTest.m
//  IATBluetooth
//
//  Created by Kurt Arnlund on 12/9/16.
//  Copyright © 2016 Ingenious Arts and Technologies LLC. All rights reserved.
//

@import IATFoundationUtilities;
@import IATBluetooth;
#import <XCTest/XCTest.h>
//#import <IATFoundationUtilities/IATFoundationUtilities-Swift.h>
//#import <IATBluetooth/IATBluetooth-Swift.h>

@interface IATBTSerivceObjCTest : XCTestCase <BTDiscoveryEventDelegate>
@property (nonatomic, weak) IATBTService *service;
@property (nonatomic, weak) IATBTDeviceInformationService *device_information_service;
@property (nonatomic, weak) IATBTServiceGroup *service_group;
@property (nonatomic, strong) IATBTDiscovery *discovery;
@property (nonatomic, weak) IATBTPeripheral *peripheral;
@property (nonatomic, assign) int stateEventCounter;
@property (nonatomic, strong) XCTestExpectation *scanning_expectation;
@end

@implementation IATBTSerivceObjCTest

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
    self.stateEventCounter = 0;
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testObjectInterfaces {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    IATBTService *randomService =
//        [[IATBTService alloc] initWithServiceUUID: [[NSUUID new] UUIDString]  required:false characteristicDescriptions:nil];
    [[IATBTService alloc] initWithServiceUUID:@ "B5161D82-AAB0-4E55-8D96-C59D816E6971" required:true characteristicDescriptions:@{} ];
    
    IATBTDeviceInformationService *di_service = [IATBTDeviceInformationService new];
    
    self.service = randomService;
    self.device_information_service = di_service;
    
    self.discovery = [[IATBTDiscovery alloc] initWithName:@"objc-tests" withServices:@[di_service, randomService]];
    self.discovery.delegate = self
    
    self.peripheral = self.discovery.peripherals.firstObject;
    self.service_group = self.peripheral.serviceGroup;

    XCTAssertNotNil(self.service);
    XCTAssertNotNil(self.device_information_service);
    XCTAssertNotNil(self.discovery);
    XCTAssertNotNil(self.peripheral);
    XCTAssertNotNil(self.service_group);
    
    self.scanning_expectation = [self expectationWithDescription:@"scanning state expectations"];
    [self.discovery startScanning];
    
    __block BOOL stop = false;
    [self waitForExpectationsWithTimeout:15 handler:^(NSError * _Nullable error) {
        stop = true;
    }];
    
    while (!stop)
    {
        NSDate *loopUntil = [NSDate dateWithTimeIntervalSinceNow:15.0f];
        [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:loopUntil];
    }
}

- (void)bluetoothEventWithState:(enum BTDiscoveryEvent)state {
    self.stateEventCounter++;
    [self.scanning_expectation fulfill];
}

@end
