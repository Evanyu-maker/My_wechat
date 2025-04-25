const grpc = require('@grpc/grpc-js');
const protoLoader = require('@grpc/proto-loader');
const path = require('path');
const { v4: uuidv4 } = require('uuid');

// 导入自定义模块
const emailModule = require('./email.js');
const const_module = require('./const.js');
const config_module = require('./config.js');
const redisModule = require('./redis.js');

// 加载proto文件
const PROTO_PATH = path.join(__dirname, 'message.proto');
const packageDefinition = protoLoader.loadSync(PROTO_PATH, {
    keepCase: true,
    longs: String,
    enums: String,
    defaults: true,
    oneofs: true
});
const message_proto = grpc.loadPackageDefinition(packageDefinition).message;

/**
 * 生成并发送验证码
 * @param {Object} call RPC调用参数
 * @param {Function} callback 回调函数
 */
async function GetVarifyCode(call, callback) {
    console.log("开始处理验证码请求，email是:", call.request.email)
    try{
        // 生成6位数字验证码
        const varifyCode = Math.floor(100000 + Math.random() * 900000).toString();
        console.log("生成的验证码是: ", varifyCode)
        
        // 将验证码存入Redis，设置过期时间
        console.log("开始保存验证码到Redis...");
        const saveResult = await redisModule.saveVarifyCode(
            call.request.email, 
            varifyCode, 
            config_module.code_expire
        );
        
        if (!saveResult) {
            console.log("存储验证码到Redis失败");
            return callback(null, { 
                email: call.request.email,
                error: const_module.Errors.REDIS_ERROR
            });
        }
        console.log("验证码已成功保存到Redis");
        
        // 发送邮件
        console.log("开始发送邮件...");
        let text_str = `您的验证码为 ${varifyCode}，请在${config_module.code_expire}秒内完成注册，过期将失效。`;
        let mailOptions = {
            from: 'evan_yutang@163.com',
            to: call.request.email,
            subject: '验证码',
            text: text_str,
        };

        try {
            console.log("正在发送邮件...");
            let send_res = await emailModule.SendMail(mailOptions);
            console.log("邮件发送成功，结果:", send_res);
        } catch (emailError) {
            console.error("邮件发送失败，详细错误：", emailError);
            return callback(null, { 
                email: call.request.email,
                error: const_module.Errors.Exception
            });
        }

        console.log("所有操作完成，准备返回结果");
        callback(null, { 
            email: call.request.email,
            error: const_module.Errors.SUCCESS,
            code: varifyCode
        }); 
    } catch(error) {
        console.error("处理过程中发生异常：", error);
        callback(null, { 
            email: call.request.email,
            error: const_module.Errors.Exception
        }); 
    }
}

/**
 * 主函数，启动gRPC服务器
 */
function main() {
    // 初始化Redis连接
    redisModule.initRedis().catch(error => {
        console.error("Redis初始化失败:", error);
        console.log("服务将继续运行，但验证码功能可能不可用");
    });
    
    // 创建gRPC服务器
    var server = new grpc.Server()
    
    // 注册服务方法 - 只注册GetVarifyCode方法
    server.addService(message_proto.VarifyService.service, { 
        GetVarifyCode: GetVarifyCode
    })
    
    // 绑定端口并启动服务
    server.bindAsync('0.0.0.0:50051', grpc.ServerCredentials.createInsecure(), (error, port) => {
        if (error) {
            console.error('服务器绑定失败:', error);
            return;
        }
        server.start();
        console.log('grpc server started on port:', port);
    });
}

// 启动服务
main()