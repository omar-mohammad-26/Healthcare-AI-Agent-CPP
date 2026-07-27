#include "services/PatientService.h"

#include <utility>

namespace healthcare::services
{
PatientService::PatientService(
    drogon::orm::DbClientPtr databaseClient
)
    : patientRepository_(
          std::move(databaseClient)
      )
{
}

void PatientService::listPatients(
    std::string searchTerm,
    SuccessCallback successCallback,
    ErrorCallback errorCallback
) const
{
    patientRepository_.findAll(
        std::move(searchTerm),
        [
            successCallback =
                std::move(successCallback)
        ](
            healthcare::repositories::
                PatientRepository::PatientList patients
        ) mutable
        {
            Json::Value patientArray(
                Json::arrayValue
            );

            for (const auto &patient : patients)
            {
                Json::Value patientJson;

                patientJson["id"] =
                    patient.id;

                patientJson["full_name"] =
                    patient.fullName;

                patientJson["phone"] =
                    patient.phone;

                patientJson["age"] =
                    patient.age;

                patientJson["gender"] =
                    patient.gender;

                if (patient.medicalHistory.has_value())
                {
                    patientJson["medical_history"] =
                        patient.medicalHistory.value();
                }
                else
                {
                    patientJson["medical_history"] =
                        Json::nullValue;
                }

                patientJson["created_at"] =
                    patient.createdAt;

                patientArray.append(
                    std::move(patientJson)
                );
            }

            Json::Value responseBody;

            responseBody["count"] =
                static_cast<Json::UInt64>(
                    patients.size()
                );

            responseBody["data"] =
                std::move(patientArray);

            successCallback(
                std::move(responseBody)
            );
        },
        std::move(errorCallback)
    );
}
}