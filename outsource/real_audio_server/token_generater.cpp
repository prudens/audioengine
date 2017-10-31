#include "token_generater.h"
namespace audio_engine{
	int64_t token_number = 0;
	struct Token
	{
		int64_t Generate( std::string udserid )
		{
			token_number++;
			return token_number;
		}
		bool Check( int64_t token )
		{
			return true;
		}
	};
	Token s_token;
	int64_t TokenGenerater::NewToken( std::string userid )
	{
		return s_token.Generate( userid );
	}

	bool TokenGenerater::CheckToken( int64_t token )
	{
		return s_token.Check( token );
	}
}