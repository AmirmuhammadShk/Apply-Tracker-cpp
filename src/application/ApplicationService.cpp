#include "apply-tracker/application/ApplicationService.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace apply_tracker::application {

ApplicationService::ApplicationService(
    domain::ApplicationRepository& repository
)
    : repository_(repository) {
}

void ApplicationService::addApplication(
    domain::JobApplication application
) {
    if (application.date.empty()) {
        application.date = currentDate();
    }

    if (application.status.empty()) {
        application.status = "Applied";
    }

    validate(application);
    repository_.add(application);
}

std::vector<domain::JobApplication>
ApplicationService::listApplications() {
    return repository_.list();
}

void ApplicationService::validate(
    const domain::JobApplication& application
) {
    if (application.role.empty()) {
        throw std::invalid_argument(
            "Role cannot be empty"
        );
    }

    if (application.company.empty()) {
        throw std::invalid_argument(
            "Company cannot be empty"
        );
    }

    if (application.jobDescription.empty()) {
        throw std::invalid_argument(
            "Job description cannot be empty"
        );
    }

    if (application.location.empty()) {
        throw std::invalid_argument(
            "Location cannot be empty"
        );
    }

    if (application.type.empty()) {
        throw std::invalid_argument(
            "Type cannot be empty"
        );
    }

    if (application.workingType.empty()) {
        throw std::invalid_argument(
            "Working type cannot be empty"
        );
    }
}

std::string ApplicationService::currentDate() {
    const auto now =
        std::chrono::system_clock::now();

    const std::time_t currentTime =
        std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};

#ifdef _WIN32
    localtime_s(&localTime, &currentTime);
#else
    localtime_r(&currentTime, &localTime);
#endif

    std::ostringstream output;

    output << std::put_time(
        &localTime,
        "%Y-%m-%d"
    );

    return output.str();
}

} // namespace apply_tracker::application