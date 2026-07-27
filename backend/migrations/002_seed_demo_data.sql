BEGIN;

-- All records in this file are fictional demonstration data
-- created exclusively for this academic project.

INSERT INTO departments
(
    name,
    description
)
VALUES
    (
        'Cardiology',
        'Diagnosis and treatment of heart-related conditions.'
    ),
    (
        'Neurology',
        'Diagnosis and treatment of the brain and nervous system.'
    ),
    (
        'Pediatrics',
        'Medical care for infants, children, and adolescents.'
    ),
    (
        'Oncology',
        'Diagnosis and treatment of cancer-related conditions.'
    ),
    (
        'General Medicine',
        'Primary care, routine examinations, and general treatment.'
    )
    ON CONFLICT (name)
DO UPDATE SET
    description = EXCLUDED.description;

WITH doctor_seed
         (
          full_name,
          specialty,
          department_name
             ) AS
         (
             VALUES
                 (
                     'Amit Patel',
                     'Cardiology',
                     'Cardiology'
                 ),
                 (
                     'Lina Chen',
                     'Cardiology',
                     'Cardiology'
                 ),
                 (
                     'Robert Jones',
                     'Neurology',
                     'Neurology'
                 ),
                 (
                     'Maria Gonzalez',
                     'Pediatrics',
                     'Pediatrics'
                 ),
                 (
                     'Wei Zhang',
                     'Oncology',
                     'Oncology'
                 ),
                 (
                     'Olivia Brown',
                     'General Medicine',
                     'General Medicine'
                 ),
                 (
                     'Samuel Lee',
                     'Cardiology',
                     'Cardiology'
                 ),
                 (
                     'Fatima Khan',
                     'General Medicine',
                     'General Medicine'
                 )
         )
INSERT INTO doctors
(
    full_name,
    specialty,
    department_id
)
SELECT
    seed.full_name,
    seed.specialty,
    department.id
FROM doctor_seed AS seed
         JOIN departments AS department
              ON department.name = seed.department_name
WHERE NOT EXISTS
          (
              SELECT 1
              FROM doctors AS existing
              WHERE existing.full_name = seed.full_name
          );

INSERT INTO patients
(
    full_name,
    phone,
    age,
    gender,
    medical_history
)
VALUES
    (
        'Sarah Cohen',
        '555-0101',
        34,
        'F',
        'No major medical issues.'
    ),
    (
        'David Levi',
        '555-0102',
        45,
        'M',
        'History of hypertension.'
    ),
    (
        'John Smith',
        '555-0103',
        29,
        'M',
        'Mild asthma.'
    ),
    (
        'Emily Davis',
        '555-0104',
        52,
        'F',
        'Type 2 diabetes.'
    ),
    (
        'Michael Brown',
        '555-0105',
        60,
        'M',
        'History of heart disease.'
    ),
    (
        'Angela White',
        '555-0106',
        39,
        'F',
        'No known chronic conditions.'
    ),
    (
        'Chris Green',
        '555-0107',
        27,
        'M',
        'Seasonal allergies.'
    ),
    (
        'Nora Black',
        '555-0108',
        22,
        'F',
        'No known medical conditions.'
    ),
    (
        'Liam Moore',
        '555-0109',
        48,
        'M',
        'Elevated cholesterol.'
    ),
    (
        'Olga Ivanova',
        '555-0110',
        55,
        'F',
        'Knee arthritis.'
    )
    ON CONFLICT (phone)
DO UPDATE SET
    full_name = EXCLUDED.full_name,
           age = EXCLUDED.age,
           gender = EXCLUDED.gender,
           medical_history = EXCLUDED.medical_history;

WITH appointment_seed
         (
          patient_phone,
          doctor_name,
          day_offset,
          start_hour,
          start_minute,
          appointment_status
             ) AS
         (
             VALUES
                 (
                     '555-0101',
                     'Amit Patel',
                     -3,
                     9,
                     0,
                     'completed'
                 ),
                 (
                     '555-0102',
                     'Robert Jones',
                     -2,
                     10,
                     0,
                     'completed'
                 ),
                 (
                     '555-0104',
                     'Maria Gonzalez',
                     -1,
                     11,
                     0,
                     'completed'
                 ),
                 (
                     '555-0101',
                     'Amit Patel',
                     1,
                     9,
                     0,
                     'scheduled'
                 ),
                 (
                     '555-0102',
                     'Lina Chen',
                     1,
                     10,
                     0,
                     'scheduled'
                 ),
                 (
                     '555-0103',
                     'Robert Jones',
                     2,
                     9,
                     0,
                     'scheduled'
                 ),
                 (
                     '555-0104',
                     'Maria Gonzalez',
                     2,
                     10,
                     30,
                     'scheduled'
                 ),
                 (
                     '555-0105',
                     'Wei Zhang',
                     3,
                     11,
                     0,
                     'scheduled'
                 ),
                 (
                     '555-0106',
                     'Olivia Brown',
                     3,
                     13,
                     0,
                     'scheduled'
                 ),
                 (
                     '555-0107',
                     'Samuel Lee',
                     4,
                     9,
                     30,
                     'scheduled'
                 ),
                 (
                     '555-0108',
                     'Fatima Khan',
                     4,
                     14,
                     0,
                     'scheduled'
                 ),
                 (
                     '555-0109',
                     'Amit Patel',
                     5,
                     10,
                     0,
                     'scheduled'
                 ),
                 (
                     '555-0110',
                     'Lina Chen',
                     5,
                     11,
                     0,
                     'scheduled'
                 ),
                 (
                     '555-0108',
                     'Amit Patel',
                     6,
                     15,
                     0,
                     'scheduled'
                 )
         ),
     resolved_appointments AS
         (
             SELECT
                 patient.id AS patient_id,
                 doctor.id AS doctor_id,
                 (
                             CURRENT_DATE
                         + seed.day_offset * INTERVAL '1 day'
                         + seed.start_hour * INTERVAL '1 hour'
                         + seed.start_minute * INTERVAL '1 minute'
                     ) AS appointment_datetime,
                 seed.appointment_status
             FROM appointment_seed AS seed
                      JOIN patients AS patient
                           ON patient.phone = seed.patient_phone
                      JOIN doctors AS doctor
                           ON doctor.full_name = seed.doctor_name
         )
INSERT INTO appointments
(
    patient_id,
    doctor_id,
    appointment_datetime,
    status
)
SELECT
    seed.patient_id,
    seed.doctor_id,
    seed.appointment_datetime,
    seed.appointment_status
FROM resolved_appointments AS seed
WHERE NOT EXISTS
          (
              SELECT 1
              FROM appointments AS existing
              WHERE existing.patient_id = seed.patient_id
                AND existing.doctor_id = seed.doctor_id
                AND existing.appointment_datetime =
                    seed.appointment_datetime
          );

WITH treatment_seed
         (
          patient_phone,
          doctor_name,
          diagnosis,
          treatment_plan,
          days_ago
             ) AS
         (
             VALUES
                 (
                     '555-0102',
                     'Amit Patel',
                     'Mild hypertension',
                     'Monitor blood pressure and schedule a follow-up visit.',
                     10
                 ),
                 (
                     '555-0107',
                     'Fatima Khan',
                     'Seasonal allergies',
                     'Avoid known triggers and monitor symptoms.',
                     8
                 ),
                 (
                     '555-0103',
                     'Robert Jones',
                     'Tension headache',
                     'Improve hydration and sleep habits.',
                     7
                 ),
                 (
                     '555-0109',
                     'Olivia Brown',
                     'Elevated cholesterol',
                     'Continue lifestyle counseling and repeat the lipid panel.',
                     6
                 ),
                 (
                     '555-0103',
                     'Olivia Brown',
                     'Mild asthma',
                     'Review symptom control during the next follow-up.',
                     5
                 ),
                 (
                     '555-0110',
                     'Maria Gonzalez',
                     'Knee osteoarthritis',
                     'Continue low-impact exercise and schedule follow-up.',
                     4
                 ),
                 (
                     '555-0104',
                     'Wei Zhang',
                     'Routine diabetes follow-up',
                     'Continue monitoring and repeat blood testing.',
                     3
                 ),
                 (
                     '555-0105',
                     'Amit Patel',
                     'Cardiology follow-up',
                     'Continue observation and return for the next review.',
                     2
                 )
         ),
     resolved_treatments AS
         (
             SELECT
                 patient.id AS patient_id,
                 doctor.id AS doctor_id,
                 seed.diagnosis,
                 seed.treatment_plan,
                 CURRENT_TIMESTAMP
                     - seed.days_ago * INTERVAL '1 day'
    AS created_at
FROM treatment_seed AS seed
    JOIN patients AS patient
ON patient.phone = seed.patient_phone
    JOIN doctors AS doctor
    ON doctor.full_name = seed.doctor_name
    )
INSERT INTO treatments
(
    patient_id,
    doctor_id,
    diagnosis,
    treatment_plan,
    created_at
)
SELECT
    seed.patient_id,
    seed.doctor_id,
    seed.diagnosis,
    seed.treatment_plan,
    seed.created_at
FROM resolved_treatments AS seed
WHERE NOT EXISTS
          (
              SELECT 1
              FROM treatments AS existing
              WHERE existing.patient_id = seed.patient_id
                AND existing.diagnosis = seed.diagnosis
          );

COMMIT;