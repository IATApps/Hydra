//
//  CHRHChannelView.h
//  hydra
//
//  Created by Kurt Arnlund on 12/6/15.
//
//

#import <UIKit/UIKit.h>

@class CHRHChannelView;

@protocol CHRHChannelViewDelegate <NSObject>
@required
- (IBAction)userWantsToEditVoltage:(CHRHChannelView *)chView control:(id)sender;
- (IBAction)userWantsToEditMaxCurrent:(CHRHChannelView *)chView control:(id)sender;
@end

IB_DESIGNABLE @interface CHRHChannelView : UIView
@property (nonatomic, assign) IBInspectable NSUInteger channelNumber;
@property (nonatomic, weak) IBOutlet id <CHRHChannelViewDelegate> delegate;

@property (nonatomic, assign) BOOL enabled;
@property (nonatomic, assign) BOOL voltage_controlled;

@property (nonatomic, weak) IBOutlet UILabel  *channelLabel;
@property (nonatomic, weak) IBOutlet UIButton *lcdVoltage;
@property (nonatomic, weak) IBOutlet UIImageView  *vcLed;
@property (nonatomic, weak) IBOutlet UILabel  *vcLabel;

@property (nonatomic, weak) IBOutlet UIButton *lcdCurrent;
@property (nonatomic, weak) IBOutlet UILabel  *voltage;
@property (nonatomic, weak) IBOutlet UILabel  *current;
@property (nonatomic, weak) IBOutlet UILabel  *maxCurrent;
@property (nonatomic, weak) IBOutlet UIImageView  *ccLed;
@property (nonatomic, weak) IBOutlet UILabel  *ccLabel;

- (void)setUnknownMode;

- (void)setLedMode:(int)ledMode;

- (void)setVoltageDisplay:(int)inVoltage;
- (void)setCurrentDisplay:(int)inCurrent;
- (void)setMaxCurrentDisplay:(int)inCurrent;

- (void)valueBeingSentMode:(BOOL)valueBeingSent;
@end
