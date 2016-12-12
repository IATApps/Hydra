//
//  SettingsViewController.m
//  Hydra
//
//  Created by User on 4/9/13.
//  Copyright (c) 2013 CHRobotics. All rights reserved.
//

#import "SettingsViewController.h"
#import "AppDelegate.h"
//#import "iGate.h"
#import "HydraPacket.h"

@interface SettingsViewController ()<UITextFieldDelegate>
@property (weak, nonatomic) IBOutlet UISwitch *ch1VoltageCutoffOverride;
@property (weak, nonatomic) IBOutlet UISwitch *ch2VoltageCutoffOverride;
@property (weak, nonatomic) IBOutlet UISwitch *ch3VoltageCutoffOverride;
@property (weak, nonatomic) IBOutlet UISwitch *ch1LowCurrent;
@property (weak, nonatomic) IBOutlet UISwitch *ch2LowCurrent;
@property (weak, nonatomic) IBOutlet UISwitch *ch3LowCurrent;
@property (weak, nonatomic) IBOutlet UITextField *lowVoltageCutoff;
@property (weak, nonatomic) IBOutlet UIButton *saveBtn;
@property (weak, nonatomic) IBOutlet UILabel *firmwareLabel;
@property (weak, nonatomic) IBOutlet UIButton *firmwareButton;
@property (nonatomic, strong) HydraPacket *hydraPacket;

@end

@implementation SettingsViewController{
    UIView *overlayView;
    AppDelegate *appDelegate;
    uint8_t packet[19];
    int retryCount;
    NSTimer *resendTimer;
    bool sendingLVC;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor colorWithPatternImage:[UIImage imageNamed:@"steel_gradient.jpg"]];
    self.saveBtn.enabled = NO;
    self.lowVoltageCutoff.delegate = self;
    appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(stateUpdate:)
                                                 name:@"stateUpdate"
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(recievedSuccess:)
                                                 name:@"success"
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(firmwareVersionReport:)
                                                 name:@"firmwareVersionReport"
                                               object:nil];
}

-(void)viewWillAppear:(BOOL)animated{
    [super viewWillAppear:animated];
    self.ch1LowCurrent.on = appDelegate.ch1LC;
    self.ch2LowCurrent.on = appDelegate.ch2LC;
    self.ch3LowCurrent.on = appDelegate.ch3LC;
    
    self.ch1VoltageCutoffOverride.on = appDelegate.ch1CVO;
    self.ch2VoltageCutoffOverride.on = appDelegate.ch2CVO;
    self.ch3VoltageCutoffOverride.on = appDelegate.ch3CVO;
    
    CiGateState state = appDelegate.iGate.state;
    if (state != CiGateStateBonded) {
        overlayView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 320, 500)];
        overlayView.backgroundColor = [UIColor colorWithWhite:1 alpha:.3];
        [self.view addSubview:overlayView];
    } else {
        [overlayView removeFromSuperview];
        overlayView = nil;
    }
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    
    [self firmwarePressed:self.firmwareButton];
}


-(void)recievedConfig:(NSNotification *) notification{
   
}

-(void)stateUpdate:(NSNotification *) notification{
    CiGateState state = appDelegate.iGate.state;
    if (state == CiGateStateBonded) {
        [overlayView removeFromSuperview];
    }
}
 
- (IBAction)tap:(UITapGestureRecognizer *)sender {
    [self.lowVoltageCutoff resignFirstResponder];
}

- (IBAction)firmwarePressed:(id)sender {
    self.firmwareButton.enabled = NO;
    
    self.hydraPacket = [[HydraPacket alloc] init];
    [self.hydraPacket addAddress:0xAA];
    [self.hydraPacket addChecksum];
    
    NSLog(@" firmware request: %@", [self.hydraPacket debugDescription]);

    [self sendPacket];
}

- (IBAction)savePressed:(UIButton *)sender {
    sendingLVC = NO;
    [self.lowVoltageCutoff resignFirstResponder];

    [self setSettingsPacket];

    self.saveBtn.enabled = NO;
    [self sendPacket];
}

- (void)sendPacket {
    retryCount = 0;
    [appDelegate sendCommand:self.hydraPacket.packet];
    
    // There is no ACK for firmware requests
    if (([self.hydraPacket addressByte] != 0xAA) && ([self.hydraPacket addressByte] != 0x00)) {
        resendTimer = [NSTimer scheduledTimerWithTimeInterval:.5
                                                       target:self
                                                     selector:@selector(resendData:)
                                                     userInfo:nil
                                                      repeats:YES];
    }
}

- (void)sendPacketWithLen:(UInt8)len {
    retryCount = 0;
    [appDelegate sendCommand:[NSData dataWithBytes:packet length:len]];
    resendTimer = [NSTimer scheduledTimerWithTimeInterval:.5
                                                   target:self
                                                 selector:@selector(resendData:)
                                                 userInfo:nil
                                                  repeats:YES];
}

-(void)resendData:(NSTimer*)timer{
    if (retryCount < 5){
        retryCount++;
        [appDelegate sendCommand:[NSData dataWithBytes:packet length:19]];
    }
    else{
        self.firmwareButton.enabled = YES;
        self.saveBtn.enabled = YES;

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

//- (void)sendLVCPacket{
//    //TODO: overwrite this to send the LVC data and not the voltage/current data.
//    sendingLVC = YES;
//    uint16_t ch1voltage = appDelegate.ch1TargetVoltage;
//    uint16_t ch1current = appDelegate.ch1MaxCurrent;
//    uint16_t ch2voltage = appDelegate.ch2TargetVoltage;
//    uint16_t ch2current = appDelegate.ch2MaxCurrent;
//    uint16_t ch3voltage = appDelegate.ch3TargetVoltage;
//    uint16_t ch3current = appDelegate.ch3MaxCurrent;
//    
//    self.hydraPacket = [[HydraPacket alloc] init];
//    [self.hydraPacket addAddress:0x00];
//    [self.hydraPacket addBatchForCurrent:ch1current voltage:ch1voltage enabled:appDelegate.ch1OE lcMode:appDelegate.ch1LC];
//    [self.hydraPacket addBatchForCurrent:ch2current voltage:ch2voltage enabled:appDelegate.ch2OE lcMode:appDelegate.ch2LC];
//    [self.hydraPacket addBatchForCurrent:ch3current voltage:ch3voltage enabled:appDelegate.ch3OE lcMode:appDelegate.ch3LC];
//    [self.hydraPacket addChecksum];
//    
//    NSAssert([self.hydraPacket isChecksumValid], @"hydra packet checksum fail");
//    
//    [self sendPacket];
//}

- (void)setSettingsPacket {
    appDelegate.ch1LC = self.ch1LowCurrent.on;
    appDelegate.ch2LC = self.ch2LowCurrent.on;
    appDelegate.ch3LC = self.ch3LowCurrent.on;
    appDelegate.ch1CVO = self.ch1VoltageCutoffOverride.on;
    appDelegate.ch2CVO = self.ch2VoltageCutoffOverride.on;
    appDelegate.ch3CVO = self.ch3VoltageCutoffOverride.on;
    
    uint16_t vinCutoffVoltage = MIN(MAX([self.lowVoltageCutoff.text integerValue], 0), 14);
    uint16_t ch1voltage = appDelegate.ch1TargetVoltage;
    uint16_t ch1current = appDelegate.ch1MaxCurrent;
    uint16_t ch2voltage = appDelegate.ch2TargetVoltage;
    uint16_t ch2current = appDelegate.ch2MaxCurrent;
    uint16_t ch3voltage = appDelegate.ch3TargetVoltage;
    uint16_t ch3current = appDelegate.ch3MaxCurrent;
    
    self.hydraPacket = [[HydraPacket alloc] init];
    [self.hydraPacket addAddress:0x00];
    [self.hydraPacket addBatchForCurrent:ch1current voltage:ch1voltage enabled:appDelegate.ch1OE lcMode:appDelegate.ch1LC];
    [self.hydraPacket addBatchForCurrent:ch2current voltage:ch2voltage enabled:appDelegate.ch2OE lcMode:appDelegate.ch2LC];
    [self.hydraPacket addBatchForCurrent:ch3current voltage:ch3voltage enabled:appDelegate.ch3OE lcMode:appDelegate.ch3LC];
    [self.hydraPacket addBatchForVINCutoff:vinCutoffVoltage voltageCutoffOverride1:self.ch1VoltageCutoffOverride.isOn voltageCutoffOverride2:self.ch2VoltageCutoffOverride.isOn voltageCutoffOverride3:self.ch3VoltageCutoffOverride.isOn];
    [self.hydraPacket addChecksum];
    
    NSAssert([self.hydraPacket isChecksumValid], @"hydra packet checksum fail");
}

- (void)firmwareVersionReport:(NSNotification *)notification {
    NSDictionary *userInfo = notification.userInfo;
    if (userInfo[@"version"]) {
        self.firmwareLabel.text = userInfo[@"version"];
    }
}

- (void)recievedSuccess:(NSNotification *) notification {
    [resendTimer invalidate];
    resendTimer = nil;
}

- (IBAction)swipeRight:(UISwipeGestureRecognizer *)sender {
    self.tabBarController.selectedIndex--;
}

- (IBAction)swipeLeft:(id)sender {
    self.tabBarController.selectedIndex++;
}

- (IBAction)edit:(id)sender {
    self.saveBtn.enabled = YES;
    [self.lowVoltageCutoff resignFirstResponder];
}

- (IBAction)lowCurrentInfo:(id)sender {
    NSString *title = NSLocalizedString(@"Low Current Mode", @"Low Current Mode Title");
    NSString *msg = NSLocalizedString(@"Each output on the Hydra can be configured to operate in high-efficiency, low-current mode. In this mode, the maximum output current is limited (on the order of 100 mA max per output), but the efficiency of each output at low currents is increased significantly.", @"Low Current Mode Explanation");
    
    UIAlertController *lowCurrentInfo = [UIAlertController alertControllerWithTitle:title message:msg preferredStyle:UIAlertControllerStyleActionSheet];
    
    NSString *okayTitle = NSLocalizedString(@"Okay", @"Okay button title");
    UIAlertAction *okay = [UIAlertAction actionWithTitle:okayTitle style:UIAlertActionStyleCancel handler:^(UIAlertAction * _Nonnull action) {
        
    }];
    [lowCurrentInfo addAction:okay];
    [self showViewController:lowCurrentInfo sender:sender];
}

- (IBAction)lowVoltageCutoffInfo:(id)sender {
    NSString *title = NSLocalizedString(@"Low Voltage Cutoff", @"Low Voltage Cutoff Title");
    NSString *msg = NSLocalizedString(@"The Hydra can be configured to automatically disable all of its outputs and transition to a low-power sleep mode if the input voltage drops below a user-defined threshold. This can be used to prevent the Hydra from over-discharging batteries. Once in sleep mode, the Hydra must be power-cycled to re-enabled the outputs even if the input voltage rises above the threshold again.", @"Low Voltage Cutoff Explanation");
    
    UIAlertController *lowCurrentInfo = [UIAlertController alertControllerWithTitle:title message:msg preferredStyle:UIAlertControllerStyleActionSheet];
    
    NSString *okayTitle = NSLocalizedString(@"Okay", @"Okay button title");
    UIAlertAction *okay = [UIAlertAction actionWithTitle:okayTitle style:UIAlertActionStyleCancel handler:^(UIAlertAction * _Nonnull action) {
        
    }];
    [lowCurrentInfo addAction:okay];
    [self showViewController:lowCurrentInfo sender:sender];
}

-(void)textFieldDidEndEditing:(UITextField *)textField{
    self.saveBtn.enabled = YES;
}

- (void)viewDidUnload {
    [self setSaveBtn:nil];
    [self setLowVoltageCutoff:nil];
    [super viewDidUnload];
}
@end
