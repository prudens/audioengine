#pragma once
#include "message_queue.h"
#include "api/ISnailAudioEngine.h"
#include "api/ISnailAudioModule.h"
#include <vector>
namespace audio_engine
{
	class AudioClient : public IAudioRoomEventHandler,
		public IRealAudioEventHandler,
		public IMessageEventHandler,
		public IMicOrderEventHandler,
		public IUserEventHandler
	{
	public:
		AudioClient( int mid );
		~AudioClient();
		void Login( int roomid, std::string uid );
		void Logout();
		void GetUserList();
		int  GetLoginedStatus();
		bool IsRecordingMsg();
		bool IsPlayingMsg();
		void EnableSpeak( bool enable );
		void EnablePlay( bool enable );
		void RecordMsg( bool bstop, bool stt );
		void PlayMsg( const char* msgid, bool bStop );
		void GetMsgList( int index, int count );
		void StartSTT( const char* msgid );
		void CancelSTT();
		bool IsSTT();
		const char* GetUserid();
		void TurnOnSpeakingStatus( bool status, std::string uid );
		void SetRoomAttr( std::string key, std::string value );
		void GetRoomAttr( std::string key );
		void SetUserExtend( std::string uid, std::string value );
		void GetUserExtend( std::string uid );
		void KickOff( std::string uid );
		void BlockUser( std::string uid, bool enable );
		void DisableSpeak( std::string uid, bool enable );
	public:
		virtual void RespondLogin( const char* roomkey, UID uid, int ec )override;

		/**
		* @brief     Called when the network reconnect successfully.
		* @return    void
		* @param     const char * roomkey. the roomkey passed to Login().
		* @param     UID uid
		*            the UID passed to Login().
		*/
		virtual void NotifyReConnected( const char* roomkey, UID uid )override;

		/**
		* @brief     Called back when Logout() finish.
		* @return    void
		* @param     const char * roomkey. the roomkey passed  to Login().
		* @param     int ec.see error code.
		*/
		virtual void RespondLogout( const char* roomkey, UID uid, int ec )override;

		/**
		* @brief     notify a new user joined this room.
		* @return    void
		* @param     UID uid.the UID of the remote user
		*/
		virtual void NotifyUserEnterRoom( UID uid )override;

		/**
		* @brief     notify a user leave this room.
		* @return    void
		* @param     UID uid
		*            the UID of the remote user
		*/
		virtual void NotifyUserLeaveRoom( UID uid )override;

		/**
		* @brief     notify the user is speaking.
		* @return    void
		* @param     UID uid. the UID of the remote user or yourself.
		*/
		virtual void NotifyUserSpeaking( UID uid, int volume )override;
		/**
		* @brief     notify the network is lost.
		* @return    void
		*/
		virtual void NotifyConnectionLost()override;
		/**
		* @brief     notify this room is closed.
		* @return    void
		* @param     const char * roomkey.The roomkey passed to Login().
		* @param     int reason. see ROOM_CLOSE_SEASON
		*/
		virtual void NotifyRoomClose( const char* roomkey, int reason )override;
		/**
		* @brief     notify the room attribute had changed
		* @return    void
		* @param     const char * name
		* @param     const char * value
		*/
		virtual void NotifyRoomAttrChanged( const char* name, const char* value )override;
		/**
		* @brief     notify the user attribute had changed
		* @return    void
		* @param     const char * extend
		*
		*/
		/**
		* @brief     notify the user attribute had changed
		* @return    void
		* @param     UID uid. the attribute of user.
		* @param     const char * name
		* @param     const char * value
		*/
		virtual void NotifyUserExtendChanged( UID uid, const char* name, const char* value )override;

		/**
		* @brief     the sdk start recording audio message, response of the StartRecord().
		* @return    void
		*/
		virtual void NotifyAudioMsgRecordBegin()override;
		/**
		* @brief     Called when the function StopRecord is called.
		*            WARNING:an error occurred if call StartRecord(),sdk will report an error code by this function,and url is null.
		* @return    void
		* @param     const char * url. The audio message id that record audio successfully, is NULL if ec != 0.
		* @param     int ec. see error code.
		*/
		virtual void NotifyAudioMsgRecordEnd( const char* url, int ec )override;
		/**
		* @brief     Called if start playing a audio message.
		* @return    void
		* @param     const char * url.the audio message id that record audio message.
		*/
		virtual void NotifyAudioMsgPlayBegin( const char* url )override;
		/**
		* @brief     Called after playing url completely.
		*            WARNING: sdk will stop play url and report a error code by this function,
		*                     when an error occurred(url not found,unsupported format,etc. ).
		* @return    void
		* @param     const char * url. the audio message id that record audio message.
		* @param     int ec.see error code.
		*/
		virtual void NotifyAudioMsgPlayEnd( const char* url, int ec )override;

		/**
		* @brief     notify that receive a new message.
		*            NOTE: you maybe give a new type message, so you should parse it by yourself.
		* @return    void
		* @param     Message* msg.
		*/
		virtual void NotifyRecvMsg( const Message* msg )override;
		/**
		* @brief     Called back when Login() sucessfully or GetMessageList() is called.
		* @return    void
		* @param     msgarr_t * msglist. the list of history msg.
		*/
		virtual void RespondGetHitoryMsgList( int ec, const MessageList* messages )override;
		/**
		* @brief     Call back when SendMessage is called.
		* @return    void
		* @param     int msg_type.see eMsgType
		* @param     const char * data
		*            if type = text_msg, data is text, text format is utf8;
		*            if type = audio_msg,data is audio message url.
		* @param     int length.the length of data.
		* @param     int ec.see error code.
		*/
		virtual void RespondSendMsg( int msg_type, const char* data, int length, const char* extend, UID to_user, int msg_id, int ec )override;
		/**
		* @brief     Called back when AudioToText() is called.
		* @return    void
		* @param     const char * url.audio message id.
		* @param     int ec.see error code.
		*/
		virtual void RespondSpeechToText( const char* url, const char* text, int ec )override;

		/**
		* @brief     Called back when KickOff() is called.
		* @return    void
		* @param     UID uid. The user id passed to KickOff().
		* @param     int ec. see error code
		*/
		virtual void RespondKickOff( UID uid, int ec )override;

		/**
		* @brief     notify a user is kickoff. If the user is yours, the Caller must call Logout().
		* @return    void
		* @param     UID uid
		*/
		virtual void NotifyKiceOff( UID uid )override;

		/**
		* @brief     notify caller had a same uid logined on other devices.
		* @return    void
		*/
		virtual void NotifyDuplicateLogined()override;
		/**
		* @brief     Called back when DisableSpeaking() is called.
		* @return    void
		* @param     UID uid.passed to DisableSpeaking().
		* @param     bool disable. passed to DisableSpeaking().
		* @param     int ec see error code
		*/
		virtual void RespondDisableSpeaking( UID uid, bool disable, int ec )override;
		/**
		* @brief     notify a user disable(enable) speak.
		* @return    void
		* @param     UID uid
		* @param     bool disable
		*/
		virtual void NotifyDisableSpeaking( UID uid, bool disable )override;
		/**
		* @brief     Called back when SetRoomAttr() is called.
		* @return    void
		* @param     const char * name. passed to SetRoomAttr().
		* @param     const char * value.passed to SetRoomAttr().
		* @param     int ec.see error code
		*/
		virtual void RespondSetRoomAttr( const char* name, const char* value, int ec )override;
		/**
		* @brief     Called back when SetUserAttr() is called.
		* @return    void
		* @param     UID uid
		* @param     const char * name.passed to SetUserAttr().
		* @param     const char * value.passed to SetUserAttr().
		* @param     int ec
		*/
		virtual void RespondSetUserExtend( UID uid, const char* name, const char* value, int ec )override;
		/**
		* @brief     Called back when BlockUser() is called.
		* @return    void
		* @param     UID uid. passed to BlockUser().
		* @param     bool block.passed to BlockUser().
		* @param     int ec
		*/
		virtual void RespondBlockUser( UID uid, bool block, int ec )override;
	private:
		IAudioRoom *_room;
		IMessageModule* _message;
		IRealAudioModule* _real_audio;
		IMicOrderModule* _mic_order;
		IUserModule*   _user;
		IVolumeModule* _volume;
		int _mid;
		std::string _userid;
		bool _turn_on_speak = false;
		std::string _turn_on_speak_uid;
	};
}