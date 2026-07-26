# Healthcare AI Agent - C++

A university final project for the course **AI Agents in C++**.

The project is a clinic and hospital management system with a
Gemini-powered AI Agent. Users will be able to ask questions in natural
language and perform operations such as finding patients, checking doctor
availability, booking appointments, cancelling appointments, reviewing
treatment history, and generating clinic analytics.

## Team

- Omar Mohammad
- Nadeen
- Hisham

## Planned Technology Stack

### Backend

- C++20
- Drogon
- CMake
- PostgreSQL
- Drogon ORM
- Gemini API
- JWT authentication
- bcrypt password hashing

### Frontend

- React
- TypeScript
- Bootstrap 5
- Fetch API
- Server-Sent Events

### Infrastructure

- Docker
- Docker Compose

## Planned Core Entities

- Users
- Departments
- Doctors
- Patients
- Appointments
- Treatments

## AI Agent

The Agent will use Function Tools to query and modify the PostgreSQL
database.

A multi-step appointment workflow may use:

1. `search_patient`
2. `available_slots`
3. `book_appointment`

The Agent will combine the tool results into one final response.

## Data Policy

All patient, doctor, appointment, and treatment data used in this project
is fictional demonstration data created only for academic purposes.

## Project Status

Initial C++ and CMake project foundation.
