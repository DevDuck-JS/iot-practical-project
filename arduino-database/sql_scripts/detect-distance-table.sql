CREATE TABLE trigger_threshold (
    id INT AUTO_INCREMENT PRIMARY KEY,
    distance INT,
    update_at DATETIME
);

INSERT INTO trigger_threshold (distance, update_at)
VALUES
  (20,  NOW());
