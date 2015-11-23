//
//  DataLoggingViewController.m
//  Hydra
//
//  Created by User on 4/11/13.
//
//

#import "DataLoggingViewController.h"
#import <MessageUI/MFMailComposeViewController.h>
#import "AppDelegate.h"

@interface DataLoggingViewController ()<UINavigationControllerDelegate, MFMailComposeViewControllerDelegate>
@property (strong, nonatomic) IBOutlet UITextView *logView;

@end

@implementation DataLoggingViewController{
    int updateCounter;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self displayContent];
    updateCounter = 0;
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(gotUpdate)
                                                 name:@"recievedData"
                                               object:nil];
}

- (IBAction)clearLog:(id)sender {
    NSString *documentsDirectory = DOCUMENTS_FOLDER;
    NSString *fileName = [NSString stringWithFormat:@"%@/hydraLog.csv",
                          documentsDirectory];
    [@"" writeToFile:fileName
              atomically:NO
                encoding:NSStringEncodingConversionAllowLossy
                   error:nil];
    self.logView.text = @"";
}

-(void)gotUpdate{
    if (updateCounter > 20){
        updateCounter = 10;
        [self displayContent];
    }
    if (updateCounter < 10){
        [self displayContent];
    }
}
- (IBAction)startLoggingPressed:(UIButton *)sender {
    AppDelegate *delegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
    if(delegate.logData){
        delegate.logData = NO;
        [sender setTitle:@"Start Logging" forState:UIControlStateNormal];
    }else{
        delegate.logData = YES;
        [sender setTitle:@"Stop Logging" forState:UIControlStateNormal];
    }
}

-(void) writeToLogFile:(NSString*)content{
    //make a file name to write the data to using the documents directory:
    NSString *fileName = [NSString stringWithFormat:@"%@/hydraLog.csv",
                          DOCUMENTS_FOLDER];
    NSFileHandle *fileHandle = [NSFileHandle fileHandleForWritingAtPath:fileName];
    if (fileHandle){
        [fileHandle seekToEndOfFile];
        [fileHandle writeData:[content dataUsingEncoding:NSUTF8StringEncoding]];
        [fileHandle closeFile];
    }
    else{
        content = [NSString stringWithFormat:@"%@\n",content];
        [content writeToFile:fileName
                  atomically:NO
                    encoding:NSStringEncodingConversionAllowLossy
                       error:nil];
    }
}


-(void) displayContent{
    NSString *documentsDirectory = DOCUMENTS_FOLDER;
    NSString *fileName = [NSString stringWithFormat:@"%@/hydraLog.csv",
                          documentsDirectory];

    self.logView.text = [[NSString alloc] initWithContentsOfFile:fileName
                                                    usedEncoding:nil
                                                           error:nil];
    
}
- (IBAction)swipeRight:(UISwipeGestureRecognizer *)sender {
    self.tabBarController.selectedIndex--;
}

- (IBAction)emailPressed:(UIButton *)sender {
    NSString *documentsDirectory = DOCUMENTS_FOLDER;
    NSString *fileName = [NSString stringWithFormat:@"%@/hydraLog.csv",
                          documentsDirectory];
    
    NSData *attachmentData = [NSData dataWithContentsOfFile:fileName];
    if ([MFMailComposeViewController canSendMail]){
        MFMailComposeViewController *mailer = [[MFMailComposeViewController alloc] init];
        mailer.mailComposeDelegate = self;
        [mailer addAttachmentData:attachmentData mimeType:@"text/plain" fileName:@"HydraLog.csv"];
        [mailer setSubject:@"Hydra Log File"];
        [self presentModalViewController:mailer animated:YES];
    }
    else{
        [[[UIAlertView alloc] initWithTitle:@"Failure"
                                    message:@"Your device doesn't support in-app emails"
                                   delegate:nil
                          cancelButtonTitle:@"OK"
                          otherButtonTitles:nil] show];
    }
}
- (void)mailComposeController:(MFMailComposeViewController *)controller didFinishWithResult:(MFMailComposeResult)result error:(NSError *)error
{
    
    [self dismissModalViewControllerAnimated:YES];
    
}
- (void)viewDidUnload {
    [self setLogView:nil];
    [super viewDidUnload];
}
@end
