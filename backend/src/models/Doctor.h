#pragma once

#include <optional>
#include <string>
#include <vector>

namespace healthcare::models
{
    struct Doctor
    {
        int id{};
        std::string fullName;
        std::string specialty;
        int departmentId{};
        std::string departmentName;
    };

    struct DoctorScheduleEntry
    {
        int appointmentId{};
        int patientId{};
        std::string patientName;
        std::string appointmentDatetime;
        std::string status;
    };

    struct DoctorSchedule
    {
        int doctorId{};
        std::string doctorName;
        std::string specialty;
        std::string departmentName;
        std::string date;

        std::vector<DoctorScheduleEntry> appointments;
    };

    enum class DoctorScheduleOutcome
    {
        Found,
        DoctorNotFound
    };

    struct DoctorScheduleResult
    {
        DoctorScheduleOutcome outcome;
        std::optional<DoctorSchedule> schedule;
    };

    struct DoctorAvailability
    {
        int doctorId{};
        std::string doctorName;
        std::string specialty;
        std::string departmentName;
        std::string date;

        std::vector<std::string> availableSlots;
    };

    enum class DoctorAvailabilityOutcome
    {
        Found,
        DoctorNotFound
    };

    struct DoctorAvailabilityResult
    {
        DoctorAvailabilityOutcome outcome;
        std::optional<DoctorAvailability> availability;
    };
}