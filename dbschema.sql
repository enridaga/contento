-- phpMyAdmin SQL Dump
-- version 4.2.10
-- http://www.phpmyadmin.net
--
-- Host: localhost:8889
-- Generation Time: Dec 06, 2014 at 06:29 PM
-- Server version: 5.5.38
-- PHP Version: 5.3.29

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

--
-- Database: `contento`
--

-- --------------------------------------------------------

--
-- Table structure for table `RELATION`
--

CREATE TABLE `RELATION` (
`ID` int(11) NOT NULL,
  `OBJECT` int(11) NOT NULL,
  `ATTRIBUTE` int(11) NOT NULL,
  `HOLDS` tinyint(1) DEFAULT NULL,
  `CONTEXT` int(11) NOT NULL
) ENGINE=InnoDB AUTO_INCREMENT=532 DEFAULT CHARSET=utf8;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `RELATION`
--
ALTER TABLE `RELATION`
 ADD PRIMARY KEY (`ID`), ADD UNIQUE KEY `OBJECT_2` (`OBJECT`,`ATTRIBUTE`,`CONTEXT`), ADD KEY `OBJECT` (`OBJECT`), ADD KEY `ATTRIBUTE` (`ATTRIBUTE`), ADD KEY `CONTEXT` (`CONTEXT`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `RELATION`
--
ALTER TABLE `RELATION`
MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=532;
--
-- Constraints for dumped tables
--

--
-- Constraints for table `RELATION`
--
ALTER TABLE `RELATION`
ADD CONSTRAINT `relation_ibfk_1` FOREIGN KEY (`OBJECT`) REFERENCES `ITEM` (`id`) ON DELETE CASCADE,
ADD CONSTRAINT `relation_ibfk_2` FOREIGN KEY (`ATTRIBUTE`) REFERENCES `ITEM` (`id`) ON DELETE CASCADE,
ADD CONSTRAINT `relation_ibfk_3` FOREIGN KEY (`CONTEXT`) REFERENCES `CONTEXT` (`ID`) ON DELETE CASCADE;
