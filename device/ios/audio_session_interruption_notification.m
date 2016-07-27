//
//  audio_session_interruption_notification.m
//  snail_real_audio
//
//  Created by 陳偉榮 on 16/6/24.
//  Copyright © 2016年 snail_audio. All rights reserved.
//

#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#import "audio_session_interruption_notification.h"
#import "audio_session_observer.h"
@implementation AudioSessionInterruptionNotification{
        BOOL _isActive;
        AVAudioSession* _session;
        AudioSessionObserver* _observer;
}

+ (instancetype)sharedInstance {
    static dispatch_once_t onceToken;
    static AudioSessionInterruptionNotification *sharedInstance = nil;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[AudioSessionInterruptionNotification alloc] init];
    });
    return sharedInstance;
}

- (instancetype)init {
    if (self = [super init]) {
        _session = [AVAudioSession sharedInstance];
        
        NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
        [center addObserver:self
                   selector:@selector(handleInterruptionNotification:)
                       name:AVAudioSessionInterruptionNotification
                     object:nil];
        [center addObserver:self
                   selector:@selector(handleRouteChangeNotification:)
                       name:AVAudioSessionRouteChangeNotification
                     object:nil];
        // TODO(tkchin): Maybe listen to SilenceSecondaryAudioHintNotification.
        [center addObserver:self
                   selector:@selector(handleMediaServicesWereLost:)
                       name:AVAudioSessionMediaServicesWereLostNotification
                     object:nil];
        [center addObserver:self
                   selector:@selector(handleMediaServicesWereReset:)
                       name:AVAudioSessionMediaServicesWereResetNotification
                     object:nil];
    }
    return self;
}

-(void)setObserver:(AudioSessionObserver*)observer{
    _observer = observer;
}

- (BOOL)isActive {
    @synchronized(self) {
        return _isActive;
    }
}

#pragma mark - Notifications

- (void)handleInterruptionNotification:(NSNotification *)notification {
    NSNumber* typeNumber =
    notification.userInfo[AVAudioSessionInterruptionTypeKey];
    AVAudioSessionInterruptionType type =
    (AVAudioSessionInterruptionType)typeNumber.unsignedIntegerValue;
    switch (type) {
        case AVAudioSessionInterruptionTypeBegan:
            NSLog(@"Audio session interruption began.");
            self->_isActive = NO;
            [self notifyDidBeginInterruption];
            break;
        case AVAudioSessionInterruptionTypeEnded: {
            NSLog(@"Audio session interruption ended.");
            [self updateAudioSessionAfterEvent];
            NSNumber *optionsNumber =
            notification.userInfo[AVAudioSessionInterruptionOptionKey];
            AVAudioSessionInterruptionOptions options =
            optionsNumber.unsignedIntegerValue;
            BOOL shouldResume =
            options & AVAudioSessionInterruptionOptionShouldResume;
            [self notifyDidEndInterruptionWithShouldResumeSession:shouldResume];
            break;
        }
    }
}

- (void)handleRouteChangeNotification:(NSNotification *)notification {
    // Get reason for current route change.
    NSNumber* reasonNumber =
    notification.userInfo[AVAudioSessionRouteChangeReasonKey];
    AVAudioSessionRouteChangeReason reason =
    (AVAudioSessionRouteChangeReason)reasonNumber.unsignedIntegerValue;
    NSLog(@"Audio route changed:");
    switch (reason) {
        case AVAudioSessionRouteChangeReasonUnknown:
            NSLog(@"Audio route changed: ReasonUnknown");
            break;
        case AVAudioSessionRouteChangeReasonNewDeviceAvailable:
            NSLog(@"Audio route changed: NewDeviceAvailable");
            break;
        case AVAudioSessionRouteChangeReasonOldDeviceUnavailable:
            NSLog(@"Audio route changed: OldDeviceUnavailable");
            break;
        case AVAudioSessionRouteChangeReasonCategoryChange:
            NSLog(@"Audio route changed: CategoryChange to :%@",
                  self->_session.category);
            break;
        case AVAudioSessionRouteChangeReasonOverride:
            NSLog(@"Audio route changed: Override");
            break;
        case AVAudioSessionRouteChangeReasonWakeFromSleep:
            NSLog(@"Audio route changed: WakeFromSleep");
            break;
        case AVAudioSessionRouteChangeReasonNoSuitableRouteForCategory:
            NSLog(@"Audio route changed: NoSuitableRouteForCategory");
            break;
        case AVAudioSessionRouteChangeReasonRouteConfigurationChange:
            NSLog(@"Audio route changed: RouteConfigurationChange");
            break;
    }
    AVAudioSessionRouteDescription* previousRoute =
    notification.userInfo[AVAudioSessionRouteChangePreviousRouteKey];
    // Log previous route configuration.
    NSLog(@"Previous route: %@\nCurrent route:%@",
          previousRoute, self->_session.currentRoute);
    [self notifyDidChangeRouteWithReason:reason previousRoute:previousRoute];
}

- (void)handleMediaServicesWereLost:(NSNotification *)notification {
    NSLog(@"Media services were lost.");
    [self updateAudioSessionAfterEvent];
    [self notifyMediaServicesWereLost];
}

- (void)handleMediaServicesWereReset:(NSNotification *)notification {
    NSLog(@"Media services were reset.");
    [self updateAudioSessionAfterEvent];
    [self notifyMediaServicesWereReset];
}



- (void)updateAudioSessionAfterEvent {

}

- (void)notifyDidBeginInterruption {
    if(_observer)
    {
        _observer->OnInterruptionBegin();
    }
}

- (void)notifyDidEndInterruptionWithShouldResumeSession:
(BOOL)shouldResumeSession {
     if(_observer)
     {
         _observer->OnInterruptionEnd();
     }
    
}

- (void)notifyDidChangeRouteWithReason:(AVAudioSessionRouteChangeReason)reason
                         previousRoute:(AVAudioSessionRouteDescription *)previousRoute {
    if(_observer)
    {
        _observer->OnValidRouteChange();
    }
}

- (void)notifyMediaServicesWereLost {

}

- (void)notifyMediaServicesWereReset {

}



@end