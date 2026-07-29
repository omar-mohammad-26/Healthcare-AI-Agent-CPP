#include "agent/tools/AddTreatmentTool.h"

#include <drogon/orm/Exception.h>
#include <drogon/orm/Result.h>

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <utility>

namespace
{
std::string trim(
    const std::string &value
)
{
    const auto firstCharacter =
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

    if (firstCharacter == value.end())
    {
        return "";
    }

    const auto lastCharacter =
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

    return std::string(
        firstCharacter,
        lastCharacter
    );
}
}

namespace healthcare::agent::tools
{
AddTreatmentTool::AddTreatmentTool(
    drogon::orm::DbClientPtr databaseClient
)
    : databaseClient_(
          std::move(databaseClient)
      )
{
}

std::string AddTreatmentTool::name() const
{
    return "add_treatment";
}

std::string AddTreatmentTool::description() const
{
    return
        "Adds a diagnosis and treatment plan to a "
        "fictional patient's medical record.";
}

Json::Value AddTreatmentTool::parametersSchema() const
{
    Json::Value patientIdProperty;

    patientIdProperty["type"] =
        "integer";

    patientIdProperty["description"] =
        "The unique ID of the patient.";

    patientIdProperty["minimum"] =
        1;

    Json::Value doctorIdProperty;

    doctorIdProperty["type"] =
        "integer";

    doctorIdProperty["description"] =
        "The unique ID of the treating doctor.";

    doctorIdProperty["minimum"] =
        1;

    Json::Value diagnosisProperty;

    diagnosisProperty["type"] =
        "string";

    diagnosisProperty["description"] =
        "The diagnosis recorded by the doctor.";

    diagnosisProperty["minLength"] =
        1;

    diagnosisProperty["maxLength"] =
        1000;

    Json::Value treatmentPlanProperty;

    treatmentPlanProperty["type"] =
        "string";

    treatmentPlanProperty["description"] =
        "The recommended treatment plan.";

    treatmentPlanProperty["maxLength"] =
        4000;

    Json::Value properties;

    properties["patient_id"] =
        std::move(patientIdProperty);

    properties["doctor_id"] =
        std::move(doctorIdProperty);

    properties["diagnosis"] =
        std::move(diagnosisProperty);

    properties["treatment_plan"] =
        std::move(treatmentPlanProperty);

    Json::Value required(
        Json::arrayValue
    );

    required.append("patient_id");
    required.append("doctor_id");
    required.append("diagnosis");

    Json::Value schema;

    schema["type"] =
        "object";

    schema["properties"] =
        std::move(properties);

    schema["required"] =
        std::move(required);

    schema["additionalProperties"] =
        false;

    return schema;
}

void AddTreatmentTool::execute(
    const Json::Value &arguments,
    SuccessCallback successCallback,
    ErrorCallback errorCallback
) const
{
    if (!arguments.isObject())
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "arguments",
                "Tool arguments must be a JSON object."
            }
        );

        return;
    }

    if (!arguments.isMember("patient_id") ||
        !arguments["patient_id"].isInt())
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "patient_id",
                "Patient ID is required and must be an integer."
            }
        );

        return;
    }

    const int patientId =
        arguments["patient_id"].asInt();

    if (patientId <= 0)
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "patient_id",
                "Patient ID must be a positive integer."
            }
        );

        return;
    }

    if (!arguments.isMember("doctor_id") ||
        !arguments["doctor_id"].isInt())
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "doctor_id",
                "Doctor ID is required and must be an integer."
            }
        );

        return;
    }

    const int doctorId =
        arguments["doctor_id"].asInt();

    if (doctorId <= 0)
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "doctor_id",
                "Doctor ID must be a positive integer."
            }
        );

        return;
    }

    if (!arguments.isMember("diagnosis") ||
        !arguments["diagnosis"].isString())
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "diagnosis",
                "Diagnosis is required and must be a string."
            }
        );

        return;
    }

    const std::string diagnosis =
        trim(
            arguments["diagnosis"].asString()
        );

    if (diagnosis.empty() ||
        diagnosis.size() > 1000)
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "diagnosis",
                "Diagnosis must contain between 1 and 1000 characters."
            }
        );

        return;
    }

    std::string treatmentPlan;

    if (arguments.isMember("treatment_plan"))
    {
        if (!arguments["treatment_plan"].isString())
        {
            errorCallback(
                {
                    healthcare::agent::
                        AgentToolErrorType::Validation,
                    "treatment_plan",
                    "Treatment plan must be a string."
                }
            );

            return;
        }

        treatmentPlan =
            trim(
                arguments["treatment_plan"]
                    .asString()
            );
    }

    if (treatmentPlan.size() > 4000)
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Validation,
                "treatment_plan",
                "Treatment plan must not exceed 4000 characters."
            }
        );

        return;
    }

    if (!databaseClient_)
    {
        errorCallback(
            {
                healthcare::agent::
                    AgentToolErrorType::Database,
                "",
                "The PostgreSQL client is not available."
            }
        );

        return;
    }

    auto sharedSuccessCallback =
        std::make_shared<SuccessCallback>(
            std::move(successCallback)
        );

    auto sharedErrorCallback =
        std::make_shared<ErrorCallback>(
            std::move(errorCallback)
        );

    databaseClient_->execSqlAsync(
        R"(
            WITH validation AS
            (
                SELECT
                    EXISTS
                    (
                        SELECT 1
                        FROM patients
                        WHERE id = CAST($1 AS INTEGER)
                    ) AS patient_exists,

                    EXISTS
                    (
                        SELECT 1
                        FROM doctors
                        WHERE id = CAST($2 AS INTEGER)
                    ) AS doctor_exists
            ),
            inserted AS
            (
                INSERT INTO treatments
                (
                    patient_id,
                    doctor_id,
                    diagnosis,
                    treatment_plan
                )
                SELECT
                    CAST($1 AS INTEGER),
                    CAST($2 AS INTEGER),
                    CAST($3 AS TEXT),
                    NULLIF(
                        CAST($4 AS TEXT),
                        ''
                    )
                FROM validation
                WHERE validation.patient_exists
                  AND validation.doctor_exists
                RETURNING
                    id,
                    patient_id,
                    doctor_id,
                    diagnosis,
                    treatment_plan,
                    created_at
            )
            SELECT
                validation.patient_exists,
                validation.doctor_exists,

                inserted.id,
                inserted.patient_id,
                patient.full_name AS patient_name,

                inserted.doctor_id,
                doctor.full_name AS doctor_name,
                doctor.specialty AS doctor_specialty,

                inserted.diagnosis,
                inserted.treatment_plan,
                inserted.created_at
            FROM validation
            LEFT JOIN inserted
                ON TRUE
            LEFT JOIN patients AS patient
                ON patient.id = inserted.patient_id
            LEFT JOIN doctors AS doctor
                ON doctor.id = inserted.doctor_id
        )",
        [
            sharedSuccessCallback,
            sharedErrorCallback,
            patientId,
            doctorId
        ](
            const drogon::orm::Result &result
        )
        {
            if (result.size() == 0)
            {
                (*sharedErrorCallback)(
                    {
                        healthcare::agent::
                            AgentToolErrorType::Database,
                        "",
                        "The treatment insert returned no result."
                    }
                );

                return;
            }

            const auto row =
                result[0];

            if (!row["patient_exists"].as<bool>())
            {
                (*sharedErrorCallback)(
                    {
                        healthcare::agent::
                            AgentToolErrorType::NotFound,
                        "patient_id",
                        "The requested patient does not exist."
                    }
                );

                return;
            }

            if (!row["doctor_exists"].as<bool>())
            {
                (*sharedErrorCallback)(
                    {
                        healthcare::agent::
                            AgentToolErrorType::NotFound,
                        "doctor_id",
                        "The requested doctor does not exist."
                    }
                );

                return;
            }

            if (row["id"].isNull())
            {
                (*sharedErrorCallback)(
                    {
                        healthcare::agent::
                            AgentToolErrorType::Database,
                        "",
                        "The treatment could not be created."
                    }
                );

                return;
            }

            Json::Value treatment;

            treatment["id"] =
                row["id"].as<int>();

            treatment["patient_id"] =
                row["patient_id"].as<int>();

            treatment["patient_name"] =
                row["patient_name"].as<std::string>();

            treatment["doctor_id"] =
                row["doctor_id"].as<int>();

            treatment["doctor_name"] =
                row["doctor_name"].as<std::string>();

            treatment["doctor_specialty"] =
                row["doctor_specialty"]
                    .as<std::string>();

            treatment["diagnosis"] =
                row["diagnosis"].as<std::string>();

            if (row["treatment_plan"].isNull())
            {
                treatment["treatment_plan"] =
                    Json::Value(Json::nullValue);
            }
            else
            {
                treatment["treatment_plan"] =
                    row["treatment_plan"]
                        .as<std::string>();
            }

            treatment["created_at"] =
                row["created_at"].as<std::string>();

            Json::Value treatmentResult;

            treatmentResult["status"] =
                "created";

            treatmentResult["data"] =
                std::move(treatment);

            Json::Value toolResult;

            toolResult["tool"] =
                "add_treatment";

            toolResult["patient_id"] =
                patientId;

            toolResult["doctor_id"] =
                doctorId;

            toolResult["result"] =
                std::move(treatmentResult);

            (*sharedSuccessCallback)(
                std::move(toolResult)
            );
        },
        [
            sharedErrorCallback
        ](
            const drogon::orm::DrogonDbException
                &exception
        )
        {
            (*sharedErrorCallback)(
                {
                    healthcare::agent::
                        AgentToolErrorType::Database,
                    "",
                    exception.base().what()
                }
            );
        },
        patientId,
        doctorId,
        diagnosis,
        treatmentPlan
    );
}
}