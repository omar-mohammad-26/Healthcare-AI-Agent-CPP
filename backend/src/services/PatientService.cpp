#include "services/PatientService.h"

#include <algorithm>
#include <cctype>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace
{
std::string trim(
    const std::string &value
)
{
    const auto first =
        std::find_if_not(
            value.begin(),
            value.end(),
            [](
                const unsigned char character
            )
            {
                return std::isspace(character);
            }
        );

    if (first == value.end())
    {
        return {};
    }

    const auto last =
        std::find_if_not(
            value.rbegin(),
            value.rend(),
            [](
                const unsigned char character
            )
            {
                return std::isspace(character);
            }
        ).base();

    return std::string(first, last);
}

std::optional<std::string> normalizeGender(
    const std::string &value
)
{
    std::string normalized =
        trim(value);

    std::transform(
        normalized.begin(),
        normalized.end(),
        normalized.begin(),
        [](
            const unsigned char character
        )
        {
            return static_cast<char>(
                std::tolower(character)
            );
        }
    );

    if (normalized == "m" ||
        normalized == "male")
    {
        return "M";
    }

    if (normalized == "f" ||
        normalized == "female")
    {
        return "F";
    }

    if (normalized == "other")
    {
        return "Other";
    }

    return std::nullopt;
}

Json::Value patientToJson(
    const healthcare::models::Patient &patient
)
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

    return patientJson;
}
}

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
                patientArray.append(
                    patientToJson(patient)
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

void PatientService::createPatient(
    healthcare::models::CreatePatientInput input,
    SuccessCallback successCallback,
    CreateErrorCallback errorCallback
) const
{
    input.fullName =
        trim(input.fullName);

    input.phone =
        trim(input.phone);

    if (input.medicalHistory.has_value())
    {
        input.medicalHistory =
            trim(input.medicalHistory.value());

        if (input.medicalHistory->empty())
        {
            input.medicalHistory.reset();
        }
    }

    if (input.fullName.size() < 2 ||
        input.fullName.size() > 120)
    {
        errorCallback(
            {
                PatientServiceErrorType::Validation,
                "full_name",
                "Full name must contain between 2 and 120 characters."
            }
        );

        return;
    }

    if (input.phone.size() < 5 ||
        input.phone.size() > 30)
    {
        errorCallback(
            {
                PatientServiceErrorType::Validation,
                "phone",
                "Phone must contain between 5 and 30 characters."
            }
        );

        return;
    }

    if (input.age < 0 ||
        input.age > 130)
    {
        errorCallback(
            {
                PatientServiceErrorType::Validation,
                "age",
                "Age must be between 0 and 130."
            }
        );

        return;
    }

    const auto normalizedGender =
        normalizeGender(input.gender);

    if (!normalizedGender.has_value())
    {
        errorCallback(
            {
                PatientServiceErrorType::Validation,
                "gender",
                "Gender must be M, F, or Other."
            }
        );

        return;
    }

    input.gender =
        normalizedGender.value();

    if (input.medicalHistory.has_value() &&
        input.medicalHistory->size() > 2000)
    {
        errorCallback(
            {
                PatientServiceErrorType::Validation,
                "medical_history",
                "Medical history cannot exceed 2000 characters."
            }
        );

        return;
    }

    auto sharedSuccessCallback =
        std::make_shared<SuccessCallback>(
            std::move(successCallback)
        );

    auto sharedErrorCallback =
        std::make_shared<CreateErrorCallback>(
            std::move(errorCallback)
        );

    patientRepository_.create(
        std::move(input),
        [
            sharedSuccessCallback,
            sharedErrorCallback
        ](
            std::optional<
                healthcare::models::Patient
            > patient
        )
        {
            if (!patient.has_value())
            {
                (*sharedErrorCallback)(
                    {
                        PatientServiceErrorType::Conflict,
                        "phone",
                        "A patient with this phone number already exists."
                    }
                );

                return;
            }

            Json::Value responseBody;
            responseBody["status"] =
                "created";

            responseBody["data"] =
                patientToJson(
                    patient.value()
                );

            (*sharedSuccessCallback)(
                std::move(responseBody)
            );
        },
        [
            sharedErrorCallback
        ](
            std::string errorMessage
        )
        {
            (*sharedErrorCallback)(
                {
                    PatientServiceErrorType::Database,
                    "",
                    std::move(errorMessage)
                }
            );
        }
    );
}
}