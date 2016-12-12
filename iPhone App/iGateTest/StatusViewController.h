//
//  ViewController.h
//  Hydra
//
//  Created by User on 3/12/13.
//  Copyright (c) 2013 CHRobotics. All rights reserved.
//

#import <UIKit/UIKit.h>
//#import "iGate.h"
#define DEGREES_TO_RADIANS(angle) ((angle) / 180.0 * M_PI)
#define RADIANS_TO_DEGREES(radians) ((radians) * 180 / M_PI)
#define MAX_VOLTAGE 14
#define MIN_VOLTAGE 2.5

#define MAX_CURRENT 2.5
#define MIN_CURRENT 0

#define CH1_VIEW_FRAME CGRectMake(0, 20, 320, 85)
#define CH2_VIEW_FRAME CGRectMake(0, 106, 320, 85)
#define CH3_VIEW_FRAME CGRectMake(0, 192, 320, 85)

#define CENTER_STAGE CGPointMake(160, 50)


@interface StatusViewController : UIViewController 
{
    CiGate *iGate;
    CiGateState _state;
    //UITapGestureRecognizer *singleTapRec;
    UIActivityIndicatorView *proInd;
}

@end
