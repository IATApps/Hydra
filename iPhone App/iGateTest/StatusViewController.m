//
//  ViewController.m
//  Hydra
//
//  Created by User on 3/12/13.
//  Copyright (c) 2013 CHRobotics. All rights reserved.
//

#import "StatusViewController.h"
#import "CHRHChannelView.h"
#import "AppDelegate.h"
#import "Utilities.h"
#import "HydraPacket.h"

@interface StatusViewController () <CHRHChannelViewDelegate>

@property (strong, nonatomic) AppDelegate *appDelegate;
/* These are wrappers around all the information on the channel. */
@property (weak, nonatomic) IBOutlet CHRHChannelView *ch1view;
@property (weak, nonatomic) IBOutlet CHRHChannelView *ch2view;
@property (weak, nonatomic) IBOutlet CHRHChannelView *ch3view;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *ch1Top;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *ch2Top;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *ch3Top;

/* These display the actual voltage/current values that we get back from the hydra */

/* These are the albels that display the max current as given by the hydra */

/* These are the LEDs associated with each channels CC and CV labels*/

/* The control view is the entire view that gets hidden when we are not editing values */
@property (weak, nonatomic) IBOutlet UIView *controlView;
@property (weak, nonatomic) IBOutlet UIImageView *knob;
@property (weak, nonatomic) IBOutlet UIButton *enableBtn;

@property (weak, nonatomic) IBOutlet UILabel *inputVoltageLabel;
@property (weak, nonatomic) IBOutlet UILabel *controlLabel;

/* notice selectedParam is not an IBOutlet. It is just a pointer to whichever voltage/current label we are currently editing.*/
@property (weak, nonatomic) UILabel *selectedParam;
@property (weak, nonatomic) IBOutlet CHRHChannelView *selectedChannel;

@property (nonatomic, strong) HydraPacket *packet;

@end

@implementation StatusViewController {
    UIView *overlay; //the dark overlay that shows up before the bluetooth is bonded
    UILabel *status; //displays the current bluetooth status
    float prevADC; //this is for doing the math to calculate the turns on the control dial
    float param_value; //the current value of the current/voltage we are editing
    bool voltage_selected; //are we currently editing a voltage or a current
    bool editing; //tells whether we are currently trying to set voltage/currents
    NSTimer *resendTimer; // don't get a success packet? use this to resend the packet
    int retryCount; //a counter to keep track of how many times we have sent a given packet
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    //These are notifications that we get from the bluetooth. They are sent by the app delegate which is also the bluetooth delegate.
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(stateUpdate:)
                                                 name:@"stateUpdate"
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(recievedData:)
                                                 name:@"recievedData"
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(recievedConfig:)
                                                 name:@"recievedConfig"
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(recievedSuccess:)
                                                 name:@"success"
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(recievedReject:)
                                                 name:@"rejected"
                                               object:nil];
    //basic initialization stuff
    self.ch1view.enabled = YES; //start the channels enabled.
    self.ch2view.enabled = YES;
    self.ch3view.enabled = YES;
    
    self.view.backgroundColor = [UIColor colorWithPatternImage:[UIImage imageNamed:@"steel_gradient.jpg"]];
    self.controlView.backgroundColor = [UIColor colorWithPatternImage:[UIImage imageNamed:@"controlCanvas.png"]];

    self.controlView.alpha = 0;
    param_value = 0;
    editing = NO;

    // 00000000-0000-0000-0C86-003D067A170D  -> UUID for a sample of NVC_MDCS71
    // If the app want to connect to a known device only, set the bonded UUID when
    // init iGate, or use setBondDevUUID.
    // In the test program, the bond UUID is set according to the message printed
    // in last connection.
    // In an actual app, the bond UUID and the device name may be stored and reloaded
    // when the app restarts.
    
    overlay = [Utilities overlayView];
    status = [[UILabel alloc] initWithFrame:CGRectMake(0, 200, 320, 30)];
    status.textAlignment = UITextAlignmentCenter;
    status.textColor = [UIColor whiteColor];
    status.backgroundColor = [UIColor clearColor];
    status.text = @"";
    [overlay addSubview:status];
    [self.view addSubview:overlay];
    self.appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
}

#pragma mark - User Interface Buttons
/** switches from the main screen to the settings screen*/
- (IBAction)swipeLeft:(UISwipeGestureRecognizer *)sender {
    self.tabBarController.selectedIndex++;
}

/* when one of the voltage/current buttons is pressed it calls this method. Which grabs the current voltage/current value and brings up the control panel so you can edit the values.
 */
- (void)setupForChannelNumber:(NSUInteger)channelNumber control:(NSString*)controlType channelView:(CHRHChannelView *)chView control:(id)sender {
    int chTargetVoltage = 0;
    switch (channelNumber) {
        case 1:
            chTargetVoltage = self.appDelegate.ch1TargetVoltage;
            break;
        case 2:
            chTargetVoltage = self.appDelegate.ch2TargetVoltage;
            break;
        case 3:
            chTargetVoltage = self.appDelegate.ch3TargetVoltage;
            break;
            
        default:
            break;
    }
    int chTargetCurrent = 0;
    switch (channelNumber) {
        case 1:
            chTargetCurrent = self.appDelegate.ch1MaxCurrent;
            break;
        case 2:
            chTargetCurrent = self.appDelegate.ch2MaxCurrent;
            break;
        case 3:
            chTargetCurrent = self.appDelegate.ch3MaxCurrent;
            break;
            
        default:
            break;
    }
    
    if ([controlType isEqualToString:@"Voltage"]) {
        param_value = (float)chTargetVoltage/MILLI_SCALER;
    } else  {
        param_value = (float)chTargetCurrent/MILLI_SCALER;
    }
    
    [chView setVoltageDisplay: chTargetVoltage];
    [chView setCurrentDisplay: chTargetCurrent];
    
    NSString *controlLabel = [NSString stringWithFormat:@"Channel %d %@ Control", channelNumber, controlType];
    [self.controlLabel setText: controlLabel];

    self.selectedChannel = chView;
    self.selectedParam = sender;
    
    NSLayoutConstraint *top = nil;
    switch (channelNumber) {
        case 1:
            top = self.ch1Top;
            break;
        case 2:
            top = self.ch2Top;
            break;
        case 3:
            top = self.ch3Top;
            break;
            
        default:
            break;
    }

    if (!chView.enabled){
        [self.enableBtn setTitle:@"Enable" forState:UIControlStateNormal];
    }else{
        [self.enableBtn setTitle:@"Disable" forState:UIControlStateNormal];
    }

    [UIView animateWithDuration:0.33 animations:^{
        top.constant = CENTER_STAGE.y;
        NSArray *channels = @[self.ch1view, self.ch2view, self.ch3view];
        for (UIView *channel in channels) {
            if (channel != chView) {
                channel.alpha = 0;
            }
        }
        [chView layoutIfNeeded];
        self.controlView.alpha = 1;
    }];

    editing = YES;
}

- (IBAction)userWantsToEditVoltage:(CHRHChannelView *)chView control:sender {
    NSUInteger channelNum = chView.channelNumber;
    [self setupForChannelNumber:channelNum control:@"Voltage" channelView:chView control:sender];
    voltage_selected = true;
}

- (IBAction)userWantsToEditMaxCurrent:(CHRHChannelView *)chView control:sender {
    NSUInteger channelNum = chView.channelNumber;
    [self setupForChannelNumber:channelNum control:@"Current" channelView:chView control:sender];
    voltage_selected = false;
}

/* all the logic to turn the control knob and update the labels with new values. */
- (IBAction)drag:(UIPanGestureRecognizer *)gesture {
    CGPoint tapLocation = [gesture locationInView:self.controlView];
    float x = tapLocation.x-self.knob.center.x;
    float y = (tapLocation.y-self.knob.center.y)*-1;
    float angle = atanf(y/x);
    if (y>0 && x<0){
        angle += M_PI;
    }
    else if(y<=0 && x<0){
        angle += M_PI;
    }
    else if(y<0 && x>0){
        angle += M_PI * 2;
    }
    angle = M_PI*2-angle;
    self.knob.transform = CGAffineTransformMakeRotation(angle);
    if(gesture.state == UIGestureRecognizerStateBegan){
        prevADC = angle;
        return;
    }
    float diff = (angle-prevADC);
    if (diff>3){
        diff = 2*M_PI-diff;
    }else if (diff < -3){
        diff = 2*M_PI+diff;
    }
    param_value += diff/5;
    if (voltage_selected){
        if (param_value > MAX_VOLTAGE){
            param_value = MAX_VOLTAGE;
        }
        else if (param_value < MIN_VOLTAGE){
            param_value = MIN_VOLTAGE;
        }
    }
    else {
        if (param_value > MAX_CURRENT){
            param_value = MAX_CURRENT;
        }
        else if (param_value < MIN_CURRENT){
            param_value = MIN_CURRENT;
        }
    }
    prevADC = angle;
    if(!voltage_selected){//current
        self.selectedParam.text = [NSString stringWithFormat:@"MAX %.02f", param_value];
    }else{
        self.selectedParam.text = [NSString stringWithFormat:@"%.02f", param_value];
    }
}

/* ok we like our new voltage and current so we tell the app delegate to send the new values to the hydra */
- (IBAction)acceptPressed:(UIButton *)sender {
    NSUInteger channel = self.selectedChannel.channelNumber;
    BOOL enabled = self.selectedChannel.enabled;
    float current = 0.0;
    float voltage = 0.0;
    voltage = enabled ? [self.selectedChannel.voltage.text floatValue] : 0.0;
    current = enabled ? [self.selectedChannel.maxCurrent.text stringByReplacingOccurrencesOfString:@"MAX " withString:@""].floatValue : 0.0;

    switch (channel) {
        case 1:
            self.appDelegate.ch1TargetVoltage = voltage * MILLI_SCALER;
            self.appDelegate.ch1MaxCurrent = current * MILLI_SCALER;
            break;
        case 2:
            self.appDelegate.ch2TargetVoltage = voltage * MILLI_SCALER;
            self.appDelegate.ch2MaxCurrent = current * MILLI_SCALER;
            break;
        case 3:
            self.appDelegate.ch3TargetVoltage = voltage * MILLI_SCALER;
            self.appDelegate.ch3MaxCurrent = current * MILLI_SCALER;
            break;
        default:
            break;
    }
    
    //set the buttons to be that yellow color until we get a success packet back.
    [self.selectedChannel valueBeingSentMode:YES];
    
    //this method is the one that creates the bluetooth packet string and then sends it to the bluetooth delegate to be sent
    [self updateHydra];
    
    //restore the channel view positions and alpha
    [self restoreChannels];
    self.controlView.alpha = 0;
    self.selectedParam = nil;
    
    editing = NO;
}

/* user decided not to update any voltage or currents */
- (IBAction)cancelPressed:(UIButton *)sender {
    [self restoreChannels];
    self.controlView.alpha = 0;
    self.selectedParam = nil;
    self.selectedChannel = nil;
    editing = NO;
}

/* a little bit of clean up after we are done editing values. make sure all the channels are displayed again and whatnot. */
-(void)restoreChannels {
    [UIView animateWithDuration:0.33 animations:^{
        self.ch1Top.constant = 0.0;
        self.ch2Top.constant = self.ch1view.bounds.size.height + 5;
        self.ch3Top.constant = self.ch1view.bounds.size.height + self.ch2view.bounds.size.height + 10;
        [self.ch1view layoutIfNeeded];
        [self.ch2view layoutIfNeeded];
        [self.ch3view layoutIfNeeded];
    }];
    [self.ch1view setEnabled: self.ch1view.enabled];
    [self.ch2view setEnabled: self.ch2view.enabled];
    [self.ch3view setEnabled: self.ch3view.enabled];
}

/*enabling and disabling channels.*/
- (IBAction)enablePressed:(UIButton *)enableBtn {
    CHRHChannelView *channel = ((CHRHChannelView*)self.selectedChannel);
    if (channel.enabled){
        channel.enabled = NO;
        [enableBtn setTitle:@"Enable" forState:UIControlStateNormal];
        [enableBtn setTitle:@"Enable" forState:UIControlStateSelected];
    }
    else{
        channel.enabled = YES;
        [enableBtn setTitle:@"Disable" forState:UIControlStateNormal];
        [enableBtn setTitle:@"Disable" forState:UIControlStateSelected];
    }
}

#pragma mark - update Hydra
/* This is what creates the packet that the bluetooth delegate sends back to the hydra*/
-(void)updateHydra {
    uint16_t ch1voltage = self.appDelegate.ch1TargetVoltage;
    uint16_t ch1current = self.appDelegate.ch1MaxCurrent;
    uint16_t ch2voltage = self.appDelegate.ch2TargetVoltage;
    uint16_t ch2current = self.appDelegate.ch2MaxCurrent;
    uint16_t ch3voltage = self.appDelegate.ch3TargetVoltage;
    uint16_t ch3current = self.appDelegate.ch3MaxCurrent;
    
    
    self.packet = [[HydraPacket alloc] init];
    
    [self.packet addAddress:0x00];
    [self.packet addBatchForCurrent:ch1current voltage:ch1voltage enabled:self.ch1view.enabled lcMode:self.appDelegate.ch1LC];
    [self.packet addBatchForCurrent:ch2current voltage:ch2voltage enabled:self.ch2view.enabled lcMode:self.appDelegate.ch2LC];
    [self.packet addBatchForCurrent:ch3current voltage:ch3voltage enabled:self.ch3view.enabled lcMode:self.appDelegate.ch3LC];
    [self.packet addChecksum];
    
    NSAssert([self.packet isChecksumValid], @"packet checksum was not valid");
    
    NSLog(@"Output (%d bytes) %@", self.packet.packet.length, [self.packet debugDescription]);
    
    [self.appDelegate sendCommand:self.packet.packet];
    retryCount = 0;
    resendTimer = [NSTimer scheduledTimerWithTimeInterval:2
                                     target:self
                                   selector:@selector(resendData:)
                                   userInfo:nil
                                    repeats:YES];
}
/* this is called by the timer and it just resends the data up to five times as long as the timer keeps calling it. The resendTimer gets invalidated when there is a success packet notification recieved. */
-(void)resendData:(NSTimer*)timer {
    NSLog(@"trying to resend");
    if (retryCount < 5) {
        retryCount++;
        [self.appDelegate sendCommand:self.packet.packet];
    } else {
        retryCount = 0;
        
        [resendTimer invalidate];

        UIAlertController *alert = [UIAlertController alertControllerWithTitle:NSLocalizedString(@"Connection Error", @"Connection Error") message:NSLocalizedString(@"Failed to recieve success packet from Hydra", @"Failed to recieve success packet from Hydra") preferredStyle:UIAlertControllerStyleActionSheet];
        
        UIAlertAction *okay = [UIAlertAction actionWithTitle:NSLocalizedString(@"Okay", @"Okay") style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
        }];
        
        [alert addAction:okay];
        [self presentViewController:alert animated:YES completion:^{
            
        }];
    }
}

#pragma mark - Recieve Info Methods
/* update all the labels because we just got new data */
-(void)recievedData:(NSNotification *)notification {
    if (editing) return;
    
    NSDictionary *userInfo = notification.userInfo;
    [self.ch1view setVoltageDisplay:[userInfo[CH_1_VOLT] intValue]];
    [self.ch1view setCurrentDisplay:[userInfo[CH_1_CURR] intValue]];

    [self.ch2view setVoltageDisplay:[userInfo[CH_2_VOLT] intValue]];
    [self.ch2view setCurrentDisplay:[userInfo[CH_2_CURR] intValue]];

    [self.ch3view setVoltageDisplay:[userInfo[CH_3_VOLT] intValue]];
    [self.ch3view setCurrentDisplay:[userInfo[CH_3_CURR] intValue]];

    self.inputVoltageLabel.text = [NSString stringWithFormat:@"%.2f",[userInfo[IN_VOLT] floatValue]/MILLI_SCALER];
    
    //Set the CV or CC led's based on what data we just got back
    [self.ch1view setLedMode: [userInfo[CH_1_MODE] intValue]];
    [self.ch2view setLedMode: [userInfo[CH_2_MODE] intValue]];
    [self.ch3view setLedMode: [userInfo[CH_3_MODE] intValue]];
}

-(void)recievedConfig:(NSNotification *) notification{
    if (editing) return;
    [self.ch1view setMaxCurrentDisplay:self.appDelegate.ch1MaxCurrent];
    [self.ch2view setMaxCurrentDisplay:self.appDelegate.ch2MaxCurrent];
    [self.ch3view setMaxCurrentDisplay:self.appDelegate.ch3MaxCurrent];
    self.ch1view.enabled = self.appDelegate.ch1OE;
    self.ch2view.enabled = self.appDelegate.ch2OE;
    self.ch3view.enabled = self.appDelegate.ch3OE;
    [self restoreChannels];
}

/* we got a success packet. Turn all the channels blue and invalidate the resend timer */
-(void)recievedSuccess:(NSNotification *) notification {
    [self.selectedChannel valueBeingSentMode: NO];
    self.selectedChannel = nil;
    [resendTimer invalidate];
    resendTimer = nil;
}

-(void)recievedReject:(NSNotification *) notification {
    [self.selectedChannel valueBeingSentMode: YES];
    self.selectedChannel = nil;
    [resendTimer invalidate];
    resendTimer = nil;    
}

/* We got a bluetooth state update. If we are bonded dismiss the overlay. If not, then just update the status label...*/
-(void)stateUpdate:(NSNotification *) notification{
    if (![self.view.subviews containsObject:overlay]){
        [self.view addSubview:overlay];
    }
    NSDictionary *userInfo = notification.userInfo;
    NSUInteger state = [userInfo[@"state"] integerValue];
    NSString *statusText = status.text;
    switch(state)
    {
        case CiGateStateInit: statusText = @"Init"; break;
        case CiGateStateIdle: statusText = @"Idle"; break;
        case CiGateStatePoweredOff: statusText = @"Powered Off"; break;
        case CiGateStateUnknown: statusText = @"Unkown State"; break;
        case CiGateStateUnsupported: statusText = @"State Unsupported"; break;
        case CiGateStateSearching: statusText = @"Searching"; break;
        case CiGateStateConnecting: statusText = @"Connecting"; break;
        case CiGateStateConnected: statusText = @"Connected"; break;
        case CiGateStateBonded: statusText = @"Bonded"; break;
    }
    status.text = statusText;

    if (state == CiGateStateBonded) {
        [overlay removeFromSuperview];
    }
}
#pragma mark - View Did Unload

- (void)viewDidUnload {
    [self setControlLabel:nil];
    [self setInputVoltageLabel:nil];
    [super viewDidUnload];
}
@end
