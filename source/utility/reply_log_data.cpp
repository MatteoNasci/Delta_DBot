#include "utility/reply_log_data.h"

mln::reply_log_data_t::reply_log_data_t(const dpp::interaction_create_t* const in_event_data, const dpp::cluster* const in_cluster, const bool in_is_first_response) :
    event_data{ in_event_data }, cluster{ in_cluster }, is_first_response{ in_is_first_response }
{

}