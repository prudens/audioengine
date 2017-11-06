#pragma once

//sdk  error code
#define ERR_OK                             0   // 
#define ERR_GENERIC                       -1   // 
#define ERR_NOT_INITIALIZE                -2   //
#define ERR_INVALID_ARGUMENT              -4
#define ERR_INVALID_AUTHO_KEY             -5
#define ERR_INVALID_ROOM_KEY              -6
#define ERR_INVALID_USER_ID               -7
#define ERR_NOT_SUPPORTED                 -8    // the function is not supported.
#define ERR_NOT_VERSION_SUPPORTED         -9    // the sdk is not supported ,please update sdk.
#define ERR_NOT_LOGINED                   -10   // user is not logined.
#define ERR_SERVER_CONNECT_FAILED         -11
#define ERR_TIME_OUT                      -12   // the async function is called timeout.
#define ERR_ALREADY_IN_USE                -13   // last operator has not finished,can not call it.
#define ERR_USER_NOT_FOUND                -15
#define ERR_MSG_NOT_FOUND                 -16
#define ERR_ROOM_NOT_FOUND                -17

// audio device manager error code
#define ERR_ADM_NO_FOUND_SPEAKER          -201
#define ERR_ADM_NO_FOUND_MICPHONE         -202
#define ERR_ADM_OPEN_SPEAKER_FAILED       -203
#define ERR_ADM_OPEN_MICPHONE_FAILED      -204
#define ERR_ADM_NO_VALID_DATA             -205   // can not record activity sound.
#define ERR_ADM_NO_RECORD_PERMISSION      -206   // need record permission.
#define ERR_ADM_TIME_TOO_SHORT            -207   // record audio message's time is too short to failed.
#define ERR_ADM_TIME_TOO_LONG             -208   // record audio message's time is too long to stop recording.

//network error code
#define ERR_NETWORK_POOL                  -301   // network is not so good.
#define ERR_NETWORK_BROKEN                -302   // network is broken.
#define ERR_NET_FILE_UPLOAD_FAILED        -303   // upload file failed.
#define ERR_NET_FILE_DOWNLOAD_FAILED      -304   // download file failed.

//local file error code
#define ERR_FILE_NOT_FOUND                -401   
#define ERR_FILE_OPEN_FAILED              -402