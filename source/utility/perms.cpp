#include "utility/caches.h"
#include "utility/event_data_lite.h"
#include "utility/json_err.h"
#include "utility/perms.h"
#include "utility/response.h"
#include "utility/utility.h"

#include <dpp/channel.h>
#include <dpp/cluster.h>
#include <dpp/coro/task.h>
#include <dpp/emoji.h>
#include <dpp/guild.h>
#include <dpp/message.h>
#include <dpp/permissions.h>
#include <dpp/restresults.h>
#include <dpp/role.h>
#include <dpp/snowflake.h>

#include <cstdint>
#include <exception>
#include <format>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

//https://discord.com/developers/docs/topics/permissions#permission-overwrites
std::optional<dpp::permission> mln::perms::get_base_permission(const uint64_t guild_owner, const dpp::guild_member& member, event_data_lite_t& lite_data, const std::map<dpp::snowflake, dpp::role>* const resolved_map) {
	//If the member is the server owner, return all permissions
	if (member.user_id == guild_owner) {
		return ~0;
	}

	//This is the base everyone role that all guilds have, same id as the guild
	const std::optional<std::shared_ptr<const dpp::role>> everyone = mln::caches::get_role(member.guild_id, member.guild_id, true, lite_data, resolved_map);
	if (!everyone.has_value()) {
		return std::nullopt;
	}
	dpp::permission total_perms = everyone.value()->permissions;

	//Retrieve all the member roles and add the permissions to the everyone permission
	for (const dpp::snowflake& role_id : member.get_roles()) {
		const std::optional<std::shared_ptr<const dpp::role>> role = mln::caches::get_role(member.guild_id, role_id, true, lite_data, resolved_map);
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
dpp::task<std::optional<dpp::permission>> mln::perms::get_base_permission_task(const uint64_t guild_owner, const dpp::guild_member& member, event_data_lite_t& lite_data, const std::map<dpp::snowflake, dpp::role>* const resolved_map) {
	//If the member is the server owner, return all permissions
	if (member.user_id == guild_owner) {
		co_return ~0;
	}

	//This is the base everyone role that all guilds have, same id as the guild
	const std::optional<std::shared_ptr<const dpp::role>> everyone = co_await mln::caches::get_role_task(member.guild_id, member.guild_id, true, lite_data, resolved_map);
	if (!everyone.has_value()) {
		co_return std::nullopt;
	}
	dpp::permission total_perms = everyone.value()->permissions;
	
	//Retrieve all the member roles and add the permissions to the everyone permission
	for (const dpp::snowflake& role_id : member.get_roles()) {
		const std::optional<std::shared_ptr<const dpp::role>> role = co_await mln::caches::get_role_task(member.guild_id, role_id, false, lite_data, resolved_map);
		if (!role.has_value()) {
			co_return std::nullopt;
		}
		total_perms |= role.value()->permissions;
	}

	//If the final perms have the admin flag, return all perms
	if (total_perms & dpp::permissions::p_administrator) {
		co_return ~0;
	}
	co_return total_perms;
}

dpp::permission mln::perms::get_overwrite_permission(const dpp::permission& base_permission, const dpp::channel& channel, const dpp::guild_member& member, const std::map<dpp::snowflake, dpp::permission>* const resolved_map) {
	//If the member is an admin, return all permissions
	if (base_permission & dpp::permissions::p_administrator) {
		return ~0;
	}

	//If the member permission is in cache return it
	if (resolved_map != nullptr) {
		const std::map<dpp::snowflake, dpp::permission>::const_iterator it = resolved_map->find(member.user_id);
		if (it != resolved_map->end()) {
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

std::optional<dpp::permission> mln::perms::get_computed_permission(const uint64_t guild_owner, const dpp::channel& channel, const dpp::guild_member& member, event_data_lite_t& lite_data, const std::map<dpp::snowflake, dpp::role>* const resolved_role_map, const std::map<dpp::snowflake, dpp::permission>* const resolved_perms_map) {
	const std::optional<dpp::permission> base_perm = mln::perms::get_base_permission(guild_owner, member, lite_data, resolved_role_map);
	if (!base_perm.has_value()) {
		const std::string err_text = std::format("Failed to retrieve perms data! guild owner id: [{}], channel id: [{}], guild member: [g: {}, u: {}].", 
			guild_owner, static_cast<uint64_t>(channel.id), static_cast<uint64_t>(member.guild_id), static_cast<uint64_t>(member.user_id));

		mln::response::respond(lite_data, err_text, true, err_text);

		return std::nullopt;
	}

	return mln::perms::get_overwrite_permission(base_perm.value(), channel, member, resolved_perms_map);
}
dpp::task<std::optional<dpp::permission>> mln::perms::get_computed_permission_task(const uint64_t guild_owner, const dpp::channel& channel, const dpp::guild_member& member, event_data_lite_t& lite_data, const std::map<dpp::snowflake, dpp::role>* const resolved_role_map, const std::map<dpp::snowflake, dpp::permission>* const resolved_perms_map) {
	const std::optional<dpp::permission> base_perm = co_await mln::perms::get_base_permission_task(guild_owner, member, lite_data, resolved_role_map);
	if (!base_perm.has_value()) {
		const std::string err_text = std::format("Failed to retrieve perms data! guild owner id: [{}], channel id: [{}], guild member: [g: {}, u: {}].", 
			guild_owner, static_cast<uint64_t>(channel.id), static_cast<uint64_t>(member.guild_id), static_cast<uint64_t>(member.user_id));

		co_await mln::response::co_respond(lite_data, err_text, true, err_text);

		co_return std::nullopt;
	}

	co_return mln::perms::get_overwrite_permission(base_perm.value(), channel, member, resolved_perms_map);
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

dpp::task<std::optional<dpp::permission>> mln::perms::get_additional_perms_required_task(const dpp::message& msg, const uint64_t guild_id, event_data_lite_t& lite_data) {
	if (lite_data.creator == nullptr) {
		throw std::exception("Creator pointer must be valid for permissions api request!");
	}

	dpp::cluster& bot = *lite_data.creator;

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
			const dpp::error_info err = emojis_result.get_error();
			const std::string err_text = std::format("Failed operation, the bot couldn't retrieve the server's emojis to verify if the selected message contains external emojis! Error: [{}], details: [{}].", mln::get_json_err_text(err.code), err.human_readable);

			co_await mln::response::co_respond(lite_data, err_text, true, err_text);

			co_return std::nullopt;
		}

		if (!std::holds_alternative<dpp::emoji_map>(emojis_result.value)) {
			const static std::string s_err_text = "Failed operation, the bot couldn't retrieve the server's emojis to verify if the selected message contains external emojis! Received invalid data!";

			co_await mln::response::co_respond(lite_data, s_err_text, true, s_err_text);

			co_return std::nullopt;
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
			const dpp::error_info err = stickers_result.get_error();
			const std::string err_text = std::format("Failed operation, the bot couldn't retrieve the server's stickers to verify if the selected message contains external stickers! Error: [{}], details: [{}].", mln::get_json_err_text(err.code), err.human_readable);

			co_await mln::response::co_respond(lite_data, err_text, true, err_text);

			co_return std::nullopt;
		}

		if (!std::holds_alternative<dpp::sticker_map>(stickers_result.value)) {
			const static std::string s_err_text = "Failed operation, the bot couldn't retrieve the server's stickers to verify if the selected message contains external stickers! Received invalid data!";

			co_await mln::response::co_respond(lite_data, s_err_text, true, s_err_text);

			co_return std::nullopt;
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

std::optional<dpp::permission> mln::perms::get_additional_perms_required(const dpp::message& msg, const uint64_t guild_id, event_data_lite_t& lite_data) {
	if (lite_data.creator == nullptr) {
		throw std::exception("Creator pointer must be valid for permissions api request!");
	}

	dpp::cluster& bot = *lite_data.creator;

	dpp::permission result{ 0 };
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
		dpp::confirmation_callback_t emojis_result;
		lite_data.creator->guild_emojis_get(guild_id, [&emojis_result](const dpp::confirmation_callback_t& conf) { emojis_result = conf; });
		if (emojis_result.is_error()) {
			const dpp::error_info err = emojis_result.get_error();
			const std::string err_text = std::format("Failed operation, the bot couldn't retrieve the server's emojis to verify if the selected message contains external emojis! Error: [{}], details: [{}].", mln::get_json_err_text(err.code), err.human_readable);

			mln::response::respond(lite_data, err_text, true, err_text);

			return std::nullopt;
		}

		if (!std::holds_alternative<dpp::emoji_map>(emojis_result.value)) {
			const static std::string s_err_text = "Failed operation, the bot couldn't retrieve the server's emojis to verify if the selected message contains external emojis! Received invalid data!";

			mln::response::respond(lite_data, s_err_text, true, s_err_text);

			return std::nullopt;
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
		dpp::confirmation_callback_t stickers_result;
		lite_data.creator->guild_stickers_get(guild_id, [&stickers_result](const dpp::confirmation_callback_t& conf) { stickers_result = conf; });
		if (stickers_result.is_error()) {
			const dpp::error_info err = stickers_result.get_error();
			const std::string err_text = std::format("Failed operation, the bot couldn't retrieve the server's stickers to verify if the selected message contains external stickers! Error: [{}], details: [{}].", mln::get_json_err_text(err.code), err.human_readable);

			mln::response::respond(lite_data, err_text, true, err_text);

			return std::nullopt;
		}

		if (!std::holds_alternative<dpp::sticker_map>(stickers_result.value)) {
			const static std::string s_err_text = "Failed operation, the bot couldn't retrieve the server's stickers to verify if the selected message contains external stickers! Received invalid data!";

			mln::response::respond(lite_data, s_err_text, true, s_err_text);

			return std::nullopt;
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

	return result;
}