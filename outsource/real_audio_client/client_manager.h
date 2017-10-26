#pragma once
#include<unordered_map>
#include <memory>
#include "audio_client.h"

class ClientManager
{
public:
    ClientManager(std::string logPath);
    ~ClientManager();
    int  NewClient( MessageQueue*queue );
    void DeleteClient(int mid );
    void Login(int mid = 1, int rid = 50,std::string uid="");
    void Logout(int mid = 1);
    void CallFromMainThread(int mid, void* id );
    void GetUserList(int mid);
    int  GetLoginedStatus(int mid);
    bool IsRecordingMsg(int mid);
    bool IsPlayingMsg(int mid);
    void EnableSpeak(int mid,bool enable);
    void EnablePlay(int mid, bool enable);
    void RecordMsg(int mid, bool bStop,bool stt);
    void PlayMsg( int mid, const char* msgid, bool bStop );
    void GetMsgList( int mid,int index,int count );
    void StartSTT(int mid,const char*msgid);
    void CancelSTT( int mid );
    bool IsSTT( int mid );
    const char* GetUserid(int mid);
    void TurnOnSpeakingStatus(int mid, bool status,std::string uid );
    void SetRoomAttr(int mid ,std::string key,std::string value);
    void SetUserAttr( int mid, std::string uid, std::string key, std::string value );
    void GetUserAttr(int mid, std::string uid, std::string key);
    void KickOff( int mid, std::string uid );
    void BlockUser(int mid, std::string uid, bool enable );
    void DisableSpeak( int mid, std::string uid, bool enable );
private:
    std::unordered_map< int, std::shared_ptr<AudioClient> > _audio_clients;
};