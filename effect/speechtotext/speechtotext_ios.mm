#include "speechtotext_ios.h"

#import "iflyMSC/iflyMSC.h"


@interface IflyAsr : NSObject<IFlySpeechRecognizerDelegate>

-(instancetype) InitWithAppid:(const char*)appid;
-(bool) start;
-(void) stop;
-(bool) writeAudio:( void*)audioSample length:(int)length;
-(int)  getResult:(std::string&) strResult;
@end


@implementation IflyAsr

static bool init_module_ = false;
static NSString * _cloudGrammerid =nil;//在线语法grammerID
bool        has_result_ = false;
bool        has_error_  = false;
NSMutableString *curResult;//当前session的结果
IFlySpeechRecognizer *_iFlySpeechRecognizer;//语法识别对象
-(instancetype) InitWithAppid:(const char*)appid
{
    if(!init_module_)
    {
        init_module_ = true;
        NSString *initString = [[NSString alloc] initWithFormat:@"appid=%s",appid];
        [IFlySpeechUtility createUtility:initString];
    }
    curResult = [[NSMutableString alloc]init];
    [self initRecognizer];
    return self;
}

-(bool) start
{
    has_result_ = false;
    has_error_  = false;
    return [_iFlySpeechRecognizer startListening] == YES;
}

-(void) stop
{
    [_iFlySpeechRecognizer stopListening];
}

-(void) cancel
{
    [_iFlySpeechRecognizer cancel];
}

-(bool) writeAudio:( void*)audioSample length:(int)length
{
    NSData* data = [NSData dataWithBytes:audioSample length:length];
    return [_iFlySpeechRecognizer writeAudio:data] == YES;
}

-(int)  getResult:(std::string&) strResult
{
    if(curResult.length>0)
    {
        strResult = [curResult UTF8String];
    }
    if( has_result_)
    {
        return 1;
    }
    else if (has_error_)
    {
        return -1;
    }
    else
    {
        return 0;
    }
    
}

#pragma mark - IFlySpeechRecognizerDelegate begin

/**
 * 音量变化回调
 * volume   录音的音量，音量范围0~30
 ****/
- (void) onVolumeChanged: (int)volume
{
    NSString * vol = [NSString stringWithFormat:@"音量：%d",volume];
    //NSLog(@"%@",vol);
}

/**
 开始识别回调
 ****/
- (void) onBeginOfSpeech
{
    //NSLog(@"%s","正在录音");
    //    [_popUpView showText:@"正在录音"];
}

/**
 停止识别回调
 ****/
- (void) onEndOfSpeech
{
    //NSLog(@"%s","停止录音");
    //    [_popUpView showText: @"停止录音"];
}



/**
 识别结果回调（注：无论是否正确都会回调）
 error.errorCode =
 0     听写正确
 other 听写出错
 ****/
- (void) onError:(IFlySpeechError *) error
{
    //NSLog(@"error=%d",[error errorCode]);
    
    NSString *text ;
    
    if (error.errorCode ==0 ) {
        
        if (curResult.length==0 || [curResult hasPrefix:@"nomatch"])
        {
            text = @"无匹配结果";
        }
        else
        {
            text = @"识别成功";
        }
    }
    else
    {
        text = [NSString stringWithFormat:@"发生错误：%d %@",error.errorCode,error.errorDesc];

        has_error_ = true;
    }
   // NSLog(@"识别结果%@",text);
}

-(void) onCancel
{
    //NSLog(@"取消语音识别");
}

/**
 识别结果回调
 result 识别结果，NSArray的第一个元素为NSDictionary，
 NSDictionary的key为识别结果，value为置信度
 isLast：表示最后一次
 ****/
- (void) onResults:(NSArray *) results isLast:(BOOL)isLast
{
    NSMutableString *result = [[NSMutableString alloc] init];
    NSDictionary *dic = results[0];
    
    for (NSString *key in dic) {
        
        [result appendFormat:@"%@",key];
    }
    if (isLast) {
        has_result_ = true;
        //NSLog(@"result is:%@",curResult);
    }
    
    [curResult appendString:result];
    
}


#pragma mark - IFlySpeechRecognizerDelegate end



/**
 设置识别参数
 ****/
-(void)initRecognizer
{
    //语法识别实例
    
    //单例模式，无UI的实例
    if (_iFlySpeechRecognizer == nil) {
        _iFlySpeechRecognizer = [IFlySpeechRecognizer sharedInstance];
    }
    _iFlySpeechRecognizer.delegate = self;
    
    if (_iFlySpeechRecognizer != nil)
    {
        //设置听写模式
        [_iFlySpeechRecognizer setParameter:@"iat" forKey:[IFlySpeechConstant IFLY_DOMAIN]];
        //设置听写结果格式为plain
        [_iFlySpeechRecognizer setParameter:@"plain" forKey:[IFlySpeechConstant RESULT_TYPE]];
        //参数意义与IATViewController保持一致，详情可以参照其解释
        [_iFlySpeechRecognizer setParameter:@"30000" forKey:[IFlySpeechConstant SPEECH_TIMEOUT]];
        [_iFlySpeechRecognizer setParameter:@"3000"  forKey:[IFlySpeechConstant VAD_EOS]];
        [_iFlySpeechRecognizer setParameter:@"3000"  forKey:[IFlySpeechConstant VAD_BOS]];
        [_iFlySpeechRecognizer setParameter:@"16000" forKey:[IFlySpeechConstant SAMPLE_RATE]];
        [_iFlySpeechRecognizer setParameter:@"zh_cn" forKey:[IFlySpeechConstant LANGUAGE]];
        [_iFlySpeechRecognizer setParameter:@"普通话" forKey:[IFlySpeechConstant ACCENT]];
        [_iFlySpeechRecognizer setParameter:@"-1"    forKey:[IFlySpeechConstant AUDIO_SOURCE]];
    }
}
@end



class SpeechToTextIOSImpl
{
public:
    SpeechToTextIOSImpl(std::string appid)
    {
        asr_ = [[IflyAsr alloc] InitWithAppid:appid.c_str()];
    }
    int Write( const char* audioSample, std::size_t nSamples )
    {
        if(stop_)
        {
            return -1;
        }
       // return 1;
        bool ret = [asr_ writeAudio:(void*)audioSample length:(int)nSamples];
        return ret;
    }
    
    int GetResult(std::string &strText)
    {
        //return 1;
        return [asr_ getResult:strText];
    }
    
    bool Start()
    {
       // return false;
        return [asr_ start];
    }
    
    void Stop()
    {
        stop_ = true;
        [asr_ stop];
    }
    
    bool isStop()
    {
        return stop_;
    }
    
    void Cancel()
    {
        [asr_ cancel];
    }
private:
    IflyAsr* asr_;
    bool stop_= false;
};



SpeechToTextIOS::SpeechToTextIOS(std::string appid)
{
    impl_ = new SpeechToTextIOSImpl(appid);
}


SpeechToTextIOS::~SpeechToTextIOS()
{
    delete impl_;
}
int SpeechToTextIOS::Write( const char* audioSample, std::size_t nSamples )
{
    return impl_->Write(audioSample, nSamples);
}

int SpeechToTextIOS::GetResult(std::string &strText)
{
    return impl_->GetResult(strText);
}

bool SpeechToTextIOS::Start()
{
    return impl_->Start();
}

void SpeechToTextIOS::Finish()
{
    impl_->Stop();
}

void SpeechToTextIOS::Cancel()
{
    impl_->Cancel();
}
bool SpeechToTextIOS::isStop()
{
    return impl_->isStop();
}
