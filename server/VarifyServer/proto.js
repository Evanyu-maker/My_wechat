const path = require('path')
const grpc = require('@grpc/grpc-js')
const protoLoader = require('@grpc/proto-loader')

const PROTO_PATH = path.join(__dirname, 'message.proto')

//同步加载并解析.proto文件。
const packageDefinition = protoLoader.loadSync(PROTO_PATH, { 
    keepCase: true, 
    longs: String, 
    enums: String, 
    defaults: true, 
    oneofs: true })
const protoDescriptor = grpc.loadPackageDefinition(packageDefinition)

const message_proto = protoDescriptor.message

module.exports = message_proto

