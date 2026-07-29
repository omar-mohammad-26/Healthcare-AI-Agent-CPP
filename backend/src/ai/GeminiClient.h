#pragma once

#include "config/GeminiConfig.h"

#include <functional>
#include <string>

namespace healthcare::ai
{
    enum class GeminiErrorType
    {
        Configuration,
        Network,
        Api,
        InvalidResponse
    };

    struct GeminiError
    {
        GeminiErrorType type;
        int statusCode{};
        std::string message;
    };

    class GeminiClient final
    {
    public:
        using SuccessCallback =
            std::function<void(std::string)>;

        using ErrorCallback =
            std::function<void(GeminiError)>;

        explicit GeminiClient(
            healthcare::config::GeminiConfig configuration
        );

        [[nodiscard]]
        const std::string &model() const;

        [[nodiscard]]
        bool isConfigured() const;

        void generateText(
            const std::string &prompt,
            SuccessCallback successCallback,
            ErrorCallback errorCallback
        ) const;

    private:
        healthcare::config::GeminiConfig configuration_;
    };
}