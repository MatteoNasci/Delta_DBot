#pragma once
#ifndef H_MLN_DB_HTTP_ERR_H
#define H_MLN_DB_HTTP_ERR_H

#include <dpp/queues.h>

#include <type_traits>

namespace mln {
	enum class http_err : uint16_t {
		ok = 200, //(OK)The request completed successfully.
		created = 201, //(CREATED)The entity was created successfully.
		no_content = 204, //(NO CONTENT)	The request completed successfully but returned no content.
		not_modified = 304, //(NOT MODIFIED)	The entity was not modified(no action was taken).
		bad_request = 400, //(BAD REQUEST)	The request was improperly formatted, or the server couldn't understand it.
		unauthorized = 401, //(UNAUTHORIZED)The Authorization header was missing or invalid.
		forbidden = 403, //(FORBIDDEN)The Authorization token you passed did not have permission to the resource.
		not_found = 404, //(NOT FOUND)	The resource at the location specified doesn't exist.
		method_not_allowed = 405, //(METHOD NOT ALLOWED)	The HTTP method used is not valid for the location specified.
		too_many_requests = 429, //(TOO MANY REQUESTS)	You are being rate limited, see Rate Limits.
		gateway_unavailable = 502, //(GATEWAY UNAVAILABLE)	There was not a gateway available to process your request.Wait a bit and retry.
	};

	extern const char* get_http_err_text(const std::underlying_type<http_err>::type error);

	extern const char* get_dpp_http_err_text(const dpp::http_error error);

	extern constexpr bool is_http_rate_limited(const std::underlying_type<http_err>::type error);
}

#endif //H_MLN_DB_HTTP_ERR_H