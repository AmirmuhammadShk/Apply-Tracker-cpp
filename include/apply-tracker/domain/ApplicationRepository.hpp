#pragma once

#include "apply-tracker/domain/JobApplication.hpp"

#include <vector>

namespace apply_tracker::domain {

class ApplicationRepository {
public:
    virtual ~ApplicationRepository() = default;

    virtual void add(
        const JobApplication& application
    ) = 0;

    virtual std::vector<JobApplication> list() = 0;
};

} // namespace apply_tracker::domain