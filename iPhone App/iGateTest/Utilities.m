//
//  Utilities.m
//  Hydra
//
//  Created by User on 4/16/13.
//
//

#import "Utilities.h"

@implementation Utilities

+ (UIView *)overlayView{
    UIScreen *screen = [UIScreen mainScreen];
    CGRect fullScreen = screen.bounds;
    UIView *view = [[UIView alloc] initWithFrame:fullScreen];
    view.backgroundColor = [[UIColor darkGrayColor] colorWithAlphaComponent:0.85];
    UIActivityIndicatorView *activity = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhiteLarge];
    activity.center = view.center;
    [view addSubview:activity];
    [activity startAnimating];
    return view;
}

+ (UIView *)overlayViewWithTitle:(NSString *)title {
    UIScreen *screen = [UIScreen mainScreen];
    CGRect fullScreen = screen.bounds;
    UIView *view = [[UIView alloc] initWithFrame:fullScreen];
    view.backgroundColor = [[UIColor darkGrayColor] colorWithAlphaComponent:0.85];
    UIActivityIndicatorView *activity = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhiteLarge];
    activity.center = view.center;
    [view addSubview:activity];
    [activity startAnimating];
    
    UILabel *label = [[UILabel alloc] initWithFrame:CGRectZero];
    label.text = title;
    
    return view;
}

+ (NSString *) stringToHex:(NSString *)str
{
    NSUInteger len = [str length];
    unichar *chars = malloc(len * sizeof(unichar));
    [str getCharacters:chars];
    
    NSMutableString *hexString = [[NSMutableString alloc] init];
    
    for(NSUInteger i = 0; i < len; i++ )
    {
        // [hexString [NSString stringWithFormat:@"%02x", chars[i]]]; /*previous input*/
        [hexString appendFormat:@"%02x", chars[i]]; 
    }
    free(chars);
    
    return hexString;
}

@end
