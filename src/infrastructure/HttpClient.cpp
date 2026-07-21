#include "apply-tracker/infrastructure/HttpClient.hpp"

#include <curl/curl.h>

#include <memory>
#include <stdexcept>
#include <string>

namespace apply_tracker::infrastructure {

namespace {

std::size_t writeCallback(
    void* contents,
    std::size_t size,
    std::size_t memberSize,
    void* userData
) {
    const std::size_t totalSize =
        size * memberSize;

    auto* response =
        static_cast<std::string*>(userData);

    response->append(
        static_cast<char*>(contents),
        totalSize
    );

    return totalSize;
}

struct CurlDeleter {
    void operator()(CURL* curl) const {
        if (curl != nullptr) {
            curl_easy_cleanup(curl);
        }
    }
};

struct HeaderDeleter {
    void operator()(curl_slist* headers) const {
        if (headers != nullptr) {
            curl_slist_free_all(headers);
        }
    }
};

} // namespace

HttpClient::HttpClient() {
    const CURLcode result =
        curl_global_init(CURL_GLOBAL_DEFAULT);

    if (result != CURLE_OK) {
        throw std::runtime_error(
            "Failed to initialise libcurl"
        );
    }
}

HttpClient::~HttpClient() {
    curl_global_cleanup();
}

HttpResponse HttpClient::get(
    const std::string& url,
    const std::map<std::string, std::string>& headers
) const {
    return request(
        "GET",
        url,
        "",
        headers
    );
}

HttpResponse HttpClient::post(
    const std::string& url,
    const std::string& body,
    const std::map<std::string, std::string>& headers
) const {
    return request(
        "POST",
        url,
        body,
        headers
    );
}

HttpResponse HttpClient::request(
    const std::string& method,
    const std::string& url,
    const std::string& body,
    const std::map<std::string, std::string>& headers
) const {
    std::unique_ptr<CURL, CurlDeleter> curl{
        curl_easy_init()
    };

    if (!curl) {
        throw std::runtime_error(
            "Failed to create CURL handle"
        );
    }

    HttpResponse response;

    curl_easy_setopt(
        curl.get(),
        CURLOPT_URL,
        url.c_str()
    );

    curl_easy_setopt(
        curl.get(),
        CURLOPT_WRITEFUNCTION,
        writeCallback
    );

    curl_easy_setopt(
        curl.get(),
        CURLOPT_WRITEDATA,
        &response.body
    );

    curl_easy_setopt(
        curl.get(),
        CURLOPT_FOLLOWLOCATION,
        1L
    );

    curl_easy_setopt(
        curl.get(),
        CURLOPT_TIMEOUT,
        30L
    );

    curl_easy_setopt(
        curl.get(),
        CURLOPT_NOSIGNAL,
        1L
    );

    if (method == "POST") {
        curl_easy_setopt(
            curl.get(),
            CURLOPT_POST,
            1L
        );

        curl_easy_setopt(
            curl.get(),
            CURLOPT_POSTFIELDS,
            body.c_str()
        );

        curl_easy_setopt(
            curl.get(),
            CURLOPT_POSTFIELDSIZE,
            static_cast<long>(body.size())
        );
    }

    curl_slist* rawHeaders = nullptr;

    for (const auto& [name, value] : headers) {
        const std::string header =
            name + ": " + value;

        rawHeaders = curl_slist_append(
            rawHeaders,
            header.c_str()
        );
    }

    std::unique_ptr<curl_slist, HeaderDeleter>
        headerList{rawHeaders};

    if (headerList) {
        curl_easy_setopt(
            curl.get(),
            CURLOPT_HTTPHEADER,
            headerList.get()
        );
    }

    const CURLcode result =
        curl_easy_perform(curl.get());

    if (result != CURLE_OK) {
        throw std::runtime_error(
            std::string{"HTTP request failed: "} +
            curl_easy_strerror(result)
        );
    }

    curl_easy_getinfo(
        curl.get(),
        CURLINFO_RESPONSE_CODE,
        &response.statusCode
    );

    return response;
}

std::string HttpClient::urlEncode(
    const std::string& value
) {
    std::unique_ptr<CURL, CurlDeleter> curl{
        curl_easy_init()
    };

    if (!curl) {
        throw std::runtime_error(
            "Failed to create CURL handle"
        );
    }

    char* encoded = curl_easy_escape(
        curl.get(),
        value.c_str(),
        static_cast<int>(value.size())
    );

    if (encoded == nullptr) {
        throw std::runtime_error(
            "Failed to URL-encode value"
        );
    }

    const std::string result{encoded};
    curl_free(encoded);

    return result;
}

} // namespace apply_tracker::infrastructure