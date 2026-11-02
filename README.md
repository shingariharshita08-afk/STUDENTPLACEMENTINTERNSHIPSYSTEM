# Student Placement & Internship Management System

A console-based BCA OOP mini-project built with C++ and MySQL. The C++ program stores real records in a MySQL database and can be used to manage students, companies, job openings, applications, and interviews.

## Features

1. Add and view student profiles, including roll number, contact information, CGPA, and skills.
2. Add and view recruiting companies.
3. Post internship, full-time, and part-time openings.
4. View current job openings and deadlines.
5. Apply for a job using student and job IDs.
6. Check application eligibility based on minimum CGPA and deadline.
7. Prevent the same student from applying to the same job more than once using a database unique constraint.
8. View applications and update application status.
9. Schedule and view interviews.
10. Display a placement dashboard with counts and a selection rate.
11. Uses MySQL prepared statements for user-supplied values.
12. Demonstrates encapsulation, inheritance, and runtime polymorphism in the C++ code.

## Database design

The SQL script creates the database `placement_management_db` and five related tables:

- `students`: student profiles and CGPA.
- `companies`: companies that recruit students.
- `job_openings`: jobs and internships posted by companies.
- `applications`: connects a student to a job and tracks the application status.
- `interviews`: interview date, round, mode, venue/link, and status.

Foreign keys maintain relationships between these records. The application table has a unique constraint on `(student_id, job_id)` to prevent duplicate applications.

## Requirements on Windows

- MySQL Server (MySQL Workbench is recommended for running and inspecting SQL).
- MySQL Connector/C++ with its classic JDBC-compatible API (Developer and runtime components).
- Visual Studio 2022 with **Desktop development with C++** and CMake tools.
- CMake 3.20 or later.

Official downloads:

- MySQL Community downloads: https://dev.mysql.com/downloads/
- MySQL Connector/C++: https://dev.mysql.com/downloads/connector/cpp/
- CMake: https://cmake.org/download/
- Visual Studio: https://visualstudio.microsoft.com/downloads/

The official Windows connector binaries target MSVC. If you use VS Code, select/configure the Visual Studio C++ compiler toolchain rather than assuming a MinGW `g++` build will link to the same library.

## Setup and run

### 1. Create the database

1. Start the MySQL Server service.
2. Open MySQL Workbench and connect to your local server.
3. Open `placement_management_db.sql` and execute the entire script.
4. Refresh the SCHEMAS panel and confirm that `placement_management_db` contains the five tables listed above.

### 2. Configure the database credentials

Open `main.cpp` and find:

```cpp
const std::string DB_HOST = "tcp://127.0.0.1:3306";
const std::string DB_USER = "root";
const std::string DB_PASSWORD = "CHANGE_THIS_PASSWORD";
const std::string DB_NAME = "placement_management_db";
```

Change the placeholder password to your local MySQL account password. **Never commit your real password to a public GitHub repository.** Keep the placeholder in the GitHub copy or use a local-only configuration file that is excluded from version control.

### 3. Build with CMake

Open **Developer PowerShell for Visual Studio 2022** or a terminal configured for MSVC. Change directory to this project folder.

If necessary, find the directory containing `mysql-concpp-config.cmake` in your Connector/C++ installation, then run (replace the path with your actual path):

```powershell
cmake -S . -B build -Dmysql-concpp_DIR="C:/path/to/mysql-connector-c++"
cmake --build build --config Release
```

If CMake can already find the connector package, omit the `-Dmysql-concpp_DIR=...` argument.

### 4. Run the program

```powershell
.\build\Release\StudentPlacementSystem.exe
```

If Windows reports that a connector DLL cannot be found, add the Connector/C++ runtime library directory to `PATH` in that terminal, or place the required runtime DLLs beside the executable as described in the connector's installation documentation.

## Suggested first-time test

1. Use option **1** to add a student. Remember the generated student ID (shown in the table under option **2**).
2. Use option **3** to add a company.
3. Use option **5** to post a job. Remember its generated job ID (shown under option **6**).
4. Use option **7** to apply using the student ID and job ID. The student's CGPA must meet the role's minimum CGPA, and the application deadline must not have passed.
5. Use option **8** to view the application, then option **9** to change its status.
6. Use option **10** to schedule an interview for the application.
7. Use option **12** to see the placement dashboard.

## Verify data in MySQL Workbench

After entering sample records, run:

```sql
USE placement_management_db;
SELECT * FROM students;
SELECT * FROM companies;
SELECT * FROM job_openings;
SELECT * FROM applications;
SELECT * FROM interviews;
```

Closing the C++ program does not delete the rows stored in MySQL.

## GitHub upload checklist

Upload these source files to the repository root:

- `main.cpp`
- `placement_management_db.sql`
- `CMakeLists.txt`
- `README.md`
- `.gitignore`

Do not upload build folders, compiled executables, connector DLLs, database passwords, or private student/customer records.

## Troubleshooting

- **Access denied for user**: check the username and password in your local copy of `main.cpp`.
- **Unknown database**: execute `placement_management_db.sql` in MySQL Workbench.
- **Can't connect / connection refused**: start the MySQL Server service and check port `3306`.
- **`mysql/jdbc.h` not found**: install the Connector/C++ Developer component and configure CMake to locate its package.
- **Connector link errors with MinGW**: configure the project to use the Visual Studio/MSVC toolchain or adapt the project to a connector compatible with your compiler.
