//
//  CHRHChannelView.m
//  hydra
//
//  Created by Kurt Arnlund on 12/6/15.
//
//

#import "CHRHChannelView.h"
#import "AppDelegate.h"

@interface CHRHChannelView ()
@end

@implementation CHRHChannelView

@synthesize voltage_controlled = _voltage_controlled;

-(instancetype) init {
    self = [super init];
    if (self) {
        self.enabled = NO;
        self.voltage_controlled = YES;
    }
    return self;
}
-(instancetype) initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        self.enabled = NO;
        self.voltage_controlled = YES;
    }
    return self;
}

- (void)awakeFromNib {
    [super awakeFromNib];
    [self.lcdVoltage addTarget:self action:@selector(voltageAction:) forControlEvents:UIControlEventTouchUpInside];
    [self.lcdCurrent addTarget:self action:@selector(currentAction:) forControlEvents:UIControlEventTouchUpInside];
}

- (void)layoutSubviews {
    [super layoutSubviews];
}

- (void)prepareForInterfaceBuilder {
    [super prepareForInterfaceBuilder];
    
    [self.channelLabel setText:[NSString stringWithFormat:@"Channel %@", self.channelNumber ? @(self.channelNumber) : @"--"]];
}

- (void)setChannelNumber:(NSUInteger)channelNumber {
    _channelNumber = channelNumber;
    
    [self.channelLabel setText:[NSString stringWithFormat:@"Channel %@", self.channelNumber ? @(self.channelNumber) : @"--"]];
    [self.channelLabel setNeedsLayout];
}

- (void)setEnabled:(BOOL)enabled {
    _enabled = enabled;
    
    if (_enabled) {
        self.alpha = 1.0;
    } else {
        self.alpha = 0.5;
    }
}

- (void)setVoltage_controlled:(BOOL)voltage_controlled {
    _voltage_controlled = voltage_controlled;
    
    if (voltage_controlled) {
        [self.ccLed setImage: [UIImage imageNamed: @"redLedOff"]];
        [self.vcLed setImage: [UIImage imageNamed: @"redLedOn"]];
    } else {
        [self.ccLed setImage: [UIImage imageNamed: @"redLedOn"]];
        [self.vcLed setImage: [UIImage imageNamed: @"redLedOff"]];
    }
}

- (void)setUnknownMode {
    [self.ccLed setImage: [UIImage imageNamed: @"redLedOff"]];
    [self.vcLed setImage: [UIImage imageNamed: @"redLedOff"]];
}

- (void)setLedMode:(int)ledMode {
    switch (ledMode) {
        case 1:
            [self setEnabled: YES];
            [self setVoltage_controlled: NO];
            break;
        case 2:
            [self setEnabled: YES];
            [self setVoltage_controlled: YES];
            break;
        default:
            [self setEnabled: NO];
            [self setUnknownMode];
            break;
    }
}

- (BOOL)voltage_controlled {
    return _voltage_controlled;
}

- (void)setVoltageDisplay:(int)inVoltage {
    NSString *voltageStr = [NSString stringWithFormat:@"%.02f", (float)inVoltage/MILLI_SCALER];
    [self.voltage setText:voltageStr];
    
}

- (void)setCurrentDisplay:(int)inCurrent {
    NSString *currentStr = [NSString stringWithFormat:@"%.02f", (float)inCurrent/MILLI_SCALER];
    [self.current setText:currentStr];
}

- (void)setMaxCurrentDisplay:(int)inCurrent {
    NSString *currentStr = [NSString stringWithFormat:@"MAX %.02f", (float)inCurrent/MILLI_SCALER];
    [self.maxCurrent setText:currentStr];
}

- (void)valueBeingSentMode:(BOOL)valueBeingSent {
    UIImage *lcdMode = nil;
    if (valueBeingSent) {
        UIImage *yellow = [UIImage imageNamed:@"LCDyellow.png"];
        lcdMode = yellow;
    } else {
        UIImage *blue = [UIImage imageNamed:@"LCDblue.png"];
        lcdMode = blue;
    }
    [self.lcdVoltage setImage:lcdMode forState:UIControlStateNormal];
    [self.lcdCurrent setImage:lcdMode forState:UIControlStateNormal];
}

- (IBAction)voltageAction:(id)sender {
    if (self.delegate) {
        sender = self.voltage;
        [self.delegate userWantsToEditVoltage:self control:sender];
    }
}

- (IBAction)currentAction:(id)sender {
    if (self.delegate) {
        sender = self.maxCurrent;
        [self.delegate userWantsToEditMaxCurrent:self control:sender];
    }
}


@end
