/* Copyright 2017 Łukasz Gemborowski <lukasz.gemborowski@gmail.com> */
#ifndef GENERALS_GAME_HPP
#define GENERALS_GAME_HPP

#include <vector>
#include <map>
#include <queue>
#include "board.hpp"
#include "position.hpp"

struct settings {
	int max_players = 2;
};

struct order {
	player_id player;
	position start;
	position end;
};

struct game
{
	game(settings s, board b) :
		settings_ (s),
		board_ (b)
	{
	}

	int join()
	{
		if (players_.size() < settings_.max_players) {
			last_id_ ++;
			players_.push_back(last_id_);
			find_hometown(last_id_);
			return last_id_;
		} else {
			return 0;
		}
	}

	void tick()
	{
		for (auto &oqpair : order_queue_) {
			auto &q = oqpair.second;
			if (q.empty())
				continue;

			auto player = oqpair.first;
			auto order = q.front();
			q.pop();

			auto &start_field = board_.at(order.start.x(), order.start.y());
			auto &end_field = board_.at(order.end.x(), order.end.y());

			if (start_field.player_ != player)
				continue;

			auto count = start_field.count_;
			start_field.count_ = 0;

			if (end_field.player_ == player) {
				// transfer
				end_field.count_ += count;
			} else {
				// attack
				if (count > end_field.count_) {
					end_field.count_ = count - end_field.count_;
					end_field.player_ = player;
				} else {
					end_field.count_ -= count;
				}
			}
		}

		auto increase = false;
		if (tick_ % 10 == 0) {
			increase = true;
		}

		for (auto &f : board_) {
			if (f.player_ != 0) {
				if (f.town_ || f.hometown_)
					f.count_ ++;
				else if (increase)
					f.count_ ++;
			}
		}

		tick_ ++;
	}

	field at(std::size_t x, std::size_t y)
	{
		return board_.at(x, y);
	}

	auto map_width() const {
		return board_.width();
	}

	auto map_height() const {
		return board_.height();
	}

	void move_order(player_id player, position start, position end)
	{
		// TODO: verify player existance
		order_queue_[player].push(order{player, start, end});
	}

private:
	bool find_hometown(player_id id)
	{
		for (auto &f : board_) {
			if (f.town_&& f.player_ == 0) {
				f.hometown_ = true;
				f.player_ = id;
				return true;
			}
		}

		return false;
	}

private:
	settings settings_;
	board board_;

	std::vector<player_id> players_;
	player_id last_id_ = 0;
	std::map<player_id, std::queue<order>> order_queue_;
	int tick_ = 0;
};

#endif
