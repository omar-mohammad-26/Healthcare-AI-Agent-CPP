BEGIN;

CREATE TABLE users
(
    id            SERIAL PRIMARY KEY,
    full_name     VARCHAR(128) NOT NULL,
    email         VARCHAR(128) NOT NULL,
    password_hash VARCHAR(256) NOT NULL,
    created_at    TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT uq_users_email
        UNIQUE (email),

    CONSTRAINT ck_users_full_name_not_blank
        CHECK (BTRIM(full_name) <> ''),

    CONSTRAINT ck_users_email_not_blank
        CHECK (BTRIM(email) <> '')
);

CREATE TABLE departments
(
    id          SERIAL PRIMARY KEY,
    name        VARCHAR(128) NOT NULL,
    description TEXT,

    CONSTRAINT uq_departments_name
        UNIQUE (name),

    CONSTRAINT ck_departments_name_not_blank
        CHECK (BTRIM(name) <> '')
);

CREATE TABLE doctors
(
    id            SERIAL PRIMARY KEY,
    full_name     VARCHAR(128) NOT NULL,
    specialty     VARCHAR(128) NOT NULL,
    department_id INTEGER NOT NULL,

    CONSTRAINT fk_doctors_department
        FOREIGN KEY (department_id)
            REFERENCES departments (id)
            ON UPDATE CASCADE
            ON DELETE RESTRICT,

    CONSTRAINT ck_doctors_full_name_not_blank
        CHECK (BTRIM(full_name) <> ''),

    CONSTRAINT ck_doctors_specialty_not_blank
        CHECK (BTRIM(specialty) <> '')
);

CREATE TABLE patients
(
    id              SERIAL PRIMARY KEY,
    full_name       VARCHAR(128) NOT NULL,
    phone           VARCHAR(32),
    age             INTEGER,
    gender          VARCHAR(16),
    medical_history TEXT,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT uq_patients_phone
        UNIQUE (phone),

    CONSTRAINT ck_patients_full_name_not_blank
        CHECK (BTRIM(full_name) <> ''),

    CONSTRAINT ck_patients_age_valid
        CHECK (
            age IS NULL
                OR age BETWEEN 0 AND 130
            ),

    CONSTRAINT ck_patients_gender_valid
        CHECK (
            gender IS NULL
                OR gender IN ('M', 'F', 'Other')
            )
);

CREATE TABLE appointments
(
    id                   SERIAL PRIMARY KEY,
    patient_id           INTEGER NOT NULL,
    doctor_id            INTEGER NOT NULL,
    appointment_datetime TIMESTAMPTZ NOT NULL,
    status               VARCHAR(32) NOT NULL DEFAULT 'scheduled',
    created_at           TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT fk_appointments_patient
        FOREIGN KEY (patient_id)
            REFERENCES patients (id)
            ON UPDATE CASCADE
            ON DELETE RESTRICT,

    CONSTRAINT fk_appointments_doctor
        FOREIGN KEY (doctor_id)
            REFERENCES doctors (id)
            ON UPDATE CASCADE
            ON DELETE RESTRICT,

    CONSTRAINT ck_appointments_status_valid
        CHECK (
            status IN (
                       'scheduled',
                       'cancelled',
                       'completed'
                )
            )
);

CREATE TABLE treatments
(
    id             SERIAL PRIMARY KEY,
    patient_id     INTEGER NOT NULL,
    doctor_id      INTEGER NOT NULL,
    diagnosis      TEXT NOT NULL,
    treatment_plan TEXT,
    created_at     TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT fk_treatments_patient
        FOREIGN KEY (patient_id)
            REFERENCES patients (id)
            ON UPDATE CASCADE
            ON DELETE RESTRICT,

    CONSTRAINT fk_treatments_doctor
        FOREIGN KEY (doctor_id)
            REFERENCES doctors (id)
            ON UPDATE CASCADE
            ON DELETE RESTRICT,

    CONSTRAINT ck_treatments_diagnosis_not_blank
        CHECK (BTRIM(diagnosis) <> '')
);

CREATE INDEX ix_doctors_specialty
    ON doctors (specialty);

CREATE INDEX ix_doctors_department_id
    ON doctors (department_id);

CREATE INDEX ix_doctors_full_name
    ON doctors (full_name);

CREATE INDEX ix_patients_full_name
    ON patients (full_name);

CREATE INDEX ix_appointments_doctor_datetime
    ON appointments (
                     doctor_id,
                     appointment_datetime
        );

CREATE INDEX ix_appointments_patient_datetime
    ON appointments (
                     patient_id,
                     appointment_datetime
        );

CREATE INDEX ix_appointments_datetime
    ON appointments (appointment_datetime);

CREATE INDEX ix_appointments_status
    ON appointments (status);

CREATE UNIQUE INDEX uq_active_doctor_slot
    ON appointments (
                     doctor_id,
                     appointment_datetime
        )
    WHERE status <> 'cancelled';

CREATE UNIQUE INDEX uq_active_patient_slot
    ON appointments (
                     patient_id,
                     appointment_datetime
        )
    WHERE status <> 'cancelled';

CREATE INDEX ix_treatments_patient_id
    ON treatments (patient_id);

CREATE INDEX ix_treatments_doctor_id
    ON treatments (doctor_id);

CREATE INDEX ix_treatments_created_at
    ON treatments (created_at);

COMMENT ON TABLE patients IS
    'Contains fictional demonstration patient data for the academic project.';

COMMIT;