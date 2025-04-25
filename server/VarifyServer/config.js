const fs = require('fs');

let config = JSON.parse(fs.readFileSync('./config.json', 'utf8'));

// 配置字段，使用可选链避免未定义错误
let email_user = config.email?.user;
let email_pass = config.email?.pass;

// Redis配置
let redis_host = config.redis?.host;
let redis_port = config.redis?.port; 
let redis_password = config.redis?.password;

// 验证码配置
let code_prefix = config.code_prefix || 'code:';
let code_expire = config.code_expire || 180;

module.exports = {
    email_user,
    email_pass,
    redis_host,
    redis_port,
    redis_password,
    code_prefix,
    code_expire
};