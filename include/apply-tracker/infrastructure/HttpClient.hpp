#pragma once

#include <map>
#include <string>

namespace apply_tracker::infrastructure {

struct HttpResponse {
    long statusCode{0};
    std::string body;
};

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;

    HttpResponse get(
        const std::string& url,
        const std::map<std::string, std::string>& headers = {}
    ) const;

    HttpResponse post(
        const std::string& url,
        const std::string& body,
        const std::map<std::string, std::string>& headers = {}
    ) const;

    static std::string urlEncode(
        const std::string& value
    );

private:
    HttpResponse request(
        const std::string& method,
        const std::string& url,
        const std::string& body,
        const std::map<std::string, std::string>& headers
    ) const;
};

} // namespace apply_tracker::infrastructure