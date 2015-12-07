//
//  ViewController.m
//  Hydra
//
//  Created by User on 3/12/13.
//  Copyright (c) 2013 CHRobotics. All rights reserved.
//

#import "StatusViewController.h"
#import "Channel.h"
#import "AppDelegate.h"
#import "Utilities.h"
#import "HydraPacket.h"

@interface StatusViewController ()

@property (strong, nonatomic) AppDelegate *appDelegate;
/* These are wrappers around all the information on the channel. */
@property (strong, nonatomic) IBOutlet Channel *ch1view;
@property (strong, nonatomic) IBOutlet Channel *ch2view;
@property (strong, nonatomic) IBOutlet Channel *ch3view;

/* These display the actual voltage/current values that we get back from the hydra */
@property (strong, nonatomic) IBOutlet UILabel *ch1voltageLabel;
@property (strong, nonatomic) IBOutlet UILabel *ch1currentLabel;
@property (strong, nonatomic) IBOutlet UILabel *ch2voltageLabel;
@property (strong, nonatomic) IBOutlet UILabel *ch2currentLabel;
@property (strong, nonatomic) IBOutlet UILabel *ch3voltageLabel;
@property (strong, nonatomic) IBOutlet UILabel *ch3currentLabel;

/* These are the albels that display the max current as given by the hydra */
@property (strong, nonatomic) IBOutlet UILabel *ch1MaxCurrent;
@property (strong, nonatomic) IBOutlet UILabel *ch2MaxCurrent;
@property (strong, nonatomic) IBOutlet UILabel *ch3MaxCurrent;

/* These are the LEDs associated with each channels CC and CV labels*/
@property (strong, nonatomic) IBOutlet UIImageView *ch1_CV_led;
@property (strong, nonatomic) IBOutlet UIImageView *ch1_CC_led;
@property (strong, nonatomic) IBOutlet UIImageView *ch2_CV_led;
@property (strong, nonatomic) IBOutlet UIImageView *ch2_CC_led;
@property (strong, nonatomic) IBOutlet UIImageView *ch3_CV_led;
@property (strong, nonatomic) IBOutlet UIImageView *ch3_CC_led;

/* The control view is the entire view that gets hidden when we are not editing values */
@property (strong, nonatomic) IBOutlet UIView *controlView;
@property (strong, nonatomic) IBOutlet UIImageView *knob;
@property (strong, nonatomic) IBOutlet UIButton *enableBtn;

@property (strong, nonatomic) IBOutlet UILabel *inputVoltageLabel;
@property (strong, nonatomic) IBOutlet UILabel *controlLabel;

/* notice selectedParam is not an IBOutlet. It is just a pointer to whichever voltage/current label we are currently editing.*/
@property (strong, nonatomic) UILabel *selectedParam;

@property (nonatomic, strong) HydraPacket *packet;

@end

@implementation ViewController{
    UIView *overlay; //the dark overlay that shows up before the bluetooth is bonded
    UILabel *status; //displays the current bluetooth status
    float prevADC; //this is for doing the math to calculate the turns on the control dial
    float param_value; //the current value of the current/voltage we are editing
    bool voltage_selected; //are we currently editing a voltage or a current
    bool ch1Enable; //whether the channel has been enabled
    bool ch2Enable;
    bool ch3Enable;
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
- (IBAction)paramPressed:(UIButton *)sender {
    //get the current value from the bluetooth delegate and set the voltage/current labels
    self.ch1voltageLabel.text = [NSString stringWithFormat:@"%.02f", (float)self.appDelegate.ch1TargetVoltage/MILLI_SCALER];
    self.ch2voltageLabel.text = [NSString stringWithFormat:@"%.02f", (float)self.appDelegate.ch2TargetVoltage/MILLI_SCALER];
    self.ch3voltageLabel.text = [NSString stringWithFormat:@"%.02f", (float)self.appDelegate.ch3TargetVoltage/MILLI_SCALER];
    
    switch (sender.tag) {
        case 1: self.controlLabel.text = @"Channel 1 Voltage Control"; break;
        case 2: self.controlLabel.text = @"Channel 1 Current Control"; break;
        case 3: self.controlLabel.text = @"Channel 2 Voltage Control"; break;
        case 4: self.controlLabel.text = @"Channel 2 Current Control"; break;
        case 5: self.controlLabel.text = @"Channel 3 Voltage Control"; break;
        case 6: self.controlLabel.text = @"Channel 3 Current Control"; break;
        default: break;
    }
    // all of the voltage and current buttons have tags. 1-6 left to right, top to bottom. if it is divisible by two then it is a current button.
     
    if (sender.tag % 2 == 0) voltage_selected = false;
    else voltage_selected = true;
    
    //this shuffles the buttons around so that the field you are editing is shown in the middle of the screen
    switch (sender.tag) {
        case 1: //channel 1 voltage
            param_value = (float)self.appDelegate.ch1TargetVoltage/MILLI_SCALER;
            self.ch1view.center = CENTER_STAGE;
            self.ch2view.alpha = 0;
            self.ch3view.alpha = 0;
            self.selectedParam = self.ch1voltageLabel;
            break;
        case 2: //channel 1 current
            param_value = (float)self.appDelegate.ch1MaxCurrent/MILLI_SCALER;
            self.ch1view.center = CENTER_STAGE;
            self.ch2view.alpha = 0;
            self.ch3view.alpha = 0;
            self.selectedParam = self.ch1MaxCurrent;
            break;
        case 3://channel 2 voltage
            param_value = (float)self.appDelegate.ch2TargetVoltage/MILLI_SCALER;
            self.ch2view.center = CENTER_STAGE;
            self.ch1view.alpha = 0;
            self.ch3view.alpha = 0;
            self.selectedParam = self.ch2voltageLabel;
            break;
        case 4: //channel 2 current
            param_value = (float)self.appDelegate.ch2MaxCurrent/MILLI_SCALER;
            self.ch2view.center = CENTER_STAGE;
            self.ch1view.alpha = 0;
            self.ch3view.alpha = 0;
            self.selectedParam = self.ch2MaxCurrent;
            break;
        case 5: //channel 3 voltage
            param_value = (float)self.appDelegate.ch3TargetVoltage/MILLI_SCALER;
            self.ch3view.center = CENTER_STAGE;
            self.ch2view.alpha = 0;
            self.ch1view.alpha = 0;
            self.selectedParam = self.ch3voltageLabel;
            break;
        case 6: //channel 3 current
            param_value = (float)self.appDelegate.ch3MaxCurrent/MILLI_SCALER;
            self.ch3view.center = CENTER_STAGE;
            self.ch2view.alpha = 0;
            self.ch1view.alpha = 0;
            self.selectedParam = self.ch3MaxCurrent;
            break;
        default:
            break;
    }
    //set the opacity for the channels according to whether they are enabled/disabled
    if (self.selectedParam.superview.alpha == .5){
        [self.enableBtn setTitle:@"Enable" forState:UIControlStateNormal];
    }else{
        [self.enableBtn setTitle:@"Disable" forState:UIControlStateNormal];
    }
    self.controlView.alpha = 1;
    editing = YES;
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
    UIView *channelView = self.selectedParam.superview;
    float current;
    switch (channelView.tag) {
        case 1:            
            self.appDelegate.ch1TargetVoltage = [self.ch1voltageLabel.text floatValue] *MILLI_SCALER;
            current = [self.ch1MaxCurrent.text stringByReplacingOccurrencesOfString:@"MAX " withString:@""].floatValue;
            self.appDelegate.ch1MaxCurrent = current*MILLI_SCALER;
            break;
        case 2:
            self.appDelegate.ch2TargetVoltage = [self.ch2voltageLabel.text floatValue] *MILLI_SCALER;
            current = [self.ch2MaxCurrent.text stringByReplacingOccurrencesOfString:@"MAX " withString:@""].floatValue;
            self.appDelegate.ch2MaxCurrent = current*MILLI_SCALER;
            break;
        case 3:
            self.appDelegate.ch3TargetVoltage = [self.ch3voltageLabel.text floatValue] *MILLI_SCALER;
            current = [self.ch2MaxCurrent.text stringByReplacingOccurrencesOfString:@"MAX " withString:@""].floatValue;
            self.appDelegate.ch3MaxCurrent = current*MILLI_SCALER;
            break;
        default:
            break;
    }
    
    //set the buttons to be that yellow color until we get a success packet back.
    UIButton *button = (UIButton*)[[channelView subviews] objectAtIndex:0];
    [button setImage:[UIImage imageNamed:@"LCDyellow.png"] forState:UIControlStateNormal];
    button = (UIButton*)[[channelView subviews] objectAtIndex:1];
    [button setImage:[UIImage imageNamed:@"LCDyellow.png"] forState:UIControlStateNormal];
    
    [self updateHydra]; //this method is the one that creates the bluetooth packet string and then sends it to the bluetooth delegate to be sent
    
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
    editing = NO;
}

/* a little bit of clean up after we are done editing values. make sure all the channels are displayed again and whatnot. */
-(void)restoreChannels{
    self.ch1view.frame = CH1_VIEW_FRAME;
    self.ch2view.frame = CH2_VIEW_FRAME;
    self.ch3view.frame = CH3_VIEW_FRAME;
    self.ch1view.alpha = .5;
    self.ch2view.alpha = .5;
    self.ch3view.alpha = .5;
    if (self.ch1view.enabled){
        self.ch1view.alpha = 1;
    }
    if (self.ch2view.enabled){
        self.ch2view.alpha = 1;
    }
    if (self.ch3view.enabled){
        self.ch3view.alpha = 1;
    }
}

/*enabling and disabling channels.*/
- (IBAction)enablePressed:(UIButton *)enableBtn {
    Channel *channel = ((Channel*)self.selectedParam.superview);
    if (channel.enabled){
        channel.enabled = NO;
        channel.alpha = .5;
        [enableBtn setTitle:@"Enable" forState:UIControlStateNormal];
        [enableBtn setTitle:@"Enable" forState:UIControlStateSelected];
    }
    else{
        [enableBtn setTitle:@"Disable" forState:UIControlStateNormal];
        [enableBtn setTitle:@"Disable" forState:UIControlStateSelected];
        channel.enabled = YES;
        channel.alpha = 1;
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
    [self.packet addBatchForCurrent:ch1current voltage:ch1voltage enabled:YES lcMode:self.appDelegate.ch1LC];
    [self.packet addBatchForCurrent:ch2current voltage:ch2voltage enabled:self.ch2view.enabled lcMode:self.appDelegate.ch2LC];
    [self.packet addBatchForCurrent:ch3current voltage:ch3voltage enabled:self.ch3view.enabled lcMode:self.appDelegate.ch3LC];
    [self.packet addChecksum];
    
    NSAssert([self.packet isChecksumValid], @"packet checksum was not valid");
    
    NSLog(@"Output (%d bytes) %@", self.packet.packet.length, [self.packet debugDescription]);
    
    [self.appDelegate sendCommand:self.packet.packet];
    retryCount = 0;
    resendTimer = [NSTimer scheduledTimerWithTimeInterval:.5
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
    self.ch1voltageLabel.text = [NSString stringWithFormat:@"%.02f",[userInfo[CH_1_VOLT] floatValue]/MILLI_SCALER];
   
    self.ch1currentLabel.text = [NSString stringWithFormat:@"%.02f",[userInfo[CH_1_CURR] floatValue]/MILLI_SCALER];
    
    self.ch2voltageLabel.text = [NSString stringWithFormat:@"%.02f",[userInfo[CH_2_VOLT] floatValue]/MILLI_SCALER];
    
    self.ch2currentLabel.text = [NSString stringWithFormat:@"%.02f",[userInfo[CH_2_CURR] floatValue]/MILLI_SCALER];
    
    self.ch3voltageLabel.text = [NSString stringWithFormat:@"%.02f",[userInfo[CH_3_VOLT] floatValue]/MILLI_SCALER];
    
    self.ch3currentLabel.text = [NSString stringWithFormat:@"%.02f",[userInfo[CH_3_CURR] floatValue]/MILLI_SCALER];
    
    self.inputVoltageLabel.text = [NSString stringWithFormat:@"%.2f",[userInfo[IN_VOLT] floatValue]/MILLI_SCALER];
    
    //Set the CV or CC led's based on what data we just got back
    switch ([userInfo[CH_1_MODE] intValue]) {
        case 1:
            self.ch1_CV_led.image = [UIImage imageNamed:@"redLedOff"];
            self.ch1_CC_led.image = [UIImage imageNamed:@"redLedOn"];
            break;
        case 2:
            self.ch1_CV_led.image = [UIImage imageNamed:@"redLedOn"];
            self.ch1_CC_led.image = [UIImage imageNamed:@"redLedOff"];
            break;
        default:
            self.ch1_CV_led.image = [UIImage imageNamed:@"redLedOff"];
            self.ch1_CC_led.image = [UIImage imageNamed:@"redLedOff"];
            break;
    }
    
    switch ([userInfo[CH_2_MODE] intValue]) {
        case 1:
            self.ch2_CV_led.image = [UIImage imageNamed:@"redLedOff"];
            self.ch2_CC_led.image = [UIImage imageNamed:@"redLedOn"];
            break;
        case 2:
            self.ch2_CV_led.image = [UIImage imageNamed:@"redLedOn"];
            self.ch2_CC_led.image = [UIImage imageNamed:@"redLedOff"];
            break;
        default:
            self.ch2_CV_led.image = [UIImage imageNamed:@"redLedOff"];
            self.ch2_CC_led.image = [UIImage imageNamed:@"redLedOff"];
            break;
    }
    
    switch ([userInfo[CH_3_MODE] intValue]) {
        case 1:
            self.ch3_CV_led.image = [UIImage imageNamed:@"redLedOff"];
            self.ch3_CC_led.image = [UIImage imageNamed:@"redLedOn"];
            break;
        case 2:
            self.ch3_CV_led.image = [UIImage imageNamed:@"redLedOn"];
            self.ch3_CC_led.image = [UIImage imageNamed:@"redLedOff"];
            break;
        default:
            self.ch3_CV_led.image = [UIImage imageNamed:@"redLedOff"];
            self.ch3_CC_led.image = [UIImage imageNamed:@"redLedOff"];
            break;
    }
}

-(void)recievedConfig:(NSNotification *) notification{
    if (editing) return;
    self.ch1MaxCurrent.text = [NSString stringWithFormat:@"MAX %.02f",((float)self.appDelegate.ch1MaxCurrent)/MILLI_SCALER];
    self.ch2MaxCurrent.text = [NSString stringWithFormat:@"MAX %.02f",((float)self.appDelegate.ch2MaxCurrent)/MILLI_SCALER];
    self.ch3MaxCurrent.text = [NSString stringWithFormat:@"MAX %.02f",((float)self.appDelegate.ch3MaxCurrent)/MILLI_SCALER];
    self.ch1view.enabled = self.appDelegate.ch1OE;
    self.ch2view.enabled = self.appDelegate.ch2OE;
    self.ch3view.enabled = self.appDelegate.ch3OE;
    [self restoreChannels];
}

/* we got a success packet. Turn all the channels blue and invalidate the resend timer */
-(void)recievedSuccess:(NSNotification *) notification{
    [resendTimer invalidate];
    resendTimer = nil;
    UIButton *button = (UIButton*)[self.ch1view.subviews objectAtIndex:0];
    [button setImage:[UIImage imageNamed:@"LCDblue.png"] forState:UIControlStateNormal];
    button = (UIButton*)[self.ch1view.subviews objectAtIndex:1];
    [button setImage:[UIImage imageNamed:@"LCDblue.png"] forState:UIControlStateNormal];
    button = (UIButton*)[self.ch2view.subviews objectAtIndex:0];
    [button setImage:[UIImage imageNamed:@"LCDblue.png"] forState:UIControlStateNormal];
    button = (UIButton*)[self.ch2view.subviews objectAtIndex:1];
    [button setImage:[UIImage imageNamed:@"LCDblue.png"] forState:UIControlStateNormal];
    button = (UIButton*)[self.ch3view.subviews objectAtIndex:0];
    [button setImage:[UIImage imageNamed:@"LCDblue.png"] forState:UIControlStateNormal];
    button = (UIButton*)[self.ch3view.subviews objectAtIndex:1];
    [button setImage:[UIImage imageNamed:@"LCDblue.png"] forState:UIControlStateNormal];
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
    [self setCh1_CV_led:nil];
    [self setCh2_CV_led:nil];
    [self setCh2_CV_led:nil];
    [self setCh1MaxCurrent:nil];
    [self setCh2MaxCurrent:nil];
    [self setCh3MaxCurrent:nil];
    [self setCh1currentLabel:nil];
    [self setCh2voltageLabel:nil];
    [self setCh2currentLabel:nil];
    [self setCh1voltageLabel:nil];
    [self setCh3currentLabel:nil];
    [super viewDidUnload];
}
@end
