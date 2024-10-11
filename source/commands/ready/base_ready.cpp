#include "commands/base_action.h"
#include "commands/ready/base_ready.h"

mln::base_ready::base_ready(dpp::cluster& cluster) noexcept : base_action{ cluster } {}