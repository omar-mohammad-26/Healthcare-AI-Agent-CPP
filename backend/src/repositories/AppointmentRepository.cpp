#include "repositories/AppointmentRepository.h"

#include <drogon/orm/Exception.h>
#include <drogon/orm/Result.h>

#include <utility>

namespace healthcare::repositories
{
AppointmentRepository::AppointmentRepository(
    drogon::orm::DbClientPtr databaseClient
)
    : databaseClient_(std::move(databaseClient))
{
}

void AppointmentRepository::findAll(
    std::string status,
    std::optional<int> doctorId,
    std::optional<int> patientId,
    SuccessCallback successCallback,
    ErrorCallback errorCallback
) const
{
    if (!databaseClient_)
    {
        errorCallback(
            "The PostgreSQL client is not available."
        );
        return;
    }

    const int doctorIdFilter =
        doctorId.value_or(0);

    const int patientIdFilter =
        patientId.value_or(0);

    databaseClient_->execSqlAsync(
        R"(
            SELECT
                appointment.id,

                appointment.patient_id,
                patient.full_name AS patient_name,

                appointment.doctor_id,
                doctor.full_name AS doctor_name,
                doctor.specialty AS doctor_specialty,

                appointment.appointment_datetime,
                appointment.status,
                appointment.created_at
            FROM appointments AS appointment
            JOIN patients AS patient
                ON patient.id = appointment.patient_id
            JOIN doctors AS doctor
                ON doctor.id = appointment.doctor_id
            WHERE
                (
                    CAST($1 AS TEXT) = ''
                    OR appointment.status =
                        CAST($1 AS TEXT)
                )
                AND
                (
                    CAST($2 AS INTEGER) = 0
                    OR appointment.doctor_id =
                        CAST($2 AS INTEGER)
                )
                AND
                (
                    CAST($3 AS INTEGER) = 0
                    OR appointment.patient_id =
                        CAST($3 AS INTEGER)
                )
            ORDER BY
                appointment.appointment_datetime ASC,
                appointment.id ASC
        )",
        [
            successCallback =
                std::move(successCallback)
        ](
            const drogon::orm::Result &result
        ) mutable
        {
            AppointmentList appointments;
            appointments.reserve(result.size());

            for (const auto &row : result)
            {
                healthcare::models::Appointment
                    appointment;

                appointment.id =
                    row["id"].as<int>();

                appointment.patientId =
                    row["patient_id"].as<int>();

                appointment.patientName =
                    row["patient_name"].as<std::string>();

                appointment.doctorId =
                    row["doctor_id"].as<int>();

                appointment.doctorName =
                    row["doctor_name"].as<std::string>();

                appointment.doctorSpecialty =
                    row["doctor_specialty"]
                        .as<std::string>();

                appointment.appointmentDatetime =
                    row["appointment_datetime"]
                        .as<std::string>();

                appointment.status =
                    row["status"].as<std::string>();

                appointment.createdAt =
                    row["created_at"].as<std::string>();

                appointments.push_back(
                    std::move(appointment)
                );
            }

            successCallback(
                std::move(appointments)
            );
        },
        [
            errorCallback =
                std::move(errorCallback)
        ](
            const drogon::orm::DrogonDbException
                &exception
        ) mutable
        {
            errorCallback(
                exception.base().what()
            );
        },
        std::move(status),
        doctorIdFilter,
        patientIdFilter
    );
}
}