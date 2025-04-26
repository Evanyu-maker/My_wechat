-- 创建数据库
CREATE DATABASE IF NOT EXISTS my_telegram DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 使用数据库
USE my_telegram;

-- 用户表
CREATE TABLE IF NOT EXISTS users (
    user_id INT NOT NULL AUTO_INCREMENT,
    username VARCHAR(50) NOT NULL,
    password VARCHAR(255) NOT NULL,
    nickname VARCHAR(50) NOT NULL,
    avatar VARCHAR(255) NOT NULL DEFAULT '',
    status ENUM('online', 'offline', 'away') NOT NULL DEFAULT 'offline',
    create_time DATETIME NOT NULL,
    last_login_time DATETIME NOT NULL,
    PRIMARY KEY (user_id),
    UNIQUE KEY (username)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 好友关系表
CREATE TABLE IF NOT EXISTS friendships (
    id INT NOT NULL AUTO_INCREMENT,
    user_id INT NOT NULL,
    friend_id INT NOT NULL,
    establish_time DATETIME NOT NULL,
    PRIMARY KEY (id),
    UNIQUE KEY (user_id, friend_id),
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (friend_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 群组表
CREATE TABLE IF NOT EXISTS `groups` (
    group_id INT NOT NULL AUTO_INCREMENT,
    name VARCHAR(50) NOT NULL,
    avatar VARCHAR(255) NOT NULL DEFAULT '',
    creator_id INT NOT NULL,
    create_time DATETIME NOT NULL,
    PRIMARY KEY (group_id),
    FOREIGN KEY (creator_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 群组成员表
CREATE TABLE IF NOT EXISTS group_members (
    id INT NOT NULL AUTO_INCREMENT,
    group_id INT NOT NULL,
    user_id INT NOT NULL,
    inviter_id INT,
    role ENUM('member', 'admin', 'owner') NOT NULL DEFAULT 'member',
    join_time DATETIME NOT NULL,
    PRIMARY KEY (id),
    UNIQUE KEY (group_id, user_id),
    FOREIGN KEY (group_id) REFERENCES `groups`(group_id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (inviter_id) REFERENCES users(user_id) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 单聊消息表
CREATE TABLE IF NOT EXISTS private_messages (
    msg_id BIGINT NOT NULL AUTO_INCREMENT,
    sender_id INT NOT NULL,
    receiver_id INT NOT NULL,
    content TEXT NOT NULL,
    msg_type ENUM('text', 'image', 'voice', 'video', 'file') NOT NULL DEFAULT 'text',
    send_time DATETIME NOT NULL,
    is_read TINYINT(1) NOT NULL DEFAULT 0,
    PRIMARY KEY (msg_id),
    FOREIGN KEY (sender_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (receiver_id) REFERENCES users(user_id) ON DELETE CASCADE,
    INDEX (sender_id, receiver_id, send_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 群聊消息表
CREATE TABLE IF NOT EXISTS group_messages (
    msg_id BIGINT NOT NULL AUTO_INCREMENT,
    group_id INT NOT NULL,
    sender_id INT NOT NULL,
    content TEXT NOT NULL,
    msg_type ENUM('text', 'image', 'voice', 'video', 'file') NOT NULL DEFAULT 'text',
    send_time DATETIME NOT NULL,
    PRIMARY KEY (msg_id),
    FOREIGN KEY (group_id) REFERENCES `groups`(group_id) ON DELETE CASCADE,
    FOREIGN KEY (sender_id) REFERENCES users(user_id) ON DELETE CASCADE,
    INDEX (group_id, send_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 消息已读状态表（群消息）
CREATE TABLE IF NOT EXISTS message_read_status (
    id BIGINT NOT NULL AUTO_INCREMENT,
    msg_id BIGINT NOT NULL,
    user_id INT NOT NULL,
    read_time DATETIME NOT NULL,
    PRIMARY KEY (id),
    UNIQUE KEY (msg_id, user_id),
    FOREIGN KEY (msg_id) REFERENCES group_messages(msg_id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 好友请求表
CREATE TABLE IF NOT EXISTS friend_requests (
    request_id INT NOT NULL AUTO_INCREMENT,
    sender_id INT NOT NULL,
    receiver_id INT NOT NULL,
    message VARCHAR(255) NOT NULL DEFAULT '',
    status ENUM('pending', 'accepted', 'rejected') NOT NULL DEFAULT 'pending',
    create_time DATETIME NOT NULL,
    update_time DATETIME NOT NULL,
    PRIMARY KEY (request_id),
    FOREIGN KEY (sender_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (receiver_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 群组邀请表
CREATE TABLE IF NOT EXISTS group_invitations (
    invitation_id INT NOT NULL AUTO_INCREMENT,
    group_id INT NOT NULL,
    inviter_id INT NOT NULL,
    invitee_id INT NOT NULL,
    message VARCHAR(255) NOT NULL DEFAULT '',
    status ENUM('pending', 'accepted', 'rejected') NOT NULL DEFAULT 'pending',
    create_time DATETIME NOT NULL,
    update_time DATETIME NOT NULL,
    PRIMARY KEY (invitation_id),
    FOREIGN KEY (group_id) REFERENCES `groups`(group_id) ON DELETE CASCADE,
    FOREIGN KEY (inviter_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (invitee_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 文件表
CREATE TABLE IF NOT EXISTS files (
    file_id BIGINT NOT NULL AUTO_INCREMENT,
    uploader_id INT NOT NULL,
    file_name VARCHAR(255) NOT NULL,
    file_path VARCHAR(255) NOT NULL,
    file_size BIGINT NOT NULL,
    file_type VARCHAR(50) NOT NULL,
    upload_time DATETIME NOT NULL,
    PRIMARY KEY (file_id),
    FOREIGN KEY (uploader_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 存储过程：创建聊天群组
DELIMITER //
CREATE PROCEDURE sp_create_group(
    IN p_creator_id INT,
    IN p_group_name VARCHAR(50),
    IN p_group_avatar VARCHAR(255),
    OUT p_group_id INT
)
BEGIN
    -- 创建群组
    INSERT INTO `groups` (name, avatar, creator_id, create_time)
    VALUES (p_group_name, p_group_avatar, p_creator_id, NOW());
    
    SET p_group_id = LAST_INSERT_ID();
    
    -- 将创建者添加为群成员
    INSERT INTO group_members (group_id, user_id, role, join_time)
    VALUES (p_group_id, p_creator_id, 'owner', NOW());
END //
DELIMITER ; 