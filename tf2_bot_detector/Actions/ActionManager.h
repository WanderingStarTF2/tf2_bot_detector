#pragma once

#include "Clock.h"
#include "IConsoleLineListener.h"

#include <cppcoro/cancellation_source.hpp>
#include <cppcoro/cancellation_token.hpp>
#include <srcon.h>

#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace tf2_bot_detector
{
	enum class ActionType;
	class IAction;
	class IActionGenerator;
	class IPeriodicActionGenerator;
	class Settings;
	class WorldState;

	class ActionManager final
	{
	public:
		ActionManager(const Settings& settings);
		~ActionManager();

		void SetWorldState(WorldState* worldState) { m_WorldState = worldState; }

		void Update();

		std::string RunCommand(std::string cmd);
		std::shared_future<std::string> RunCommandAsync(std::string cmd);

		// Returns false if the action was not queued
		bool QueueAction(std::unique_ptr<IAction>&& action);

		template<typename TAction, typename... TArgs>
		bool QueueAction(TArgs&&... args)
		{
			return QueueAction(std::make_unique<TAction>(std::forward<TArgs>(args)...));
		}

		// Whenever another action triggers a send of command(s) to the game, these actions will be
		// given the chance to add themselves to the
		void AddPiggybackActionGenerator(std::unique_ptr<IActionGenerator>&& action);

		template<typename TAction, typename... TArgs>
		void AddPiggybackActionGenerator(TArgs&&... args)
		{
			return AddPiggybackActionGenerator(std::make_unique<TAction>(std::forward<TArgs>(args)...));
		}

		void AddPeriodicActionGenerator(std::unique_ptr<IPeriodicActionGenerator>&& action);

		template<typename TAction, typename... TArgs>
		void AddPeriodicActionGenerator(TArgs&&... args)
		{
			return AddPeriodicActionGenerator(std::make_unique<TAction>(std::forward<TArgs>(args)...));
		}

	private:
		// This is stupid
		struct InitSRCON
		{
			InitSRCON();
		} s_InitSRCON;

		std::timed_mutex m_RCONClientMutex;
		srcon::client m_RCONClient;
		std::thread m_RCONThread;

		struct RCONCommand
		{
			explicit RCONCommand(std::string cmd);

			bool operator==(const RCONCommand& other) const { return m_Command == other.m_Command; }

			std::string m_Command;
			std::shared_ptr<std::promise<std::string>> m_Promise;
			std::shared_future<std::string> m_Future;
		};
		std::queue<RCONCommand> m_RCONCommands;
		std::mutex m_RCONCommandsMutex;
		cppcoro::cancellation_source m_RCONCancellationSource;
		void RCONThreadFunc(cppcoro::cancellation_token cancellationToken);

		struct Writer;
		bool SendCommandToGame(std::string cmd);

		static constexpr duration_t UPDATE_INTERVAL = std::chrono::seconds(1);

		struct QueuedAction
		{
			std::unique_ptr<IAction> m_Action;
			uint32_t m_UpdateIndex;
		};

		WorldState* m_WorldState = nullptr;
		const Settings* m_Settings = nullptr;
		time_point_t m_LastUpdateTime{};
		std::vector<std::unique_ptr<IAction>> m_Actions;
		std::vector<std::unique_ptr<IActionGenerator>> m_PiggybackActionGenerators;
		std::vector<std::unique_ptr<IPeriodicActionGenerator>> m_PeriodicActionGenerators;
		std::map<ActionType, time_point_t> m_LastTriggerTime;
	};
}
