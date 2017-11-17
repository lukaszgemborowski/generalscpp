/* Copyright 2017 Łukasz Gemborowski <lukasz.gemborowski@gmail.com> */
#ifndef GENERALS_BOARD_HPP
#define GENERALS_BOARD_HPP

#include <vector>
#include "field.hpp"

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
	const std::size_t width_, height_;
	std::vector<field> fields_;
};

#endif
