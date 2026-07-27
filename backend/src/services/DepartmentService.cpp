#include "services/DepartmentService.h"

#include <utility>

namespace healthcare::services
{
    DepartmentService::DepartmentService(
        drogon::orm::DbClientPtr databaseClient
    )
        : departmentRepository_(
              std::move(databaseClient)
          )
    {
    }

    void DepartmentService::listDepartments(
        SuccessCallback successCallback,
        ErrorCallback errorCallback
    ) const
    {
        departmentRepository_.findAll(
            [
                successCallback =
                    std::move(successCallback)
            ](
                healthcare::repositories::
                    DepartmentRepository::DepartmentList
                        departments
            ) mutable
            {
                Json::Value departmentArray(
                    Json::arrayValue
                );

                for (const auto &department : departments)
                {
                    Json::Value departmentJson;

                    departmentJson["id"] =
                        department.id;

                    departmentJson["name"] =
                        department.name;

                    if (department.description.has_value())
                    {
                        departmentJson["description"] =
                            department.description.value();
                    }
                    else
                    {
                        departmentJson["description"] =
                            Json::nullValue;
                    }

                    departmentArray.append(
                        departmentJson
                    );
                }

                Json::Value responseBody;

                responseBody["count"] =
                    static_cast<Json::UInt64>(
                        departments.size()
                    );

                responseBody["data"] =
                    std::move(departmentArray);

                successCallback(
                    std::move(responseBody)
                );
            },
            std::move(errorCallback)
        );
    }
}