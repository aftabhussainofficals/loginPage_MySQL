# Login Page with MySQL — C++

A console-based authentication system built in C++ with MySQL backend. Supports user login, signup, and password recovery.

---

## Features

- User Login
- User Signup
- Forgot Password (view & reset via email)

---

## Prerequisites

- [MySQL Server 9.x](https://dev.mysql.com/downloads/mysql/)
- [MinGW / g++](https://www.mingw-w64.org/) (C++ compiler)
- `libmysql.dll` (included)

---

## Database Setup

Run the provided SQL file in your MySQL client:

```sql
source mysql.sql
```

This creates the `login` database and a `users` table with a test user:
- **Username:** `test` | **Password:** `test` | **Email:** `test@test.com`

---

## Configuration

Update the credentials in `main.cpp` if needed:

```cpp
const char* DB_HOST     = "localhost";
const char* DB_USER     = "root";
const char* DB_PASSWORD = "your_password";
const char* DB_NAME     = "login";
const int   DB_PORT     = 3306;
```

---

## Build & Run

```bat
build.bat
login.exe
```

> The build script uses `g++` and links against MySQL Server at `C:\Program Files\MySQL\MySQL Server 9.7`.

---

## Project Structure

```
loginPage_MySQL/
├── main.cpp        # Application source
├── mysql.sql       # Database schema & seed data
├── build.bat       # Build script
├── libmysql.dll    # MySQL client library
└── login.exe       # Compiled binary
```

---

## ⚠️ Security Notice

This project stores passwords in **plain text** and is vulnerable to **SQL injection**. It is intended for **learning purposes only** — do not use in production.
