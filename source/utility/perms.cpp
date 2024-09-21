#include "utility/perms.h"
#include "utility/caches.h"
#include "utility/utility.h"
#include "utility/response.h"
#include "utility/reply_log_data.h"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>

//https://discord.com/developers/docs/topics/permissions#permission-overwrites
std::optional<dpp::permission> mln::perms::get_base_permission(const dpp::guild& guild, const dpp::guild_member& member, const dpp::interaction_create_t* const event_data) {
	//If the member is the server owner, return all permissions
	if (member.user_id == guild.owner_id) {
		return ~0;
	}

	//This is the base everyone role that all guilds have, same id as the guild
	const std::optional<std::shared_ptr<const dpp::role>> everyone = mln::caches::get_role(guild.id, event_data);
	if (!everyone.has_value()) {
		return std::nullopt;
	}
	dpp::permission total_perms = everyone.value()->permissions;

	//Retrieve all the member roles and add the permissions to the everyone permission
	for (const dpp::snowflake& role_id : member.get_roles()) {
		const std::optional<std::shared_ptr<const dpp::role>> role = mln::caches::get_role(role_id, event_data);
		if (!role.has_value()) {
			return std::nullopt;
		}
		total_perms |= role.value()->permissions;
	}

	//If the final perms have the admin flag, return all perms
	if (total_perms & dpp::permissions::p_administrator) {
		return ~0;
	}
	return total_perms;
}
dpp::task<std::optional<dpp::permission>> mln::perms::get_base_permission_task(const dpp::guild& guild, const dpp::guild_member& member, const dpp::interaction_create_t* const event_data) {
	//If the member is the server owner, return all permissions
	if (member.user_id == guild.owner_id) {
		co_return ~0;
	}

	//the role_task_called boolean is used to make sure the get_role_task is called the first time with the optional bool param = true, to make sure to cache all the roles received from api on the first call.
	bool role_task_called = false;
	//This is the base everyone role that all guilds have, same id as the guild
	std::optional<std::shared_ptr<const dpp::role>> everyone = mln::caches::get_role(guild.id, event_data);
	if (!everyone.has_value()) {
		everyone = co_await mln::caches::get_role_task(guild.id, guild.id, !role_task_called);
		role_task_called = true;
		if (!everyone.has_value()) {
			co_return std::nullopt;
		}
	}
	dpp::permission total_perms = everyone.value()->permissions;
	
	//Retrieve all the member roles and add the permissions to the everyone permission
	for (const dpp::snowflake& role_id : member.get_roles()) {
		std::optional<std::shared_ptr<const dpp::role>> role = mln::caches::get_role(role_id, event_data);
		if (!role.has_value()) {
			role = co_await mln::caches::get_role_task(guild.id, guild.id, !role_task_called);
			role_task_called = true;
			if (!role.has_value()) {
				co_return std::nullopt;
			}
		}
		total_perms |= role.value()->permissions;
	}

	//If the final perms have the admin flag, return all perms
	if (total_perms & dpp::permissions::p_administrator) {
		co_return ~0;
	}
	co_return total_perms;
}

std::optional<dpp::permission> mln::perms::get_overwrite_permission(const dpp::permission& base_permission, const dpp::channel& channel, const dpp::guild_member& member, const dpp::interaction_create_t* const event_data) {
	//If the member is an admin, return all permissions
	if (base_permission & dpp::permissions::p_administrator) {
		return ~0;
	}

	//If the member permission is in cache return it
	if (event_data != nullptr) {
		const auto it = event_data->command.resolved.member_permissions.find(member.user_id);
		if (it != event_data->command.resolved.member_permissions.end()) {
			return it->second;
		}
	}

	//Apply deny and allow from everyone's role overwrite
	dpp::permission total_perm = base_permission;
	for (const dpp::permission_overwrite& overwrite : channel.permission_overwrites) {
		if (overwrite.id == channel.guild_id && 
			overwrite.type == dpp::overwrite_type::ot_role) {

			total_perm &= ~(overwrite.deny);
			total_perm |= overwrite.allow;
			break;
		}
	}

	//Apply role specific overwrites
	dpp::permission allow{0}, deny{0};
	for (const dpp::snowflake& role_id : member.get_roles()) {
		//Ignore the everyone overwrite that we handled above
		if (role_id == channel.guild_id) {
			continue;
		}

		//Find the role overwrite associated with the current role_id, then add it to the allow/deny
		for (const dpp::permission_overwrite& overwrite : channel.permission_overwrites) {
			if (overwrite.id == role_id &&
				overwrite.type == dpp::overwrite_type::ot_role) {

				deny |= overwrite.deny;
				allow |= overwrite.allow;
				break;
			}
		}
	}

	total_perm &= ~deny;
	total_perm |= allow;

	//Apply member specific overwrites
	for (const dpp::permission_overwrite& overwrite : channel.permission_overwrites) {
		if (overwrite.id == member.user_id &&
			overwrite.type == dpp::overwrite_type::ot_member) {

			total_perm &= ~(overwrite.deny);
			total_perm |= overwrite.allow;
			break;
		}
	}

	return total_perm;
}

std::optional<dpp::permission> mln::perms::get_computed_permission(const dpp::guild& guild, const dpp::channel& channel, const dpp::guild_member& member, const dpp::interaction_create_t* const event_data) {
	const std::optional<dpp::permission> base_perm = mln::perms::get_base_permission(guild, member, event_data);
	if (!base_perm.has_value()) {
		return std::nullopt;
	}

	return mln::perms::get_overwrite_permission(base_perm.value(), channel, member, event_data);
}
dpp::task<std::optional<dpp::permission>> mln::perms::get_computed_permission_task(const dpp::guild& guild, const dpp::channel& channel, const dpp::guild_member& member, const dpp::interaction_create_t* const event_data) {
	const std::optional<dpp::permission> base_perm = co_await mln::perms::get_base_permission_task(guild, member, event_data);
	if (!base_perm.has_value()) {
		co_return std::nullopt;
	}

	co_return mln::perms::get_overwrite_permission(base_perm.value(), channel, member, event_data);
}
dpp::task<std::optional<dpp::permission>> mln::perms::get_computed_permission_full(const dpp::guild& guild, const dpp::channel& channel, const dpp::guild_member& member, const reply_log_data_t& reply_log_data) {
	std::optional<dpp::permission> perms = mln::perms::get_computed_permission(guild, channel, member, reply_log_data.event_data);
	if (!perms.has_value()) {
		perms = co_await mln::perms::get_computed_permission_task(guild, channel, member, reply_log_data.event_data);
		if (!perms.has_value()) {
			if (reply_log_data.event_data && reply_log_data.cluster) {
				const std::string err_text = std::format("Failed to retrieve perms data! guild_channel_member_id: [{}, {}, {}].", 
					static_cast<uint64_t>(guild.id), static_cast<uint64_t>(channel.id), static_cast<uint64_t>(member.user_id));
				mln::utility::conf_callback_is_error(
					co_await mln::response::make_response(reply_log_data.is_first_response, *reply_log_data.event_data, err_text),
					*reply_log_data.cluster, reply_log_data.event_data, err_text);
			}
		}
	}
	
	co_return perms;
}

bool mln::perms::check_permissions(const std::vector<dpp::permission>& permissions, const std::vector<dpp::permissions>& permissions_to_check) {

	for (const dpp::permissions& flag_to_check : permissions_to_check) {
		for (const dpp::permission& perm : permissions) {
			if (!perm.can(flag_to_check)) {
				return false;
			}
		}
	}

	return true;
}
bool mln::perms::check_permissions(const std::vector<dpp::permission>& permissions, const dpp::permissions permission_to_check) {
	for (const dpp::permission& perm : permissions) {
		if (!perm.can(permission_to_check)) {
			return false;
		}
	}

	return true;
}
bool mln::perms::check_permissions(const std::vector<dpp::permission>& permissions, const uint64_t permission_to_check) {
	for (const dpp::permission& perm : permissions) {
		if (!perm.can(static_cast<dpp::permissions>(permission_to_check))) {
			return false;
		}
	}

	return true;
}
bool mln::perms::check_permissions(const dpp::permission permission, const std::vector<dpp::permissions>& permissions_to_check) {
	for (const dpp::permissions& flag_to_check : permissions_to_check) {
		if (!permission.can(flag_to_check)) {
			return false;
		}
	}

	return true;
}
bool mln::perms::check_permissions(const dpp::permission permission, const dpp::permissions permission_to_check) {
	return permission.can(permission_to_check);
}
bool mln::perms::check_permissions(const dpp::permission permission, const uint64_t permission_to_check) {
	return permission.can(static_cast<dpp::permissions>(permission_to_check));
}

dpp::task<dpp::permission> mln::perms::get_additional_perms_required(const dpp::message& msg, dpp::cluster& bot, const uint64_t guild_id) {
	dpp::permission result{0};
	if (msg.tts) {
		result.add(dpp::permissions::p_send_tts_messages);
	}
	if (msg.attachments.size() != 0) {
		result.add(dpp::permissions::p_attach_files);
	}
	if (msg.mention_everyone) {
		result.add(dpp::permissions::p_mention_everyone);
	}
	if (msg.is_voice_message()) {
		result.add(dpp::permissions::p_send_voice_messages);
	}
	//Check if external emojis used
	const std::vector<dpp::snowflake> msg_emojis = mln::utility::extract_emojis(msg.content);
	if (msg_emojis.size() != 0) {
		const dpp::confirmation_callback_t emojis_result = co_await bot.co_guild_emojis_get(guild_id);
		if (emojis_result.is_error()) {
			bot.log(dpp::loglevel::ll_error, "Failed operation, the bot couldn't retrieve the server's emojis to verify if the selected message contains external emojis! Error: " + emojis_result.get_error().human_readable);
			co_return result;
		}

		const dpp::emoji_map& e_map = std::get<dpp::emoji_map>(emojis_result.value);
		for (const dpp::snowflake& emoji_id : msg_emojis) {
			const auto it = e_map.find(emoji_id);
			if (it == e_map.end()) {
				result.add(dpp::permissions::p_use_external_emojis);
				break;
			}
		}
	}
	//Check if external stickers used
	if (msg.stickers.size() != 0) {
		const dpp::confirmation_callback_t stickers_result = co_await bot.co_guild_stickers_get(guild_id);
		if (stickers_result.is_error()) {
			bot.log(dpp::loglevel::ll_error, "Failed operation, the bot couldn't retrieve the server's stickers to verify if the selected message contains external stickers! Error: " + stickers_result.get_error().human_readable);
			co_return result;
		}
		const dpp::sticker_map& st_map = std::get<dpp::sticker_map>(stickers_result.value);
		for (const dpp::sticker& sticker : msg.stickers) {
			const auto it = st_map.find(sticker.id);
			if (it == st_map.end()) {
				result.add(dpp::permissions::p_use_external_stickers);
				break;
			}
		}
	}

	co_return result;
}