#pragma once

#include <optional>
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

    struct CreateAppointmentInput
    {
        int patientId{};
        int doctorId{};
        std::string appointmentDatetime;
    };

    enum class CreateAppointmentOutcome
    {
        Created,
        PatientNotFound,
        DoctorNotFound,
        PastDatetime,
        Conflict
    };

    struct CreateAppointmentResult
    {
        CreateAppointmentOutcome outcome;
        std::optional<Appointment> appointment;
    };

    enum class CancelAppointmentOutcome
    {
        Cancelled,
        NotFound,
        AlreadyCancelled,
        Completed,
        Conflict
    };

    struct CancelAppointmentResult
    {
        CancelAppointmentOutcome outcome;
        std::optional<Appointment> appointment;
    };
}