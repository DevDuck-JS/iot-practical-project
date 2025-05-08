CREATE TABLE user (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_type VARCHAR(50),
    password VARCHAR(100),
    update_at DATETIME
);

INSERT INTO user (user_type, password, update_at)
VALUES
  ('Admin', '123A45', NOW()),
  ('Residence', '789B56', NOW());