#include "repositories/DoctorRepository.h"

#include <drogon/orm/Exception.h>
#include <drogon/orm/Result.h>

#include <optional>
#include <string>
#include <utility>

namespace healthcare::repositories
{
DoctorRepository::DoctorRepository(
    drogon::orm::DbClientPtr databaseClient
)
    : databaseClient_(std::move(databaseClient))
{
}

void DoctorRepository::findAll(
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

    databaseClient_->execSqlAsync(
        R"(
            SELECT
                doctor.id,
                doctor.full_name,
                doctor.specialty,
                doctor.department_id,
                department.name AS department_name
            FROM doctors AS doctor
            JOIN departments AS department
                ON department.id = doctor.department_id
            ORDER BY doctor.full_name ASC
        )",
        [
            successCallback =
                std::move(successCallback)
        ](
            const drogon::orm::Result &result
        ) mutable
        {
            DoctorList doctors;
            doctors.reserve(result.size());

            for (const auto &row : result)
            {
                healthcare::models::Doctor doctor;

                doctor.id =
                    row["id"].as<int>();

                doctor.fullName =
                    row["full_name"].as<std::string>();

                doctor.specialty =
                    row["specialty"].as<std::string>();

                doctor.departmentId =
                    row["department_id"].as<int>();

                doctor.departmentName =
                    row["department_name"].as<std::string>();

                doctors.push_back(
                    std::move(doctor)
                );
            }

            successCallback(
                std::move(doctors)
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
        }
    );
}

void DoctorRepository::findSchedule(
    const int doctorId,
    std::string date,
    ScheduleSuccessCallback successCallback,
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

    const std::string requestedDate = date;

    databaseClient_->execSqlAsync(
        R"(
            WITH selected_doctor AS
            (
                SELECT
                    doctor.id,
                    doctor.full_name,
                    doctor.specialty,
                    department.name AS department_name
                FROM doctors AS doctor
                JOIN departments AS department
                    ON department.id = doctor.department_id
                WHERE doctor.id = CAST($1 AS INTEGER)
            )
            SELECT
                selected_doctor.id AS doctor_id,
                selected_doctor.full_name AS doctor_name,
                selected_doctor.specialty,
                selected_doctor.department_name,

                appointment.id AS appointment_id,
                appointment.patient_id,
                patient.full_name AS patient_name,
                appointment.appointment_datetime,
                appointment.status
            FROM selected_doctor
            LEFT JOIN appointments AS appointment
                ON appointment.doctor_id =
                    selected_doctor.id
               AND
                    (
                        appointment.appointment_datetime
                            AT TIME ZONE 'UTC'
                    )::date = CAST($2 AS DATE)
               AND appointment.status <> 'cancelled'
            LEFT JOIN patients AS patient
                ON patient.id = appointment.patient_id
            ORDER BY
                appointment.appointment_datetime ASC,
                appointment.id ASC
        )",
        [
            successCallback =
                std::move(successCallback),
            requestedDate
        ](
            const drogon::orm::Result &result
        ) mutable
        {
            if (result.size() == 0)
            {
                successCallback(
                    {
                        healthcare::models::
                            DoctorScheduleOutcome::
                                DoctorNotFound,
                        std::nullopt
                    }
                );

                return;
            }

            const auto firstRow = result[0];

            healthcare::models::DoctorSchedule schedule;

            schedule.doctorId =
                firstRow["doctor_id"].as<int>();

            schedule.doctorName =
                firstRow["doctor_name"].as<std::string>();

            schedule.specialty =
                firstRow["specialty"].as<std::string>();

            schedule.departmentName =
                firstRow["department_name"]
                    .as<std::string>();

            schedule.date =
                requestedDate;

            for (const auto &row : result)
            {
                if (row["appointment_id"].isNull())
                {
                    continue;
                }

                healthcare::models::DoctorScheduleEntry
                    scheduleEntry;

                scheduleEntry.appointmentId =
                    row["appointment_id"].as<int>();

                scheduleEntry.patientId =
                    row["patient_id"].as<int>();

                scheduleEntry.patientName =
                    row["patient_name"].as<std::string>();

                scheduleEntry.appointmentDatetime =
                    row["appointment_datetime"]
                        .as<std::string>();

                scheduleEntry.status =
                    row["status"].as<std::string>();

                schedule.appointments.push_back(
                    std::move(scheduleEntry)
                );
            }

            successCallback(
                {
                    healthcare::models::
                        DoctorScheduleOutcome::Found,
                    std::move(schedule)
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
        doctorId,
        std::move(date)
    );
}

void DoctorRepository::findAvailableSlots(
    const int doctorId,
    std::string date,
    AvailabilitySuccessCallback successCallback,
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

    const std::string requestedDate = date;

    databaseClient_->execSqlAsync(
        R"(
            WITH selected_doctor AS
            (
                SELECT
                    doctor.id,
                    doctor.full_name,
                    doctor.specialty,
                    department.name AS department_name
                FROM doctors AS doctor
                JOIN departments AS department
                    ON department.id = doctor.department_id
                WHERE doctor.id = CAST($1 AS INTEGER)
            ),
            candidate_slots AS
            (
                SELECT
                    generate_series
                    (
                        CAST($2 AS DATE) + TIME '09:00:00',
                        CAST($2 AS DATE) + TIME '16:30:00',
                        INTERVAL '30 minutes'
                    ) AS slot_start
            ),
            available_slots AS
            (
                SELECT
                    candidate_slots.slot_start
                FROM candidate_slots
                WHERE NOT EXISTS
                (
                    SELECT 1
                    FROM appointments AS appointment
                    WHERE appointment.doctor_id =
                              CAST($1 AS INTEGER)
                      AND appointment.status <> 'cancelled'
                      AND appointment.appointment_datetime >=
                          (
                              candidate_slots.slot_start
                                  AT TIME ZONE 'UTC'
                          )
                      AND appointment.appointment_datetime <
                          (
                              (
                                  candidate_slots.slot_start +
                                  INTERVAL '30 minutes'
                              )
                              AT TIME ZONE 'UTC'
                          )
                )
            )
            SELECT
                selected_doctor.id AS doctor_id,
                selected_doctor.full_name AS doctor_name,
                selected_doctor.specialty,
                selected_doctor.department_name,

                CASE
                    WHEN available_slots.slot_start IS NULL
                    THEN NULL
                    ELSE TO_CHAR
                    (
                        available_slots.slot_start,
                        'YYYY-MM-DD"T"HH24:MI:SS"Z"'
                    )
                END AS slot_datetime
            FROM selected_doctor
            LEFT JOIN available_slots
                ON TRUE
            ORDER BY
                available_slots.slot_start ASC
        )",
        [
            successCallback =
                std::move(successCallback),
            requestedDate
        ](
            const drogon::orm::Result &result
        ) mutable
        {
            if (result.size() == 0)
            {
                successCallback(
                    {
                        healthcare::models::
                            DoctorAvailabilityOutcome::
                                DoctorNotFound,
                        std::nullopt
                    }
                );

                return;
            }

            const auto firstRow = result[0];

            healthcare::models::DoctorAvailability
                availability;

            availability.doctorId =
                firstRow["doctor_id"].as<int>();

            availability.doctorName =
                firstRow["doctor_name"].as<std::string>();

            availability.specialty =
                firstRow["specialty"].as<std::string>();

            availability.departmentName =
                firstRow["department_name"]
                    .as<std::string>();

            availability.date =
                requestedDate;

            for (const auto &row : result)
            {
                if (row["slot_datetime"].isNull())
                {
                    continue;
                }

                availability.availableSlots.push_back(
                    row["slot_datetime"].as<std::string>()
                );
            }

            successCallback(
                {
                    healthcare::models::
                        DoctorAvailabilityOutcome::Found,
                    std::move(availability)
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
        doctorId,
        std::move(date)
    );
}
}