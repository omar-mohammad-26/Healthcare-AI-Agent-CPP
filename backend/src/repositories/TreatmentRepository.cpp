#include "repositories/TreatmentRepository.h"

#include <drogon/orm/Exception.h>
#include <drogon/orm/Result.h>

#include <utility>

namespace healthcare::repositories
{
TreatmentRepository::TreatmentRepository(
    drogon::orm::DbClientPtr databaseClient
)
    : databaseClient_(std::move(databaseClient))
{
}

void TreatmentRepository::findAll(
    std::optional<int> patientId,
    std::optional<int> doctorId,
    std::string searchTerm,
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

    const int patientIdFilter =
        patientId.value_or(0);

    const int doctorIdFilter =
        doctorId.value_or(0);

    databaseClient_->execSqlAsync(
        R"(
            SELECT
                treatment.id,

                treatment.patient_id,
                patient.full_name AS patient_name,

                treatment.doctor_id,
                doctor.full_name AS doctor_name,
                doctor.specialty AS doctor_specialty,

                treatment.diagnosis,
                treatment.treatment_plan,
                treatment.created_at
            FROM treatments AS treatment
            JOIN patients AS patient
                ON patient.id = treatment.patient_id
            JOIN doctors AS doctor
                ON doctor.id = treatment.doctor_id
            WHERE
                (
                    CAST($1 AS INTEGER) = 0
                    OR treatment.patient_id =
                        CAST($1 AS INTEGER)
                )
                AND
                (
                    CAST($2 AS INTEGER) = 0
                    OR treatment.doctor_id =
                        CAST($2 AS INTEGER)
                )
                AND
                (
                    CAST($3 AS TEXT) = ''
                    OR treatment.diagnosis ILIKE
                        '%' || CAST($3 AS TEXT) || '%'
                    OR treatment.treatment_plan ILIKE
                        '%' || CAST($3 AS TEXT) || '%'
                    OR patient.full_name ILIKE
                        '%' || CAST($3 AS TEXT) || '%'
                    OR doctor.full_name ILIKE
                        '%' || CAST($3 AS TEXT) || '%'
                )
            ORDER BY
                treatment.created_at DESC,
                treatment.id DESC
        )",
        [
            successCallback =
                std::move(successCallback)
        ](
            const drogon::orm::Result &result
        ) mutable
        {
            TreatmentList treatments;
            treatments.reserve(result.size());

            for (const auto &row : result)
            {
                healthcare::models::Treatment treatment;

                treatment.id =
                    row["id"].as<int>();

                treatment.patientId =
                    row["patient_id"].as<int>();

                treatment.patientName =
                    row["patient_name"].as<std::string>();

                treatment.doctorId =
                    row["doctor_id"].as<int>();

                treatment.doctorName =
                    row["doctor_name"].as<std::string>();

                treatment.doctorSpecialty =
                    row["doctor_specialty"]
                        .as<std::string>();

                treatment.diagnosis =
                    row["diagnosis"].as<std::string>();

                treatment.treatmentPlan =
                    row["treatment_plan"]
                        .as<std::string>();

                treatment.createdAt =
                    row["created_at"].as<std::string>();

                treatments.push_back(
                    std::move(treatment)
                );
            }

            successCallback(
                std::move(treatments)
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
        patientIdFilter,
        doctorIdFilter,
        std::move(searchTerm)
    );
}
}