DROP TABLE IF EXISTS `Conf_Pres`;
DROP TABLE IF EXISTS `User`;
DROP TABLE IF EXISTS `Presentation`;
DROP TABLE IF EXISTS `Conference`;

CREATE TABLE IF NOT EXISTS `User` (`id` INT NOT NULL AUTO_INCREMENT, 
`login` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL, 
`password` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
`first_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
`last_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
`email` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,
`title` VARCHAR(1024) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,
PRIMARY KEY (`id`), KEY `fn` (`first_name`), KEY `ln` (`last_name`));

CREATE TABLE IF NOT EXISTS `Presentation` (`id` INT NOT NULL AUTO_INCREMENT,
`title` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
`theme` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
`annotation` VARCHAR(5012) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,
`author` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
`date` DATE NOT NULL,
PRIMARY KEY (`id`), KEY (`title`));

CREATE TABLE IF NOT EXISTS `Conference` (`id` INT NOT NULL AUTO_INCREMENT,
`title` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
`starttime` DATETIME NOT NULL,
`place` VARCHAR(512) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
PRIMARY KEY (`id`), KEY (`title`));

CREATE TABLE IF NOT EXISTS `Conf_Pres` (`id` INT NOT NULL AUTO_INCREMENT,
`conf_id` INT NOT NULL,
`pres_id` INT NOT NULL,
`conf_title` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
`pres_title` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
PRIMARY KEY (`id`),
FOREIGN KEY (`conf_id`) REFERENCES `Conference`(`id`) ON UPDATE CASCADE ON DELETE CASCADE,
FOREIGN KEY (`pres_id`) REFERENCES `Presentation`(`id`) ON UPDATE CASCADE ON DELETE CASCADE);

INSERT INTO `User`(`login`, `password`, `first_name`, `last_name`, `email`, `title`) VALUES
('Jackie', '123456', 'Jack', 'Shepard', 'shepard@gmail.com', 'Doc'),
('John', '4815162342', 'John', 'Lock', 'lock@gmail.com', 'Hunter'),
('JustKate', '678173', 'Kate', 'Austin', 'austinkate@gmail.com', 'Ms'),
('Sawyer', 'aptegjgb', 'James', 'Ford', 'jamesford@gmail.com', 'Mr'),
('Said', 'easypass', 'Said', 'Jarah', 'jarah@mail.ru', 'Officer');

INSERT INTO `Presentation`(`title`, `theme`, `annotation`, `author`, `date`) VALUES
('Regularization', 'Machine Learning', 'Benefits of using regularization in ML', 'Andrew Lang', '2022-05-10'),
('Computer Vision frontiers', 'Computer Vision', 'What does future hold for Computer Vision?', 'Jack Philips', '2022-07-15'),
('Genome studying problems', 'Bioinformatics', 'How we can overcome difficulties while exploring genomes', 'Paul Ford', '2022-03-20');

INSERT INTO `Conference`(`title`, `starttime`, `place`) VALUES
('CV2022', '2022-10-10 08:00:00', 'Los Angeles, CA'),
('ML2022', '2022-11-15 10:00:00', 'Moscow, Russia'),
('BIOINF2022', '2022-12-20 09:00:00', 'Saint-Petersburg, Russia');

INSERT INTO `Conf_Pres`(`conf_id`, `pres_id`, `conf_title`, `pres_title`) VALUES
('1', '2', 'CV2022', 'Computer Vision frontiers'),
('2', '1', 'ML2022', 'Regularization'),
('3', '3', 'BIOINF2022', 'Genome studying problems');
