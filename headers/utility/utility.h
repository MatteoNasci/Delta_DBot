#pragma once
#ifndef H_MLN_DB_UTILITY_H
#define H_MLN_DB_UTILITY_H

#include "database/database_callbacks.h"

#include <dpp/guild.h>
#include <dpp/coro/task.h>
#include <dpp/dispatcher.h>
#include <dpp/cluster.h>
#include <dpp/message.h>
#include <dpp/coro/async.h>
#include <dpp/restresults.h>
#include <dpp/permissions.h>

#include <optional>
#include <string>
#include <functional>
#include <vector>
#include <memory>

namespace mln {
	class bot_delta;
	class utility {
	public:
		static const database_callbacks_t& get_any_results_callback();
		static database_callbacks_t get_any_results_callback(bool* found);

		//if the return ptr is null check the out parameter. If the out_parameter returned is not a channel then the channel was not found
		static dpp::task<std::tuple<dpp::channel*, dpp::channel>> get_channel(const dpp::interaction_create_t& event_data, const dpp::snowflake& channel_id, dpp::cluster& bot);
		static dpp::task<std::tuple<dpp::channel*, dpp::channel>> get_channel(const dpp::snowflake& channel_id, dpp::cluster& bot);
		static dpp::task<std::tuple<dpp::guild*, dpp::guild>> get_guild(const dpp::interaction_create_t& event_data, dpp::cluster& bot);
		static dpp::task<std::tuple<dpp::guild*, dpp::guild>> get_guild(const dpp::snowflake& guild_id, dpp::cluster& bot);
		static dpp::task<std::optional<dpp::guild_member>> get_member(const dpp::interaction_create_t& event_data, const dpp::snowflake& user_id, dpp::cluster& bot);
		static dpp::task<std::optional<dpp::guild_member>> get_member(const dpp::interaction_create_t& event_data, const dpp::guild* const guild, const dpp::snowflake& user_id, dpp::cluster& bot);
		static dpp::task<std::optional<dpp::guild_member>> get_member(const dpp::guild* const guild, const dpp::snowflake& user_id, dpp::cluster& bot);
		//Returns usr first, second bot
		static dpp::task<std::tuple<std::optional<dpp::guild_member>, std::optional<dpp::guild_member>>> get_members(const dpp::interaction_create_t& event_data, dpp::cluster& bot);

		//checks permissions in the given channel of given guild
		static bool check_permissions(const dpp::guild* guild, const dpp::channel* channel, const std::vector<dpp::guild_member*>& users, const std::vector<dpp::permissions>& permissions_to_check);
		static bool check_permissions(const dpp::guild* guild, const dpp::channel* channel, const std::vector<dpp::guild_member*>& users, dpp::permissions permission_to_check);
		static bool check_permissions(const dpp::guild* guild, const dpp::channel* channel, const dpp::guild_member* user, const std::vector<dpp::permissions>& permissions_to_check);
		static bool check_permissions(const dpp::guild* guild, const dpp::channel* channel, const dpp::guild_member* user, dpp::permissions permission_to_check);

		static bool check_permissions(const dpp::guild* guild, const dpp::channel* channel, const std::vector<dpp::guild_member>& users, const std::vector<dpp::permissions>& permissions_to_check);
		static bool check_permissions(const dpp::guild* guild, const dpp::channel* channel, const std::vector<dpp::guild_member>& users, dpp::permissions permission_to_check);
		static bool check_permissions(const dpp::guild* guild, const dpp::channel* channel, const dpp::guild_member& user, const std::vector<dpp::permissions>& permissions_to_check);
		static bool check_permissions(const dpp::guild* guild, const dpp::channel* channel, const dpp::guild_member& user, dpp::permissions permission_to_check);

		static bool check_permissions(const dpp::guild* guild, const dpp::channel* channel, const std::vector<std::optional<dpp::guild_member>>& users, const std::vector<dpp::permissions>& permissions_to_check);
		static bool check_permissions(const dpp::guild* guild, const dpp::channel* channel, const std::vector<std::optional<dpp::guild_member>>& users, dpp::permissions permission_to_check);
		static bool check_permissions(const dpp::guild* guild, const dpp::channel* channel, const std::optional<dpp::guild_member>& user, const std::vector<dpp::permissions>& permissions_to_check);
		static bool check_permissions(const dpp::guild* guild, const dpp::channel* channel, const std::optional<dpp::guild_member>& user, dpp::permissions permission_to_check);

		static bool check_permissions(const std::vector<dpp::permission>& permissions, const std::vector<dpp::permissions>& permissions_to_check);
		static bool check_permissions(const std::vector<dpp::permission>& permissions, dpp::permissions permission_to_check);
		static bool check_permissions(dpp::permission permission, const std::vector<dpp::permissions>& permissions_to_check);
		static bool check_permissions(dpp::permission permission, dpp::permissions permission_to_check);
		
		static dpp::task<void> co_conclude_thinking_response(dpp::async<dpp::confirmation_callback_t>& thinking, const dpp::interaction_create_t& event_data, const dpp::cluster& bot, const std::string& to_respond, const std::tuple<bool, dpp::loglevel>& log_always = {true, dpp::loglevel::ll_error});
		static dpp::task<void> co_conclude_thinking_response(std::optional<dpp::async<dpp::confirmation_callback_t>>& thinking, const dpp::interaction_create_t& event_data, const dpp::cluster& bot, const std::string& to_respond, const std::tuple<bool, dpp::loglevel>& log_always = {true, dpp::loglevel::ll_error});

		static dpp::job manage_paginated_embed(dpp::interaction_create_t event_data, dpp::cluster* bot, const std::shared_ptr<std::vector<std::string>> text_ptr, uint64_t time_limit_seconds);

		static bool extract_generic_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id);
		static bool extract_generic_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& name);
		static bool extract_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id);
		static bool extract_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& name);
		static bool extract_ephemeral_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id);
		static bool extract_ephemeral_attachment_url_data(const std::string& attachment_url, uint64_t& out_guild_id, uint64_t& out_channel_id, std::string& name);
		static bool extract_message_url_data(const std::string& msg_url, uint64_t& out_guild_id, uint64_t& out_channel_id, uint64_t& out_message_id);
	private:
		static void any_results_da_callback(void*, int, mln::db_column_data_t&&);
		static bool any_results_td_callback(void*, int);
	};
}

#endif //H_MLN_DB_UTILITY_H