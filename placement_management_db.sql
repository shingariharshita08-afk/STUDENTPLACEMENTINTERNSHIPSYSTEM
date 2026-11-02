-- Student Placement & Internship Management System
-- Run this entire script in MySQL Workbench before starting the C++ program.

CREATE DATABASE IF NOT EXISTS placement_management_db
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_unicode_ci;

USE placement_management_db;

CREATE TABLE IF NOT EXISTS students (
    id INT AUTO_INCREMENT PRIMARY KEY,
    roll_number VARCHAR(30) NOT NULL UNIQUE,
    full_name VARCHAR(120) NOT NULL,
    email VARCHAR(160) NOT NULL UNIQUE,
    phone VARCHAR(30) NOT NULL,
    cgpa DECIMAL(3,2) NOT NULL,
    skills TEXT NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT chk_students_cgpa CHECK (cgpa >= 0.00 AND cgpa <= 10.00)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS companies (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(150) NOT NULL UNIQUE,
    industry VARCHAR(100) NOT NULL,
    website VARCHAR(255) NOT NULL DEFAULT 'N/A',
    contact_email VARCHAR(160) NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS job_openings (
    id INT AUTO_INCREMENT PRIMARY KEY,
    company_id INT NOT NULL,
    title VARCHAR(150) NOT NULL,
    description TEXT NOT NULL,
    min_cgpa DECIMAL(3,2) NOT NULL DEFAULT 0.00,
    openings INT NOT NULL DEFAULT 1,
    job_type ENUM('Internship', 'Full-time', 'Part-time') NOT NULL,
    location VARCHAR(120) NOT NULL,
    last_date DATE NOT NULL,
    is_active BOOLEAN NOT NULL DEFAULT TRUE,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT chk_job_min_cgpa CHECK (min_cgpa >= 0.00 AND min_cgpa <= 10.00),
    CONSTRAINT chk_job_openings CHECK (openings > 0),
    CONSTRAINT fk_jobs_company FOREIGN KEY (company_id)
        REFERENCES companies(id) ON UPDATE CASCADE ON DELETE RESTRICT,
    INDEX idx_jobs_company (company_id),
    INDEX idx_jobs_open (is_active, last_date)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS applications (
    id INT AUTO_INCREMENT PRIMARY KEY,
    student_id INT NOT NULL,
    job_id INT NOT NULL,
    applied_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    status ENUM('Applied', 'Under Review', 'Shortlisted', 'Rejected', 'Selected')
        NOT NULL DEFAULT 'Applied',
    CONSTRAINT uq_student_job_application UNIQUE (student_id, job_id),
    CONSTRAINT fk_applications_student FOREIGN KEY (student_id)
        REFERENCES students(id) ON UPDATE CASCADE ON DELETE RESTRICT,
    CONSTRAINT fk_applications_job FOREIGN KEY (job_id)
        REFERENCES job_openings(id) ON UPDATE CASCADE ON DELETE RESTRICT,
    INDEX idx_applications_status (status)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS interviews (
    id INT AUTO_INCREMENT PRIMARY KEY,
    application_id INT NOT NULL,
    interview_datetime DATETIME NOT NULL,
    round_name VARCHAR(100) NOT NULL,
    mode ENUM('Online', 'In-person', 'Phone') NOT NULL,
    venue_or_link VARCHAR(500) NOT NULL,
    notes TEXT NOT NULL,
    status ENUM('Scheduled', 'Completed', 'Cancelled') NOT NULL DEFAULT 'Scheduled',
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_interviews_application FOREIGN KEY (application_id)
        REFERENCES applications(id) ON UPDATE CASCADE ON DELETE CASCADE,
    INDEX idx_interviews_datetime (interview_datetime)
) ENGINE=InnoDB;

-- Optional sample records for testing: uncomment this block if you want demo data.
-- INSERT INTO students (roll_number, full_name, email, phone, cgpa, skills)
-- VALUES ('DEMO001', 'Aarav Sharma', 'aarav@example.com', '9876543210', 8.20, 'C++, SQL, Python');
-- INSERT INTO companies (name, industry, website, contact_email)
-- VALUES ('Example Technologies', 'Information Technology', 'https://example.com', 'careers@example.com');
-- INSERT INTO job_openings (company_id, title, description, min_cgpa, openings, job_type, location, last_date)
-- VALUES (1, 'Software Intern', 'Assist with application development.', 7.00, 3, 'Internship', 'Chandigarh / Hybrid', '2027-12-31');
