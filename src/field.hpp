/* Copyright 2017 Łukasz Gemborowski <lukasz.gemborowski@gmail.com> */
#ifndef GENERALS_FIELD_HPP
#define GENERALS_FIELD_HPP

#include <ostream>

using player_id = int;

struct field
{
	player_id player_ = 0;
	int count_ = 1;
	bool hometown_ = false;
	bool town_ = false;
};

std::ostream& operator << (std::ostream &os, const field &f)
{
	os << "field { player: " << f.player_ << "; count: " << f.count_
		<< "; hometown: " << f.hometown_ << "; town: " << f.town_ << "; };";
	return os;
}

#endif
