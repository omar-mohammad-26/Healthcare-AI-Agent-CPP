#include "repositories/DoctorRepository.h"

#include <drogon/orm/Exception.h>
#include <drogon/orm/Result.h>

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
            const drogon::orm::DrogonDbException
                &exception
        ) mutable
        {
            errorCallback(
                exception.base().what()
            );
        }
    );
}
}