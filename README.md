# 仿微信即时通讯系统 💬

## 项目简介 📋
这是一个基于Qt Material设计的现代化即时通讯系统。项目采用分布式微服务架构，通过以下方式实现高性能和高可用：

- **分层架构** 📚
  - 表现层：基于Qt Material的现代化客户端
  - 网关层：GateServer处理请求分发和负载均衡
  - 服务层：VarifyServer处理用户认证等核心业务
  - 数据层：采用Redis+MySQL的混合存储方案

- **技术特点** 🔧
  - 使用C++开发客户端和网关服务，保证高性能
  - 采用Node.js开发认证服务，提供灵活的业务处理能力
  - 基于gRPC的服务间通信，确保高效的数据传输
  - 使用Asio实现异步I/O，提升并发处理能力

- **高可用设计** ⚡
  - GateServer采用连接池管理，支持动态扩缩容
  - Redis集群提供高速缓存，降低数据库压力
  - 使用长连接机制，减少连接建立开销
  - 支持服务热备份，确保系统可靠性

## 当前版本 (V1.0) ✨
### 已实现功能
- 基础用户系统 👥
  - 账号注册
  - 用户登录
  - 基本个人信息管理
- 即时通讯 📨
  - 基础消息收发功能
  - 文本消息支持

## 开发路线图 🗺️
### 即将推出 (V1.1) 🚀
- 好友系统
  - 好友添加/删除 👥
  - 好友列表管理 📋
  - 好友状态显示 🟢
- 消息增强
  - 消息历史记录 📜
  - 在线状态显示 ✅
  - 消息已读状态 ✓✓

### 未来规划 (V2.0) 🔮
- AI助手集成 🤖
  - 智能回复
  - 内容推荐
  - 自动对话
- 多媒体消息 📱
  - 图片发送
  - 语音消息
  - 视频通话
- 群聊功能 👥👥
  - 群创建与管理
  - 群消息
  - 群公告
- 表情系统 😊
  - 自定义表情包
  - 动态表情
- 安全性增强 🔒
  - 端到端加密
  - 双因素认证
- 云端存储 ☁️
  - 消息云备份
  - 文件云存储

## 系统架构 🏗️

### 客户端 (Client) 💻
- 基于Qt Material UI框架开发，提供现代化的用户界面
- 使用C++开发，支持跨平台运行
- 采用异步通信机制，确保消息的实时性
- 实现了用户登录、注册等基础功能
- 项目使用Visual Studio 2019构建，支持x64架构

### 服务端 (Server) 🖥️
#### GateServer（C++实现）
- **负载均衡服务器**：使用Asio实现高性能的异步IO
- **网络框架**：
  - 使用AsioIOServerPool管理连接池
  - 实现了HTTP长连接（HttpConnection）
  - 采用gRPC进行服务间通信
- **数据存储**：
  - Redis连接池管理（RedisConPool）
  - Redis数据管理（RedisMgr）
  - MySQL数据库支持（使用mysqlcppconn）
- **配置管理**：使用INI格式的配置文件（ConfigMgr）
- **核心逻辑**：LogicSystem处理业务逻辑

#### VarifyServer（Node.js实现）
- **验证服务器**：处理用户认证相关功能
- **技术栈**：
  - Node.js运行环境
  - gRPC通信协议
  - Redis数据存储
  - 邮件服务支持
- **配置管理**：使用JSON格式的配置文件

## 技术栈详解
### 前端
- **UI框架**: Qt Material UI
- **开发语言**: C++ 11及以上
- **构建工具**: Visual Studio 2019, qmake
- **版本控制**: Git

### 后端
- **GateServer**:
  - C++ 11
  - Asio异步IO库
  - gRPC
  - Redis
  - MySQL
  - Protobuf
- **VarifyServer**:
  - Node.js
  - gRPC
  - Redis
  - JSON配置

### 网络通信
- TCP/IP长连接
- HTTP协议
- gRPC服务间通信
- Protobuf消息序列化

## 主要功能
- 用户系统
  - 账号注册（支持邮箱验证）
  - 用户登录
  - 个人信息管理
- 即时通讯
  - 单聊消息
  - 在线状态显示
  - 消息历史记录
- 好友系统
  - 好友添加/删除
  - 好友列表管理
  - 好友状态显示
- 数据同步
  - 多设备消息同步
  - 离线消息存储

## 系统要求
### 开发环境
- Visual Studio 2019或更高版本
- Qt 5.x或更高版本
- Node.js 12.x或更高版本
- C++编译器支持C++11及以上标准
- Redis服务器
- MySQL服务器

### 运行环境
- 操作系统: Windows/Linux
- 内存: 4GB及以上
- 存储空间: 1GB及以上

## 安装说明
1. 克隆仓库
```bash
git clone [repository-url]
```

2. 安装依赖
```bash
# VarifyServer依赖安装
cd My_wechat/server/VarifyServer
npm install

# 确保Redis和MySQL服务已启动
```

3. 编译服务端
```bash
cd My_wechat/server
# 编译GateServer
cd GateServer
# 使用Visual Studio 2019打开GateServer.sln并编译

# 配置VarifyServer
cd ../VarifyServer
# 修改config.json配置文件
```

4. 编译客户端
```bash
cd My_wechat/client/My_wechat
# 使用Visual Studio 2019打开My_wechat.sln并编译
```

## 配置说明
### GateServer配置
配置文件：`config.ini`
- 服务器端口配置
- Redis连接配置
- MySQL数据库配置
- 日志级别设置

### VarifyServer配置
配置文件：`config.json`
- 服务器端口配置
- Redis连接信息
- 邮件服务配置
- gRPC服务配置

## 运行说明
1. 启动服务端
```bash
# 启动VarifyServer
cd My_wechat/server/VarifyServer
node server.js

# 启动GateServer
cd ../GateServer
./GateServer
```

2. 运行客户端程序
```bash
# 运行编译好的客户端程序
./My_wechat
```

## 项目结构
```
My_wechat/
├── client/
│   └── My_wechat/          # Qt客户端项目
│       ├── My_wechat/      # 源代码目录
│       ├── x64/           # 编译输出目录
│       └── .vscode/       # VS Code配置
├── server/
│   ├── GateServer/        # 负载均衡服务器
│   │   ├── src/          # 源代码
│   │   ├── config.ini    # 配置文件
│   │   └── proto/        # protobuf文件
│   └── VarifyServer/     # 验证服务器
│       ├── config.json   # 配置文件
│       ├── server.js     # 主程序
│       └── node_modules/ # Node.js依赖
```

## 开发规范
- 代码风格遵循C++标准规范
- 使用Proto3定义通信协议
- 遵循RESTful API设计规范
- 使用统一的日志记录格式

## 调试指南
- GateServer: 使用Visual Studio调试器
- VarifyServer: 使用Node.js调试工具
- 客户端: Qt Creator或Visual Studio调试器

## 参与贡献 🤝
1. Fork 项目 🔀
2. 创建新的分支 (`git checkout -b feature/AmazingFeature`) 🌿
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`) ✏️
4. 推送到分支 (`git push origin feature/AmazingFeature`) 📤
5. 开启一个 Pull Request 🎯

## 许可证 📄
[待补充]

## 联系方式 📬
[待补充]

## 常见问题 ❓
1. 连接Redis失败 🔌
   - 检查Redis服务是否启动
   - 验证配置文件中的连接信息

2. 编译错误 ⚠️
   - 确保安装了所有必要的依赖
   - 检查Visual Studio版本是否正确

3. 客户端无法连接服务器 🌐
   - 检查服务器IP和端口配置
   - 确认防火墙设置

## 技术支持 💪
如果您在使用过程中遇到任何问题，请通过以下方式获取帮助：
- 提交 Issue 🎫
- 查看文档 📚
- 加入开发者社区 👥

## 联系方式
[待补充]

## 常见问题
1. 连接Redis失败
   - 检查Redis服务是否启动
   - 验证配置文件中的连接信息

2. 编译错误
   - 确保安装了所有必要的依赖
   - 检查Visual Studio版本是否正确

3. 客户端无法连接服务器
   - 检查服务器IP和端口配置
   - 确认防火墙设置 