#include "repositories/DepartmentRepository.h"

#include <drogon/orm/Exception.h>
#include <drogon/orm/Result.h>

#include <utility>

namespace healthcare::repositories
{
DepartmentRepository::DepartmentRepository(
    drogon::orm::DbClientPtr databaseClient
)
    : databaseClient_(std::move(databaseClient))
{
}

void DepartmentRepository::findAll(
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
                name,
                description
            FROM departments
            ORDER BY name ASC
        )",
        [
            successCallback =
                std::move(successCallback)
        ](
            const drogon::orm::Result &result
        ) mutable
        {
            DepartmentList departments;
            departments.reserve(result.size());

            for (const auto &row : result)
            {
                healthcare::models::Department department;

                department.id =
                    row["id"].as<int>();

                department.name =
                    row["name"].as<std::string>();

                if (!row["description"].isNull())
                {
                    department.description =
                        row["description"].as<std::string>();
                }

                departments.push_back(
                    std::move(department)
                );
            }

            successCallback(
                std::move(departments)
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