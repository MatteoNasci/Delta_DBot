#include "bot_delta.h"
#include "utility/constants.h"

size_t mln::constants::get_min_characters_text_id() noexcept
{
	return mln::bot_delta::min_text_id_size();
}

size_t mln::constants::get_max_characters_text_id() noexcept
{
	return mln::bot_delta::max_text_id_size();
}
