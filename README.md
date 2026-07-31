# Healthcare AI Agent - C++

A university final project for the course **AI Agents in C++**.

The project is a clinic and hospital management system built with **C++20**, **Drogon**, **PostgreSQL**, **Gemini**, and a **React + TypeScript + Bootstrap 5** frontend.

Users interact with the system through a natural-language chat interface. The AI Agent chooses the required Function Tools, executes them against the PostgreSQL database, and returns one final answer.

---

## Team

- Omar Mohammad
- Nadeen
- Hisham

---

## Project Features

- C++20 backend using Drogon
- PostgreSQL database with six related healthcare tables
- More than ten meaningful REST endpoints
- Gemini API integration
- Eight registered Agent Function Tools
- Multi-step Agentic Loop
- Support for multiple Gemini function calls in one turn
- React and TypeScript chat interface
- Bootstrap 5 responsive design
- Automatic chat scrolling
- Friendly frontend error handling
- Live backend health status and retry support
- Automated C++ backend tests
- PowerShell API smoke tests
- Docker Compose development environment
- Production Docker images for backend and frontend
- Fictional demonstration healthcare data

---

## System Architecture

```text
React + TypeScript Frontend
            |
            | HTTP / JSON
            v
       Drogon C++ API
            |
            v
        AgentRunner
            |
            v
        Gemini API
            |
            v
   Agent Function Tools
            |
            v
 Services and Repositories
            |
            v
       PostgreSQL
```

---

## Technology Stack

### Backend

- C++20
- Drogon
- CMake
- JsonCpp
- PostgreSQL
- Drogon database client
- Gemini API

### Frontend

- React
- TypeScript
- Vite
- Bootstrap 5
- Bootstrap Icons
- Fetch API

### Infrastructure

- Docker
- Docker Compose
- Nginx for the production frontend
- PostgreSQL 15

### Testing

- CTest
- Custom C++ test executables
- PowerShell API smoke tests
- ESLint
- TypeScript compiler
- Vite production build verification

---

## Database Schema

The database contains the following tables:

- `users`
- `departments`
- `doctors`
- `patients`
- `appointments`
- `treatments`

### Main Relationships

```text
Departments 1 ----- * Doctors

Patients    1 ----- * Appointments
Doctors     1 ----- * Appointments

Patients    1 ----- * Treatments
Doctors     1 ----- * Treatments
```

The database is initialized automatically from:

```text
backend/migrations/001_initial_schema.sql
backend/migrations/002_seed_demo_data.sql
```

The complete Mermaid ERD, relationship descriptions, constraints, indexes, and testing guide are available in:

```text
docs/DATABASE_AND_TESTING.md
```

---

## Database Integrity

The PostgreSQL schema includes:

- Primary keys for every table
- Foreign-key relationships
- Unique email, department-name, and patient-phone constraints
- Patient age validation from 0 through 130
- Supported patient gender values
- Appointment status validation
- Required treatment diagnosis text
- Indexes for common searches
- Protection against conflicting active doctor appointment slots
- Protection against conflicting active patient appointment slots

Cancelled appointments do not block the same time slot.

---

## AI Agent Function Tools

The Agent has eight registered Function Tools:

1. `search_patient`
2. `list_doctors`
3. `doctor_schedule`
4. `available_slots`
5. `book_appointment`
6. `cancel_appointment`
7. `patient_history`
8. `add_treatment`

Each tool has:

- A unique name
- A description
- A JSON parameter schema
- Validation logic
- Database-backed execution
- A structured JSON result

---

## Agentic Loop

The Agent follows this workflow:

1. The user sends a natural-language prompt.
2. The prompt and tool declarations are sent to Gemini.
3. Gemini selects one or more Function Tools.
4. `AgentRunner` executes the selected tools.
5. Tool results are returned to Gemini.
6. Gemini may request additional tools.
7. The loop continues until Gemini produces a final answer.
8. The frontend displays the answer and the tools used.

The Agent currently allows a maximum of five tool-execution steps per request.

### Example Multi-Step Request

```text
Find the patient Sarah Cohen, list the doctors, then show the
available appointment slots for doctor Amit Patel on 2026-08-03.
```

This request may execute:

```text
search_patient
list_doctors
available_slots
```

The Agent then combines the tool results into one final response.

---

## Representative API Endpoints

### Health

| Method | Endpoint | Description |
|---|---|---|
| GET | `/health` | Backend health status |
| GET | `/health/database` | PostgreSQL connection status |
| GET | `/health/gemini` | Gemini connection status |

### Agent

| Method | Endpoint | Description |
|---|---|---|
| GET | `/agent/tools` | Return registered Agent tools |
| GET | `/agent/gemini-tools` | Return Gemini function declarations |
| POST | `/agent/tools/{name}` | Execute one Agent tool directly |
| POST | `/agent/gemini/preview` | Preview Gemini tool selection |
| POST | `/agent/run` | Run the complete Agent workflow |

### Healthcare Data

| Method | Endpoint | Description |
|---|---|---|
| GET | `/departments` | List departments |
| GET | `/doctors` | List doctors |
| GET | `/doctors/{id}/schedule` | Get a doctor's schedule |
| GET | `/doctors/{id}/available-slots` | Get available slots |
| GET | `/patients` | List patients |
| POST | `/patients` | Create a patient |
| GET | `/appointments` | List appointments |
| POST | `/appointments` | Create an appointment |
| POST | `/appointments/{id}/cancel` | Cancel an appointment |
| GET | `/treatments` | List treatments |

---

## Frontend

The frontend provides a single-screen healthcare chat interface.

It includes:

- Natural-language prompt input
- Example healthcare prompts
- Loading state while Agent tools execute
- Final Gemini response
- Tool-step badges
- Automatic scrolling to new messages
- Friendly network, timeout, and quota errors
- Backend connection retry button
- Live backend connection status
- Responsive Bootstrap 5 layout
- Fictional-data notice

Default frontend address:

```text
http://localhost:3000
```

---

## Environment Configuration

Copy the root environment example:

```powershell
Copy-Item .env.example .env
```

Then configure the Gemini API key inside `.env`:

```env
GEMINI_API_KEY=your_real_api_key
```

The real API key must remain only in the local `.env` file and must not be committed.

Important environment variables include:

```env
BACKEND_PORT=8000
FRONTEND_PORT=3000

POSTGRES_EXPOSED_PORT=5433
POSTGRES_DB=healthcare_db
POSTGRES_USER=healthcare_user
POSTGRES_PASSWORD=healthcare_pass

GEMINI_API_KEY=
GEMINI_MODEL=gemini-3.5-flash
GEMINI_BASE_URL=https://generativelanguage.googleapis.com

VITE_API_BASE_URL=http://localhost:8000
```

---

## Development Docker Setup

Start PostgreSQL, the Drogon backend, and the Vite frontend:

```powershell
docker compose up -d --build
```

Check the running services:

```powershell
docker compose ps
```

View backend and frontend logs:

```powershell
docker compose logs -f backend frontend
```

Open the application:

```text
http://localhost:3000
```

Backend API:

```text
http://localhost:8000
```

PostgreSQL host port:

```text
5433
```

Stop the development stack:

```powershell
docker compose down
```

Do not add `-v` unless the PostgreSQL data volume should also be deleted.

---

## Production Docker Setup

The production configuration uses:

- A compiled release-mode C++ backend image
- An Nginx frontend image
- PostgreSQL 15
- A separate production database volume

Start the production stack:

```powershell
docker compose -f docker-compose.prod.yml up -d --build
```

Check the production services:

```powershell
docker compose -f docker-compose.prod.yml ps
```

View production logs:

```powershell
docker compose -f docker-compose.prod.yml logs -f
```

Stop the production stack:

```powershell
docker compose -f docker-compose.prod.yml down
```

---

## Automated Backend Tests

Backend tests are stored in:

```text
backend/tests/
```

The current test suite includes:

- `GeminiResponseTests.cpp`
- `ToolRegistryTests.cpp`
- `GeminiConversationTests.cpp`

The tests verify:

- Gemini response parsing
- Multiple Gemini function calls
- Registration of all eight Agent tools
- Gemini conversation construction
- Function-response messages

Run the tests after configuring and building the backend:

```powershell
ctest --test-dir build --output-on-failure
```

Expected result:

```text
100% tests passed, 0 tests failed out of 3
```

---

## API Smoke Tests

The API smoke-test script is:

```text
scripts/api-smoke-tests.ps1
```

It verifies:

1. Backend health
2. Database health
3. Department listing
4. Doctor listing
5. Patient listing
6. Appointment listing
7. Treatment listing
8. Agent tool registry
9. Gemini function declarations
10. Frontend CORS preflight

Run the script while the backend is running:

```powershell
powershell -ExecutionPolicy Bypass `
    -File scripts/api-smoke-tests.ps1
```

Expected result:

```text
Passed: 10
Failed: 0

All API smoke tests passed.
```

These smoke tests do not send a live Gemini prompt and therefore do not consume Gemini API quota.

---

## Frontend Verification

Install dependencies when required:

```powershell
npm --prefix frontend install
```

Run the frontend linter:

```powershell
npm --prefix frontend run lint
```

Create a production build:

```powershell
npm --prefix frontend run build
```

Both commands must finish without errors.

---

## Manual Verification

### Backend Health

```powershell
Invoke-RestMethod `
    -Method Get `
    -Uri "http://localhost:8000/health"
```

Expected response:

```json
{
  "service": "healthcare-ai-agent",
  "status": "ok",
  "version": "0.1.0"
}
```

### Database Health

```powershell
Invoke-RestMethod `
    -Method Get `
    -Uri "http://localhost:8000/health/database"
```

### Agent Tool Registry

```powershell
Invoke-RestMethod `
    -Method Get `
    -Uri "http://localhost:8000/agent/tools" |
ConvertTo-Json -Depth 20
```

The response should report eight registered tools.

### Live Agent Request

```powershell
$body = @{
    prompt = "Find the patient Sarah Cohen, then list all doctors."
} | ConvertTo-Json

Invoke-RestMethod `
    -Method Post `
    -Uri "http://localhost:8000/agent/run" `
    -ContentType "application/json" `
    -Body $body |
ConvertTo-Json -Depth 20
```

A live Agent request requires:

- A configured Gemini API key
- Internet access
- Available Gemini API quota

External Gemini quota problems do not affect the local backend tests or API smoke tests.

---

## Project Structure

```text
Healthcare-AI-Agent-CPP/
|
|-- backend/
|   |-- migrations/
|   |   |-- 001_initial_schema.sql
|   |   `-- 002_seed_demo_data.sql
|   |
|   |-- src/
|   |   |-- agent/
|   |   |   `-- tools/
|   |   |-- ai/
|   |   |-- config/
|   |   |-- controllers/
|   |   |-- models/
|   |   |-- repositories/
|   |   |-- services/
|   |   `-- main.cpp
|   |
|   |-- tests/
|   |   |-- GeminiConversationTests.cpp
|   |   |-- GeminiResponseTests.cpp
|   |   |-- ToolRegistryTests.cpp
|   |   `-- CMakeLists.txt
|   |
|   |-- CMakeLists.txt
|   `-- config.json
|
|-- docs/
|   `-- DATABASE_AND_TESTING.md
|
|-- frontend/
|   |-- src/
|   |   |-- services/
|   |   |   `-- agentApi.ts
|   |   |-- App.tsx
|   |   |-- App.css
|   |   `-- main.tsx
|   |-- Dockerfile
|   |-- package.json
|   `-- vite.config.ts
|
|-- scripts/
|   `-- api-smoke-tests.ps1
|
|-- CMakeLists.txt
|-- Dockerfile
|-- Dockerfile.dev
|-- docker-compose.yml
|-- docker-compose.prod.yml
|-- .env.example
`-- README.md
```

---

## Detailed Documentation

The full database ERD, constraints, relationships, automated tests, smoke tests, and final verification checklist are documented in:

```text
docs/DATABASE_AND_TESTING.md
```

---

## Data Policy

All patient, doctor, appointment, and treatment information included in this project is fictional demonstration data created only for academic purposes.

The project must not be used with real patient information or as a real medical decision-making system.

---

## Current Status

The project currently includes:

- Functional Drogon REST backend
- PostgreSQL integration
- Six related database tables
- Fictional seeded healthcare data
- Eight Agent Function Tools
- Gemini tool declarations
- Multi-step and multi-function Agent execution
- Natural-language final responses
- React and TypeScript chat frontend
- Bootstrap 5 interface
- Automatic chat scrolling
- Friendly frontend errors and backend retry support
- Backend health monitoring
- Three automated backend test executables
- Ten API smoke tests
- Development Docker Compose stack
- Production Docker deployment
- Database ERD and testing documentation
