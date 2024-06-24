Channal类

功能:
1. 记录一个fd, 并且一个channal只记录一个fd. 这个fd有可能是可读,可写或错误的来源. 他不拥有这个fd, 只是只读.
2. 记录一个回调函数, 这个回调函数供eventLoop调用.
3. 使能函数, 读写 错误.
4. 判断使能.
5. 设置回调。


公共：
函数指针结构
1. set回调, 使能读写错误.
2. 构造函数 event*和fd是初始化
3. 三大使能设置event_
4. 设置revent(int revt), 这个由Poller设置, 表示当前事件是什么类型, 和event对着来
5. handleEvent()核心函数: 判断revent是什么类型执行什么回调函数.
6. 面对Poller
   1. index设置函数
   2. 返回主人eventLoop实体指针给Poller函数.
   3. 返回idx值给Poller

私有:
1. 使能位event_, 表示用户关心的IO事件. 给Poller看的, 
2.  fd值
3. 三个回调函数指针
4. revents记录的是POLLIN这些状态, 这个值由Poller设置

私有函数:
1. update函数?_>最终调用Poller的函数update.

流程:
1. 初始化epoll: poller来做
2. 添加fd到epoll中, 由channal来做, 然后channal调用update
3. 循环等待: poller来做
   1. 组装触发事件, 由poller来做, 
4. 调用回调: eventLoop来做
5. 清理资源?

1. 初始化channal装入event和感兴趣的fd
   1. channal设置回调, 
   2. 使能回调, 这个函数里包含着update, 将信息传给event了已经. 从这里可以看出channal也不在eventLoop类中. 在三者之外.
2. 然后让loop就行. 


关于updateChannel
1. Poller  
   1. Poller中有一个pollfd_数组, 其实是一个event结构体, 每个数组都有一个idx, 在这个函数中先判断channal的idx是否<0, 则为新用户, 为他在Poller中构件数据库. 其实就是构件event结构体.
      1. fd, events感兴趣的事件, 当前事件 = 0, 推入pollfds中.
      2. 设置channal的idx为size - 1;
      3. 设置Poller本地的channal_的map, 让fd对应一个指针.
   2. 如果不是新用户
      1. 更新现存的任何数据. 拿出idx找出对应的event结构体
         1. 更新event, 现存revents=0(这里相当去掉上次的事件),
      2. 如果这次channal不关心任何事情, 将epollfd数组对应的channal的idx位置的fd改成-1即可.
   3. 总而言之, 这里可以看出, Poller有一个event结构体数组用来保存channal感兴趣的event数据, 还有用来找查channal的map
2. EventLoop中
   1. 单纯的调用poller的updateChannal, 不做任何事情.
3. 


Poller类
1. 公有
   1. channalUpdate
   2. Poll()函数{epoll, 然后将pollfd数组和cList给fill函数}, 具体的信息在pollfd里.
2. 私有函数
   1. fillActiveChannel函数(往eventLoop类的channalList数组填充)
      1. 用Map找出pollfd相应对应的channal指针
      2. 将channal指针调用set函数设置channal的revent成员
      3. 将channal推入eventLoop的actChannal数组中.
3. 私有成员
   1. pollfd_数组{明显自定义的一个结构}
   2. channal_Map

1. 从过程可以看到, Poller感知fd的相应, 然后修改对应的channal, 将对应的channal推入eventLoop的actC里面.
2. 再来Poller管理者pollfd数组, 掌握着所有感兴趣不感兴趣的信息. 也就是Poller实现了整个epoll过程. 

定时器Timer

1. 私有成员
   1. 定时器fd
   2. 回调函数
   3. 周期
   4. 时间戳
   5. 是否重复
2. 公有成员
   1. 构造函数
   2. 析构函数
   3. run函数调用回调

时间戳TimerStamp

1. 私有成员
   1. 至少我看来就是一个时间戳没什么特别的

定时器队列TimerQueue

1. 公有函数
   1. 添加定时器cb, Tstmp, 周期(供eventLoop调用三种不同的实现)
   2. 取消定期是ID
   3. getExpire(Tstmp now), 目的就是为了除掉过期的定时器.
   4. 插入定时器timer *
   5. 


定时器队列TimerQueue

这个队列好像是用一个定时器, 也就是类内固定一个fd, 然后自己排列一个队列来保存各种将要触发的时间, 每次添加定时器都会更新这个队列, 到时间之后触发的回调函数也会更新这个队列.

我要实现的功能,
1. 首先有两个set, 用来装内部的定时器, 一个用来装有效的定时器.
2. 添加定时器和取消定时器来供被人使用.
3. 拥有一个定时器实例, 仅仅一个, 拥有一个Channal绑定了这个实例, 意思就是这个定时器触发了就用这个来执行回调.

公开调用的函数:
1. 构造函数
2. 添加定时器addTimer
3. 取消定时器cancel

私有函数:
1. addTimerInLoop(Timer*)
2. cancelInLoop(TImerId)
3. handleRead();
4. vec getExpired(TimeStamp), 将过期的定时器返回出去, 然后将过期的定时器提出队列
5. reset(vec, stamp)
6. insert(Timer* timer)


私有变量
1. loop指针
2. timerfd 类内定时器fd
3. Channel类实体
4. set类的TimerList

给cancel调用的
1. ActiveTimerSet类型就是pari(Timer*, int64)时间戳应该是
2. 一个activeTimers的set
3. 一个cancelingTimers的set
4. 一个callingExpiredTimers的bool类型

关于std::bind(&function, this)就是func其实是一个void func(this)的类型, 因为func在类内, 故要用bind将this这个参数先消除才行.

初始化类内的成员, 尤其是Channel, 需要使能. 私有变量都要初始化, 

1. createTimerfd函数其实就是简单的构件一个timerfd, 然后返回一个timerfd的fd给类内
2. 关于addtimer(cb, Timestamp, interval)
   1. 调用runInLoop通过判断是否为本线程调用这个函数来执行不同的命令.
      1. addTimerInLoop: 做两件事, 一件事是insert往两个set中添加定时器, 一件事是利用刚刚的返回值重新设置linux的定时器时间触发(没有设置周期)
         1. insert: 将定时器插入到两个set中, 返回该定时器是不是第一个触发
         2. resetTimerfd: 先算timestamp, 转换成秒和n秒的itimespac形式. 没有设置周期
3. 



定时器timer.h

实现的功能为, 提供回调函数用来处理, 记录定时器要触发的时间戳, 定时器周琦, 是否重复, 唯一的序列号seq_, 这个类就是用来记录信息的, 没有定时器实体fd

1. 公有
   1. 构造函数
   2. run函数提供用来弄回调函数
   3. get期望的时间戳
   4. isrepeat
   5. showseq
   6. restart函数输入一个Timestamp, 用来更新定时器的时间戳, 表示你要到这里相应。 
2. 私有变量
   1. 回调函数
   2. 时间戳
   3. 周期
   4. 是否重复
   5. 一个唯一的序列号
   

一个全局静态的构件序列号实例 static, 用来给序列号赋值

时间戳TimerStamp

记录一个从纪元开始到现在的微秒绝对值

1. 私有成员
   1. int64_t类型的微秒纪元绝对时间戳.
   2. 一个换算单位const static类型. 1000 1000
2. 公有
   1. 默认构造=0
   2. 微秒构造(time_t, 微秒), 作用就是转换成微秒放到私有成员中
   3. 展示微秒时间戳
   4. 展示秒时间戳
   5. toString
   6. 交换两个时间戳类实例的时间戳
   7. 时间戳是否有效? 时间戳大于0

Timestamp Timestamp::now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t seconds = tv.tv_sec;
  return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}
这个为静态函数, 用来获取当前时间戳, 需要头文件sys/time.h

规定int64_t用来装微秒时间戳, time_t用来装秒的时间戳, 之间的转换用静态转换.

定时器Id

将定时器封装了一下, 友元为TimerQueue
1. 私有
   1. 定时器指针
   2. int64_t 的序列号
2. 构造
   1. 默认构造
   2. 传入构造 

原子操作类: AtuomicInt64

这个类放在Timer这个类的私有成员中, 用来创建一个唯一的序列号, 因为定时信息没有序列号不好找.


关于wakeup这个函数

这个东西是伴随着eventloop初始化就已经存在的东西, 是一个单独的机制, 意思就是用eventfd创建一个fd保存在类内部然后再用一个channal来接受这个fd. 然后这个channel使能读就完事了. 然后回调函数就是handleread这个函数, 就是单独设计给他用的

thread类就是用来保存thread信息用的

thread.strat被ELT调用来创建线程fd， 传入ELT的threadfunc函数， 故这个时候会产生tid， name

1. 私有成员
   1. 是否启动， 是， 否两个
   2. 是否joined， 一个
   3. 线程ID， 因为pthread用来保存实例
   4. tid号
   5. string的name
   6. 回调函数， 这个回调函数是线程初始化的回调， 最初来自ET创建时传入的参数。

一个静态的原子计数器

1. 公有函数
   1. 检查启动状态ed
   2. start函数， 用来启动线程， 事实上thread类仍然与ET类同在一个线程当中， 故要将thread信息打包成类data传入pthread_create中。 
   3. jion函数
   4. tid展示
   5. numCreatedget函数， 操作原子计数器
   6. name函数 单纯返回string

构造（回调， name）


EventLoopThread简单的整个流程

main函数中创建一个实例， 这个实例传入用来初始化线程的cb，和名字, ELT.startLoop(), 

开始之后， 调用类内Thread类的start函数。类内的Thread类使用create函数传入ELT的threadfunc函数用来执行。

threadfunc函数， 他会在新线程内部创建EL实例， 这样实例就保存在thread内了， 然后将loop指针给到ELT的类内保存。当然还要调动ELT类内的初始化函数。 最后调用EL的loop函数。这里有一个点需要提起，线程使用的函数要用ELT类内的成员函数，因为要将线程内部调用的ET的指针传出来。