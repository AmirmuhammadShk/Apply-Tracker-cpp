#include "apply-tracker/application/ApplicationService.hpp"
#include "apply-tracker/config/AppConfig.hpp"
#include "apply-tracker/domain/JobApplication.hpp"
#include "apply-tracker/infrastructure/GoogleAuth.hpp"
#include "apply-tracker/infrastructure/GoogleSheetsRepository.hpp"
#include "apply-tracker/infrastructure/HttpClient.hpp"

#include <algorithm>
#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr std::size_t maximumColumnWidth = 28;

std::string readRequiredLine(
    const std::string& prompt
) {
    while (true) {
        std::cout << prompt;

        std::string value;
        std::getline(std::cin, value);

        if (!value.empty()) {
            return value;
        }

        std::cout << "This value is required.\n";
    }
}

std::string readOptionalLine(
    const std::string& prompt,
    const std::string& defaultValue = ""
) {
    std::cout << prompt;

    std::string value;
    std::getline(std::cin, value);

    if (value.empty()) {
        return defaultValue;
    }

    return value;
}

std::string truncate(
    const std::string& value,
    std::size_t width
) {
    if (value.size() <= width) {
        return value;
    }

    if (width <= 3) {
        return value.substr(0, width);
    }

    return value.substr(0, width - 3) + "...";
}

void updateWidth(
    std::size_t& width,
    const std::string& value
) {
    width = std::max(
        width,
        std::min(
            value.size(),
            maximumColumnWidth
        )
    );
}

void printSeparator(
    const std::vector<std::size_t>& widths
) {
    std::cout << '+';

    for (const std::size_t width : widths) {
        std::cout
            << std::string(width + 2, '-')
            << '+';
    }

    std::cout << '\n';
}

void printRow(
    const std::vector<std::string>& values,
    const std::vector<std::size_t>& widths
) {
    std::cout << '|';

    for (std::size_t index = 0;
         index < values.size();
         ++index) {
        std::cout
            << ' '
            << std::left
            << std::setw(
                static_cast<int>(widths[index])
            )
            << truncate(values[index], widths[index])
            << " |";
    }

    std::cout << '\n';
}

void printApplications(
    const std::vector<
        apply_tracker::domain::JobApplication
    >& applications
) {
    if (applications.empty()) {
        std::cout << "No applications found.\n";
        return;
    }

    const std::vector<std::string> headers = {
        "Date",
        "Role",
        "Company",
        "Job Description",
        "Location",
        "Type",
        "Working Type",
        "Salary",
        "Status"
    };

    std::vector<std::size_t> widths;

    for (const auto& header : headers) {
        widths.push_back(header.size());
    }

    for (const auto& application : applications) {
        updateWidth(widths[0], application.date);
        updateWidth(widths[1], application.role);
        updateWidth(widths[2], application.company);

        updateWidth(
            widths[3],
            application.jobDescription
        );

        updateWidth(widths[4], application.location);
        updateWidth(widths[5], application.type);

        updateWidth(
            widths[6],
            application.workingType
        );

        updateWidth(widths[7], application.salary);
        updateWidth(widths[8], application.status);
    }

    printSeparator(widths);
    printRow(headers, widths);
    printSeparator(widths);

    for (const auto& application : applications) {
        printRow(
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
            },
            widths
        );
    }

    printSeparator(widths);

    std::cout
        << applications.size()
        << " application(s)\n";
}

void addApplication(
    apply_tracker::application::
        ApplicationService& service
) {
    apply_tracker::domain::JobApplication
        application;

    std::cout << "\nAdd job application\n";
    std::cout << "-------------------\n";

    application.date =
        readOptionalLine(
            "Date [today]: "
        );

    application.role =
        readRequiredLine(
            "Role: "
        );

    application.company =
        readRequiredLine(
            "Company: "
        );

    application.jobDescription =
        readRequiredLine(
            "Job description: "
        );

    application.location =
        readRequiredLine(
            "Location: "
        );

    application.type =
        readRequiredLine(
            "Type, for example Permanent: "
        );

    application.workingType =
        readRequiredLine(
            "Working type, for example Hybrid: "
        );

    application.salary =
        readOptionalLine(
            "Salary: "
        );

    application.status =
        readOptionalLine(
            "Status [Applied]: ",
            "Applied"
        );

    service.addApplication(application);

    std::cout
        << "Application added successfully.\n";
}

void runCli(
    apply_tracker::application::
        ApplicationService& service
) {
    bool running = true;

    while (running) {
        std::cout
            << "\nApply Tracker\n"
            << "-------------\n"
            << "1. Add application\n"
            << "2. List applications\n"
            << "0. Exit\n"
            << "Choose an option: ";

        std::string choice;
        std::getline(std::cin, choice);

        try {
            if (choice == "1") {
                addApplication(service);
            } else if (choice == "2") {
                printApplications(
                    service.listApplications()
                );
            } else if (choice == "0") {
                running = false;
            } else {
                std::cout << "Unknown option.\n";
            }
        } catch (const std::exception& exception) {
            std::cerr
                << "Operation failed: "
                << exception.what()
                << '\n';
        }
    }

    std::cout << "Goodbye!\n";
}

} // namespace

int main(int argc, char* argv[]) {
    try {
        const std::string configPath =
            argc > 1
                ? argv[1]
                : "config/app.json";

        const auto config =
            apply_tracker::config::AppConfig::load(
                configPath
            );

        apply_tracker::infrastructure::HttpClient
            httpClient;

        apply_tracker::infrastructure::GoogleAuth
            googleAuth{
                httpClient,
                config.serviceAccountFile
            };

        apply_tracker::infrastructure::
            GoogleSheetsRepository repository{
                httpClient,
                googleAuth,
                config.spreadsheetId,
                config.worksheetName
            };

        apply_tracker::application::
            ApplicationService service{
                repository
            };

        runCli(service);

        return 0;
    } catch (const std::exception& exception) {
        std::cerr
            << "Fatal error: "
            << exception.what()
            << '\n';

        return 1;
    }
}