const grpc = require('@grpc/grpc-js');
const protoLoader = require('@grpc/proto-loader');
const path = require('path');

// 加载 proto 文件
const PROTO_PATH = path.join(__dirname, 'message.proto');
const packageDefinition = protoLoader.loadSync(PROTO_PATH, {
    keepCase: true,
    longs: String,
    enums: String,
    defaults: true,
    oneofs: true
});

const message_proto = grpc.loadPackageDefinition(packageDefinition).message;

// 创建 gRPC 客户端
const client = new message_proto.VarifyService(
    'localhost:50051',
    grpc.credentials.createInsecure()
);

// 测试获取验证码
function testGetVarifyCode() {
    const request = {
        email: "2413757964@qq.com"  // 请替换为您要测试的邮箱
    };

    client.GetVarifyCode(request, (error, response) => {
        if (error) {
            console.error('Error:', error);
            return;
        }
        console.log('获取验证码响应:', response);
    });
}

// 执行测试
testGetVarifyCode(); 