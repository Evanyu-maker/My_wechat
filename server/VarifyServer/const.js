let code_prefix = 'code:';

const Errors = {
    // 通用错误码
    SUCCESS: 0,
    ERROR: 1,
    Exception: 2, 
    
    // 验证码相关错误码
    CODE_EXPIRED: 3,    // 验证码已过期
    CODE_INVALID: 4,    // 验证码无效
    REDIS_ERROR: 5      // Redis操作错误
}

module.exports = {
    code_prefix,
    Errors,
}