#pragma once

#include <string>

namespace healthcare::models
{
    struct Treatment
    {
        int id{};

        int patientId{};
        std::string patientName;

        int doctorId{};
        std::string doctorName;
        std::string doctorSpecialty;

        std::string diagnosis;
        std::string treatmentPlan;
        std::string createdAt;
    };
}