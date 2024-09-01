#pragma once
#include "tetris_core.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXUserAgent.h>
#include "json.hpp"
#include "utils.hpp"
#include <boost/algorithm/string/join.hpp>
#pragma comment(lib, "ixwebsocket.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Secur32.lib")
#pragma comment(lib, "Ws2_32.lib")

using namespace TetrisAI;
using json = nlohmann::json;

namespace Payload
{
	const std::string command[10] = {"hold", "move_left", "move_right", "sonic_left", "sonic_right", "rotate_cw", "rotate_ccw", "drop", "sonic_drop", "none"};
	typedef std::string SessionId;
	struct PlayerInfo
	{
		std::string userId;
		std::string creator;
		std::string bot;
	};
	struct PieceData
	{
		char piece;
		int x;
		int y;
		int rotation;
	};
	struct GameState
	{
		std::vector<std::vector<char>> board;
		std::deque<char> queue;
		std::deque<int> garbageQueued;
		char held;
		PieceData current;
		bool canHold;
		int combo;
		bool b2b;
		int score;
		int piecesPlaced;
		bool dead;
	};
	struct PlayerData
	{
		SessionId sessionId;
		bool playing;
		PlayerInfo info;
		int wins;
		GameState state;
	};
	struct RoomData
	{
		std::string id;
		double initialMultiplier;
		double finalMultiplier;
		int startMargin;
		int endMargin;
		bool gameOngoing;
		bool roundOngoing;
		int startedAt;
		int endedAt;
	};
	struct RequestMove
	{
		GameState gameState;
		std::vector<PlayerData> players;
	};
	struct RoundStarted
	{
		int startsAt;
		RoomData roomData;
	};
}

class Botris_Client
{
	using pfunc = void (Botris_Client::*)(json);
	TetrisConfig config;
	TetrisParam param;
	Payload::SessionId session_id;
	Payload::RoomData room_data;

private:
	std::string API_TOKEN;
	std::string ROOM_KEY;
	ix::WebSocket web_socket;

	uint8_t translate_mino(std::string mino)
	{
		switch (mino[0])
		{
		case 'S':
			return S;
		case 'Z':
			return Z;
		case 'J':
			return J;
		case 'L':
			return L;
		case 'T':
			return T;
		case 'O':
			return O;
		case 'I':
			return I;
		}
		return EMPTY;
	}

	std::vector<std::string> translate_command(std::string &path)
	{
		std::vector<std::string> translated_path;
		for (auto &p : path)
		{
			switch (p)
			{
			case 'v':
				translated_path.push_back("hold");
				break;
			case 'L':
				translated_path.push_back("sonic_left");
				break;
			case 'R':
				translated_path.push_back("sonic_right");
				break;
			case 'l':
				translated_path.push_back("move_left");
				break;
			case 'r':
				translated_path.push_back("move_right");
				break;
			case 'd':
				translated_path.push_back("drop");
				break;
			case 'D':
				translated_path.push_back("sonic_drop");
				break;
			case 'c':
				translated_path.push_back("rotate_cw");
				break;
			case 'z':
				translated_path.push_back("rotate_ccw");
				break;
			}
		}
		return translated_path;
	}

	void read_config()
	{
		FILE* file = fopen("best_param.txt", "r");
		if (file == NULL) {
			utils::println(utils::ERR, " -> Failed to open best_param.txt");
			return;
		}
		for (int i = 0; i < END_OF_PARAM; ++i)
		{
			fscanf(file, "%lf\n", &param.weight[i]);
		}
		fclose(file);
	}

	void data_safeshift(json &data, std::string target)
	{
		if (data[target].is_null())
		{
			utils::println(utils::WARN, " -> Cancelled data shift, reason: " + target + " is null");
			return;
		}
		data = data[target];
	}

	void str_safeassign(json &data, std::string &target, std::string key)
	{
		if (data[key].is_null())
		{
			utils::println(utils::WARN, " -> Cancelled data assignment, reason: " + key + " is null");
			return;
		}
		target = data[key];
	}

	void int_safeassign(json &data, int &target, std::string key)
	{
		if (data[key].is_null())
		{
			utils::println(utils::WARN, " -> Cancelled data assignment, reason: " + key + " is null");
			return;
		}
		target = data[key];
	}

	void double_safeassign(json &data, double &target, std::string key)
	{
		if (data[key].is_null())
		{
			utils::println(utils::WARN, " -> Cancelled data assignment, reason: " + key + " is null");
			return;
		}
		target = data[key];
	}

	void bool_safeassign(json &data, bool &target, std::string key)
	{
		if (data[key].is_null())
		{
			utils::println(utils::WARN, " -> Cancelled data assignment, reason: " + key + " is null");
			return;
		}
		target = data[key];
	}

	void update_room_data(json data)
	{
		utils::println(utils::INFO, " -> Updating room data");
		data_safeshift(data, "payload");
		data_safeshift(data, "roomData");
		str_safeassign(data, room_data.id, "id");
		double_safeassign(data, room_data.initialMultiplier, "initialMultiplier");
		double_safeassign(data, room_data.finalMultiplier, "finalMultiplier");
		int_safeassign(data, room_data.startMargin, "startMargin");
		int_safeassign(data, room_data.endMargin, "endMargin");
		bool_safeassign(data, room_data.gameOngoing, "gameOngoing");
		bool_safeassign(data, room_data.roundOngoing, "roundOngoing");
		int_safeassign(data, room_data.startedAt, "startedAt");
		int_safeassign(data, room_data.endedAt, "endedAt");
		utils::println(utils::INFO, " -> Room data updated");
	}

	void handle_msg_room_data(json data)
	{
		update_room_data(data);
	}
	void handle_msg_authenticated(json data)
	{
		data_safeshift(data, "payload");
		str_safeassign(data, session_id, "sessionId");
		utils::println(utils::INFO, " -> Authenticated with session id: " + session_id);
	}
	void handle_msg_player_joined(json data)
	{
		data_safeshift(data, "payload");
		data_safeshift(data, "playerData");
		data_safeshift(data, "info");
		std::string bot, creator;
		str_safeassign(data, bot, "bot");
		str_safeassign(data, creator, "creator");
		utils::println(utils::INFO, " -> Player joined: " + bot + " (" + creator + ")");
	}
	void handle_msg_player_left(json data)
	{
		data_safeshift(data, "payload");
		std::string session;
		str_safeassign(data, session, "sessionId");
		utils::println(utils::INFO, " -> Player left with sessionId " + session);
	}
	void handle_msg_player_banned(json data)
	{
		data_safeshift(data, "payload");
		data_safeshift(data, "playerData");
		data_safeshift(data, "info");
		std::string bot, creator;
		str_safeassign(data, bot, "bot");
		str_safeassign(data, creator, "creator");
		utils::println(utils::INFO, " -> Player banned: " + bot + " (" + creator + ")");
	}
	void handle_msg_player_unbanned(json data)
	{
		data_safeshift(data, "payload");
		data_safeshift(data, "playerData");
		data_safeshift(data, "info");
		std::string bot, creator;
		str_safeassign(data, bot, "bot");
		str_safeassign(data, creator, "creator");
		utils::println(utils::INFO, " -> Player unbanned: " + bot + " (" + creator + ")");
	}
	void handle_msg_settings_changed(json data)
	{
		utils::println(utils::INFO, " -> Settings changed");
		update_room_data(data);
	}
	void handle_msg_host_changed(json data)
	{
		data_safeshift(data, "payload");
		data_safeshift(data, "playerData");
		data_safeshift(data, "info");
		std::string bot, creator;
		str_safeassign(data, bot, "bot");
		str_safeassign(data, creator, "creator");
		utils::println(utils::INFO, " -> Host changed: " + bot + " (" + creator + ")");
	}
	void handle_msg_game_started(json data)
	{
		utils::println(utils::INFO, " -> Game started");
	}
	void handle_msg_round_started(json data)
	{
		update_room_data(data);
		data_safeshift(data, "payload");
		int startsAt;
		int_safeassign(data, startsAt, "startsAt");
		utils::println(utils::INFO, " -> Round starts at: " + std::to_string(startsAt));
	}
	void handle_msg_request_move(json data)
	{
		data_safeshift(data, "payload");
		data_safeshift(data, "gameState");
		config.allow_x = false;
		TetrisMap map(10, 40);
		for (int i = 0; i < 24; ++i)
		{
			for (int j = 0; j < 10; ++j)
			{
				if (!data["board"][i][j].is_null())
				{
					map.mutate(j, i);
				}
			}
		}
		// for (int i = 20; i >= 0; --i) {
		// 	printf("%2d ", i);
		// 	for (int j = 0; j < 10; ++j) {
		// 		printf("%s", map.full(j, i) ? "[]" : "  ");
		// 	}
		// 	printf("\n");
		// }
		map.scan();
		TetrisNextManager next_manager(config);
		std::queue<uint8_t> queue;
		for (auto &i : data["queue"])
		{
			queue.push(translate_mino(i));
		}
		next_manager.active = TetrisActive(config.default_x, config.default_y, config.default_r, translate_mino(data["current"]["piece"]));
		if (data["held"].is_null())
		{
			next_manager.hold = EMPTY;
		}
		else
		{
			next_manager.hold = translate_mino(data["held"]);
		}
		next_manager.queue = queue;
		config.can_hold = data["canHold"];
		int last_delay = -1;
		int16_t total = 0;
		std::random_device rd;
		std::uniform_int_distribution<> dis(0, map.width - 1);
		std::uniform_int_distribution<> mess_dis(0, 99);
		std::mt19937 gen(rd());
		TetrisPendingLineManager pending(dis, mess_dis, gen);
		for (auto &i : data["garbageQueued"])
		{
			if (last_delay == -1)
			{
				last_delay = i["delay"];
			}
			else if (i["delay"] != last_delay)
			{
				pending.push_lines(total, last_delay);
				last_delay = i["delay"];
				total = 1;
			}
			else
			{
				++total;
			}
		}
		if (total != 0)
		{
			pending.push_lines(total, last_delay);
		}
		utils::println(utils::INFO, " -> Last delay: " + std::to_string(last_delay));
		read_config();
		TetrisStatus status(data["b2b"], data["combo"], next_manager, pending);
		TetrisTree runner(map, config, status, param);
		auto result = runner.run();
		auto path = translate_command(result.front().path);
		ws_make_move(path);
	}
	void handle_msg_player_action(json data) {}
	void handle_msg_player_damage_received(json data) {}
	void handle_msg_round_over(json data) {}
	void handle_msg_game_over(json data) {}
	void handle_msg_game_reset(json data)
	{
		utils::println(utils::INFO, " -> Game reset");
		update_room_data(data);
	}
	void handle_msg_error(json data)
	{
		std::string reason;
		str_safeassign(data, reason, "payload");
		utils::println(utils::ERR, " -> Error: " + reason);
	}

	std::map<std::string, pfunc> message_type_to_function = {
		{"\"room_data\"", &Botris_Client::handle_msg_room_data},
		{"\"authenticated\"", &Botris_Client::handle_msg_authenticated},
		{"\"player_joined\"", &Botris_Client::handle_msg_player_joined},
		{"\"player_left\"", &Botris_Client::handle_msg_player_left},
		{"\"player_banned\"", &Botris_Client::handle_msg_player_banned},
		{"\"player_unbanned\"", &Botris_Client::handle_msg_player_unbanned},
		{"\"settings_changed\"", &Botris_Client::handle_msg_settings_changed},
		{"\"host_changed\"", &Botris_Client::handle_msg_host_changed},
		{"\"game_started\"", &Botris_Client::handle_msg_game_started},
		{"\"round_started\"", &Botris_Client::handle_msg_round_started},
		{"\"request_move\"", &Botris_Client::handle_msg_request_move},
		{"\"player_action\"", &Botris_Client::handle_msg_player_action},
		{"\"player_damage_received\"", &Botris_Client::handle_msg_player_damage_received},
		{"\"round_over\"", &Botris_Client::handle_msg_round_over},
		{"\"game_over\"", &Botris_Client::handle_msg_game_over},
		{"\"game_reset\"", &Botris_Client::handle_msg_game_reset},
		{"\"error\"", &Botris_Client::handle_msg_error}};

	void handle_ws_open()
	{
		utils::println(utils::INFO, " -> Connection established yay");
	}
	void handle_ws_connection_error(std::string error)
	{
		utils::println(utils::ERR, " -> Connection error: " + error);
	}
	void handle_ws_message(std::string message)
	{
		json data = json::parse(message);
		std::string message_type = data["type"].dump();
		(this->*message_type_to_function[message_type])(data);
	}

	void ws_start()
	{
		std::string url("wss://botrisbattle.com/ws?token=" + API_TOKEN + "&roomKey=" + ROOM_KEY);
		web_socket.setUrl(url);
		web_socket.setOnMessageCallback([&](const ix::WebSocketMessagePtr &msg)
										{
			if (msg->type == ix::WebSocketMessageType::Message)
				handle_ws_message(msg->str);
			else if (msg->type == ix::WebSocketMessageType::Open)
				handle_ws_open();
			else if (msg->type == ix::WebSocketMessageType::Error)
				handle_ws_connection_error(msg->errorInfo.reason); });
		web_socket.start();
	}

public:
	void run()
	{
		get_secrets();
		ws_start();
	}

	void get_secrets()
	{
		std::ifstream file("key.json");
		json data;
		file >> data;
		API_TOKEN = data["api_token"];
		ROOM_KEY = data["room_key"];
	}

	void ws_make_move(std::vector<std::string> moves)
	{
		json commands(moves), data;
		data["type"] = "action";
		data["payload"] = {{"commands", commands}};

		utils::println(utils::INFO, "<-  Making move: " + boost::algorithm::join(moves, ","));
		web_socket.send(data.dump());
	}
};
