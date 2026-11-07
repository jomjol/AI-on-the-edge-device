#include "api_router.h"

#include <string>

#include "api_info.h"
#include "api_response.h"
#include "basic_auth.h"

namespace
{

// One endpoint: path relative to /api/v1 and the method it answers to.
// The same path may appear several times with different methods.
struct ApiRoute
{
    httpd_method_t method;
    const char *path;
    esp_err_t (*handler)(httpd_req_t *req);
};

const ApiRoute api_routes[] = {
    {HTTP_GET, "/info", api_info_get_handler},
};

constexpr char API_PATH_PREFIX[] = "/api/v1";

// Returns the request path relative to /api/v1, without the query string.
std::string get_route_path(const httpd_req_t *req)
{
    std::string path(req->uri);
    path = path.substr(0, path.find('?'));
    return path.substr(sizeof(API_PATH_PREFIX) - 1);
}

// Finds the route for the request and calls it. All of /api/v1/ comes through here,
// so it takes up one URI handler slot however many routes the table has.
esp_err_t api_router_handler(httpd_req_t *req)
{
    const std::string path = get_route_path(req);
    std::string allowed_methods; // Methods that exist for this path, for the Allow header of a 405

    for (const ApiRoute &route : api_routes)
    {
        if (path != route.path)
        {
            continue;
        }

        if (route.method == req->method)
        {
            return route.handler(req);
        }

        allowed_methods += (allowed_methods.empty() ? "" : ", ");
        allowed_methods += http_method_str(route.method);
    }

    if (allowed_methods.empty())
    {
        return api_send_error(req, "404 Not Found", "not_found", "Unknown endpoint");
    }

    // httpd keeps the pointer until the response is sent, and allowed_methods outlives that
    httpd_resp_set_hdr(req, "Allow", allowed_methods.c_str());
    return api_send_error(req, "405 Method Not Allowed", "method_not_allowed", "Method not supported on this endpoint");
}

// Runs the router behind basic auth, with the 401 in the same JSON format as the other errors.
esp_err_t api_authorized_router_handler(httpd_req_t *req)
{
    return basic_auth_request_filter(req, api_router_handler, [](httpd_req_t *unauthorized_req) {
        httpd_resp_set_hdr(unauthorized_req, "WWW-Authenticate", BASIC_AUTH_CHALLENGE);
        return api_send_error(unauthorized_req, "401 Unauthorized", "unauthorized", "Valid credentials are required");
    });
}

} // namespace

void register_api_uri(httpd_handle_t server)
{
    httpd_uri_t api_handle = {
        .uri      = "/api/v1/*",
        .method   = static_cast<httpd_method_t>(HTTP_ANY), // The router checks the method, so wrong ones get a JSON error
        .handler  = api_authorized_router_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &api_handle);
}
