# Healthcare AI Agent — C++

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
- PostgreSQL database with related healthcare tables
- More than ten meaningful REST endpoints
- Gemini API integration
- Eight registered Agent Function Tools
- Multi-step Agentic Loop
- Support for multiple Gemini function calls in one turn
- React and TypeScript chat interface
- Bootstrap 5 responsive design
- Live backend health status
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

---

## Database Schema

The database contains the following main tables:

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

The database is initialized automatically from the SQL files inside:

```text
backend/migrations/
```

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

The Agent then combines all results into one final response.

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
| POST | `/agent/run` | Run the complete AI Agent workflow |
| POST | `/agent/gemini/preview` | Preview Gemini tool selection |
| GET | `/agent/gemini-tools` | Return the registered Gemini tool declarations |

### Healthcare Data

The backend also contains REST routes for:

- Departments
- Doctors
- Doctor schedules
- Available appointment slots
- Patients
- Appointments
- Treatments

---

## Frontend

The frontend provides a single-screen healthcare chat interface.

It includes:

- Natural-language prompt input
- Example healthcare prompts
- Loading state while Agent tools execute
- Final Gemini response
- Tool-step badges
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

### Frontend Build

```powershell
cd frontend
npm install
npm run build
npm run lint
cd ..
```

### Agent Request

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

A live Agent request requires a configured Gemini API key and available Gemini quota.

---

## Project Structure

```text
Healthcare-AI-Agent-CPP/
|
|-- backend/
|   |-- migrations/
|   |-- src/
|   |   |-- agent/
|   |   |   |-- tools/
|   |   |-- ai/
|   |   |-- config/
|   |   |-- controllers/
|   |   |-- models/
|   |   |-- repositories/
|   |   |-- services/
|   |   `-- main.cpp
|   |-- CMakeLists.txt
|   `-- config.json
|
|-- frontend/
|   |-- src/
|   |   |-- services/
|   |   |-- App.tsx
|   |   |-- App.css
|   |   `-- main.tsx
|   |-- Dockerfile
|   |-- package.json
|   `-- vite.config.ts
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

## Data Policy

All patient, doctor, appointment, and treatment information included in this project is fictional demonstration data created only for academic purposes.

The project must not be used with real patient information or as a real medical decision-making system.

---

## Current Status

The project currently includes:

- Functional Drogon REST backend
- PostgreSQL integration
- Fictional seeded healthcare data
- Eight Agent Function Tools
- Gemini tool declarations
- Multi-step and multi-function Agent execution
- Natural-language final responses
- React and TypeScript chat frontend
- Bootstrap 5 interface
- Backend health monitoring
- Development Docker Compose stack
- Production Docker deployment