#pragma once

#include "discord-cpp/types.h"

namespace Discord
{
	DISCORDGAME_API inline FString StatusToString(const discord::Status Status)
	{
		switch (Status)
		{
		case discord::Status::Offline:
				return TEXT("Offline");
			case discord::Status::Online:
				return TEXT("Online");
			case discord::Status::Idle:
				return TEXT("Idle");
			case discord::Status::DoNotDisturb:
				return TEXT("Do Not Disturb");
		}
		
		return TEXT("");
	}
}
