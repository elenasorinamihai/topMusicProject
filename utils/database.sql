DROP DATABASE IF EXISTS `topMusic`;
CREATE DATABASE `topMusic` CHARACTER SET utf8 COLLATE utf8_unicode_ci;
use `topMusic`;

CREATE TABLE `user` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `userName` varchar(255) DEFAULT NULL,
  `password` varchar(255) DEFAULT NULL,
  `type` int(4)  NOT NULL,
  `isBlocked` tinyint(1) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

CREATE TABLE `genre` (
	`id` bigint(20) NOT NULL AUTO_INCREMENT,
    `name` varchar(255) NOT NULL,
    PRIMARY KEY(`id`)    
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

CREATE TABLE `song`(
	`id` bigint(20) NOT NULL AUTO_INCREMENT,
    `name` varchar(255) NOT NULL,
    `description` varchar(255) NOT NULL DEFAULT '',
    `externalLink` varchar(255) NOT NULL DEFAULT '',
    PRIMARY KEY(`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

CREATE TABLE `songGenre`(
	`id` bigint(20) NOT NULL AUTO_INCREMENT,
    `song_Fk` bigint(20) NOT NULL,
    `genre_Fk` bigint(20) NOT NULL,
    PRIMARY KEY (`id`),
    KEY `song_Fk` (`song_Fk`),
    KEY `genre_Fk` (`genre_Fk`),
    CONSTRAINT `songGenre_ibfk_1` FOREIGN KEY (`song_Fk`) REFERENCES `song` (`id`),
	CONSTRAINT `songGenre_ibfk_2` FOREIGN KEY (`genre_Fk`) REFERENCES `genre` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

CREATE TABLE `comment` (
	`id` bigint(20) NOT NULL AUTO_INCREMENT,
    `song_Fk` bigint(20) NOT NULL,
    `user_Fk` bigint(20) NOT NULL,
    `content` varchar(255) NOT NULL,
    PRIMARY KEY (`id`),
    KEY `song_Fk` (`song_Fk`),
    KEY `user_Fk` (`user_Fk`),
    CONSTRAINT `comment_ibfk_1` FOREIGN KEY (`song_Fk`) REFERENCES `song` (`id`),
    CONSTRAINT `comment_ibfk_2` FOREIGN KEY (`user_Fk`) REFERENCES `user` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

CREATE TABLE `vote` (
	`id` bigint(20) NOT NULL AUTO_INCREMENT,
    `song_Fk` bigint(20) NOT NULL,
    `user_Fk` bigint(20) NOT NULL,
    `upVotes` int(20) DEFAULT 0,
	`downVotes` int (20) DEFAULT 0,
    PRIMARY KEY (`id`),
    KEY `song_Fk` (`song_Fk`),
    KEY `user_Fk` (`user_Fk`),
    CONSTRAINT `vote_ibfk_1` FOREIGN KEY (`song_Fk`) REFERENCES `song` (`id`),
    CONSTRAINT `vote_ibfk_2` FOREIGN KEY (`user_Fk`) REFERENCES `user` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;