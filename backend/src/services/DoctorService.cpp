#include "services/DoctorService.h"

#include <utility>

namespace healthcare::services
{
    DoctorService::DoctorService(
        drogon::orm::DbClientPtr databaseClient
    )
        : doctorRepository_(
              std::move(databaseClient)
          )
    {
    }

    void DoctorService::listDoctors(
        SuccessCallback successCallback,
        ErrorCallback errorCallback
    ) const
    {
        doctorRepository_.findAll(
            [
                successCallback =
                    std::move(successCallback)
            ](
                healthcare::repositories::
                    DoctorRepository::DoctorList doctors
            ) mutable
            {
                Json::Value doctorArray(
                    Json::arrayValue
                );

                for (const auto &doctor : doctors)
                {
                    Json::Value doctorJson;

                    doctorJson["id"] =
                        doctor.id;

                    doctorJson["full_name"] =
                        doctor.fullName;

                    doctorJson["specialty"] =
                        doctor.specialty;

                    doctorJson["department_id"] =
                        doctor.departmentId;

                    doctorJson["department_name"] =
                        doctor.departmentName;

                    doctorArray.append(
                        std::move(doctorJson)
                    );
                }

                Json::Value responseBody;

                responseBody["count"] =
                    static_cast<Json::UInt64>(
                        doctors.size()
                    );

                responseBody["data"] =
                    std::move(doctorArray);

                successCallback(
                    std::move(responseBody)
                );
            },
            std::move(errorCallback)
        );
    }
}