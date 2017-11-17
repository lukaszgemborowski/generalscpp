/* Copyright 2017 Łukasz Gemborowski <lukasz.gemborowski@gmail.com> */
#ifndef GENERALS_NCURSES_HPP
#define GENERALS_NCURSES_HPP

#include <ncurses.h>
#include <stack>
#include <map>

enum class Color : int
{
	Red = COLOR_RED,
	Blue = COLOR_BLUE,
	Green = COLOR_GREEN,
	Black = COLOR_BLACK,
	White = COLOR_WHITE
};

struct ncurses
{
	ncurses()
	{
		initscr();
		cbreak();
		noecho();
		start_color();
	}

	~ncurses()
	{
		endwin();
	}

	ncurses(const ncurses&) = delete;
	ncurses& operator=(const ncurses&) = delete;

	void push_color(Color foreground, Color background)
	{
		auto index = 0;
		auto pos = color_map_.find(std::make_tuple(foreground, background));

		if (pos == color_map_.end()) {
			index = init_color(foreground, background);
		} else {
			index = pos->second;
		}

		auto attr = COLOR_PAIR(index);
		attron(attr);
		attributes_.push(attr);
	}

	void push(int attribute)
	{
		attron(attribute);
		attributes_.push(attribute);
	}

	void pop()
	{
		attroff(attributes_.top());
		attributes_.pop();
	}

	void popall()
	{
		while (attributes_.size())
			pop();
	}

private:
	int init_color(Color f, Color b)
	{
		init_pair(color_index_, static_cast<int>(f), static_cast<int>(b));
		color_map_[std::make_tuple(f, b)] = color_index_ ++;

		return color_index_- 1;
	}

private:
	std::map<std::tuple<Color, Color>, int> color_map_;
	int color_index_ = 1;
	std::stack<int> attributes_;
};

#endif
