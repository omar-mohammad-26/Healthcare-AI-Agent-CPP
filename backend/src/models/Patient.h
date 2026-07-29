#pragma once

#include <optional>
#include <string>

namespace healthcare::models
{
    struct Patient
    {
        int id{};
        std::string fullName;
        std::string phone;
        int age{};
        std::string gender;
        std::optional<std::string> medicalHistory;
        std::string createdAt;
    };

    struct CreatePatientInput
    {
        std::string fullName;
        std::string phone;
        int age{};
        std::string gender;
        std::optional<std::string> medicalHistory;
    };
}