const Redis = require('ioredis');
const config_module = require('./config.js');
const const_module = require('./const.js');

// 创建Redis客户端
let redisClient = null;
let redisAvailable = false;

/**
 * 初始化Redis连接
 */
async function initRedis() {
    try {
        // 如果已经初始化且可用，则直接返回
        if (redisClient && redisAvailable) {
            return;
        }

        // 创建Redis客户端
        redisClient = new Redis({
            host: config_module.redis_host || '127.0.0.1',
            port: config_module.redis_port || 6380,
            password: config_module.redis_password || '',
            retryStrategy(times) {
                if (times > 3) { // 最多重试3次
                    return null;
                }
                return Math.min(times * 100, 3000);
            }
        });

        // 监听连接事件
        redisClient.on('connect', () => {
            console.log('Redis连接成功');
            redisAvailable = true;
        });

        // 监听错误事件
        redisClient.on('error', (err) => {
            console.error('Redis连接错误:', err);
            redisAvailable = false;
        });

    } catch (error) {
        console.error('初始化Redis失败:', error);
        redisAvailable = false;
    }
}

/**
 * 存储验证码
 * @param {string} email 邮箱
 * @param {string} code 验证码
 * @param {number} expireTime 过期时间（秒）
 */
async function saveVarifyCode(email, code, expireTime = 180) {
    try {
        await initRedis();
        if (!redisAvailable) {
            console.warn('Redis不可用，验证码无法存储');
            return true; // 继续流程，不中断服务
        }
        
        const key = `${const_module.code_prefix}${email}`;
        await redisClient.set(key, code, 'EX', expireTime);
        console.log(`验证码已存储，邮箱: ${email}, 验证码: ${code}, 过期时间: ${expireTime}秒`);
        return true;
    } catch (error) {
        console.error('存储验证码失败:', error);
        return false;
    }
}

/**
 * 获取验证码
 * @param {string} email 邮箱
 * @returns {Promise<string|null>} 验证码
 */
async function getVarifyCode(email) {
    try {
        await initRedis();
        if (!redisAvailable) {
            console.warn('Redis不可用，无法获取验证码');
            return null;
        }
        
        const key = `${const_module.code_prefix}${email}`;
        return await redisClient.get(key);
    } catch (error) {
        console.error('获取验证码失败:', error);
        return null;
    }
}

/**
 * 验证验证码
 * @param {string} email 邮箱
 * @param {string} code 验证码
 * @returns {Promise<boolean>} 验证结果
 */
async function validateVarifyCode(email, code) {
    try {
        await initRedis();
        if (!redisAvailable) {
            console.warn('Redis不可用，无法验证验证码');
            return false;
        }
        
        const key = `${const_module.code_prefix}${email}`;
        const storedCode = await redisClient.get(key);
        
        // 如果验证码不存在或不匹配
        if (!storedCode || storedCode !== code) {
            return false;
        }
        
        // 验证成功后删除验证码
        await redisClient.del(key);
        return true;
    } catch (error) {
        console.error('验证验证码失败:', error);
        return false;
    }
}

module.exports = {
    initRedis,
    saveVarifyCode,
    getVarifyCode,
    validateVarifyCode
}; 