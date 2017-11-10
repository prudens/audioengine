//
//  SnailRealAudio.m
//  Created by 张乃淦 on 16/8/25.
//  Copyright © 2016年 snail_audio. All rights reserved.

#ifndef SNAIL_AUDIO_ENGINE_OC_H
#define SNAIL_AUDIO_ENGINE_OC_H
#import <Foundation/Foundation.h>
#include "SnailAudioEngineHelper.h"
#ifdef __cplusplus
extern "C"{
#endif
    int  InitAudioSDK( const char* wkfolder, int region );
    void CleanAudioSDK();
    
    int  SetAuthoKey( const char* authokey );
    
    int  SetLogLevel( int level );
    
    int  SetLocale( const char* locale );
    
    const char* GetErrorDescription( int ec );
    
    const char* GetVersion();
#ifdef __cplusplus
}
#endif
        
@interface Message : NSObject
@property()int       MsgID;
@property(strong, nonatomic,nonnull)   NSString* UserID;
@property(strong, nonatomic,nonnull)   NSString* Extends;
@property( nonatomic)                  int64_t   MsgTime;
@property()           int              MsgType;
@property(strong,nonatomic,nonnull)    NSString* Content;
@end

@interface User : NSObject
@property(strong, nonatomic,nonnull) NSString* UserID;
@property(strong, nonatomic,nullable)NSString* Extends;
@property()bool IsDisableSpeak;
@property() bool IsBlocked;
@end

@interface Module : NSObject
-(int)Id;
-( nonnull NSString*)Description;
@end

@protocol AudioRoomEventHandler<NSObject>
-(void)RespondLogin:( nonnull NSString*)roomkey UserID:(nonnull NSString*)uid ErrorCode:(int)ec;
-(void)RespondLogout:(nonnull NSString*)roomkey UserID:( nonnull NSString*)uid ErrorCode:(int)ec;
-(void)RespondSetRoomAttr:(nonnull NSString*)name Value:( nonnull NSString*)value ErrorCode:(int)ec;
-(void)NotifyRoomAttrChanged:( nonnull NSString*)name Value:(nullable NSString*)value;
-(void)NotifyConnectionLost;
-(void)NotifyReConnected:(nonnull NSString*)roomkey UserID:( nonnull NSString*)uid;
-(void)NotifyRoomClose:(nonnull NSString*)roomkey Reason:(int)reason;
-(void)NotifyDuplicateLogined;
@end
@interface AudioRoom:NSObject
-(void)Release:(bool)sync;
-(void)RegisterEventHandler:(nonnull id<AudioRoomEventHandler>)handler UsePoll:(bool)usePoll;
-(nullable Module*) GetModule:(int)id;
-(int)Login:(nonnull NSString*)roomkey UserID:(nonnull NSString*)uid UserExtend:( nullable NSString*) userExtend;
-(int)Logout;
-(int)GetLoginStatus;
-(int)SetRoomAttr:(nonnull NSString*)name Value:(nullable NSString*)value;
-(int)GetRoomAttr:(nonnull NSString*)name Value:(nonnull NSString*)value;
-(void)Poll;
@end


@protocol UserEventHandler<NSObject>
-(void)RespondSetUserAttr:(nonnull NSString*)uid Name:(nonnull NSString*)name Value:(nullable NSString*)value ErrorCode:(int)ec;
-(void)NotifyUserAttrChanged:(nonnull NSString*)uid Name:(nonnull NSString*)name Value:(nullable NSString*)value;
-(void)NotifyUserEnterRoom:(nonnull NSString*)uid;
-(void)NotifyUserLeaveRoom:(nonnull NSString*)uid;
-(void)NotifyUserSpeaking:(nonnull NSString*)uid Volume:(int)volume;
-(void)RespondKickOff:(nonnull NSString*)uid ErrorCode:(int)ec;
-(void)NotifyKiceOff:(nonnull NSString*)uid;
@end
@interface UserModule : Module
-(void)RegisterEventHandler:(id<UserEventHandler>)handler;
-(int)SetUserAttr:(nonnull NSString*)uid Name:(nonnull NSString*)name Value:(nullable NSString*)value;
-(int)GetUserCount;
-(int)GetUserList:(nullable NSMutableArray<User*>*)userList;
-(int)GetUserByID:(nonnull NSString*)uid User:(nullable User*)user;
-(int)KickOff:(nonnull NSString*)uid;
@end



@protocol RealAudioEventHandler <NSObject>
-(void)RespondDisableSpeaking:(nonnull NSString*)uid Disable:(bool)disable ErrorCodec:(int)ec;
-(void)NotifyDisableSpeaking:(nonnull NSString*)uid Disable:(bool)disable;
-(void)RespondBlockUser:(nonnull NSString*)uid Block:(bool)block ErrorCode:(int)ec;
@end
@interface RealAudioModule : Module
-(void)RegisterEventHandler:(nonnull id<RealAudioEventHandler>)handler;
-(void)EnableSpeak:(bool)enable;
-(int)IsEnableSpeak;
-(void)EnablePlayout:(bool)enable;
-(int)IsEnablePlayout;
-(int)BlockUser:(nonnull NSString*)uid Block:(bool)block;
-(int)DisableSpeaking:(nonnull NSString*)uid DisSpeak:(bool) disable;
@end




@protocol MessageEventHandler <NSObject>
-(void)NotifyRecvHistoryMsgList:(nonnull NSArray<Message*>*)messages;
-(void)NotifyAudioMsgRecordBegin;
-(void)NotifyAudioMsgRecordEnd:(nullable NSString*)msgUrl ErrorCode:(int)ec;
-(void)NotifyAudioMsgPlayBegin:(nonnull NSString*)msgUrl;
-(void)NotifyAudioMsgPlayEnd:(nullable NSString*)msgUrl ErrorCode:(int)ec;
-(void)RespondSendMsg:(int)msgType Data:(nonnull NSString*)data Extend:(nullable NSString*)extend UserID:(nullable NSString*)toUser MsgID:(int)id ErrorCode:(int)ec;
-(void)NotifyRecvMsg:(nonnull Message*)msg;
-(void)RespondGetHitoryMsgList:(nullable NSArray<Message*>*)msgList ErrorCode:(int)ec;
-(void)RespondSpeechToText:(nonnull NSString*)msgUrl Text:(nullable NSString*)text ErrorCode:(int)ec;
@end
@interface MessageModule : Module
-(void)RegisterEventHandler:(nonnull id<MessageEventHandler>)handler;
-(int)StartRecord:(bool)syncSoundToText;
-(void)StopRecord:(bool)cancel;
-(bool)IsRecordingAudioMsg;
-(int)StartPlayout:(nonnull NSString*)msgUrl;
-(void)StopPlayout;
-(bool)IsPlayingAudioMsg;
-(int)GetAudioMsgTimeSpan:(nonnull NSString*)msgUrl;
-(bool)IsAudioMsgLocalExsit:(nonnull NSString*)msgUrl;
-(int)SendMsg:(int)msgType Data:(nonnull NSString*)data Extend:(nullable NSString*) extend SendToUser:(nullable NSString*)toUser;
-(int)GetHistoryMsgList:(int)msgId MsgCount:(int)count;
-(int)StartSpeechToText:(nonnull NSString*)msgUrl;
-(void)StopSpeechToText:(bool)cancel;
-(bool)IsSpeechToTextNow;
-(nullable NSString*)GetTextOfSpeech:(nonnull NSString*)msgUrl;
@end

@interface VolumeModule: Module
-(int)AdjustAudioVolume:(nonnull NSString*)uid Volume:(int)volume;
-(int)AdjustPlaybackVolume:(int)volume;
-(int)AdjustRecordingVolume:(int)volume;
-(int)GetPlaybackDeviceVolume;
@end

@protocol MicOrderEventHandler <NSObject>
-(void)NotifyMicOrderUserJoin:(nonnull NSString*)uid;
-(void)RespondAddToMicList:(nonnull NSString*)uid ErrorCode:(int)ec;
-(void)NotifyMicOrderUserLeave:(nonnull NSString*)uid;
-(void)RespondRemoveFromMicList:(nonnull NSString*)uid ErrorCode:(int)ec;
-(void)RespondSetMicOrder:(nonnull NSString*)uid Order:(int)order;
-(void)NotifyMicOrderUserChanged:(nonnull NSString*)uid;
-(void)NotifyMicOrderChanged;
-(void)RespondMicOrderClear;
-(void)NotifyMicOrderClear;
@end

@interface MicOrderModule : Module
-(void)RegisterEventHandler:(nonnull id<MessageEventHandler>)handler;
-(int)AddToMicList:(nonnull NSString*)uid;
-(int)RemoveFromMicList:(nonnull NSString*)uid;
-(int)SetMicOrder:(nonnull NSString*)uid Order:(int)order;
-(int)GetMicOrder:(nonnull NSString*)uid;
-(int)GetMicUserCount;
-(int)GetMicUserList:(nullable NSArray<User*>*)userList;
-(int)GetMicUser:(nonnull NSString*)uid User:(nullable User*)user;
@end

#endif
