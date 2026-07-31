# Healthcare AI Agent Frontend

This directory contains the **React + TypeScript** chat interface for the Healthcare AI Agent project.

The frontend communicates with the **C++ Drogon backend** through HTTP. It allows users to ask healthcare-related questions in natural language and displays the final Gemini response together with the Function Tools executed by the Agent.

---

## Features

- Natural-language healthcare chat
- Example prompts
- Gemini-generated final answers
- Agent Function Tool step badges
- Loading state during Agent execution
- Automatic scrolling to new messages
- Backend connection status
- Backend connection retry button
- Friendly network, timeout, and Gemini quota errors
- Responsive Bootstrap 5 interface
- Fictional healthcare data notice

---

## Technology Stack

- React
- TypeScript
- Vite
- Bootstrap 5
- Bootstrap Icons
- Fetch API

---

## Backend Configuration

The frontend reads the backend address from:

```env
VITE_API_BASE_URL=http://localhost:8000
```

When `VITE_API_BASE_URL` is not configured, the frontend uses:

```text
http://localhost:8000
```

The Gemini API key must never be placed in the frontend or exposed through an environment variable beginning with `VITE_`.

The private Gemini key belongs only in the root project `.env` file:

```env
GEMINI_API_KEY=your_private_api_key
```

The frontend sends requests to the backend, and the backend communicates securely with Gemini.

---

## Install Dependencies

From the repository root:

```powershell
npm --prefix frontend install
```

Or from inside the `frontend` directory:

```powershell
npm install
```

---

## Run the Frontend Locally

The C++ backend must be running at:

```text
http://localhost:8000
```

From the repository root, start the frontend with:

```powershell
npm --prefix frontend run dev
```

Or from inside the `frontend` directory:

```powershell
npm run dev
```

The default Vite development address is:

```text
http://localhost:5173
```

---

## Run the Complete Project with Docker

From the repository root:

```powershell
docker compose up -d --build
```

This starts:

- PostgreSQL
- Drogon backend
- React frontend

Open the application at:

```text
http://localhost:3000
```

The backend is available at:

```text
http://localhost:8000
```

Check the running services:

```powershell
docker compose ps
```

View the frontend and backend logs:

```powershell
docker compose logs -f frontend backend
```

Stop following the logs with `Ctrl + C`.

Stop the project with:

```powershell
docker compose down
```

Do not add `-v` unless the PostgreSQL data volume should also be deleted.

---

## Run the Linter

From the repository root:

```powershell
npm --prefix frontend run lint
```

The command should complete without ESLint errors.

---

## Create a Production Build

From the repository root:

```powershell
npm --prefix frontend run build
```

The production files are generated inside:

```text
frontend/dist/
```

---

## Production Docker Image

The production frontend image is defined in:

```text
frontend/Dockerfile
```

It builds the React application and serves the generated production files through Nginx.

Start the complete production stack from the repository root:

```powershell
docker compose -f docker-compose.prod.yml up -d --build
```

Check the production services:

```powershell
docker compose -f docker-compose.prod.yml ps
```

The production frontend is available at:

```text
http://localhost:3000
```

Stop the production stack:

```powershell
docker compose -f docker-compose.prod.yml down
```

---

## Main Source Files

```text
frontend/
|
|-- src/
|   |-- services/
|   |   `-- agentApi.ts
|   |-- App.tsx
|   |-- App.css
|   `-- main.tsx
|
|-- Dockerfile
|-- package.json
|-- vite.config.ts
`-- README.md
```

### `src/App.tsx`

Contains:

- Main chat interface
- Message rendering
- Example prompts
- Backend health status
- Backend retry handling
- Agent loading state
- Automatic message scrolling
- Friendly error display
- Agent tool-step badges

### `src/services/agentApi.ts`

Contains the centralized HTTP functions for:

- Checking backend health
- Sending prompts to `/agent/run`
- Parsing backend responses
- Validating response data
- Handling API errors

### `src/App.css`

Contains the chat layout and custom component styling used together with Bootstrap 5.

### `src/main.tsx`

Initializes the React application and imports the required Bootstrap resources.

---

## Useful Chat Examples

### Search for a patient

```text
Find the patient Sarah Cohen.
```

### List doctors

```text
List all doctors and their specialties.
```

### Check available appointments

```text
Show available slots for doctor ID 1 on YYYY-MM-DD.
```

### Multi-step Agent request

```text
Find Sarah Cohen, list the doctors, and show the available slots for doctor ID 1 on YYYY-MM-DD.
```

Replace `YYYY-MM-DD` with a valid future date.

---

## Data Policy

The frontend displays only fictional demonstration healthcare information created for this academic project.

It must not be used with real patient information or as a real medical diagnosis, treatment, or decision-making system.