//
//  Channel.m
//  Hydra
//
//  Created by User on 4/9/13.
//  Copyright (c) 2013 CHRobotics. All rights reserved.
//

#import "Channel.h"

@implementation Channel

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect
{
    // Drawing code
}
*/
-(id)init{
    self.enabled = YES;
    return [super init];
}
-(id)initWithFrame:(CGRect)frame{
    self.enabled = YES;
    return [super initWithFrame:frame];
}

@end
