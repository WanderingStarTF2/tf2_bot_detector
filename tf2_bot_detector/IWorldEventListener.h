#pragma once

#include "Clock.h"

#include <string_view>

namespace tf2_bot_detector
{
	class PlayerRef;
	class WorldState;

	// TODO move to other file
	class PlayerRef
	{
	public:
		class SteamID GetSteamID() const;
	};

	class IWorldEventListener
	{
	public:
		virtual ~IWorldEventListener() = default;

		virtual void OnUpdate(WorldState& world, bool consoleLinesUpdated) = 0;
		virtual void OnTimestampUpdate(WorldState& world) = 0;
		virtual void OnPlayerStatusUpdate(WorldState& world, const PlayerRef& player) = 0;
		virtual void OnChatMsg(WorldState& world, const PlayerRef& player, const std::string_view& msg) = 0;
	};

	class BaseWorldEventListener : public IWorldEventListener
	{
	public:
		void OnUpdate(WorldState& world, bool consoleLinesUpdated) override {}
		void OnTimestampUpdate(WorldState& world) override {}
		void OnPlayerStatusUpdate(WorldState& world, const PlayerRef& player) override {}
		void OnChatMsg(WorldState& world, const PlayerRef& player, const std::string_view& msg) override {}
	};
}
