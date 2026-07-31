FROM drogonframework/drogon:latest AS build

USER root

WORKDIR /workspace

COPY CMakeLists.txt ./
COPY backend ./backend

RUN cmake \
        -S . \
        -B build \
        -DCMAKE_BUILD_TYPE=Release \
    && cmake \
        --build build \
        --parallel 2


FROM drogonframework/drogon:latest AS runtime

USER root

WORKDIR /app

COPY --from=build \
    /workspace/build/backend/healthcare_ai_agent \
    /app/healthcare_ai_agent

COPY --from=build \
    /workspace/build/backend/config.json \
    /app/config.json

EXPOSE 8000

CMD ["/app/healthcare_ai_agent"]
