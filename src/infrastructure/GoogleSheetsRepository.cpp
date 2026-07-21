#include "apply-tracker/infrastructure/GoogleSheetsRepository.hpp"

#include <sstream>
#include <stdexcept>
#include <utility>

namespace apply_tracker::infrastructure {

GoogleSheetsRepository::GoogleSheetsRepository(
    const HttpClient& httpClient,
    GoogleAuth& googleAuth,
    std::string spreadsheetId,
    std::string worksheetName
)
    : httpClient_(httpClient),
      googleAuth_(googleAuth),
      spreadsheetId_(std::move(spreadsheetId)),
      worksheetName_(std::move(worksheetName)) {
}

void GoogleSheetsRepository::add(
    const domain::JobApplication& application
) {
    const std::string range =
        HttpClient::urlEncode(
            escapedWorksheetName() + "!A:I"
        );

    const std::string url =
        "https://sheets.googleapis.com/v4/"
        "spreadsheets/" +
        spreadsheetId_ +
        "/values/" +
        range +
        ":append"
        "?valueInputOption=RAW"
        "&insertDataOption=INSERT_ROWS";

    const nlohmann::json body = {
        {
            "majorDimension",
            "ROWS"
        },
        {
            "values",
            {
                {
                    application.date,
                    application.role,
                    application.company,
                    application.jobDescription,
                    application.location,
                    application.type,
                    application.workingType,
                    application.salary,
                    application.status
                }
            }
        }
    };

    const HttpResponse response =
        httpClient_.post(
            url,
            body.dump(),
            {
                {
                    "Authorization",
                    authorizationHeader()
                },
                {
                    "Content-Type",
                    "application/json"
                }
            }
        );

    if (
        response.statusCode < 200 ||
        response.statusCode >= 300
    ) {
        throw std::runtime_error(
            "Failed to append Google Sheets row. HTTP " +
            std::to_string(response.statusCode) +
            ": " +
            response.body
        );
    }
}

std::vector<domain::JobApplication>
GoogleSheetsRepository::list() {
    const std::string range =
        HttpClient::urlEncode(
            escapedWorksheetName() + "!A2:I"
        );

    const std::string url =
        "https://sheets.googleapis.com/v4/"
        "spreadsheets/" +
        spreadsheetId_ +
        "/values/" +
        range +
        "?majorDimension=ROWS"
        "&valueRenderOption=FORMATTED_VALUE";

    const HttpResponse response =
        httpClient_.get(
            url,
            {
                {
                    "Authorization",
                    authorizationHeader()
                }
            }
        );

    if (
        response.statusCode < 200 ||
        response.statusCode >= 300
    ) {
        throw std::runtime_error(
            "Failed to read Google Sheets rows. HTTP " +
            std::to_string(response.statusCode) +
            ": " +
            response.body
        );
    }

    const nlohmann::json json =
        nlohmann::json::parse(response.body);

    std::vector<domain::JobApplication>
        applications;

    if (!json.contains("values")) {
        return applications;
    }

    for (const auto& row : json.at("values")) {
        domain::JobApplication application;

        application.date =
            cellAsString(row, 0);

        application.role =
            cellAsString(row, 1);

        application.company =
            cellAsString(row, 2);

        application.jobDescription =
            cellAsString(row, 3);

        application.location =
            cellAsString(row, 4);

        application.type =
            cellAsString(row, 5);

        application.workingType =
            cellAsString(row, 6);

        application.salary =
            cellAsString(row, 7);

        application.status =
            cellAsString(row, 8);

        applications.push_back(
            std::move(application)
        );
    }

    return applications;
}

std::string
GoogleSheetsRepository::authorizationHeader() {
    return "Bearer " + googleAuth_.accessToken();
}

std::string
GoogleSheetsRepository::escapedWorksheetName() const {
    std::string escaped;
    escaped.reserve(worksheetName_.size() + 2);

    escaped.push_back('\'');

    for (const char character : worksheetName_) {
        if (character == '\'') {
            escaped.push_back('\'');
        }

        escaped.push_back(character);
    }

    escaped.push_back('\'');

    return escaped;
}

std::string GoogleSheetsRepository::cellAsString(
    const nlohmann::json& row,
    std::size_t index
) {
    if (
        !row.is_array() ||
        index >= row.size() ||
        row[index].is_null()
    ) {
        return "";
    }

    if (row[index].is_string()) {
        return row[index].get<std::string>();
    }

    if (row[index].is_boolean()) {
        return row[index].get<bool>()
            ? "true"
            : "false";
    }

    if (row[index].is_number_integer()) {
        return std::to_string(
            row[index].get<long long>()
        );
    }

    if (row[index].is_number_unsigned()) {
        return std::to_string(
            row[index].get<unsigned long long>()
        );
    }

    if (row[index].is_number_float()) {
        std::ostringstream output;
        output << row[index].get<double>();
        return output.str();
    }

    return row[index].dump();
}

} // namespace apply_tracker::infrastructure