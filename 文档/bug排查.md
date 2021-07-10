bug1： 无法添加描述符，file existed 。发现没有把acceppt得到的fd注册到epoll_fd上

bug2： bad weak_ptr。没有初始化对象时没有用智能指针包裹

bug3：bad function call。一路排查已经发现接受连接是成功的，现在怀疑线程池里面的包装的ThreadPoolTask可能有问题，可能function有问题。使用浏览器浏览网页，发现第一次event = 1时，正常运行，第二次event等于1,即建立连接之后处理请求有问题，已经锁定时(task.fun)(task.args)"调用问题。最终确定问题时taskQueue初始化方式错误，用resize似乎并不能有效初始化。

bug4： readn读出0个字符串，terminate called after throwing an instance of 'std::length_error'。 经过长时间的分析发现，是因为read不能接受数据，为什么在这个程序不能接收，而在之前的版本可以接收呢。 这个要搞明白

bug5:读取到了数据不返回网页，原因：数据是一次传输完成的，原先的程序设计只有一个阶段，只分析了头部，没有分析其他部分，改成while循环，使其遍历每个过程。

bug6:消息队列分析问题。段错误，一个是因为状态装换问题，一个是段问题，目前定位到了ANLYSIS部分，即最后消息部分。如果不弹出定时器中过时定时器，则不会出现段错误，可能时分离不成功，但是已经验证了分离已经成功，最后发现时timer被弹出析构时，没有判断，使用了空指针，出现段错误

bug7,状态机不能正确结束.write阻塞

bug8,完成网页传输后不能正常跳出

bug9:多线程部分，如果浏览器刷新页面，两次之后就会一直等待，然后停止加载，线程池里面的head会开始疯狂递增 ,最终发现时长连接reset()s时状态置错，导致recive一直无法正常工作。真的时一步一步排查终于解决问题，好难

1.无法通过webbench的压测:准确来说是短连接无法正常相应， 原因是短连接的task()没有被析构，但是如果使用MyEpoll::del,task就会析构两次，所以智能指针的计数肯定有问题. 新发现webbench一开始的时候是读不出数据的，而读不出数据需要readagain,但是因为状态机设置错误，所以没有正确readagain。isError = true;这个时候跳不出循环，所以也无法避免超过次数和读取发生错误时跳出循环。只有需要 keeplive和readagain的需要重置状态，其余均需要析构掉TASk

2.日志文件被删除就无法写入日志

3.程序过一段事件就抽风的bug，时间堆弹出超时任务时，因为修改了task的析构函数，此时已经没有执行从epoll_fd删除超时字符的功能。仍然存在断开连接，服务器就抽风.用gdb调试发现epoll_wait重复触发已经断开的fd。用isof -i（显示socket文件描述符） 4查看，发现myserver有一个连接处于CLOSE_WAIT状态。该状态是因为客户端断开连接之后，服务器端没有调用close(fd)，实际上就是没有正常析构Task，因为close（fd）在task里面。

!!!最后查看代码终于发现，超时重读事件也被reset()了，即添加了计时器之后哦，仍然进行了reset(),这时就把超时重读事件设置为了0，所以才会出现again_time = 1, 然后又等于0.取消这个reset()之后，就不会发生在客户端关闭连接之后，epoll_wait()会触发，然后发现超时也读不出东西，然后从epoll_fd中删除该fd，然后关闭该fd，结束了close_wait()状态

bug1： 无法添加描述符，file existed 。发现没有把acceppt得到的fd注册到epoll_fd上 bug2： bad weak_ptr。没有初始化对象时没有用智能指针包裹 bug3：bad function call。一路排查已经发现接受连接是成功的，现在怀疑线程池里面的包装的ThreadPoolTask可能有问题，可能function有问题。使用浏览器浏览网页，发现第一次event = 1时，正常运行，第二次event等于1,即建立连接之后处理请求有问题，已经锁定时(task.fun)(task.args)"调用问题。最终确定问题时taskQueue初始化方式错误，用resize似乎并不能有效初始化。 bug4： readn读出0个字符串，terminate called after throwing an instance of 'std::length_error'。 经过长时间的分析发现，是因为read不能接受数据，为什么在这个程序不能接收，而在之前的版本可以接收呢。 这个要搞明白 bug5:读取到了数据不返回网页，原因：数据是一次传输完成的，原先的程序设计只有一个阶段，只分析了头部，没有分析其他部分，改成while循环，使其遍历每个过程。