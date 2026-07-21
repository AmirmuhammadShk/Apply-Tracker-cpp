#pragma once

#include "apply-tracker/infrastructure/HttpClient.hpp"

#include <chrono>
#include <filesystem>
#include <mutex>
#include <string>

namespace apply_tracker::infrastructure {

class GoogleAuth {
public:
    GoogleAuth(
        const HttpClient& httpClient,
        std::filesystem::path serviceAccountFile
    );

    std::string accessToken();

private:
    const HttpClient& httpClient_;
    std::filesystem::path serviceAccountFile_;

    std::mutex mutex_;
    std::string cachedToken_;

    std::chrono::system_clock::time_point
        tokenExpiry_;

    std::string clientEmail_;
    std::string privateKey_;
    std::string tokenUri_;

    void loadServiceAccount();

    std::string createJwtAssertion() const;
    std::string requestAccessToken();

    static std::string base64UrlEncode(
        const std::string& input
    );

    static std::string signRs256(
        const std::string& data,
        const std::string& privateKey
    );
};

} // namespace apply_tracker::infrastructure