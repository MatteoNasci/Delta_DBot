#pragma once
#ifndef H_MLN_DB_JSON_ERR_H
#define H_MLN_DB_JSON_ERR_H

#include <cstdint>
#include <type_traits>

namespace mln {
	enum class json_err : uint32_t {
		error = 0,//	General error(such as a malformed request body, amongst other things)
		unknown_account = 10001,//	Unknown account
		unknown_application = 10002,//	Unknown application
		unknown_channel = 10003,//	Unknown channel
		unknown_guild = 10004,//	Unknown guild
		unknown_integration = 10005,//	Unknown integration
		unknown_invite = 10006,//	Unknown invite
		unknown_member = 10007,//	Unknown member
		unknown_message = 10008,//	Unknown message
		unknown_permission_overwrite = 10009,//	Unknown permission overwrite
		unknown_provider = 10010,//	Unknown provider
		unknown_role = 10011,//	Unknown role
		unknown_token = 10012,//	Unknown token
		unknown_user = 10013,//	Unknown user
		unknown_emoji = 10014,//	Unknown emoji
		unknown_webhook = 10015,//	Unknown webhook
		unknown_webhook_service = 10016,//	Unknown webhook service
		unknown_session = 10020,//	Unknown session
		unknown_ban = 10026,//	Unknown ban
		unknown_SKU = 10027,//	Unknown SKU
		unknown_stoore_listing = 10028,//	Unknown Store Listing
		unknown_entitlement = 10029,//	Unknown entitlement
		unknown_build = 10030,//	Unknown build
		unknown_lobby = 10031,//	Unknown lobby
		unknown_branch = 10032,//	Unknown branch
		unknown_store_delivery_layout = 10033,//	Unknown store directory layout
		unknown_redistributable = 10036,//	Unknown redistributable
		unknown_gift_code = 10038,//	Unknown gift code
		unknown_stream = 10049,//	Unknown stream
		unknown_premium_server_subscribe_cooldown = 10050,//	Unknown premium server subscribe cooldown
		unknown_guild_template = 10057,//	Unknown guild template
		unknown_discoverable_server_category = 10059,//	Unknown discoverable server category
		unknown_sticker = 10060,//	Unknown sticker
		unknown_sticker_pack = 10061,//	Unknown sticker pack
		unknown_interaction = 10062,//	Unknown interaction
		unknown_application_command = 10063,//	Unknown application command
		unknown_voice_state = 10065,//	Unknown voice state
		unknown_application_command_permissions = 10066,//	Unknown application command permissions
		unknown_stage_instance = 10067,//	Unknown Stage Instance
		unknown_guild_member_verification_form = 10068,//	Unknown Guild Member Verification Form
		unknown_guild_welcome_screen = 10069,//	Unknown Guild Welcome Screen
		unknown_guild_scheduled_event = 10070,//	Unknown Guild Scheduled Event
		unknown_guild_scheduled_event_user = 10071,//	Unknown Guild Scheduled Event User
		unknown_tag = 10087,//	Unknown Tag
		bots_cannot_use_this_endpoint = 20001,//	Bots cannot use this endpoint
		only_bots_can_use_this_endpoint = 20002,//	Only bots can use this endpoint
		explicit_content_not_allowed = 20009,//	Explicit content cannot be sent to the desired recipient(s)
		action_not_authorized_on_application = 20012,//	You are not authorized to perform this action on this application
		action_not_allowed_by_slowmode = 20016,//	This action cannot be performed due to slowmode rate limit
		action_reserved_to_account_owner = 20018,//	Only the owner of this account can perform this action
		edit_message_not_allowed_announcement_rate_limit = 20022,//	This message cannot be edited due to announcement rate limits
		under_minimum_age = 20024,//	Under minimum age
		channel_write_rate_limit_hit = 20028,//	The channel you are writing has hit the write rate limit
		server_write_rate_limit_hit = 20029,//	The write action you are performing on the server has hit the write rate limit
		words_not_allowed = 20031,//	Your Stage topic, server name, server description, or channel names contain words that are not allowed
		guild_premium_subscription_level_low = 20035,//	Guild premium subscription level too low
		max_number_of_guilds_reached = 30001,//	Maximum number of guilds reached(100)
		max_number_of_friends_reached = 30002,//	Maximum number of friends reached(1000)
		max_number_of_channel_pins_reached = 30003,//	Maximum number of pins reached for the channel(50)
		max_number_of_recipients_reached = 30004,//	Maximum number of recipients reached(10)
		max_number_of_guild_roles_reached = 30005,//	Maximum number of guild roles reached(250)
		max_number_of_webhooks_reached = 30007,//	Maximum number of webhooks reached(15)
		max_number_of_emojis_reached = 30008,//	Maximum number of emojis reached
		max_number_of_reactions_reached = 30010,//	Maximum number of reactions reached(20)
		max_number_of_group_DMs_reached = 30011,//	Maximum number of group DMs reached(10)
		max_number_of_guild_channels_reached = 30013,//	Maximum number of guild channels reached(500)
		max_number_of_message_attachments_reached = 30015,//	Maximum number of attachments in a message reached(10)
		max_number_of_invites_reached = 30016,//	Maximum number of invites reached(1000)
		max_number_of_animated_emojis_reached = 30018,//	Maximum number of animated emojis reached
		max_number_of_server_members_reached = 30019,//	Maximum number of server members reached
		max_number_of_server_categories_reached = 30030,//	Maximum number of server categories has been reached(5)
		guild_already_has_template = 30031,//	Guild already has a template
		max_number_of_application_commands_reached = 30032,//	Maximum number of application commands reached
		max_number_of_thread_participants_reached = 30033,//	Maximum number of thread participants has been reached(1000)
		max_number_of_daily_application_command_creates_reached = 30034,//	Maximum number of daily application command creates has been reached(200)
		max_number_of_bans_non_guild_members_reached = 30035,//	Maximum number of bans for non - guild members have been exceeded
		max_number_of_bans_fetches_reached = 30037,//	Maximum number of bans fetches has been reached
		max_number_of_uncompleted_guild_scheduled_events_reached = 30038,//	Maximum number of uncompleted guild scheduled events reached(100)
		max_number_of_stickers_reached = 30039,//	Maximum number of stickers reached
		max_number_of_prune_requests_reached = 30040,//	Maximum number of prune requests has been reached.Try again later
		max_number_of_guild_widget_settings_updates_reached = 30042,//	Maximum number of guild widget settings updates has been reached.Try again later
		max_number_of_edits_old_messages = 30046,//	Maximum number of edits to messages older than 1 hour reached.Try again later
		max_number_of_pinned_threads_forum_channel_reached = 30047,//	Maximum number of pinned threads in a forum channel has been reached
		max_number_of_tags_forum_channel_reached = 30048,//	Maximum number of tags in a forum channel has been reached
		too_high_bitrate_for_channel = 30052,//	Bitrate is too high for channel of this type
		max_number_of_premium_emojis_reached = 30056,//	Maximum number of premium emojis reached(25)
		max_number_of_webhooks_per_guild_reached = 30058,//	Maximum number of webhooks per guild reached(1000)
		max_number_of_channel_permission_overwrites_reached = 30060,//	Maximum number of channel permission overwrites reached(1000)
		channels_too_large = 30061,//	The channels for this guild are too large
		unauthorized_invalid_token = 40001,//	Unauthorized.Provide a valid token and try again
		action_not_allowed_unverified_account = 40002,//	You need to verify your account in order to perform this action
		opening_DMs_too_fast = 40003,//	You are opening direct messages too fast
		send_messages_temporarily_disabled = 40004,//	Send messages has been temporarily disabled
		request_entity_too_large = 40005,//	Request entity too large.Try sending something smaller in size
		feature_temporarily_disabled_server_side = 40006,//	This feature has been temporarily disabled server - side
		user_banned_from_guild = 40007,//	The user is banned from this guild
		connection_revoked = 40012,//	Connection has been revoked
		target_user_not_connected_voice = 40032,//	Target user is not connected to voice
		message_already_crossposted = 40033,//	This message has already been crossposted
		application_command_name_duplicate_found = 40041,//	An application command with that name already exists
		application_interaction_failed_to_send = 40043,//	Application interaction failed to send
		cannot_send_message_forum_channel = 40058,//	Cannot send a message in a forum channel
		interaction_has_already_been_acknowledged = 40060,//	Interaction has already been acknowledged
		tag_names_must_be_unique = 40061,//	Tag names must be unique
		service_resource_rate_limited = 40062,//	Service resource is being rate limited
		no_tags_available_for_non_moderators = 40066,//	There are no tags available that can be set by non - moderators
		tag_required_create_forum_post = 40067,//	A tag is required to create a forum post in this channel
		entitlement_already_granted_for_this_resource = 40074,//	An entitlement has already been granted for this resource
		cloudflare_blocked_request = 40333,//	Cloudflare is blocking your request.This can often be resolved by setting a proper User Agent
		missing_access = 50001,//	Missing access
		invalid_account_type = 50002,//	Invalid account type
		action_not_available_DM_channel = 50003,//	Cannot execute action on a DM channel
		guild_widget_disabled = 50004,//	Guild widget disabled
		cannot_edit_message_not_author = 50005,//	Cannot edit a message authored by another user
		cannot_send_empty_message = 50006,//	Cannot send an empty message
		cannot_send_messages_to_user = 50007,//	Cannot send messages to this user
		cannot_send_messages_non_text_channel = 50008,//	Cannot send messages in a non - text channel
		channel_verification_level_too_high = 50009,//	Channel verification level is too high for you to gain access
		oauth2_application_without_bot = 50010,//	OAuth2 application does not have a bot
		oauth2_application_limit_reached = 50011,//	OAuth2 application limit reached
		oauth2_invalid_state = 50012,//	Invalid OAuth2 state
		missing_permissions_for_action = 50013,//	You lack permissions to perform that action
		invalid_authentication_token = 50014,//	Invalid authentication token provided
		note_too_long = 50015,//	Note was too long
		provided_incorrect_messages_amount_delete = 50016,//	Provided too few or too many messages to delete.Must provide at least 2 and fewer than 100 messages to delete
		invalid_MFA_level = 50017,//	Invalid MFA Level
		pin_allowed_only_original_channel = 50019,//	A message can only be pinned to the channel it was sent in
		invalid_or_taken_invite_code = 50020,//	Invite code was either invalid or taken
		action_not_allowed_system_message = 50021,//	Cannot execute action on a system message
		action_not_allowed_channel = 50024,//	Cannot execute action on this channel type
		oauth2_invalid_access_token = 50025,//	Invalid OAuth2 access token provided
		missing_required_oauth2_scope = 50026,//	Missing required OAuth2 scope
		invalid_webhook_token = 50027,//	Invalid webhook token provided
		invalid_role = 50028,//	Invalid role
		invalid_recipient = 50033,//	Invalid Recipient(s)
		message_too_old_bulk_delete = 50034,//	A message provided was too old to bulk delete
		invalid_form_body = 50035,//	Invalid form body(returned for both application / json and multipart / form - data bodies), or invalid Content - Type provided
		guild_invite_accepted_bot_not_present = 50036,//	An invite was accepted to a guild the application's bot is not in
		invalid_activity_action = 50039,//	Invalid Activity Action
		invalid_API_version = 50041,//	Invalid API version provided
		file_exceeds_max_size = 50045,//	File uploaded exceeds the maximum size
		invalid_file_uploaded = 50046,//	Invalid file uploaded
		cannot_self_redeem_gift = 50054,//	Cannot self - redeem this gift
		invalid_guild = 50055,//	Invalid Guild
		invalid_SKU = 50057,//	Invalid SKU
		invalid_request_origin = 50067,//	Invalid request origin
		invalid_message_type = 50068,//	Invalid message type
		payment_source_required_gift_redeem = 50070,//	Payment source required to redeem gift
		cannot_modify_system_webhook = 50073,//	Cannot modify a system webhook
		cannot_delete_channel_required_community_guilds = 50074,//	Cannot delete a channel required for Community guilds
		cannot_edit_message_stickers = 50080,//	Cannot edit stickers within a message
		invalid_sticker = 50081,//	Invalid sticker sent
		operation_not_allowed_archived_thread = 50083,//	Tried to perform an operation on an archived thread, such as editing a message or adding a user to the thread
		invalid_thread_notification_settings = 50084,//	Invalid thread notification settings
		before_value_earlier_thread_creation_date = 50085,//	before value is earlier than the thread creation date
		community_server_channel_only_text_allowed = 50086,//	Community server channels must be text channels
		event_entity_mismatch = 50091,//	The entity type of the event is different from the entity you are trying to start the event for
		server_not_available_location = 50095,//	This server is not available in your location
		monetization_enabled_required = 50097,//	This server needs monetization enabled in order to perform this action
		missing_boosts_for_action = 50101,//	This server needs more boosts to perform this action
		invalid_JSON_request_body = 50109,//	The request body contains invalid JSON.
		owner_cannot_be_pending_member = 50131,//	Owner cannot be pending member
		ownership_cannot_be_transferred_to_bot = 50132,//	Ownership cannot be transferred to a bot user
		failed_asset_resize_below_max_size = 50138,//	Failed to resize asset below the maximum size : 262144
		emoji_subscription_roles_mismatch = 50144,//	Cannot mix subscription and non subscription roles for an emoji
		emoji_premium_normal_conversion_not_allowed = 50145,//	Cannot convert between premium emoji and normal emoji
		uploaded_file_not_found = 50146,//	Uploaded file not found.
		voice_messages_additional_content_unsupported = 50159,//	Voice messages do not support additional content.
		voice_messages_single_audio_attachment_required = 50160,//	Voice messages must have a single audio attachment.
		voice_messages_supporting_metadata_required = 50161,//	Voice messages must have supporting metadata.
		voice_messages_edit_not_allowed = 50162,//	Voice messages cannot be edited.
		cannot_delete_guild_subscription_integration = 50163,//	Cannot delete guild subscription integration
		cannot_send_voice_message_channel = 50173,//	You cannot send voice messages in this channel.
		user_verification_required = 50178,//	The user account must first be verified
		send_sticker_missing_permission = 50600,//	You do not have permission to send this sticker.
		operation_requires_two_factor = 60003,//	Two factor is required for this operation
		discord_tag_no_users = 80004,//	No users with DiscordTag exist
		blocked_reaction = 90001,//	Reaction was blocked
		burst_reactions_not_allowed = 90002,//	User cannot use burst reactions
		application_not_available = 110001,//	Application not yet available.Try again later
		API_resource_overloaded = 130000,//	API resource is currently overloaded.Try again a little later
		stage_already_open = 150006,//	The Stage is already open
		reply_not_allowed_missing_permission = 160002,//	Cannot reply without permission to read message history
		thread_already_created = 160004,//	A thread has already been created for this message
		locked_thread = 160005,//	Thread is locked
		max_number_of_active_threads_reached = 160006,//	Maximum number of active threads reached
		max_number_of_active_announcement_threads_reached = 160007,//	Maximum number of active announcement threads reached
		uploaded_lottie_file_invalid_JSON = 170001,//	Invalid JSON for uploaded Lottie file
		uploaded_lottie_rasterized_images_not_allowed = 170002,//	Uploaded Lotties cannot contain rasterized images such as PNG or JPEG
		max_sticker_framerate_exceeded = 170003,//	Sticker maximum framerate exceeded
		max_sticker_frame_count_exceeded = 170004,//	Sticker frame count exceeds maximum of 1000 frames
		max_lottie_animation_dimensions_exceeded = 170005,//	Lottie animation maximum dimensions exceeded
		sticker_framerate_exceeded_limits = 170006,//	Sticker frame rate is either too small or too large
		max_sticker_animation_duration_exceeded = 170007,//	Sticker animation duration exceeds maximum of 5 seconds
		cannot_update_finished_event = 180000,//	Cannot update a finished event
		failed_stage_create_for_stage_event = 180002,//	Failed to create stage needed for stage event
		automod_blocked_message = 200000,//	Message was blocked by automatic moderation
		automod_blocked_title = 200001,//	Title was blocked by automatic moderation
		webhook_missing_name_or_id = 220001,//	Webhooks posted to forum channels must have a thread_name or thread_id
		webhook_both_name_and_id_not_allowed = 220002,//	Webhooks posted to forum channels cannot have both a thread_name and thread_id
		webhook_create_thread_not_allowed = 220003,//	Webhooks can only create threads in forum channels
		webhook_services_not_allowed = 220004,//	Webhook services cannot be used in forum channels
		blocked_message_harmful_links = 240000,//	Message blocked by harmful links filter
		cannot_enable_onboarding_missing_requirements = 350000,//	Cannot enable onboarding, requirements are not met
		cannot_update_onboarding_below_requirements = 350001,//	Cannot update onboarding while below requirements
		ban_users_failed = 500000,//	Failed to ban users
		poll_voting_blocked = 520000,//	Poll voting blocked
		poll_expired = 520001,//	Poll expired
		invalid_channel_poll_creation = 520002,//	Invalid channel type for poll creation
		edit_poll_message_not_allowed = 520003,//	Cannot edit a poll message
		poll_emoji_not_allowed = 520004,//	Cannot use an emoji included with the poll
		non_poll_message_cannot_expire = 520006,//	Cannot expire a non - poll message
	};

	extern const char* get_json_err_text(const std::underlying_type<json_err>::type error) noexcept;
	extern constexpr bool is_json_rate_limited(const std::underlying_type<json_err>::type error) noexcept;
}

#endif //H_MLN_DB_JSON_ERR_H