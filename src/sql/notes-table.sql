-- Create an FTS table
CREATE VIRTUAL TABLE Notes USING fts5(title, body);