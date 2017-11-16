/*
Copyright 2017 Łukasz Gemborowski <lukasz.gemborowski@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <ncurses.h>
#include <array>
#include <random>
#include <map>
#include <stack>
#include <tuple>
#include <thread>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

enum class Color : int
{
	Red = COLOR_RED,
	Blue = COLOR_BLUE,
	Green = COLOR_GREEN,
	Black = COLOR_BLACK,
	White = COLOR_WHITE
};

using player_id = int;

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

struct field
{
	player_id player_ = 0;
	int count_ = 1;
	bool hometown_ = false;
	bool town_ = false;
};


struct board
{
	board(std::size_t width, std::size_t height) :
		width_ (width),
		height_ (height),
		fields_ (width * height, field{})
	{
		generate();
	}

	const auto width() const { return width_; }
	const auto height() const { return height_; }

	field& at(std::size_t x, std::size_t y)
	{
		return fields_[y * width() + x];
	}

	const field& at(std::size_t x, std::size_t y) const
	{
		return fields_[y * width() + x];
	}

	bool adjecent(std::size_t x, std::size_t y, Color color) const
	{
	}

	auto begin() { return fields_.begin(); }
	auto end() { return fields_.end(); }

private:
	void generate()
	{
		at(0, 0).town_ = true;
		at(3, 3).town_ = true;
	}

private:
	std::size_t width_, height_;
	std::vector<field> fields_;
};


struct position
{
	position() = default;
	position(std::size_t x, std::size_t y) : x_(x), y_(y) {}
	std::size_t x_ = 0, y_ = 0;

	auto x() const { return x_; }
	auto y() const { return y_; }
};

struct settings {
	int max_players = 2;
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
		for (auto &f : board_) {
			if ((f.town_ || f.hometown_) && f.player_ != 0)
				f.count_ ++;
		}
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
};

struct client
{
	client(game &g) : game_(g) {
		id_ = g.join();

		if (id_ == 0) throw "Failed";
	}

	field at(std::size_t x, std::size_t y)
	{
		return game_.at(x, y);
	}

	auto map_width() const {
		return game_.map_width();
	}

	auto map_height() const {
		return game_.map_height();
	}

private:
	game &game_;
	player_id id_;
};

struct client_field : public field
{
	client_field(const field &f) :
		field(f) {}

	bool cursor_ = false;
};

struct client_view
{
	client_view(client &c) : client_(c) {}

	auto at(std::size_t x, std::size_t y)
	{
		auto f = client_field(client_.at(x, y));

		if (x == cursor_.x_ && y == cursor_.y_)
			f.cursor_ = true;

		return f;
	}

	auto width() const { return client_.map_width(); }
	auto height() const { return client_.map_height(); }

	Color playerColor(player_id id) const
	{
		auto m = std::map<player_id, Color> {
			{0, Color::White},
			{1, Color::Red},
			{2, Color::Blue},
			{3, Color::Green}
		};

		return m[id];
	}

	void move_cursor(int x, int y) {
		if (x < 0 && cursor_.x_ > 0)
			cursor_.x_ --;
		if (x > 0 && cursor_.x_ < width() - 1)
			cursor_.x_ ++;
		if (y < 0 && cursor_.y_ > 0)
			cursor_.y_ --;
		if (y > 0 && cursor_.y_ < height() - 1)
			cursor_.y_ ++;
	}

private:
	client &client_;
	position cursor_;
};

template<typename B>
void draw(ncurses &nc, B &b)
{
	for (auto x = 0u; x < b.width(); x ++) {
		for (auto y = 0u; y < b.height(); y ++) {
			auto f = b.at(x, y);

			nc.push_color(b.playerColor(f.player_), Color::Black);

			if (f.hometown_) {
				nc.push(A_BOLD);
				nc.push(A_UNDERLINE);
			}

			if (f.town_) {
				nc.push(A_DIM);
			}

			if (f.cursor_) {
				nc.push(A_REVERSE);
			}

			auto count = '0';
			if (f.count_ < 10)
				count += f.count_;
			else
				count += f.count_ / 10;
			mvaddch(y, x, count);

			nc.popall();
		}
	}
}

struct keyboard_listener
{
	keyboard_listener(boost::asio::io_service &io) :
		io_ (io)
	{
	}

	~keyboard_listener()
	{
	}

	template<typename F>
	void listen(F func)
	{
		thread_ = std::thread(&keyboard_listener::process<F>, this, func);
	}

private:
	template<typename F>
	void process(F func)
	{
		while (true) {
			auto c = getch();
			io_.dispatch([c, func]() { func(c); } );
		}
	}

private:
	boost::asio::io_service &io_;
	std::thread thread_;
};

struct keyboard
{
	keyboard(client_view &v) :
		client_view_(v){}

	void operator()(int key)
	{
		// TODO: config based
		if (key == 'w')
			client_view_.move_cursor(0, -1);
		if (key == 's')
			client_view_.move_cursor(0, 1);
		if (key == 'a')
			client_view_.move_cursor(-1, 0);
		if (key == 'd')
			client_view_.move_cursor(1, 0);
	}

private:
	client_view &client_view_;
};

struct single_player_instance
{
	single_player_instance() :
		io_ {},
		frame_timer_ (io_),
		board_ (10, 10),
		game_ (settings{}, board_),
		client_ (game_),
		view_ (client_),
		keyboard_ (view_),
		key_listener_ (io_)
	{
	}

	void key(int k)
	{
		keyboard_(k);
		redraw();
	}

	void redraw()
	{
		draw(ncurses_, view_);
		refresh();
	}

	void frame()
	{
		game_.tick();
		redraw();
	}

	void schedule_frame()
	{
		frame_timer_.expires_from_now(boost::posix_time::seconds(1));
		frame_timer_.async_wait([this](const boost::system::error_code &)
			{
				schedule_frame();
				frame();
			});
	}

	void run()
	{
		key_listener_.listen([this](int k) { key(k); });
		redraw();
		schedule_frame();
		io_.run();
	}

private:
	boost::asio::io_service io_;
	boost::asio::deadline_timer frame_timer_;
	ncurses ncurses_;
	board board_;
	game game_;
	client client_;
	client_view view_;
	keyboard keyboard_;
	keyboard_listener key_listener_;
};

int main()
{
	single_player_instance p;
	p.run();

	return 0;
}
