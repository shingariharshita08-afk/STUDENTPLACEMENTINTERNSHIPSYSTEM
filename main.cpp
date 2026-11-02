#include <mysql/jdbc.h>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>

// Update these values for your local MySQL installation.
// Never commit a real password to a public GitHub repository.
const std::string DB_HOST = "tcp://127.0.0.1:3306";
const std::string DB_USER = "root";
const std::string DB_PASSWORD = "CHANGE_THIS_PASSWORD";
const std::string DB_NAME = "placement_management_db";

// Base class used to demonstrate inheritance and runtime polymorphism.
class Person {
private:
    std::string fullName;
    std::string email;
    std::string phone;

public:
    Person(const std::string& name, const std::string& emailAddress,
           const std::string& phoneNumber)
        : fullName(name), email(emailAddress), phone(phoneNumber) {}

    virtual ~Person() = default;

    const std::string& getFullName() const { return fullName; }
    const std::string& getEmail() const { return email; }
    const std::string& getPhone() const { return phone; }

    virtual std::string getRole() const = 0;
};

class Student final : public Person {
private:
    std::string rollNumber;
    double cgpa;
    std::string skills;

public:
    Student(const std::string& roll, const std::string& name,
            const std::string& email, const std::string& phone,
            double grade, const std::string& skillList)
        : Person(name, email, phone), rollNumber(roll), cgpa(grade), skills(skillList) {}

    std::string getRole() const override { return "Student"; }
    const std::string& getRollNumber() const { return rollNumber; }
    double getCgpa() const { return cgpa; }
    const std::string& getSkills() const { return skills; }
};

class Company {
private:
    std::string name;
    std::string industry;
    std::string website;
    std::string contactEmail;

public:
    Company(const std::string& companyName, const std::string& companyIndustry,
            const std::string& companyWebsite, const std::string& email)
        : name(companyName), industry(companyIndustry), website(companyWebsite), contactEmail(email) {}

    const std::string& getName() const { return name; }
    const std::string& getIndustry() const { return industry; }
    const std::string& getWebsite() const { return website; }
    const std::string& getContactEmail() const { return contactEmail; }
};

class JobOpening {
private:
    int companyId;
    std::string title;
    std::string description;
    double minimumCgpa;
    int openings;
    std::string jobType;
    std::string location;
    std::string lastDate;

public:
    JobOpening(int company, const std::string& jobTitle, const std::string& jobDescription,
               double minCgpa, int numberOfOpenings, const std::string& type,
               const std::string& jobLocation, const std::string& deadline)
        : companyId(company), title(jobTitle), description(jobDescription),
          minimumCgpa(minCgpa), openings(numberOfOpenings), jobType(type),
          location(jobLocation), lastDate(deadline) {}

    int getCompanyId() const { return companyId; }
    const std::string& getTitle() const { return title; }
    const std::string& getDescription() const { return description; }
    double getMinimumCgpa() const { return minimumCgpa; }
    int getOpenings() const { return openings; }
    const std::string& getJobType() const { return jobType; }
    const std::string& getLocation() const { return location; }
    const std::string& getLastDate() const { return lastDate; }
};

class PlacementManagementSystem {
private:
    std::unique_ptr<sql::Connection> connection;

    static std::string readText(const std::string& prompt) {
        while (true) {
            std::cout << prompt;
            std::string value;
            std::getline(std::cin, value);
            if (!std::cin) {
                throw std::runtime_error("Input stream closed.");
            }
            if (value.find_first_not_of(" \t\r\n") != std::string::npos) {
                return value;
            }
            std::cout << "This field cannot be empty. Please try again.\n";
        }
    }

    static int readInt(const std::string& prompt, int minimum, int maximum) {
        while (true) {
            const std::string input = readText(prompt);
            try {
                std::size_t used = 0;
                const long value = std::stol(input, &used);
                if (used == input.size() && value >= minimum && value <= maximum) {
                    return static_cast<int>(value);
                }
            } catch (const std::exception&) {
                // Print the same validation message below.
            }
            std::cout << "Enter a whole number from " << minimum << " to " << maximum << ".\n";
        }
    }

    static double readDoubleInRange(const std::string& prompt, double minimum, double maximum) {
        while (true) {
            const std::string input = readText(prompt);
            try {
                std::size_t used = 0;
                const double value = std::stod(input, &used);
                if (used == input.size() && std::isfinite(value) &&
                    value >= minimum && value <= maximum) {
                    return value;
                }
            } catch (const std::exception&) {
                // Print the same validation message below.
            }
            std::cout << "Enter a number from " << minimum << " to " << maximum << ".\n";
        }
    }

    static std::string readDate(const std::string& prompt) {
        const std::regex pattern(R"(^\d{4}-\d{2}-\d{2}$)");
        while (true) {
            const std::string value = readText(prompt);
            if (std::regex_match(value, pattern)) {
                return value;
            }
            std::cout << "Use YYYY-MM-DD format, for example 2026-12-15.\n";
        }
    }

    static std::string readDateTime(const std::string& prompt) {
        const std::regex pattern(R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}(:\d{2})?$)");
        while (true) {
            const std::string value = readText(prompt);
            if (std::regex_match(value, pattern)) {
                return value.size() == 16 ? value + ":00" : value;
            }
            std::cout << "Use YYYY-MM-DD HH:MM or YYYY-MM-DD HH:MM:SS format.\n";
        }
    }

    void printMenu() const {
        std::cout << "\n========== STUDENT PLACEMENT & INTERNSHIP MANAGEMENT SYSTEM ==========\n"
                  << "1.  Add student\n"
                  << "2.  View students\n"
                  << "3.  Add company\n"
                  << "4.  View companies\n"
                  << "5.  Post job / internship\n"
                  << "6.  View job openings\n"
                  << "7.  Apply for a job\n"
                  << "8.  View applications\n"
                  << "9.  Update application status\n"
                  << "10. Schedule interview\n"
                  << "11. View interviews\n"
                  << "12. Placement dashboard / report\n"
                  << "0.  Exit\n"
                  << "======================================================================\n";
    }

    void showStudents() {
        std::unique_ptr<sql::Statement> statement(connection->createStatement());
        std::unique_ptr<sql::ResultSet> result(statement->executeQuery(
            "SELECT id, roll_number, full_name, email, phone, cgpa, skills "
            "FROM students ORDER BY id"));

        std::cout << "\nSTUDENTS\n"
                  << std::left << std::setw(6) << "ID" << std::setw(16) << "Roll No."
                  << std::setw(24) << "Name" << std::setw(28) << "Email"
                  << std::setw(15) << "Phone" << std::setw(8) << "CGPA" << "Skills\n"
                  << std::string(115, '-') << "\n";
        bool found = false;
        while (result->next()) {
            found = true;
            std::cout << std::left << std::setw(6) << result->getInt("id")
                      << std::setw(16) << result->getString("roll_number")
                      << std::setw(24) << result->getString("full_name")
                      << std::setw(28) << result->getString("email")
                      << std::setw(15) << result->getString("phone")
                      << std::fixed << std::setprecision(2) << std::setw(8) << result->getDouble("cgpa")
                      << result->getString("skills") << "\n";
        }
        if (!found) std::cout << "No students have been added yet.\n";
        std::cout << "\n";
    }

    void showCompanies() {
        std::unique_ptr<sql::Statement> statement(connection->createStatement());
        std::unique_ptr<sql::ResultSet> result(statement->executeQuery(
            "SELECT id, name, industry, website, contact_email FROM companies ORDER BY id"));

        std::cout << "\nCOMPANIES\n"
                  << std::left << std::setw(6) << "ID" << std::setw(26) << "Company"
                  << std::setw(22) << "Industry" << std::setw(30) << "Website"
                  << "Contact email\n" << std::string(100, '-') << "\n";
        bool found = false;
        while (result->next()) {
            found = true;
            std::cout << std::left << std::setw(6) << result->getInt("id")
                      << std::setw(26) << result->getString("name")
                      << std::setw(22) << result->getString("industry")
                      << std::setw(30) << result->getString("website")
                      << result->getString("contact_email") << "\n";
        }
        if (!found) std::cout << "No companies have been added yet.\n";
        std::cout << "\n";
    }

    void showJobs(bool activeOnly = false) {
        const std::string query = activeOnly
            ? "SELECT j.id, c.name AS company_name, j.title, j.job_type, j.location, "
              "j.min_cgpa, j.openings, j.last_date, j.is_active "
              "FROM job_openings j JOIN companies c ON c.id = j.company_id "
              "WHERE j.is_active = 1 AND j.last_date >= CURDATE() ORDER BY j.id DESC"
            : "SELECT j.id, c.name AS company_name, j.title, j.job_type, j.location, "
              "j.min_cgpa, j.openings, j.last_date, j.is_active "
              "FROM job_openings j JOIN companies c ON c.id = j.company_id ORDER BY j.id DESC";
        std::unique_ptr<sql::Statement> statement(connection->createStatement());
        std::unique_ptr<sql::ResultSet> result(statement->executeQuery(query));

        std::cout << "\n" << (activeOnly ? "OPEN JOBS / INTERNSHIPS" : "ALL JOBS / INTERNSHIPS") << "\n"
                  << std::left << std::setw(6) << "ID" << std::setw(24) << "Company"
                  << std::setw(25) << "Role" << std::setw(14) << "Type"
                  << std::setw(18) << "Location" << std::setw(9) << "Min CGPA"
                  << std::setw(10) << "Openings" << std::setw(13) << "Last date" << "Status\n"
                  << std::string(119, '-') << "\n";
        bool found = false;
        while (result->next()) {
            found = true;
            std::cout << std::left << std::setw(6) << result->getInt("id")
                      << std::setw(24) << result->getString("company_name")
                      << std::setw(25) << result->getString("title")
                      << std::setw(14) << result->getString("job_type")
                      << std::setw(18) << result->getString("location")
                      << std::fixed << std::setprecision(2) << std::setw(9) << result->getDouble("min_cgpa")
                      << std::setw(10) << result->getInt("openings")
                      << std::setw(13) << result->getString("last_date")
                      << (result->getInt("is_active") == 1 ? "Active" : "Inactive") << "\n";
        }
        if (!found) std::cout << "No matching job openings found.\n";
        std::cout << "\n";
    }

    void showApplications() {
        std::unique_ptr<sql::Statement> statement(connection->createStatement());
        std::unique_ptr<sql::ResultSet> result(statement->executeQuery(
            "SELECT a.id AS application_id, s.roll_number, s.full_name, c.name AS company_name, "
            "j.title AS job_title, a.applied_at, a.status "
            "FROM applications a JOIN students s ON s.id = a.student_id "
            "JOIN job_openings j ON j.id = a.job_id JOIN companies c ON c.id = j.company_id "
            "ORDER BY a.id DESC"));

        std::cout << "\nAPPLICATIONS\n"
                  << std::left << std::setw(9) << "App ID" << std::setw(16) << "Roll No."
                  << std::setw(24) << "Student" << std::setw(23) << "Company"
                  << std::setw(25) << "Role" << std::setw(21) << "Applied at" << "Status\n"
                  << std::string(118, '-') << "\n";
        bool found = false;
        while (result->next()) {
            found = true;
            std::cout << std::left << std::setw(9) << result->getInt("application_id")
                      << std::setw(16) << result->getString("roll_number")
                      << std::setw(24) << result->getString("full_name")
                      << std::setw(23) << result->getString("company_name")
                      << std::setw(25) << result->getString("job_title")
                      << std::setw(21) << result->getString("applied_at")
                      << result->getString("status") << "\n";
        }
        if (!found) std::cout << "No applications have been submitted yet.\n";
        std::cout << "\n";
    }

public:
    PlacementManagementSystem() {
        sql::Driver* driver = sql::mysql::get_driver_instance();
        connection.reset(driver->connect(DB_HOST, DB_USER, DB_PASSWORD));
        connection->setSchema(DB_NAME);
        std::cout << "Connected to MySQL database: " << DB_NAME << "\n";
    }

    void addStudent() {
        std::cout << "\n--- Add Student ---\n";
        const std::string roll = readText("University roll number: ");
        const std::string name = readText("Full name: ");
        const std::string email = readText("Email: ");
        const std::string phone = readText("Phone number: ");
        const double cgpa = readDoubleInRange("Current CGPA (0 to 10): ", 0.0, 10.0);
        const std::string skills = readText("Skills (comma-separated, e.g. C++, SQL, Python): ");

        Student student(roll, name, email, phone, cgpa, skills);
        const Person* person = &student; // Runtime polymorphism through the base class.
        std::unique_ptr<sql::PreparedStatement> statement(connection->prepareStatement(
            "INSERT INTO students (roll_number, full_name, email, phone, cgpa, skills) "
            "VALUES (?, ?, ?, ?, ?, ?)"));
        statement->setString(1, student.getRollNumber());
        statement->setString(2, student.getFullName());
        statement->setString(3, student.getEmail());
        statement->setString(4, student.getPhone());
        statement->setDouble(5, student.getCgpa());
        statement->setString(6, student.getSkills());
        statement->executeUpdate();
        std::cout << person->getRole() << " added and saved to MySQL successfully.\n";
    }

    void viewStudents() { showStudents(); }

    void addCompany() {
        std::cout << "\n--- Add Company ---\n";
        const std::string name = readText("Company name: ");
        const std::string industry = readText("Industry: ");
        const std::string website = readText("Website (or N/A): ");
        const std::string email = readText("Recruitment/contact email: ");
        Company company(name, industry, website, email);

        std::unique_ptr<sql::PreparedStatement> statement(connection->prepareStatement(
            "INSERT INTO companies (name, industry, website, contact_email) VALUES (?, ?, ?, ?)"));
        statement->setString(1, company.getName());
        statement->setString(2, company.getIndustry());
        statement->setString(3, company.getWebsite());
        statement->setString(4, company.getContactEmail());
        statement->executeUpdate();
        std::cout << "Company added and saved to MySQL successfully.\n";
    }

    void viewCompanies() { showCompanies(); }

    void postJob() {
        std::cout << "\n--- Post Job / Internship ---\n";
        showCompanies();
        const int companyId = readInt("Company ID (add the company first): ", 1, 1000000000);
        const std::string title = readText("Job / internship title: ");
        const std::string description = readText("Short role description: ");
        const double minCgpa = readDoubleInRange("Minimum required CGPA (0 to 10): ", 0.0, 10.0);
        const int openings = readInt("Number of openings: ", 1, 100000);
        std::cout << "Job type: 1. Internship  2. Full-time  3. Part-time\n";
        const int typeChoice = readInt("Choose type: ", 1, 3);
        const std::string type = typeChoice == 1 ? "Internship" : (typeChoice == 2 ? "Full-time" : "Part-time");
        const std::string location = readText("Location (or Remote): ");
        const std::string lastDate = readDate("Last application date (YYYY-MM-DD): ");
        JobOpening job(companyId, title, description, minCgpa, openings, type, location, lastDate);

        std::unique_ptr<sql::PreparedStatement> statement(connection->prepareStatement(
            "INSERT INTO job_openings (company_id, title, description, min_cgpa, openings, "
            "job_type, location, last_date, is_active) VALUES (?, ?, ?, ?, ?, ?, ?, ?, 1)"));
        statement->setInt(1, job.getCompanyId());
        statement->setString(2, job.getTitle());
        statement->setString(3, job.getDescription());
        statement->setDouble(4, job.getMinimumCgpa());
        statement->setInt(5, job.getOpenings());
        statement->setString(6, job.getJobType());
        statement->setString(7, job.getLocation());
        statement->setString(8, job.getLastDate());
        statement->executeUpdate();
        std::cout << "Job / internship posted and saved to MySQL successfully.\n";
    }

    void viewJobs() { showJobs(false); }

    void applyForJob() {
        std::cout << "\n--- Apply for Job / Internship ---\n";
        showJobs(true);
        const int studentId = readInt("Student ID: ", 1, 1000000000);
        const int jobId = readInt("Job ID: ", 1, 1000000000);

        std::unique_ptr<sql::PreparedStatement> check(connection->prepareStatement(
            "SELECT s.cgpa AS student_cgpa, j.min_cgpa, j.is_active, j.last_date, "
            "DATE_FORMAT(CURDATE(), '%Y-%m-%d') AS today "
            "FROM students s JOIN job_openings j ON j.id = ? WHERE s.id = ?"));
        check->setInt(1, jobId);
        check->setInt(2, studentId);
        std::unique_ptr<sql::ResultSet> result(check->executeQuery());
        if (!result->next()) {
            std::cout << "Student ID or Job ID was not found. Check the IDs and try again.\n";
            return;
        }
        if (result->getInt("is_active") != 1) {
            std::cout << "This job opening is inactive and cannot accept applications.\n";
            return;
        }
        if (result->getString("last_date") < result->getString("today")) {
            std::cout << "The application deadline has passed.\n";
            return;
        }
        if (result->getDouble("student_cgpa") < result->getDouble("min_cgpa")) {
            std::cout << "Not eligible: the student's CGPA is below the job's minimum requirement.\n";
            return;
        }

        std::unique_ptr<sql::PreparedStatement> statement(connection->prepareStatement(
            "INSERT INTO applications (student_id, job_id, status) VALUES (?, ?, 'Applied')"));
        statement->setInt(1, studentId);
        statement->setInt(2, jobId);
        statement->executeUpdate();
        std::cout << "Application submitted successfully and saved to MySQL.\n";
    }

    void updateApplicationStatus() {
        showApplications();
        const int applicationId = readInt("Application ID to update: ", 1, 1000000000);
        std::cout << "New status: 1. Applied  2. Under Review  3. Shortlisted  4. Rejected  5. Selected\n";
        const int choice = readInt("Choose status: ", 1, 5);
        const std::string statuses[] = {"Applied", "Under Review", "Shortlisted", "Rejected", "Selected"};

        std::unique_ptr<sql::PreparedStatement> check(connection->prepareStatement(
            "SELECT id FROM applications WHERE id = ?"));
        check->setInt(1, applicationId);
        std::unique_ptr<sql::ResultSet> result(check->executeQuery());
        if (!result->next()) {
            std::cout << "Application ID not found.\n";
            return;
        }

        std::unique_ptr<sql::PreparedStatement> statement(connection->prepareStatement(
            "UPDATE applications SET status = ? WHERE id = ?"));
        statement->setString(1, statuses[choice - 1]);
        statement->setInt(2, applicationId);
        statement->executeUpdate();
        std::cout << "Application status updated to: " << statuses[choice - 1] << "\n";
    }

    void scheduleInterview() {
        std::cout << "\n--- Schedule Interview ---\n";
        showApplications();
        const int applicationId = readInt("Application ID: ", 1, 1000000000);
        std::unique_ptr<sql::PreparedStatement> check(connection->prepareStatement(
            "SELECT id FROM applications WHERE id = ?"));
        check->setInt(1, applicationId);
        std::unique_ptr<sql::ResultSet> result(check->executeQuery());
        if (!result->next()) {
            std::cout << "Application ID not found. Submit an application first.\n";
            return;
        }

        const std::string dateTime = readDateTime("Interview date/time (YYYY-MM-DD HH:MM): ");
        const std::string round = readText("Interview round (e.g. HR, Technical, Manager): ");
        std::cout << "Mode: 1. Online  2. In-person  3. Phone\n";
        const int modeChoice = readInt("Choose mode: ", 1, 3);
        const std::string mode = modeChoice == 1 ? "Online" : (modeChoice == 2 ? "In-person" : "Phone");
        const std::string venue = readText("Meeting link / venue / phone details: ");
        const std::string notes = readText("Notes: ");

        std::unique_ptr<sql::PreparedStatement> statement(connection->prepareStatement(
            "INSERT INTO interviews (application_id, interview_datetime, round_name, mode, "
            "venue_or_link, notes, status) VALUES (?, ?, ?, ?, ?, ?, 'Scheduled')"));
        statement->setInt(1, applicationId);
        statement->setString(2, dateTime);
        statement->setString(3, round);
        statement->setString(4, mode);
        statement->setString(5, venue);
        statement->setString(6, notes);
        statement->executeUpdate();
        std::cout << "Interview scheduled and saved to MySQL.\n";
    }

    void viewInterviews() {
        std::unique_ptr<sql::Statement> statement(connection->createStatement());
        std::unique_ptr<sql::ResultSet> result(statement->executeQuery(
            "SELECT i.id AS interview_id, i.interview_datetime, i.round_name, i.mode, "
            "i.venue_or_link, i.status, s.full_name, c.name AS company_name, j.title AS job_title "
            "FROM interviews i JOIN applications a ON a.id = i.application_id "
            "JOIN students s ON s.id = a.student_id JOIN job_openings j ON j.id = a.job_id "
            "JOIN companies c ON c.id = j.company_id ORDER BY i.interview_datetime"));

        std::cout << "\nINTERVIEWS\n"
                  << std::left << std::setw(8) << "ID" << std::setw(22) << "Date / time"
                  << std::setw(15) << "Round" << std::setw(12) << "Mode"
                  << std::setw(23) << "Student" << std::setw(23) << "Company"
                  << std::setw(22) << "Role" << "Status\n"
                  << std::string(125, '-') << "\n";
        bool found = false;
        while (result->next()) {
            found = true;
            std::cout << std::left << std::setw(8) << result->getInt("interview_id")
                      << std::setw(22) << result->getString("interview_datetime")
                      << std::setw(15) << result->getString("round_name")
                      << std::setw(12) << result->getString("mode")
                      << std::setw(23) << result->getString("full_name")
                      << std::setw(23) << result->getString("company_name")
                      << std::setw(22) << result->getString("job_title")
                      << result->getString("status") << "\n";
        }
        if (!found) std::cout << "No interviews have been scheduled yet.\n";
        std::cout << "\n";
    }

    void showDashboard() {
        std::unique_ptr<sql::Statement> statement(connection->createStatement());
        std::unique_ptr<sql::ResultSet> result(statement->executeQuery(
            "SELECT (SELECT COUNT(*) FROM students) AS total_students, "
            "(SELECT COUNT(*) FROM companies) AS total_companies, "
            "(SELECT COUNT(*) FROM job_openings) AS total_jobs, "
            "(SELECT COUNT(*) FROM applications) AS total_applications, "
            "(SELECT COUNT(*) FROM applications WHERE status = 'Selected') AS selected_students, "
            "(SELECT COUNT(*) FROM interviews) AS total_interviews"));
        if (result->next()) {
            std::cout << "\n========== PLACEMENT DASHBOARD ==========\n"
                      << "Total students       : " << result->getInt("total_students") << "\n"
                      << "Registered companies : " << result->getInt("total_companies") << "\n"
                      << "Job / internship posts: " << result->getInt("total_jobs") << "\n"
                      << "Applications         : " << result->getInt("total_applications") << "\n"
                      << "Selected applications: " << result->getInt("selected_students") << "\n"
                      << "Interviews scheduled : " << result->getInt("total_interviews") << "\n";
            const int applications = result->getInt("total_applications");
            const int selected = result->getInt("selected_students");
            const double selectionRate = applications > 0 ? 100.0 * selected / applications : 0.0;
            std::cout << "Selection rate       : " << std::fixed << std::setprecision(1)
                      << selectionRate << "%\n";
        }

        std::unique_ptr<sql::Statement> topStatement(connection->createStatement());
        std::unique_ptr<sql::ResultSet> top(topStatement->executeQuery(
            "SELECT c.name AS company_name, COUNT(*) AS selected_count "
            "FROM applications a JOIN job_openings j ON j.id = a.job_id "
            "JOIN companies c ON c.id = j.company_id WHERE a.status = 'Selected' "
            "GROUP BY c.id, c.name ORDER BY selected_count DESC, c.name LIMIT 5"));
        std::cout << "\nCOMPANIES WITH SELECTED CANDIDATES\n";
        bool found = false;
        while (top->next()) {
            found = true;
            std::cout << "- " << top->getString("company_name") << ": "
                      << top->getInt("selected_count") << " selected application(s)\n";
        }
        if (!found) std::cout << "No selected applications recorded yet.\n";
        std::cout << "\n";
    }

    void run() {
        while (true) {
            printMenu();
            int choice = 0;
            try {
                choice = readInt("Choose an option: ", 0, 12);
            } catch (const std::exception&) {
                if (!std::cin) break;
                throw;
            }
            try {
                switch (choice) {
                    case 1: addStudent(); break;
                    case 2: viewStudents(); break;
                    case 3: addCompany(); break;
                    case 4: viewCompanies(); break;
                    case 5: postJob(); break;
                    case 6: viewJobs(); break;
                    case 7: applyForJob(); break;
                    case 8: showApplications(); break;
                    case 9: updateApplicationStatus(); break;
                    case 10: scheduleInterview(); break;
                    case 11: viewInterviews(); break;
                    case 12: showDashboard(); break;
                    case 0:
                        std::cout << "Thank you for using the Placement Management System.\n";
                        return;
                    default: break;
                }
            } catch (const sql::SQLException& error) {
                std::cout << "\nDatabase operation failed: " << error.what() << "\n";
                std::cout << "Check duplicate values, IDs, database setup and connection settings.\n";
            } catch (const std::exception& error) {
                if (!std::cin) return;
                std::cout << "\nOperation failed: " << error.what() << "\n";
            }
        }
    }
};

int main() {
    try {
        PlacementManagementSystem system;
        system.run();
    } catch (const sql::SQLException& error) {
        std::cerr << "Could not connect to MySQL: " << error.what() << "\n";
        std::cerr << "Check that MySQL Server is running, the database has been created, "
                  << "and DB_USER / DB_PASSWORD are correct in main.cpp.\n";
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << "\n";
        return 1;
    }
    return 0;
}
