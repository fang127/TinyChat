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
- 
