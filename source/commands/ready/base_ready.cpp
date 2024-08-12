#include "commands/ready/base_ready.h"

mln::base_ready::base_ready(bot_delta* const delta) : base_action(delta){}

mln::base_ready::base_ready(base_ready&& rhs) : base_action(std::move(rhs)) {}

mln::base_ready& mln::base_ready::operator=(base_ready&& rhs){
	base_action::operator=(std::move(rhs));
	return *this;
}
