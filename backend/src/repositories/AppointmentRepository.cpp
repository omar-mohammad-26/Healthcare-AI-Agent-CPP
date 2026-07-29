#include "repositories/AppointmentRepository.h"

#include <drogon/orm/Exception.h>
#include <drogon/orm/Result.h>

#include <optional>
#include <string>
#include <utility>

namespace
{
healthcare::models::Appointment mapAppointment(
    const drogon::orm::Row &row
)
{
    healthcare::models::Appointment appointment;

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
        row["doctor_specialty"].as<std::string>();

    appointment.appointmentDatetime =
        row["appointment_datetime"].as<std::string>();

    appointment.status =
        row["status"].as<std::string>();

    appointment.createdAt =
        row["created_at"].as<std::string>();

    return appointment;
}
}

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
                appointments.push_back(
                    mapAppointment(row)
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
            const drogon::orm::DrogonDbException &exception
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

void AppointmentRepository::create(
    healthcare::models::CreateAppointmentInput input,
    CreateSuccessCallback successCallback,
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

    databaseClient_->execSqlAsync(
        R"(
            WITH input_data AS
            (
                SELECT
                    CAST($1 AS INTEGER) AS patient_id,
                    CAST($2 AS INTEGER) AS doctor_id,
                    CAST($3 AS TIMESTAMPTZ)
                        AS appointment_datetime
            ),
            validation AS
            (
                SELECT
                    input_data.patient_id,
                    input_data.doctor_id,
                    input_data.appointment_datetime,

                    EXISTS
                    (
                        SELECT 1
                        FROM patients
                        WHERE id = input_data.patient_id
                    ) AS patient_exists,

                    EXISTS
                    (
                        SELECT 1
                        FROM doctors
                        WHERE id = input_data.doctor_id
                    ) AS doctor_exists,

                    input_data.appointment_datetime >
                        CURRENT_TIMESTAMP AS is_future
                FROM input_data
            ),
            inserted AS
            (
                INSERT INTO appointments
                (
                    patient_id,
                    doctor_id,
                    appointment_datetime,
                    status
                )
                SELECT
                    validation.patient_id,
                    validation.doctor_id,
                    validation.appointment_datetime,
                    'scheduled'
                FROM validation
                WHERE validation.patient_exists
                  AND validation.doctor_exists
                  AND validation.is_future
                ON CONFLICT DO NOTHING
                RETURNING
                    id,
                    patient_id,
                    doctor_id,
                    appointment_datetime,
                    status,
                    created_at
            )
            SELECT
                validation.patient_exists,
                validation.doctor_exists,
                validation.is_future,

                inserted.id,
                inserted.patient_id,
                patient.full_name AS patient_name,

                inserted.doctor_id,
                doctor.full_name AS doctor_name,
                doctor.specialty AS doctor_specialty,

                inserted.appointment_datetime,
                inserted.status,
                inserted.created_at
            FROM validation
            LEFT JOIN inserted
                ON TRUE
            LEFT JOIN patients AS patient
                ON patient.id = inserted.patient_id
            LEFT JOIN doctors AS doctor
                ON doctor.id = inserted.doctor_id
        )",
        [
            successCallback =
                std::move(successCallback)
        ](
            const drogon::orm::Result &result
        ) mutable
        {
            const auto row = result[0];

            if (!row["patient_exists"].as<bool>())
            {
                successCallback(
                    {
                        healthcare::models::
                            CreateAppointmentOutcome::
                                PatientNotFound,
                        std::nullopt
                    }
                );

                return;
            }

            if (!row["doctor_exists"].as<bool>())
            {
                successCallback(
                    {
                        healthcare::models::
                            CreateAppointmentOutcome::
                                DoctorNotFound,
                        std::nullopt
                    }
                );

                return;
            }

            if (!row["is_future"].as<bool>())
            {
                successCallback(
                    {
                        healthcare::models::
                            CreateAppointmentOutcome::
                                PastDatetime,
                        std::nullopt
                    }
                );

                return;
            }

            if (row["id"].isNull())
            {
                successCallback(
                    {
                        healthcare::models::
                            CreateAppointmentOutcome::
                                Conflict,
                        std::nullopt
                    }
                );

                return;
            }

            successCallback(
                {
                    healthcare::models::
                        CreateAppointmentOutcome::Created,
                    mapAppointment(row)
                }
            );
        },
        [
            errorCallback =
                std::move(errorCallback)
        ](
            const drogon::orm::DrogonDbException &exception
        ) mutable
        {
            errorCallback(
                exception.base().what()
            );
        },
        input.patientId,
        input.doctorId,
        std::move(input.appointmentDatetime)
    );
}

void AppointmentRepository::cancel(
    const int appointmentId,
    CancelSuccessCallback successCallback,
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

    databaseClient_->execSqlAsync(
        R"(
            WITH target AS
            (
                SELECT
                    id,
                    status
                FROM appointments
                WHERE id = CAST($1 AS INTEGER)
            ),
            updated AS
            (
                UPDATE appointments
                SET status = 'cancelled'
                WHERE id = CAST($1 AS INTEGER)
                  AND status = 'scheduled'
                RETURNING
                    id,
                    patient_id,
                    doctor_id,
                    appointment_datetime,
                    status,
                    created_at
            )
            SELECT
                EXISTS
                (
                    SELECT 1
                    FROM target
                ) AS appointment_exists,

                COALESCE
                (
                    (
                        SELECT status
                        FROM target
                        LIMIT 1
                    ),
                    ''
                ) AS previous_status,

                updated.id,
                updated.patient_id,
                patient.full_name AS patient_name,

                updated.doctor_id,
                doctor.full_name AS doctor_name,
                doctor.specialty AS doctor_specialty,

                updated.appointment_datetime,
                updated.status,
                updated.created_at
            FROM
                (
                    SELECT 1
                ) AS anchor
            LEFT JOIN updated
                ON TRUE
            LEFT JOIN patients AS patient
                ON patient.id = updated.patient_id
            LEFT JOIN doctors AS doctor
                ON doctor.id = updated.doctor_id
        )",
        [
            successCallback =
                std::move(successCallback)
        ](
            const drogon::orm::Result &result
        ) mutable
        {
            const auto row = result[0];

            if (!row["appointment_exists"].as<bool>())
            {
                successCallback(
                    {
                        healthcare::models::
                            CancelAppointmentOutcome::NotFound,
                        std::nullopt
                    }
                );

                return;
            }

            if (!row["id"].isNull())
            {
                successCallback(
                    {
                        healthcare::models::
                            CancelAppointmentOutcome::Cancelled,
                        mapAppointment(row)
                    }
                );

                return;
            }

            const std::string previousStatus =
                row["previous_status"].as<std::string>();

            if (previousStatus == "cancelled")
            {
                successCallback(
                    {
                        healthcare::models::
                            CancelAppointmentOutcome::
                                AlreadyCancelled,
                        std::nullopt
                    }
                );

                return;
            }

            if (previousStatus == "completed")
            {
                successCallback(
                    {
                        healthcare::models::
                            CancelAppointmentOutcome::Completed,
                        std::nullopt
                    }
                );

                return;
            }

            successCallback(
                {
                    healthcare::models::
                        CancelAppointmentOutcome::Conflict,
                    std::nullopt
                }
            );
        },
        [
            errorCallback =
                std::move(errorCallback)
        ](
            const drogon::orm::DrogonDbException &exception
        ) mutable
        {
            errorCallback(
                exception.base().what()
            );
        },
        appointmentId
    );
}
}