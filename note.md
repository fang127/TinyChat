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
- 学习饿汉模式和c++11之后的懒汉单例模式
- 使用std::thread构建一个asioIOServicePool,实现多线程处理事件，one loop one thread，主线程只负责accept
### day07
- 构建连接池，实现GateServer和VerifyGrpcClient
- 复习多线程编程中的thread,mutex,lock_guard,unique_lock,condition_variable,async.future
