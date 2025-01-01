#pragma once

#include <discord.h>

class ENGINE_API DiscordSDK final
{
	xr_string StatusDiscord_;
	xr_string PhaseDiscord_;

	std::atomic_bool NeedUpdateActivity_;

	discord::Activity ActivityDiscord_{};
	discord::Core* core{};
	bool discordAlreadyDeleted;
public:
	DiscordSDK() = default;
	~DiscordSDK();

	void InitSDK();

	void UpdateSDK();

	void UpdateActivity();

	void SetPhase(const xr_string& phase);
	void SetStatus(const xr_string& status);
};

extern ENGINE_API DiscordSDK Discord;
