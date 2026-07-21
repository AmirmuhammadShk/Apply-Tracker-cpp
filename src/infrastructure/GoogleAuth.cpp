#include "apply-tracker/infrastructure/GoogleAuth.hpp"

#include <nlohmann/json.hpp>

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include <fstream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace apply_tracker::infrastructure {

namespace {

struct BioDeleter {
    void operator()(BIO* bio) const {
        if (bio != nullptr) {
            BIO_free(bio);
        }
    }
};

struct PkeyDeleter {
    void operator()(EVP_PKEY* key) const {
        if (key != nullptr) {
            EVP_PKEY_free(key);
        }
    }
};

struct MdContextDeleter {
    void operator()(EVP_MD_CTX* context) const {
        if (context != nullptr) {
            EVP_MD_CTX_free(context);
        }
    }
};

} // namespace

GoogleAuth::GoogleAuth(
    const HttpClient& httpClient,
    std::filesystem::path serviceAccountFile
)
    : httpClient_(httpClient),
      serviceAccountFile_(
          std::move(serviceAccountFile)
      ),
      tokenExpiry_(
          std::chrono::system_clock::time_point::min()
      ) {
    loadServiceAccount();
}

void GoogleAuth::loadServiceAccount() {
    std::ifstream input{serviceAccountFile_};

    if (!input) {
        throw std::runtime_error(
            "Cannot open service account file: " +
            serviceAccountFile_.string()
        );
    }

    nlohmann::json json;
    input >> json;

    clientEmail_ =
        json.at("client_email").get<std::string>();

    privateKey_ =
        json.at("private_key").get<std::string>();

    tokenUri_ =
        json.value(
            "token_uri",
            "https://oauth2.googleapis.com/token"
        );
}

std::string GoogleAuth::accessToken() {
    std::lock_guard<std::mutex> lock{mutex_};

    const auto now =
        std::chrono::system_clock::now();

    if (
        !cachedToken_.empty() &&
        now + std::chrono::minutes{5} < tokenExpiry_
    ) {
        return cachedToken_;
    }

    return requestAccessToken();
}

std::string GoogleAuth::createJwtAssertion() const {
    const auto now =
        std::chrono::system_clock::now();

    const auto issuedAt =
        std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()
        ).count();

    const auto expiresAt =
        issuedAt + 3600;

    const nlohmann::json header = {
        {"alg", "RS256"},
        {"typ", "JWT"}
    };

    const nlohmann::json payload = {
        {"iss", clientEmail_},
        {
            "scope",
            "https://www.googleapis.com/auth/spreadsheets"
        },
        {"aud", tokenUri_},
        {"iat", issuedAt},
        {"exp", expiresAt}
    };

    const std::string encodedHeader =
        base64UrlEncode(header.dump());

    const std::string encodedPayload =
        base64UrlEncode(payload.dump());

    const std::string signingInput =
        encodedHeader + "." + encodedPayload;

    const std::string signature =
        signRs256(signingInput, privateKey_);

    return signingInput + "." + signature;
}

std::string GoogleAuth::requestAccessToken() {
    const std::string assertion =
        createJwtAssertion();

    const std::string body =
        "grant_type=" +
        HttpClient::urlEncode(
            "urn:ietf:params:oauth:grant-type:jwt-bearer"
        ) +
        "&assertion=" +
        HttpClient::urlEncode(assertion);

    const HttpResponse response =
        httpClient_.post(
            tokenUri_,
            body,
            {
                {
                    "Content-Type",
                    "application/x-www-form-urlencoded"
                }
            }
        );

    if (
        response.statusCode < 200 ||
        response.statusCode >= 300
    ) {
        throw std::runtime_error(
            "Google authentication failed with HTTP " +
            std::to_string(response.statusCode) +
            ": " +
            response.body
        );
    }

    const nlohmann::json json =
        nlohmann::json::parse(response.body);

    cachedToken_ =
        json.at("access_token").get<std::string>();

    const int expiresIn =
        json.value("expires_in", 3600);

    tokenExpiry_ =
        std::chrono::system_clock::now() +
        std::chrono::seconds{expiresIn};

    return cachedToken_;
}

std::string GoogleAuth::base64UrlEncode(
    const std::string& input
) {
    BIO* base64Bio =
        BIO_new(BIO_f_base64());

    BIO* memoryBio =
        BIO_new(BIO_s_mem());

    if (
        base64Bio == nullptr ||
        memoryBio == nullptr
    ) {
        if (base64Bio != nullptr) {
            BIO_free(base64Bio);
        }

        if (memoryBio != nullptr) {
            BIO_free(memoryBio);
        }

        throw std::runtime_error(
            "Failed to create Base64 encoder"
        );
    }

    BIO_set_flags(
        base64Bio,
        BIO_FLAGS_BASE64_NO_NL
    );

    BIO* chain =
        BIO_push(base64Bio, memoryBio);

    if (
        BIO_write(
            chain,
            input.data(),
            static_cast<int>(input.size())
        ) <= 0
    ) {
        BIO_free_all(chain);

        throw std::runtime_error(
            "Failed to encode Base64 input"
        );
    }

    if (BIO_flush(chain) != 1) {
        BIO_free_all(chain);

        throw std::runtime_error(
            "Failed to flush Base64 encoder"
        );
    }

    BUF_MEM* buffer = nullptr;
    BIO_get_mem_ptr(chain, &buffer);

    if (buffer == nullptr) {
        BIO_free_all(chain);

        throw std::runtime_error(
            "Failed to read Base64 output"
        );
    }

    std::string encoded(
        buffer->data,
        buffer->length
    );

    BIO_free_all(chain);

    for (char& character : encoded) {
        if (character == '+') {
            character = '-';
        } else if (character == '/') {
            character = '_';
        }
    }

    while (
        !encoded.empty() &&
        encoded.back() == '='
    ) {
        encoded.pop_back();
    }

    return encoded;
}

std::string GoogleAuth::signRs256(
    const std::string& data,
    const std::string& privateKey
) {
    std::unique_ptr<BIO, BioDeleter> keyBio{
        BIO_new_mem_buf(
            privateKey.data(),
            static_cast<int>(privateKey.size())
        )
    };

    if (!keyBio) {
        throw std::runtime_error(
            "Failed to create private-key buffer"
        );
    }

    std::unique_ptr<EVP_PKEY, PkeyDeleter> key{
        PEM_read_bio_PrivateKey(
            keyBio.get(),
            nullptr,
            nullptr,
            nullptr
        )
    };

    if (!key) {
        throw std::runtime_error(
            "Failed to parse service-account private key"
        );
    }

    std::unique_ptr<EVP_MD_CTX, MdContextDeleter>
        context{EVP_MD_CTX_new()};

    if (!context) {
        throw std::runtime_error(
            "Failed to create signing context"
        );
    }

    if (
        EVP_DigestSignInit(
            context.get(),
            nullptr,
            EVP_sha256(),
            nullptr,
            key.get()
        ) != 1
    ) {
        throw std::runtime_error(
            "Failed to initialise JWT signing"
        );
    }

    if (
        EVP_DigestSignUpdate(
            context.get(),
            data.data(),
            data.size()
        ) != 1
    ) {
        throw std::runtime_error(
            "Failed to update JWT signature"
        );
    }

    std::size_t signatureLength{0};

    if (
        EVP_DigestSignFinal(
            context.get(),
            nullptr,
            &signatureLength
        ) != 1
    ) {
        throw std::runtime_error(
            "Failed to determine signature size"
        );
    }

    std::vector<unsigned char> signature(
        signatureLength
    );

    if (
        EVP_DigestSignFinal(
            context.get(),
            signature.data(),
            &signatureLength
        ) != 1
    ) {
        throw std::runtime_error(
            "Failed to generate JWT signature"
        );
    }

    signature.resize(signatureLength);

    const std::string binarySignature(
        reinterpret_cast<const char*>(
            signature.data()
        ),
        signature.size()
    );

    return base64UrlEncode(binarySignature);
}

} // namespace apply_tracker::infrastructure