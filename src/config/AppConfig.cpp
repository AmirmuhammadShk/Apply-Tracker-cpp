#include "apply-tracker/config/AppConfig.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace apply_tracker::config {

AppConfig AppConfig::load(
    const std::filesystem::path& path
) {
    std::ifstream input{path};

    if (!input) {
        throw std::runtime_error(
            "Cannot open configuration file: " +
            path.string()
        );
    }

    nlohmann::json json;
    input >> json;

    AppConfig config;

    config.spreadsheetId =
        json.at("spreadsheet_id").get<std::string>();

    config.worksheetName =
        json.value("worksheet_name", "Sheet1");

    config.serviceAccountFile =
        json.at("service_account_file")
            .get<std::string>();

    config.apiPort =
        json.value("api_port", 8080);

    if (config.spreadsheetId.empty()) {
        throw std::runtime_error(
            "spreadsheet_id cannot be empty"
        );
    }

    if (config.worksheetName.empty()) {
        throw std::runtime_error(
            "worksheet_name cannot be empty"
        );
    }

    return config;
}

} // namespace apply_tracker::config