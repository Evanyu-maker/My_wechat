-- 为您的即时通讯系统创建的MySQL存储过程
-- 统一使用utf8mb4_unicode_ci排序规则，确保字符集兼容性

-- 添加用户存储过程
DROP PROCEDURE IF EXISTS proc_add_user;
DELIMITER $$
CREATE PROCEDURE proc_add_user(
    IN p_username VARCHAR(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    IN p_password VARCHAR(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    IN p_nickname VARCHAR(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    IN p_avatar VARCHAR(200) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    IN p_email VARCHAR(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    IN p_status VARCHAR(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci
)
BEGIN
    INSERT INTO users (username, password, nickname, avatar, email, status, create_time, last_login_time)
    VALUES (p_username, p_password, p_nickname, p_avatar, p_email, p_status, NOW(), NOW());
END$$
DELIMITER ;

-- 通过用户名查找用户存储过程
DROP PROCEDURE IF EXISTS proc_find_user_by_username;
DELIMITER $$
CREATE PROCEDURE proc_find_user_by_username(
    IN p_username VARCHAR(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci
)
BEGIN
    SELECT * FROM users WHERE username = p_username COLLATE utf8mb4_unicode_ci;
END$$
DELIMITER ;

-- 通过用户ID查找用户存储过程
DROP PROCEDURE IF EXISTS proc_find_user_by_id;
DELIMITER $$
CREATE PROCEDURE proc_find_user_by_id(
    IN p_user_id BIGINT
)
BEGIN
    SELECT * FROM users WHERE user_id = p_user_id;
END$$
DELIMITER ;

-- 更新用户状态存储过程
DROP PROCEDURE IF EXISTS proc_update_user_status;
DELIMITER $$
CREATE PROCEDURE proc_update_user_status(
    IN p_user_id BIGINT,
    IN p_status VARCHAR(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci
)
BEGIN
    UPDATE users SET status = p_status WHERE user_id = p_user_id;
END$$
DELIMITER ;

-- 更新最后登录时间存储过程
DROP PROCEDURE IF EXISTS proc_update_last_login_time;
DELIMITER $$
CREATE PROCEDURE proc_update_last_login_time(
    IN p_user_id BIGINT
)
BEGIN
    UPDATE users SET last_login_time = CURRENT_TIMESTAMP WHERE user_id = p_user_id;
END$$
DELIMITER ;

-- 获取好友列表存储过程
DROP PROCEDURE IF EXISTS proc_get_friend_list;
DELIMITER $$
CREATE PROCEDURE proc_get_friend_list(
    IN p_user_id BIGINT
)
BEGIN
    SELECT u.* 
    FROM users u
    INNER JOIN friendships f ON u.user_id = f.friend_id
    WHERE f.user_id = p_user_id;
END$$
DELIMITER ;

-- 验证用户密码存储过程
DROP PROCEDURE IF EXISTS proc_verify_password;
DELIMITER $$
CREATE PROCEDURE proc_verify_password(
    IN p_username VARCHAR(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    IN p_password VARCHAR(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci
)
BEGIN
    SELECT COUNT(*) AS result 
    FROM users 
    WHERE username = p_username COLLATE utf8mb4_unicode_ci 
    AND password = p_password COLLATE utf8mb4_unicode_ci;
END$$
DELIMITER ;

-- 添加好友关系存储过程（使用事务确保双向关系）
DROP PROCEDURE IF EXISTS proc_add_friend;
DELIMITER $$
CREATE PROCEDURE proc_add_friend(
    IN p_user_id BIGINT,
    IN p_friend_id BIGINT
)
BEGIN
    DECLARE existing_count INT DEFAULT 0;
    
    -- 检查是否已经是好友
    SELECT COUNT(*) INTO existing_count
    FROM friendships
    WHERE user_id = p_user_id AND friend_id = p_friend_id;
    
    IF existing_count = 0 THEN
        -- 开始事务
        START TRANSACTION;
        
        -- 添加正向好友关系
        INSERT INTO friendships (user_id, friend_id, establish_time)
        VALUES (p_user_id, p_friend_id, NOW());
        
        -- 添加反向好友关系
        INSERT INTO friendships (user_id, friend_id, establish_time)
        VALUES (p_friend_id, p_user_id, NOW());
        
        -- 提交事务
        COMMIT;
    END IF;
END$$
DELIMITER ;

-- 删除好友关系存储过程（使用事务确保双向关系）
DROP PROCEDURE IF EXISTS proc_remove_friend;
DELIMITER $$
CREATE PROCEDURE proc_remove_friend(
    IN p_user_id BIGINT,
    IN p_friend_id BIGINT
)
BEGIN
    -- 开始事务
    START TRANSACTION;
    
    -- 删除正向好友关系
    DELETE FROM friendships
    WHERE user_id = p_user_id AND friend_id = p_friend_id;
    
    -- 删除反向好友关系
    DELETE FROM friendships
    WHERE user_id = p_friend_id AND friend_id = p_user_id;
    
    -- 提交事务
    COMMIT;
END$$
DELIMITER ;

-- 批量获取用户信息存储过程
DROP PROCEDURE IF EXISTS proc_batch_get_user_info;
DELIMITER $$
CREATE PROCEDURE proc_batch_get_user_info(
    IN p_user_ids VARCHAR(1000) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci
)
BEGIN
    SET @sql = CONCAT('SELECT * FROM users WHERE FIND_IN_SET(user_id, "', p_user_ids, '")');
    PREPARE stmt FROM @sql;
    EXECUTE stmt;
    DEALLOCATE PREPARE stmt;
END$$
DELIMITER ;

-- 以下是可能未来需要的存储过程，保留但不在当前DAO中使用

-- 存储过程：用户注册（与proc_add_user功能类似，但有更完善的检查）
DROP PROCEDURE IF EXISTS sp_register_user;
DELIMITER //
CREATE PROCEDURE sp_register_user(
    IN p_username VARCHAR(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    IN p_password VARCHAR(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    IN p_nickname VARCHAR(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    IN p_avatar VARCHAR(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    IN p_email VARCHAR(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    OUT p_user_id INT
)
BEGIN
    DECLARE v_user_exists INT DEFAULT 0;
    
    -- 检查用户名是否已存在
    SELECT COUNT(*) INTO v_user_exists FROM users WHERE username = p_username COLLATE utf8mb4_unicode_ci OR email = p_email COLLATE utf8mb4_unicode_ci;
    
    IF v_user_exists > 0 THEN
        -- 用户已存在
        SET p_user_id = -1;
    ELSE
        -- 插入新用户
        INSERT INTO users (username, password, nickname, avatar, email, status, create_time, last_login_time)
        VALUES (p_username, p_password, p_nickname, p_avatar, p_email, 'offline', NOW(), NOW());
        
        -- 获取新用户ID
        SET p_user_id = LAST_INSERT_ID();
    END IF;
END //
DELIMITER ;

-- 存储过程：用户登录（与proc_verify_password类似，但有更完善的功能）
DROP PROCEDURE IF EXISTS sp_login_user;
DELIMITER //
CREATE PROCEDURE sp_login_user(
    IN p_username VARCHAR(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    IN p_password VARCHAR(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    OUT p_user_id INT
)
BEGIN
    DECLARE v_user_id INT;
    DECLARE v_password VARCHAR(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
    
    -- 查询用户ID和密码
    SELECT user_id, password INTO v_user_id, v_password 
    FROM users 
    WHERE username = p_username COLLATE utf8mb4_unicode_ci;
    
    IF v_user_id IS NULL THEN
        -- 用户不存在
        SET p_user_id = -1;
    ELSE
        IF v_password = p_password COLLATE utf8mb4_unicode_ci THEN
            -- 密码正确，更新最后登录时间和状态
            UPDATE users 
            SET last_login_time = NOW(), status = 'online' 
            WHERE user_id = v_user_id;
            
            SET p_user_id = v_user_id;
        ELSE
            -- 密码错误
            SET p_user_id = -1;
        END IF;
    END IF;
END //
DELIMITER ;

-- 存储过程：用户登出
DROP PROCEDURE IF EXISTS sp_logout_user;
DELIMITER //
CREATE PROCEDURE sp_logout_user(
    IN p_user_id INT
)
BEGIN
    UPDATE users 
    SET status = 'offline' 
    WHERE user_id = p_user_id;
END //
DELIMITER ;

-- 存储过程：获取用户信息
DROP PROCEDURE IF EXISTS sp_get_user_info;
DELIMITER //
CREATE PROCEDURE sp_get_user_info(
    IN p_user_id INT
)
BEGIN
    SELECT user_id, username, nickname, avatar, status, create_time, last_login_time
    FROM users
    WHERE user_id = p_user_id;
END //
DELIMITER ;

-- 存储过程：更新用户信息
DROP PROCEDURE IF EXISTS sp_update_user_info;
DELIMITER //
CREATE PROCEDURE sp_update_user_info(
    IN p_user_id INT,
    IN p_nickname VARCHAR(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    IN p_avatar VARCHAR(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci
)
BEGIN
    UPDATE users
    SET nickname = p_nickname,
        avatar = p_avatar
    WHERE user_id = p_user_id;
END //
DELIMITER ;

-- 存储过程：修改密码
DROP PROCEDURE IF EXISTS sp_change_password;
DELIMITER //
CREATE PROCEDURE sp_change_password(
    IN p_user_id INT,
    IN p_old_password VARCHAR(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    IN p_new_password VARCHAR(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    OUT p_result INT
)
BEGIN
    DECLARE v_password VARCHAR(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
    
    -- 获取当前密码
    SELECT password INTO v_password
    FROM users
    WHERE user_id = p_user_id;
    
    IF v_password = p_old_password COLLATE utf8mb4_unicode_ci THEN
        -- 更新密码
        UPDATE users
        SET password = p_new_password
        WHERE user_id = p_user_id;
        
        SET p_result = 0;
    ELSE
        -- 旧密码不匹配
        SET p_result = -1;
    END IF;
END //
DELIMITER ;

-- 存储过程：添加好友（增强版）
DROP PROCEDURE IF EXISTS sp_add_friend;
DELIMITER //
CREATE PROCEDURE sp_add_friend(
    IN p_user_id INT,
    IN p_friend_id INT,
    OUT p_result INT
)
BEGIN
    DECLARE v_exists INT DEFAULT 0;
    DECLARE v_friend_exists INT DEFAULT 0;
    
    -- 检查好友是否存在
    SELECT COUNT(*) INTO v_friend_exists
    FROM users
    WHERE user_id = p_friend_id;
    
    IF v_friend_exists = 0 THEN
        -- 好友不存在
        SET p_result = -2;
    ELSE
        -- 检查是否已经是好友
        SELECT COUNT(*) INTO v_exists
        FROM friendships
        WHERE (user_id = p_user_id AND friend_id = p_friend_id)
           OR (user_id = p_friend_id AND friend_id = p_user_id);
        
        IF v_exists > 0 THEN
            -- 已经是好友
            SET p_result = -1;
        ELSE
            -- 添加好友关系
            INSERT INTO friendships (user_id, friend_id, establish_time)
            VALUES (p_user_id, p_friend_id, NOW());
            
            -- 也添加反向的关系
            INSERT INTO friendships (user_id, friend_id, establish_time)
            VALUES (p_friend_id, p_user_id, NOW());
            
            SET p_result = 0;
        END IF;
    END IF;
END //
DELIMITER ;

-- 存储过程：删除好友（与proc_remove_friend类似）
DROP PROCEDURE IF EXISTS sp_remove_friend;
DELIMITER //
CREATE PROCEDURE sp_remove_friend(
    IN p_user_id INT,
    IN p_friend_id INT
)
BEGIN
    -- 删除好友关系（双向）
    DELETE FROM friendships
    WHERE (user_id = p_user_id AND friend_id = p_friend_id)
       OR (user_id = p_friend_id AND friend_id = p_user_id);
END //
DELIMITER ;

-- 存储过程：获取好友列表（与proc_get_friend_list类似，但返回更多信息）
DROP PROCEDURE IF EXISTS sp_get_friends;
DELIMITER //
CREATE PROCEDURE sp_get_friends(
    IN p_user_id INT
)
BEGIN
    SELECT u.user_id, u.username, u.nickname, u.avatar, u.status, 
           u.last_login_time, f.establish_time
    FROM users u
    JOIN friendships f ON u.user_id = f.friend_id
    WHERE f.user_id = p_user_id;
END //
DELIMITER ;

-- 存储过程：创建聊天群组
DROP PROCEDURE IF EXISTS sp_create_group;
DELIMITER //
CREATE PROCEDURE sp_create_group(
    IN p_creator_id INT,
    IN p_group_name VARCHAR(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    IN p_group_avatar VARCHAR(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
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

-- 存储过程：添加群成员
DROP PROCEDURE IF EXISTS sp_add_group_member;
DELIMITER //
CREATE PROCEDURE sp_add_group_member(
    IN p_group_id INT,
    IN p_user_id INT,
    IN p_inviter_id INT,
    IN p_role VARCHAR(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    OUT p_result INT
)
BEGIN
    DECLARE v_exists INT DEFAULT 0;
    DECLARE v_group_exists INT DEFAULT 0;
    
    -- 检查群组是否存在
    SELECT COUNT(*) INTO v_group_exists
    FROM `groups`
    WHERE group_id = p_group_id;
    
    IF v_group_exists = 0 THEN
        -- 群组不存在
        SET p_result = -2;
    ELSE
        -- 检查用户是否已经是群成员
        SELECT COUNT(*) INTO v_exists
        FROM group_members
        WHERE group_id = p_group_id AND user_id = p_user_id;
        
        IF v_exists > 0 THEN
            -- 已经是群成员
            SET p_result = -1;
        ELSE
            -- 添加到群组
            INSERT INTO group_members (group_id, user_id, inviter_id, role, join_time)
            VALUES (p_group_id, p_user_id, p_inviter_id, p_role, NOW());
            
            SET p_result = 0;
        END IF;
    END IF;
END //
DELIMITER ;

-- 存储过程：移除群成员
DROP PROCEDURE IF EXISTS sp_remove_group_member;
DELIMITER //
CREATE PROCEDURE sp_remove_group_member(
    IN p_group_id INT,
    IN p_user_id INT
)
BEGIN
    DELETE FROM group_members
    WHERE group_id = p_group_id AND user_id = p_user_id;
END //
DELIMITER ;

-- 存储过程：获取群组成员
DROP PROCEDURE IF EXISTS sp_get_group_members;
DELIMITER //
CREATE PROCEDURE sp_get_group_members(
    IN p_group_id INT
)
BEGIN
    SELECT u.user_id, u.username, u.nickname, u.avatar, u.status, 
           gm.role, gm.join_time
    FROM users u
    JOIN group_members gm ON u.user_id = gm.user_id
    WHERE gm.group_id = p_group_id;
END //
DELIMITER ;

-- 存储过程：获取用户加入的群组
DROP PROCEDURE IF EXISTS sp_get_user_groups;
DELIMITER //
CREATE PROCEDURE sp_get_user_groups(
    IN p_user_id INT
)
BEGIN
    SELECT g.group_id, g.name, g.avatar, g.creator_id, g.create_time,
           gm.role, gm.join_time
    FROM `groups` g
    JOIN group_members gm ON g.group_id = gm.group_id
    WHERE gm.user_id = p_user_id;
END //
DELIMITER ; 