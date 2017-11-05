/*!
 * \file engine.cpp
 * \brief 语音控制引擎，负责处理MessageQueue
 *
 * \author zhangnaigan
 * \date 2017/02/04
 * \version 0.1.0
 */
#include "engine.h"
#include <iostream>
#include <thread>
namespace audio_engine{
	std::mutex g_mutex;
	void io_console( MessageQueue *queue );
	void io_android( MessageQueue *queue );
	void io_file( MessageQueue*msgqueue, std::string filename );

	void run_engine( std::string filename, std::string path )
	{

		Engine engine( filename, path );
		engine.Run();
	}

	Engine::Engine( std::string filename, std::string path )
	{
		_queue = new MessageQueue;
		_client_mgr = new ClientManager( path );


		if(strncmp( filename.c_str(), "file:", 5 ) == 0)
		{
			_thread = std::thread( std::bind( io_file, _queue, filename.substr( 5 ) ) );
		}
		else // by default, processing user action from console 
		{
#if defined( WIN32) || defined(_WIN32)
			_thread = std::thread( std::bind( io_console, _queue ) );
#elif _ANDROID
			_thread = std::thread( std::bind( io_android, _queue ) );
#else
#error "Please define a platform for running"
#endif

		}
	}

	Engine::~Engine()
	{
		//下面两行代码删除顺序不能调换
		delete _client_mgr;
		delete _queue;
		_thread.join();
	}

	void Engine::Run()
	{
		while(true)
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
			stMSG msg;
			if(_queue->GetMessage( &msg ))
			{
				switch(msg.cmd)
				{
				case  CMD_IO_NEW_CLIENT:
				{
					int mid = _client_mgr->NewClient( _queue );
					logprint( "nm:%d\n>>>", mid );
				}
				break;
				case CMD_IO_DELETE_CLIENT:
				{
					_client_mgr->DeleteClient( msg.mid );
				}
				break;
				case CMD_IO_LOGIN:
				{
					_client_mgr->Login( msg.mid, msg.rid, msg.uid );
				}
				break;
				case CMD_IO_LOGOUT:
				{
					_client_mgr->Logout( msg.mid );
				}
				break;
				case CMD_ASYNC_CALLBACK:
				{
					//_client_mgr->CallFromMainThread( msg.mid, msg.id );
				}
				break;
				case CMD_IO_UL:
				{
					_client_mgr->GetUserList( msg.mid );
				}
				break;
				case CMD_IO_LS:
				{
					LoginedStatus ls = static_cast<LoginedStatus>( _client_mgr->GetLoginedStatus( msg.mid ) );
					if(ls == LSLogined)
					{
						logprint( "已登录" );
					}
					else if(ls == LSLogining)
					{
						logprint( "正在登录" );
					}
					else
					{
						logprint( "未登录" );
					}
				}
				break;
				case CMD_IO_RS:
				{
					bool brec = _client_mgr->IsRecordingMsg( msg.mid );
					if(brec)
					{
						logprint( "当前正在录制语音消息" );
					}
					else
					{
						logprint( "当前无录制语音消息" );
					}
				}
				break;
				case CMD_IO_PS:
				{
					bool bply = _client_mgr->IsPlayingMsg( msg.mid );
					if(bply)
					{
						logprint( "当前正在播放语音消息" );
					}
					else
					{
						logprint( "当前无播放语音消息" );
					}
				}
				break;
				case CMD_IO_SPEAK:
				{
					_client_mgr->EnableSpeak( msg.mid, !msg.stop );
				}
				break;
				case CMD_IO_PLAY:
				{
					_client_mgr->EnablePlay( msg.mid, !msg.stop );
				}
				break;
				case CMD_IO_REC_MSG:
				{
					_client_mgr->RecordMsg( msg.mid, msg.stop, msg.stt );
				}
				break;
				case  CMD_IO_PLAY_MSG:
				{
					_client_mgr->PlayMsg( msg.mid, msg.msgid.c_str(), msg.stop );
				}
				break;
				case CMD_IO_MSG_LIST:
				{
					_client_mgr->GetMsgList( msg.mid, (int)msg.index, (int)msg.count );
				}
				break;
				case CMD_IO_STT:
				{
					if(msg.stop == false)
					{
						_client_mgr->StartSTT( msg.mid, msg.msgid.c_str() );
					}
					else
					{
						_client_mgr->CancelSTT( msg.mid );
					}
				}
				break;
				case CMD_IO_TS:
				{
					_client_mgr->TurnOnSpeakingStatus( msg.mid, !msg.stop, msg.uid );
				}
				break;
				case CMD_IO_ROOM_ATTR:
				{
					_client_mgr->SetRoomAttr( msg.mid, msg.key, msg.value );
				}break;
				case CMD_IO_USER_ATTR:
				{
					if(!msg.value.empty())
					{
						_client_mgr->SetUserExtend( msg.mid, msg.uid, msg.value );
					}
					else
					{
						_client_mgr->GetUserExtend( msg.mid, msg.uid );
					}

				}break;
				case  CMD_IO_KICK_OFF:
				{
					_client_mgr->KickOff( msg.mid, msg.uid );
				}
				break;
				case CMD_IO_BLOCK:
				{
					_client_mgr->BlockUser( msg.mid, msg.uid, !msg.stop );
				}break;
				case CMD_IO_DISABLE_SPEAK:
				{
					_client_mgr->DisableSpeak( msg.mid, msg.uid, !msg.stop );
				}break;
				case CMD_IO_QUIT:
				{
					return;
				}
				break;
				default:
					break;
				}
			}
		}
	}

	void Engine::SendCmd()
	{

	}


}