#include "repositories/PatientRepository.h"

#include <drogon/orm/Exception.h>
#include <drogon/orm/Result.h>

#include <optional>
#include <string>
#include <utility>

namespace
{
healthcare::models::Patient mapPatient(
    const drogon::orm::Row &row
)
{
    healthcare::models::Patient patient;

    patient.id =
        row["id"].as<int>();

    patient.fullName =
        row["full_name"].as<std::string>();

    patient.phone =
        row["phone"].as<std::string>();

    patient.age =
        row["age"].as<int>();

    patient.gender =
        row["gender"].as<std::string>();

    if (!row["medical_history"].isNull())
    {
        patient.medicalHistory =
            row["medical_history"].as<std::string>();
    }

    patient.createdAt =
        row["created_at"].as<std::string>();

    return patient;
}
}

namespace healthcare::repositories
{
PatientRepository::PatientRepository(
    drogon::orm::DbClientPtr databaseClient
)
    : databaseClient_(std::move(databaseClient))
{
}

void PatientRepository::findAll(
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

    databaseClient_->execSqlAsync(
        R"(
            SELECT
                id,
                full_name,
                phone,
                age,
                gender,
                medical_history,
                created_at
            FROM patients
            WHERE
                CAST($1 AS TEXT) = ''
                OR full_name ILIKE
                    '%' || CAST($1 AS TEXT) || '%'
                OR phone ILIKE
                    '%' || CAST($1 AS TEXT) || '%'
            ORDER BY full_name ASC
        )",
        [
            successCallback =
                std::move(successCallback)
        ](
            const drogon::orm::Result &result
        ) mutable
        {
            PatientList patients;
            patients.reserve(result.size());

            for (const auto &row : result)
            {
                patients.push_back(
                    mapPatient(row)
                );
            }

            successCallback(
                std::move(patients)
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
        std::move(searchTerm)
    );
}

void PatientRepository::create(
    healthcare::models::CreatePatientInput input,
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

    const std::string medicalHistory =
        input.medicalHistory.value_or("");

    databaseClient_->execSqlAsync(
        R"(
            INSERT INTO patients
            (
                full_name,
                phone,
                age,
                gender,
                medical_history
            )
            VALUES
            (
                CAST($1 AS TEXT),
                CAST($2 AS TEXT),
                CAST($3 AS INTEGER),
                CAST($4 AS TEXT),
                NULLIF(CAST($5 AS TEXT), '')
            )
            ON CONFLICT (phone)
            DO NOTHING
            RETURNING
                id,
                full_name,
                phone,
                age,
                gender,
                medical_history,
                created_at
        )",
        [
            successCallback =
                std::move(successCallback)
        ](
            const drogon::orm::Result &result
        ) mutable
        {
            if (result.size() == 0)
            {
                successCallback(std::nullopt);
                return;
            }

            successCallback(
                std::optional<
                    healthcare::models::Patient
                >(
                    mapPatient(result[0])
                )
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
        std::move(input.fullName),
        std::move(input.phone),
        input.age,
        std::move(input.gender),
        medicalHistory
    );
}
}