# DistCp - Linux下分布式拷贝工具

标签（空格分隔）： Linux工具

---

从MySQL读取数据拷贝任务，启动线程执行拷贝命令，可以拷贝整个目录，为避免拷贝目录失败，本工具首先会根据源创建对应的目标路径，然后依次拷贝一个文件，一旦发生完成，会将命令执行结果保存到MySQL数据库，使用方式：

    distcp -t taskId -d databaseName