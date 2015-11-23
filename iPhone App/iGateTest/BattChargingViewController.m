//
//  BattChargingViewController.m
//  Hydra
//
//  Created by User on 5/8/13.
//
//

#import "BattChargingViewController.h"

@interface BattChargingViewController ()<UIPickerViewDataSource, UIPickerViewDelegate, UITextViewDelegate>
@property (strong, nonatomic) IBOutlet UIButton *pickerFinishedBtn;
@property (strong, nonatomic) IBOutlet UIPickerView *batteryTypePicker;
@property (strong, nonatomic) IBOutlet UIPickerView *channelPicker;
@property (strong, nonatomic) IBOutlet UIView *pickerHeaderView;

@property (strong, nonatomic) IBOutlet UIButton *batteryTypeBtn;
@property (strong, nonatomic) IBOutlet UIButton *channelBtn;

@property (strong, nonatomic) IBOutlet UITextField *voltageField;
@property (strong, nonatomic) IBOutlet UITextField *capacityField;
@property (strong, nonatomic) IBOutlet UITextField *chargeRateField;
@end

@implementation BattChargingViewController{
    NSArray *batteryChoices;
    NSArray *channelChoices;
}

- (IBAction)battTypeBtnPressed:(UIButton*)sender {
    self.channelPicker.hidden = YES;
    self.batteryTypePicker.hidden = NO;
    self.pickerHeaderView.hidden = NO;
    [self tap:nil];
}
- (IBAction)channelBtnPressed:(UIButton*)sender {
    self.batteryTypePicker.hidden = YES;
    self.channelPicker.hidden = NO;
    self.pickerHeaderView.hidden = NO;
    [self tap:nil];
}
- (IBAction)startBtnPressed:(UIButton *)sender {
    [self tap:nil];
}
- (IBAction)pickerDonePressed:(UIButton *)sender {
    self.batteryTypePicker.hidden = YES;
    self.channelPicker.hidden = YES;
    self.pickerHeaderView.hidden = YES;
}


-(NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView{
    return 1;
}
-(NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component{
    if (pickerView == self.batteryTypePicker){
        return batteryChoices.count;
    }
    else{
        return channelChoices.count;
    }
}
-(NSString *)pickerView:(UIPickerView *)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component{
    if (pickerView == self.batteryTypePicker){
        return [batteryChoices objectAtIndex:row];
    }
    else{
        return [channelChoices objectAtIndex:row];
    }
}
- (void)pickerView:(UIPickerView *)pickerView didSelectRow: (NSInteger)row inComponent:(NSInteger)component {
    if (pickerView == self.batteryTypePicker){
        [self.batteryTypeBtn setTitle:[batteryChoices objectAtIndex:row] forState:UIControlStateNormal];
        [self.batteryTypeBtn setTitle:[batteryChoices objectAtIndex:row] forState:UIControlStateSelected];
    }
    else{
        [self.channelBtn setTitle:[channelChoices objectAtIndex:row] forState:UIControlStateNormal];
        [self.channelBtn setTitle:[channelChoices objectAtIndex:row] forState:UIControlStateSelected];
    }
}


- (void)viewDidLoad
{
    [super viewDidLoad];
    self.pickerHeaderView.hidden = YES;
    self.batteryTypePicker.delegate = self;
    self.batteryTypePicker.showsSelectionIndicator = YES;
    self.batteryTypePicker.hidden = YES;
    self.channelPicker.delegate = self;
    self.channelPicker.showsSelectionIndicator = YES;
    self.channelPicker.hidden=YES;
    
    batteryChoices = [NSArray arrayWithObjects:@"Lead",@"Lithium",@"Aluminum",@"Petroleum", nil];
    channelChoices = [NSArray arrayWithObjects:@"Channel 1",@"Channel 2",@"Channel 3", nil];

    // Do any additional setup after loading the view.
}

- (IBAction)tap:(id)sender {
    [self.voltageField resignFirstResponder];
    [self.capacityField resignFirstResponder];
    [self.chargeRateField resignFirstResponder];
}

- (void)viewDidUnload {
    [self setPickerFinishedBtn:nil];
    [self setBatteryTypePicker:nil];
    [self setChannelPicker:nil];
    [self setPickerHeaderView:nil];
    [self setBatteryTypeBtn:nil];
    [self setChannelBtn:nil];
    [self setVoltageField:nil];
    [self setCapacityField:nil];
    [self setChargeRateField:nil];
    [super viewDidUnload];
}
@end
