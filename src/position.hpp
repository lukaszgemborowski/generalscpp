/* Copyright 2017 Łukasz Gemborowski <lukasz.gemborowski@gmail.com> */
#ifndef GENERALS_POSITION_HPP
#define GENERALS_POSITION_HPP

#include <cstdint>

struct position
{
	position() = default;
	position(std::size_t x, std::size_t y) : x_(x), y_(y) {}
	std::size_t x_ = 0, y_ = 0;

	auto x() const { return x_; }
	auto y() const { return y_; }
};

#endif
