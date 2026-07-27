#pragma once

#include <string>

namespace healthcare::models
{
    struct Appointment
    {
        int id{};

        int patientId{};
        std::string patientName;

        int doctorId{};
        std::string doctorName;
        std::string doctorSpecialty;

        std::string appointmentDatetime;
        std::string status;
        std::string createdAt;
    };
}