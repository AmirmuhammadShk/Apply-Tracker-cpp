#pragma once

#include "apply-tracker/domain/ApplicationRepository.hpp"
#include "apply-tracker/domain/JobApplication.hpp"

#include <string>
#include <vector>

namespace apply_tracker::application {

class ApplicationService {
public:
    explicit ApplicationService(
        domain::ApplicationRepository& repository
    );

    void addApplication(
        domain::JobApplication application
    );

    std::vector<domain::JobApplication>
    listApplications();

private:
    domain::ApplicationRepository& repository_;

    static void validate(
        const domain::JobApplication& application
    );

    static std::string currentDate();
};

} // namespace apply_tracker::application