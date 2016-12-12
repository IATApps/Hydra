//
//  IATBluetoothOnTimeTriggerTests.swift
//  IATBluetoothTests
//
//  Created by Kurt Arnlund on 11/28/16.
//  Copyright © 2016 Ingenious Arts and Technologies LLC. All rights reserved.
//

import XCTest
@testable import IATFoundationUtilities

class OnTimeTriggerTests: XCTestCase {
    
    let triggerRequired : IATOneTimeTrigger = IATOneTimeTrigger(name: "Tests")
    var triggerOptional : IATOneTimeTrigger?
    let triggerTimed : IATOneTimeTrigger  = IATOneTimeTrigger(name: "TimedTests")
    var triggerCount = 0

    override func setUp() {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }
    
    override func tearDown() {
        triggerOptional = nil
        triggerCount = 0
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testOneTimeReseting() {
        triggerOptional = IATOneTimeTrigger(name: "TestOptional with resetting")
        triggerOptional?.doThis {
            self.triggerCount = 1
        }
        XCTAssert(triggerCount == 1)
        triggerOptional?.reset {
            self.triggerCount = 0
        }
        XCTAssert(triggerCount == 0)
    }
    
    func testOneTimeTriggering() {
        // This is an example of a functional test case.
        // Use XCTAssert and related functions to verify your tests produce the correct results.
        triggerRequired.doThis {
            self.triggerCount += 1
        }
        XCTAssert(triggerCount == 1)
        triggerRequired.doThis {
            self.triggerCount += 1
        }
        XCTAssert(triggerCount == 1)
    }
    
    func testPerformanceOne() {
        // This is an example of a performance test case.
        self.measure {
            self.triggerTimed.doThis(once: {
                self.triggerCount += 1
            })
        }
    }

    func testPerformanceTwo() {
        // This is an example of a performance test case.
        self.measure {
            self.triggerTimed.doThis(once: {
                self.triggerCount += 1
            })
        }
    }

}
