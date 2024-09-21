#include "utility/http_err.h"

#include <unordered_map>

const char* mln::get_http_err_text(const std::underlying_type<http_err>::type error)
{
    static const char* s_invalid_error_text = "Unknown http error";
    static const char* s_server_error_text = ">502: The server had an error processing your request (these are rare).";
    static const std::unordered_map<std::underlying_type<http_err>::type, const char*> s_error_to_text_map{
        {200, "200: (OK)The request completed successfully."},
        {201, "201: (CREATED)The entity was created successfully."},
        {204, "204: (NO CONTENT) The request completed successfully but returned no content."},
        {304, "304: (NOT MODIFIED) The entity was not modified (no action was taken)."},
        {400, "400: (BAD REQUEST) The request was improperly formatted, or the server couldn't understand it."},
        {401, "401: (UNAUTHORIZED) The Authorization header was missing or invalid."},
        {403, "403: (FORBIDDEN) The Authorization token you passed did not have permission to the resource."},
        {404, "404: (NOT FOUND) The resource at the location specified doesn't exist."},
        {405, "405: (METHOD NOT ALLOWED) The HTTP method used is not valid for the location specified."},
        {429, "429: (TOO MANY REQUESTS) You are being rate limited, see Rate Limits."},
        {502, "502: (GATEWAY UNAVAILABLE) There was not a gateway available to process your request. Wait a bit and retry."},
    };

    const std::unordered_map<std::underlying_type<http_err>::type, const char*>::const_iterator it = s_error_to_text_map.find(error);
    if (it != s_error_to_text_map.end()) {
        return it->second;
    }

    if (error > 502 && error < 600) {
        return s_server_error_text;
    }

    return s_invalid_error_text;
}

const char* mln::get_dpp_http_err_text(const dpp::http_error error)
{
    static const char* s_invalid_error_text = "Unknown dpp http error";
    static constexpr size_t s_error_to_text_map_size = 13;
    static constexpr const char* s_error_to_text_map[s_error_to_text_map_size]{
        "h_success: Request successful.",
        "h_unknown: Status unknown.",
        "h_connection: Connect failed.",
        "h_bind_ip_address: Invalid local ip address.",
        "h_read: Read error.",
        "h_write: Write error.",
        "h_exceed_redirect_count: Too many 30x redirects.",
        "h_canceled: Request cancelled.",
        "h_ssl_connection: SSL connection error.",
        "h_ssl_loading_certs: SSL cert loading error.",
        "h_ssl_server_verification: SSL server verification error.",
        "h_unsupported_multipart_boundary_chars: Unsupported multipart boundary characters.",
        "h_compression: Compression error.",
    };

    if (error >= 0 && error < s_error_to_text_map_size) {
        return s_error_to_text_map[error];
    }

    return s_invalid_error_text;
}

constexpr bool mln::is_http_rate_limited(const std::underlying_type<http_err>::type error)
{
    return error == static_cast<std::underlying_type<http_err>::type>(http_err::too_many_requests);
}
