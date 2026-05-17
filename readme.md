# Login Page with MySQL — C++

A console-based authentication system built in C++ with MySQL backend. Supports user login, signup, and password recovery.

---
## Local MySQL db
![alt text](image.png)
---

## Features

- User Login with attempt limiter (3 tries)
- User Signup
- Forgot Password (reset via email)
- Password masking (`*`) while typing
- SHA2 password hashing
- SQL injection protection via prepared statements
- Credentials loaded from `.env` file

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

Since passwords are now hashed with SHA2, update the test user after setup:

```sql
UPDATE users SET password = SHA2('test', 256) WHERE username = 'test';
```

---

## Configuration

Create a `.env` file in the project root (already included, never commit it):

```env
DB_HOST=localhost
DB_USER=root
DB_PASSWORD=your_password
DB_NAME=login
DB_PORT=3306
```

The app reads this file at runtime. No credentials are stored in the source code.

---

## Build & Run

```bat
build.bat
login.exe
```

> The build script uses `g++` and links against MySQL Server at `C:\Program Files\MySQL\MySQL Server 9.7`.  
> Make sure `.env` is in the same directory as `login.exe` when running.

---

## Project Structure

```
loginPage_MySQL/
├── main.cpp        # Application source
├── mysql.sql       # Database schema & seed data
├── build.bat       # Build script
├── .env            # DB credentials (never commit this)
├── .gitignore      # Ignores .env and login.exe
├── libmysql.dll    # MySQL client library
└── login.exe       # Compiled binary
```

---

## Security

- Passwords are hashed using `SHA2(password, 256)` — never stored in plain text
- All queries use prepared statements — no SQL injection
- Credentials are loaded from `.env` — not hardcoded in source
- `.env` is listed in `.gitignore` — safe to use with Git
