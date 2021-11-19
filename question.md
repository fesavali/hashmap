1. cmp函数具体文档，应该是用来比较key的，相等的话返回0？ 

2. entry 是包含key和value的一个object吗？这个obj的定义是否又要求？

3. `hashmap_init`初始化时`size`是指hashmap 槽(buckets)的数量吗？如果我的hashmap实现中槽的数量是根据所插入的entry的数量而动态扩展，可以吗？是否需要在entry数量大于size时做出反应(例如插入失败)
OK

4. Memory Optimisation是指优化调用系统内存分配函数malloc的方式(例如通过分配合适大小的空间，减少调用malloc的次数); 还是说在进行空间申请时不实用malloc，用自己的方式实现一个malloc函数

5. qalloc.c调用了函数`reallocarray`,我的系统(Debian 10)出现错误提示缺少函数实现，需要加上头文件`<malloc.h>`; 是否可以修改qalloc.c文件，添加这个头文件?
OK
