CREATE DATABASE IF NOT EXISTS login;
USE login;

CREATE TABLE IF NOT EXISTS users (
    id       INT          AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(30)  NOT NULL UNIQUE,
    password VARCHAR(64)  NOT NULL,          -- SHA-256 produces a 64-char hex string
    email    VARCHAR(100) NOT NULL UNIQUE
);
INSERT INTO users (username, password, email)
VALUES ('test', SHA2('test', 256), 'test@test.com')
ON DUPLICATE KEY UPDATE username = username;  