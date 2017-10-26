///仅供内部开发人员调试使用


#ifndef INTERFACE_MODULE_DEBUG_OC_H
#define INTERFACE_MODULE_DEBUG_OC_H
#pragma once

#import <Foundation/Foundation.h>
#include "SnailAudioEngineHelper.h"
#ifndef INNER_MODULE_DEBUGGER
#define INNER_MODULE_DEBUGGER  151

#define TYPE_RECORD_DEV             0X01
#define TYPE_RECORD_WEBRTC          0X02
#define TYPE_RECORD_NS              0X04
#define TYPE_RECORD_VOICE_CHECK     0X08
#define TYPE_RECORD_BEFORE_ENCODE   0X10
#define TYPE_RECORD_SEND            0X20
#define TYPE_PLAYOUT_RECEIVE        0X40
#define TYPE_PLAYOUT_DECODE         0X80
#define TYPE_PLAYOUT_MIXED          0X100
#define TYPE_PLAYOUT_DEV            0X200

#define TYPE_RECORD_ALL  (TYPE_RECORD_DEV | TYPE_RECORD_WEBRTC | TYPE_RECORD_NS | TYPE_RECORD_VOICE_CHECK | TYPE_RECORD_BEFORE_ENCODE | TYPE_RECORD_SEND)
#define TYPE_PLAYOUT_ALL (TYPE_PLAYOUT_RECEIVE | TYPE_PLAYOUT_DECODE | TYPE_PLAYOUT_MIXED | TYPE_PLAYOUT_DEV)
#define TYPE_ALL         (TYPE_RECORD_ALL | TYPE_PLAYOUT_ALL)

#endif

@protocol DebugEventHandler <NSObject>
-(void)NotifyShowMessage:(NSString*)message;
@end


@interface DebugModule:Module

-(void)RegisterEventHandler:(id<DebugEventHandler>)handler;
-(int)EnableMonitorUserDelay:(bool) enable;
-(int)EnableMonitorNet:(bool)enable;
-(int)EnableSavePlayout:(bool)enable;
-(int)EnableSaveRecord:(bool)enable;
-(int)PlaySavedFile:(int)id Paly:(bool)enable;
-(int)PlayPause:(bool)enable;
@end
#endif//INTERFACE_MODULE_DEBUG_OC_H
