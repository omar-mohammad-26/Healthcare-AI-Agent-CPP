#include "ai/GeminiClient.h"

#include <drogon/drogon.h>
#include <json/json.h>

#include <memory>
#include <string>
#include <utility>

namespace
{
std::string jsonToString(
    const Json::Value &value
)
{
    Json::StreamWriterBuilder writerBuilder;

    writerBuilder["indentation"] =
        "";

    return Json::writeString(
        writerBuilder,
        value
    );
}

std::string extractGeminiText(
    const Json::Value &responseBody
)
{
    if (!responseBody.isObject() ||
        !responseBody.isMember("candidates") ||
        !responseBody["candidates"].isArray() ||
        responseBody["candidates"].empty())
    {
        return "";
    }

    const Json::Value &candidate =
        responseBody["candidates"][0];

    if (!candidate.isObject() ||
        !candidate.isMember("content") ||
        !candidate["content"].isObject() ||
        !candidate["content"].isMember("parts") ||
        !candidate["content"]["parts"].isArray())
    {
        return "";
    }

    const Json::Value &parts =
        candidate["content"]["parts"];

    std::string generatedText;

    for (const auto &part : parts)
    {
        if (!part.isObject() ||
            !part.isMember("text") ||
            !part["text"].isString())
        {
            continue;
        }

        generatedText +=
            part["text"].asString();
    }

    return generatedText;
}

std::string extractApiErrorMessage(
    const drogon::HttpResponsePtr &response
)
{
    if (!response)
    {
        return
            "Gemini returned an empty HTTP response.";
    }

    const auto responseJson =
        response->getJsonObject();

    if (responseJson &&
        responseJson->isObject() &&
        responseJson->isMember("error") &&
        (*responseJson)["error"].isObject() &&
        (*responseJson)["error"].isMember("message") &&
        (*responseJson)["error"]["message"].isString())
    {
        return
            (*responseJson)["error"]["message"]
                .asString();
    }

    const std::string responseBody(
        response->body()
    );

    if (!responseBody.empty())
    {
        return responseBody;
    }

    return
        "Gemini returned an API error without a message.";
}

Json::Value createUserContents(
    const std::string &prompt
)
{
    Json::Value textPart;

    textPart["text"] =
        prompt;

    Json::Value parts(
        Json::arrayValue
    );

    parts.append(
        std::move(textPart)
    );

    Json::Value userContent;

    userContent["role"] =
        "user";

    userContent["parts"] =
        std::move(parts);

    Json::Value contents(
        Json::arrayValue
    );

    contents.append(
        std::move(userContent)
    );

    return contents;
}

Json::Value createGenerationConfig()
{
    Json::Value generationConfig;

    generationConfig["temperature"] =
        0.0;

    generationConfig["maxOutputTokens"] =
        512;

    return generationConfig;
}

drogon::HttpRequestPtr createGeminiRequest(
    const healthcare::config::GeminiConfig &configuration,
    const Json::Value &requestBody
)
{
    auto request =
        drogon::HttpRequest::newHttpRequest();

    request->setMethod(
        drogon::Post
    );

    request->setPathEncode(
        false
    );

    request->setPath(
        "/v1beta/models/" +
        configuration.model +
        ":generateContent"
    );

    request->setContentTypeCode(
        drogon::CT_APPLICATION_JSON
    );

    request->addHeader(
        "x-goog-api-key",
        configuration.apiKey
    );

    request->setBody(
        jsonToString(requestBody)
    );

    return request;
}
}

namespace healthcare::ai
{
GeminiClient::GeminiClient(
    healthcare::config::GeminiConfig configuration
)
    : configuration_(
          std::move(configuration)
      )
{
}

const std::string &GeminiClient::model() const
{
    return configuration_.model;
}

bool GeminiClient::isConfigured() const
{
    return configuration_.isConfigured();
}

void GeminiClient::generateText(
    const std::string &prompt,
    SuccessCallback successCallback,
    ErrorCallback errorCallback
) const
{
    if (!configuration_.isConfigured())
    {
        errorCallback(
            {
                GeminiErrorType::Configuration,
                0,
                "GEMINI_API_KEY is not configured."
            }
        );

        return;
    }

    if (prompt.empty())
    {
        errorCallback(
            {
                GeminiErrorType::Configuration,
                0,
                "The Gemini prompt must not be empty."
            }
        );

        return;
    }

    Json::Value requestBody;

    requestBody["contents"] =
        createUserContents(prompt);

    requestBody["generationConfig"] =
        createGenerationConfig();

    const auto httpClient =
        drogon::HttpClient::newHttpClient(
            configuration_.baseUrl
        );

    if (!httpClient)
    {
        errorCallback(
            {
                GeminiErrorType::Network,
                0,
                "Unable to create the Gemini HTTP client."
            }
        );

        return;
    }

    const auto request =
        createGeminiRequest(
            configuration_,
            requestBody
        );

    httpClient->sendRequest(
        request,
        [
            httpClient,
            successCallback =
                std::move(successCallback),
            errorCallback =
                std::move(errorCallback)
        ](
            const drogon::ReqResult requestResult,
            const drogon::HttpResponsePtr &response
        ) mutable
        {
            static_cast<void>(httpClient);

            if (requestResult !=
                    drogon::ReqResult::Ok ||
                !response)
            {
                errorCallback(
                    {
                        GeminiErrorType::Network,
                        0,
                        "Unable to connect to the Gemini API."
                    }
                );

                return;
            }

            const int statusCode =
                static_cast<int>(
                    response->getStatusCode()
                );

            if (statusCode < 200 ||
                statusCode >= 300)
            {
                errorCallback(
                    {
                        GeminiErrorType::Api,
                        statusCode,
                        extractApiErrorMessage(
                            response
                        )
                    }
                );

                return;
            }

            const auto responseJson =
                response->getJsonObject();

            if (!responseJson)
            {
                errorCallback(
                    {
                        GeminiErrorType::InvalidResponse,
                        statusCode,
                        "Gemini returned a response that was not valid JSON."
                    }
                );

                return;
            }

            const std::string generatedText =
                extractGeminiText(
                    *responseJson
                );

            if (generatedText.empty())
            {
                errorCallback(
                    {
                        GeminiErrorType::InvalidResponse,
                        statusCode,
                        "Gemini returned no generated text."
                    }
                );

                return;
            }

            successCallback(
                generatedText
            );
        }
    );
}

void GeminiClient::generateWithTools(
    const std::string &prompt,
    const Json::Value &tools,
    StructuredSuccessCallback successCallback,
    ErrorCallback errorCallback
) const
{
    if (!configuration_.isConfigured())
    {
        errorCallback(
            {
                GeminiErrorType::Configuration,
                0,
                "GEMINI_API_KEY is not configured."
            }
        );

        return;
    }

    if (prompt.empty())
    {
        errorCallback(
            {
                GeminiErrorType::Configuration,
                0,
                "The Gemini prompt must not be empty."
            }
        );

        return;
    }

    if (!tools.isArray() ||
        tools.empty())
    {
        errorCallback(
            {
                GeminiErrorType::Configuration,
                0,
                "Gemini tools must be a non-empty JSON array."
            }
        );

        return;
    }

    Json::Value requestBody;

    requestBody["contents"] =
        createUserContents(prompt);

    requestBody["tools"] =
        tools;

    requestBody["generationConfig"] =
        createGenerationConfig();

    const auto httpClient =
        drogon::HttpClient::newHttpClient(
            configuration_.baseUrl
        );

    if (!httpClient)
    {
        errorCallback(
            {
                GeminiErrorType::Network,
                0,
                "Unable to create the Gemini HTTP client."
            }
        );

        return;
    }

    const auto request =
        createGeminiRequest(
            configuration_,
            requestBody
        );

    httpClient->sendRequest(
        request,
        [
            httpClient,
            successCallback =
                std::move(successCallback),
            errorCallback =
                std::move(errorCallback)
        ](
            const drogon::ReqResult requestResult,
            const drogon::HttpResponsePtr &response
        ) mutable
        {
            static_cast<void>(httpClient);

            if (requestResult !=
                    drogon::ReqResult::Ok ||
                !response)
            {
                errorCallback(
                    {
                        GeminiErrorType::Network,
                        0,
                        "Unable to connect to the Gemini API."
                    }
                );

                return;
            }

            const int statusCode =
                static_cast<int>(
                    response->getStatusCode()
                );

            if (statusCode < 200 ||
                statusCode >= 300)
            {
                errorCallback(
                    {
                        GeminiErrorType::Api,
                        statusCode,
                        extractApiErrorMessage(
                            response
                        )
                    }
                );

                return;
            }

            const auto responseJson =
                response->getJsonObject();

            if (!responseJson)
            {
                errorCallback(
                    {
                        GeminiErrorType::InvalidResponse,
                        statusCode,
                        "Gemini returned a response that was not valid JSON."
                    }
                );

                return;
            }

            auto parsedResponse =
                GeminiResponseParser::parse(
                    *responseJson
                );

            if (!parsedResponse.has_value())
            {
                errorCallback(
                    {
                        GeminiErrorType::InvalidResponse,
                        statusCode,
                        "Gemini returned neither text nor a valid function call."
                    }
                );

                return;
            }

            successCallback(
                std::move(
                    parsedResponse.value()
                )
            );
        }
    );
}
}