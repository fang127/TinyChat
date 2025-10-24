### day03
- 利用CRTP实现Http管理类。  
- 学习了once_flag和call_once的用法以及奇异递归模板的设计[方法](https://blog.csdn.net/iShare_Carlos/article/details/140834454)。  
- 学习了c++20的concept的用法以及c++23的deducing this的用法。  
- 将构造函数protected化，防止外部实例化。  
- 只可通过静态方法getInstance获取实例和once_flag和call_once实现线程安全的单例模式。  
- 如何利用qt发送http请求。  
### day04
- 完成GateServer对http请求的处理。包含get和post请求。
- 将qt界面注册信号槽，完成界面和逻辑的交互。前后端进行联调。
### day05
- 学习c++17 filesystem特性
- 基于nodejs实现邮箱验证服务
- 学习grpc的使用[教程](https://www.bilibili.com/video/BV1qhSwYPEcp?spm_id_from=333.788.videopod.episodes&vd_source=344f27a24899c0d3bae08b279eb001aa)
- 将服务器，客户端，邮箱验证服务器串起来
### day06
- 学习饿汉模式和c++11之后的懒汉单例模式,利用懒汉单例模式实现ConfigMgr,从 C++11 起，函数局部静态变量的初始化是线程安全的；在更早的标准下需要额外同步
- 使用std::thread构建一个asioIOServicePool,实现多线程处理事件，one loop one thread，主线程只负责accept
### day07
- 构建连接池，实现GateServer和VerifyGrpcClient
- 复习多线程编程中的thread,mutex,lock_guard,unique_lock,condition_variable,async.future
### day08
- 封装redismgr，实现程序与远程redis的增删改查
- 实现redis连接池，提高多并发的能力
- redis连接池的池数据结构pool采用的是queue，可以进一步修改，两端锁，可以同时访问两个，当前并发访问一个
### day09
- 增加验证服务器对redis的访问，将验证服务器产生的验证码注册进redis，并且设置过期时间，实现定时注册
- 前端界面完成简单的注册模块，下一步实现将注册的用户账号和密码保存到mysql
### day10
- 学习mysql的c++ connector库的使用，编写简单的mysql客户端程序连接本地mysql进行查询语句
- 完成mysql连接池，以及封装的mysql实例MySQLDao和MySQLMgr，完善了用户注册模块，将注册的信息添加到mysql中归档
- 学习golang的特性Defer，通过c++自制该特性
### day11
- qt客户端注册界面完善，加入了更加严格的用户输入的邮箱，密码，确认密码，验证码验证，确保输入的格式正确
- 实现ClickedLabel类，添加Css资源，通过点击切换密码的显示和隐藏
- 完成注册界面，登录界面以及注册完成倒计时切换登录界面的操作，通过qt槽函数和信号机制实现各个界面间的切换
### day12
- 完成qt客户端忘记密码界面，客户端通过邮箱修改新密码的操作
- 完成GateServer修改密码的功能,将客户端与服务器连接，并且更新mysql中的用户数据
### day13
- 完成StatusServer以及客户端登录界面
- GateServer新增登录模块
- 客户端先通过短连接发送GateServer登录信息，GateServer通过mysql数据库验证客户信息，客户信息正确则调用grpc向StatusServer获取长连接的通信服务器ip和port以及token，StatusServer实现服务器的负载均衡，选择连接少的服务器返回给GateServer，GateServer再通过回调函数返回客户端长连接的服务器信息
### day14
- 完成QT客户端TCP管理类
- 学习ChatServer服务器搭建
**整体框架逻辑**

1. 启动与初始化:
程序入口 (ChatServer.cpp 中的 main 函数) 创建一个 CServer 对象。
CServer 在其构造函数或 Start 方法中，会初始化服务器所需的各个模块，包括：
加载配置文件 (ConfigMgr) 来获取端口、数据库地址等信息。
初始化 AsioIOServicePool，创建一组 io_service 和工作线程，用于处理网络 I/O 事件。
初始化 LogicSystem，准备处理业务逻辑。
初始化数据库连接池 (MysqlMgr) 和 Redis 连接 (RedisMgr)。
初始化 gRPC 客户端，用于和其他微服务通信。

2. 监听与连接:
CServer 启动后，开始在指定的端口上监听新的客户端连接。
当一个新客户端连接请求到达时，CServer 的 acceptor 会接受这个连接，并创建一个新的 CSession 对象来专门管理这个客户端连接。
CServer 会将这个新创建的 CSession 对象交给 AsioIOServicePool 中的一个 io_service 来处理后续的 I/O 操作。

3. 数据收发与处理:
CSession 对象负责与它所代表的客户端进行双向通信。它会异步地从客户端套接字（socket）读取数据。
当 CSession 收到一个完整的消息包后（通常会根据自定义的协议格式，如包头+包体来解析），它不会自己处理这个消息的业务逻辑。
CSession 会将解析后的消息（通常是反序列化后的 protobuf 对象）投递给 LogicSystem。为了避免阻塞网络线程，这个投递过程通常是异步的，比如将消息放入一个队列中。

4. 业务逻辑处理:
LogicSystem 是业务逻辑的核心。它从 CSession 接收消息，并根据消息类型（如登录、发送聊天消息、登出等）执行相应的操作。
例如处理登录: LogicSystem 会查询 MysqlMgr 来验证用户名和密码。验证通过后，可能会在 RedisMgr 中记录用户的在线状态和会话信息。
例如处理聊天消息: LogicSystem 找到接收方用户。如果接收方在线且连接在本服务器实例上，LogicSystem 会找到对应的 CSession 对象，并将消息通过该 CSession 发送出去。如果接收方在另一个服务器实例上，它可能会通过 Redis 的发布/订阅功能或 gRPC 调用来转发消息。
LogicSystem 在处理过程中会频繁与 MysqlMgr (持久化数据) 和 RedisMgr (缓存、状态、消息队列) 交互。

5. 连接断开:
当客户端断开连接或发生网络错误时，对应的 CSession 对象会被销毁。
在销毁前，CSession 会通知 LogicSystem 该用户已下线。
LogicSystem 会执行清理工作，例如更新 Redis 中的用户状态，并通知其他相关用户。
**主要类的作用**
1. CServer
CServer 是整个服务器的“大脑”和“入口”。它的主要职责是：

服务器生命周期管理: 负责服务器的启动、运行和停止。
资源初始化: 创建和管理整个服务器的核心组件，如 AsioIOServicePool、LogicSystem、ConfigMgr 等。
连接监听器 (Acceptor): 持有一个 boost::asio::ip::tcp::acceptor 对象，在指定端口上监听并接受新的客户端连接。
会话创建: 每当有新连接进来，它就创建一个 CSession 实例，并将新连接的 socket 交给这个 CSession 来管理。
可以把它看作是一个“接待大厅”，负责迎接每一位客人（客户端连接），并为他们分派一个专属的服务员（CSession）。

2. CSession
CSession 代表一个与客户端建立的独立连接，是服务器与单个客户端通信的直接桥梁。

网络通信: 封装了 boost::asio::ip::tcp::socket，负责从客户端异步读取数据和向客户端异步发送数据。
协议解析: 负责处理底层的字节流，按照预定义的协议（例如：消息头+Protobuf消息体）来解析出完整的消息包。
消息转发: 当 CSession 收到一个完整的业务消息后，它不会自己处理，而是将消息传递给 LogicSystem 进行逻辑处理。
会话管理: 维护与单个客户端相关的状态，如用户ID（登录后）、心跳时间等。当连接断开时，它负责触发清理逻辑。
每个 CSession 对象就是一个“专属服务员”，只为一个客人（客户端）提供收发消息的服务。

3. LogicSystem
LogicSystem 是服务器的“业务处理中心”，它实现了聊天服务器的所有功能逻辑。

业务逻辑处理: 包含处理各种请求（如登录、注册、单聊、群聊、登出等）的具体代码。它不关心网络I/O，只关心收到的消息内容是什么以及应该如何响应。
状态管理: 管理用户的在线状态、用户信息等。它会和 UserMgr (用户管理器) 交互来获取和更新用户信息。
数据交互: 与后端数据服务（MysqlMgr 和 RedisMgr）进行通信，以存取用户数据、消息记录、会话状态等。
消息路由: 决定一个消息应该被发送给哪个或哪些 CSession。如果需要跨服务器通信，它会使用 gRPC 客户端或 Redis 的 Pub/Sub 功能。
LogicSystem 就像是“厨房后台”，CSession 把客人的订单（请求消息）送过来，LogicSystem 根据订单内容进行烹饪（处理业务），然后把做好的菜（响应消息）交还给 CSession 送回给客人。

4. AsioIOServicePool
这个类是 Boost.Asio 网络编程中的一个常见性能优化模式，用于构建高并发服务器。

I/O 服务池: 它内部创建并管理一个 boost::asio::io_service (或 io_context) 的池。通常，池的大小等于 CPU 的核心数。
线程池: 为池中的每一个 io_service 分配一个或多个线程。这些线程是真正执行网络I/O事件处理的“工人”。
负载均衡: 当 CServer 创建一个新的 CSession 时，AsioIOServicePool 会通过轮询等策略选择一个 io_service 交给这个 CSession。这样，所有客户端连接的I/O负载就被均匀地分配到了多个线程上，避免了单线程处理所有网络事件的瓶颈，极大地提高了服务器的并发处理能力。
AsioIOServicePool 就像是服务器的“引擎组”，提供了多个并行的引擎（io_service 和关联的线程），让成千上万的 CSession 能够高效地并发运行。
### day15
- 完成ChatServer框架
- 完成客户端到GateServer通过http发送登录，GateServer验证通过后通过grpc到StatusServer获取ChatServer服务器信息，然后GateServer通过http返回响应报文给客户端，客户端在根据服务器信息发送tcp连接到ChatServer请求登录，ChatServer验证通过后，允许登录返回响应报文给客户端的过程
### day16
- 初步构建聊天界面
- 动态加载聊天列表
### day17
- 实现滚动聊天布局
- 实现气泡聊天对话框
### day18
- 添加侧边切换聊天和联系人功能
- 基于事件过滤器实现位置判断，从而关闭搜索

