#include "services/TreatmentService.h"

#include <utility>

namespace healthcare::services
{
TreatmentService::TreatmentService(
    drogon::orm::DbClientPtr databaseClient
)
    : treatmentRepository_(
          std::move(databaseClient)
      )
{
}

void TreatmentService::listTreatments(
    std::optional<int> patientId,
    std::optional<int> doctorId,
    std::string searchTerm,
    SuccessCallback successCallback,
    ErrorCallback errorCallback
) const
{
    treatmentRepository_.findAll(
        patientId,
        doctorId,
        std::move(searchTerm),
        [
            successCallback =
                std::move(successCallback)
        ](
            healthcare::repositories::
                TreatmentRepository::TreatmentList
                    treatments
        ) mutable
        {
            Json::Value treatmentArray(
                Json::arrayValue
            );

            for (const auto &treatment : treatments)
            {
                Json::Value treatmentJson;

                treatmentJson["id"] =
                    treatment.id;

                treatmentJson["patient_id"] =
                    treatment.patientId;

                treatmentJson["patient_name"] =
                    treatment.patientName;

                treatmentJson["doctor_id"] =
                    treatment.doctorId;

                treatmentJson["doctor_name"] =
                    treatment.doctorName;

                treatmentJson["doctor_specialty"] =
                    treatment.doctorSpecialty;

                treatmentJson["diagnosis"] =
                    treatment.diagnosis;

                treatmentJson["treatment_plan"] =
                    treatment.treatmentPlan;

                treatmentJson["created_at"] =
                    treatment.createdAt;

                treatmentArray.append(
                    std::move(treatmentJson)
                );
            }

            Json::Value responseBody;

            responseBody["count"] =
                static_cast<Json::UInt64>(
                    treatments.size()
                );

            responseBody["data"] =
                std::move(treatmentArray);

            successCallback(
                std::move(responseBody)
            );
        },
        std::move(errorCallback)
    );
}
}