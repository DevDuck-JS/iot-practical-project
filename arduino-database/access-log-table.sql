CREATE TABLE access_log (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_type VARCHAR(50),
    password VARCHAR(100),
    access_time DATETIME
);