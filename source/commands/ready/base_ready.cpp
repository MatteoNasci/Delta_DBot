#include "commands/ready/base_ready.h"

mln::base_ready::base_ready(mln::bot_delta* const delta) : base_action(delta){}

mln::base_ready::base_ready(base_ready&& rhs) : base_action(std::forward<base_ready>(rhs)) {}

mln::base_ready& mln::base_ready::operator=(base_ready&& rhs){
	base_action::operator=(std::forward<base_ready>(rhs));
	return *this;
}
