#pragma once

#include "apply-tracker/domain/ApplicationRepository.hpp"
#include "apply-tracker/infrastructure/GoogleAuth.hpp"
#include "apply-tracker/infrastructure/HttpClient.hpp"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace apply_tracker::infrastructure {

class GoogleSheetsRepository final
    : public domain::ApplicationRepository {
public:
    GoogleSheetsRepository(
        const HttpClient& httpClient,
        GoogleAuth& googleAuth,
        std::string spreadsheetId,
        std::string worksheetName
    );

    void add(
        const domain::JobApplication& application
    ) override;

    std::vector<domain::JobApplication>
    list() override;

private:
    const HttpClient& httpClient_;
    GoogleAuth& googleAuth_;
    std::string spreadsheetId_;
    std::string worksheetName_;

    std::string authorizationHeader();
    std::string escapedWorksheetName() const;

    static std::string cellAsString(
        const nlohmann::json& row,
        std::size_t index
    );
};

} // namespace apply_tracker::infrastructure