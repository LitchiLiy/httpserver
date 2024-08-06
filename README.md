## 使用方式

1. 先把serverStart.cpp中的0.0.0.0改成你部署的为止的IP地址， 保存。
2. 在终端中直接在项目根目录调用cmake .
3. 再用make
4. 最后cd bin去执行./httpServer就可以启动服务器了


## 结构图

```plaintext
.
├── CMakeLists.txt
├── README.md
├── bin
│   ├── httpServer
│   ├── test18.exe
│   ├── test7_1.exe
│   ├── test7_2.exe
│   └── test9.exe
├── build
│   ├── CMakeCache.txt
│   ├── CMakeFiles
│   │   ├── 3.28.3
│   │   │   ├── CMakeCXXCompiler.cmake
│   │   │   ├── CMakeDetermineCompilerABI_CXX.bin
│   │   │   ├── CMakeSystem.cmake
│   │   │   └── CompilerIdCXX
│   │   │       ├── CMakeCXXCompilerId.cpp
│   │   │       ├── a.out
│   │   │       └── tmp
│   │   ├── CMakeConfigureLog.yaml
│   │   ├── CMakeDirectoryInformation.cmake
│   │   ├── Makefile.cmake
│   │   ├── Makefile2
│   │   ├── TargetDirectories.txt
│   │   ├── cmake.check_cache
│   │   ├── httpServer.dir
│   │   │   ├── DependInfo.cmake
│   │   │   ├── build.make
│   │   │   ├── cmake_clean.cmake
│   │   │   ├── compiler_depend.make
│   │   │   ├── compiler_depend.ts
│   │   │   ├── depend.make
│   │   │   ├── flags.make
│   │   │   ├── inc
│   │   │   │   ├── asyncLogging.cpp.o
│   │   │   │   ├── asyncLogging.cpp.o.d
│   │   │   │   ├── buffer.cpp.o
│   │   │   │   ├── buffer.cpp.o.d
│   │   │   │   ├── channel.cpp.o
│   │   │   │   ├── channel.cpp.o.d
│   │   │   │   ├── connector.cpp.o
│   │   │   │   ├── connector.cpp.o.d
│   │   │   │   ├── epoller.cpp.o
│   │   │   │   ├── epoller.cpp.o.d
│   │   │   │   ├── eventLoop.cpp.o
│   │   │   │   ├── eventLoop.cpp.o.d
│   │   │   │   ├── eventLoopThread.cpp.o
│   │   │   │   ├── eventLoopThread.cpp.o.d
│   │   │   │   ├── eventLoopThreadPool.cpp.o
│   │   │   │   ├── eventLoopThreadPool.cpp.o.d
│   │   │   │   ├── httpContext.cpp.o
│   │   │   │   ├── httpContext.cpp.o.d
│   │   │   │   ├── httpResponse.cpp.o
│   │   │   │   ├── httpResponse.cpp.o.d
│   │   │   │   ├── httpServer.cpp.o
│   │   │   │   ├── httpServer.cpp.o.d
│   │   │   │   ├── logFile.cpp.o
│   │   │   │   ├── logFile.cpp.o.d
│   │   │   │   ├── logStream.cpp.o
│   │   │   │   ├── logStream.cpp.o.d
│   │   │   │   ├── logging.cpp.o
│   │   │   │   ├── logging.cpp.o.d
│   │   │   │   ├── mAcceptor.cpp.o
│   │   │   │   ├── mAcceptor.cpp.o.d
│   │   │   │   ├── mInetAddress.cpp.o
│   │   │   │   ├── mInetAddress.cpp.o.d
│   │   │   │   ├── mSocket.cpp.o
│   │   │   │   ├── mSocket.cpp.o.d
│   │   │   │   ├── memoryPool.cpp.o
│   │   │   │   ├── memoryPool.cpp.o.d
│   │   │   │   ├── tcpClient.cpp.o
│   │   │   │   ├── tcpClient.cpp.o.d
│   │   │   │   ├── tcpConnection.cpp.o
│   │   │   │   ├── tcpConnection.cpp.o.d
│   │   │   │   ├── tcpServer.cpp.o
│   │   │   │   ├── tcpServer.cpp.o.d
│   │   │   │   ├── thread.cpp.o
│   │   │   │   ├── thread.cpp.o.d
│   │   │   │   ├── timer.cpp.o
│   │   │   │   ├── timer.cpp.o.d
│   │   │   │   ├── timerQueue.cpp.o
│   │   │   │   ├── timerQueue.cpp.o.d
│   │   │   │   ├── timestamp.cpp.o
│   │   │   │   └── timestamp.cpp.o.d
│   │   │   ├── link.txt
│   │   │   ├── progress.make
│   │   │   ├── serverStart.cpp.o
│   │   │   └── serverStart.cpp.o.d
│   │   ├── pkgRedirects
│   │   └── progress.marks
│   ├── Makefile
│   ├── Testing
│   │   ├── 20240709-1226
│   │   │   └── Test.xml
│   │   ├── TAG
│   │   └── Temporary
│   │       ├── CTestCostData.txt
│   │       └── LastTest_20240709-1226.log
│   ├── cmake_install.cmake
│   └── compile_commands.json
├── httpPage
│   ├── index.html
│   ├── testPage1.html
│   └── testPage2.html
├── inc
│   ├── asyncLogging.cpp
│   ├── asyncLogging.h
│   ├── atomicInt.h
│   ├── buffer.cpp
│   ├── buffer.h
│   ├── callBacks.h
│   ├── channel.cpp
│   ├── channel.h
│   ├── connector.cpp
│   ├── connector.h
│   ├── epoller.cpp
│   ├── epoller.h
│   ├── eventLoop.cpp
│   ├── eventLoop.h
│   ├── eventLoopThread.cpp
│   ├── eventLoopThread.h
│   ├── eventLoopThreadPool.cpp
│   ├── eventLoopThreadPool.h
│   ├── httpContext.cpp
│   ├── httpContext.h
│   ├── httpRequest.h
│   ├── httpResponse.cpp
│   ├── httpResponse.h
│   ├── httpServer.cpp
│   ├── httpServer.h
│   ├── lfuCache.h
│   ├── logFile.cpp
│   ├── logFile.h
│   ├── logStream.cpp
│   ├── logStream.h
│   ├── logging.cpp
│   ├── logging.h
│   ├── mAcceptor.cpp
│   ├── mAcceptor.h
│   ├── mInetAddress.cpp
│   ├── mInetAddress.h
│   ├── mSocket.cpp
│   ├── mSocket.h
│   ├── memoryPool.cpp
│   ├── memoryPool.h
│   ├── nocopyable.h
│   ├── note.md
│   ├── stackAlloc.h
│   ├── stringPiece.h
│   ├── tcpClient.cpp
│   ├── tcpClient.h
│   ├── tcpConnection.cpp
│   ├── tcpConnection.h
│   ├── tcpServer.cpp
│   ├── tcpServer.h
│   ├── thread.cpp
│   ├── thread.h
│   ├── timer.cpp
│   ├── timer.h
│   ├── timerId.h
│   ├── timerQueue.cpp
│   ├── timerQueue.h
│   ├── timestamp.cpp
│   └── timestamp.h
├── log
├── pdb
├── serverStart.cpp
└── test
    ├── main.cpp
    ├── test.cpp
    ├── test10.cpp
    ├── test11.cpp
    ├── test12.cpp
    ├── test13.cpp
    ├── test14.cpp
    ├── test15.cpp
    ├── test16.cpp
    ├── test18.cpp
    ├── test3.cpp
    ├── test4_runInLoop.cpp
    ├── test5_runAfter.cpp
    ├── test6.cpp
    ├── test7_1.cpp
    ├── test7_2.cpp
    ├── test8.cpp
    ├── test9.cpp
    └── test_eventfd.cpp


```
