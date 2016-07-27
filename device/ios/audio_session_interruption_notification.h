//
//  audio_session_interruption_notification.h
//  snail_real_audio
//
//  Created by 陳偉榮 on 16/6/24.
//  Copyright © 2016年 snail_audio. All rights reserved.
//

#ifndef audio_session_interruption_notification_h
#define audio_session_interruption_notification_h

#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>

class AudioSessionObserver;

@interface AudioSessionInterruptionNotification : NSObject
+ (instancetype)sharedInstance;
-(void)setObserver:(AudioSessionObserver*)observer;
@end


#endif /* audio_session_interruption_notification_h */
