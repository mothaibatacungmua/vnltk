CREATE USER 'vnltk'@'localhost' IDENTIFIED BY '123qwe';
CREATE DATABASE `vncorpus` CHARACTER SET utf8 COLLATE utf8_general_ci;
GRANT ALL ON `vncorpus`.* TO 'vnltk'@'localhost' IDENTIFIED BY '123qwe';