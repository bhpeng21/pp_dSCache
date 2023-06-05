db说明（在强制读主的情况下）：

假如我将主从复制的三个db分别置于不同的服务器上，这样的话浪费读性能，此时两个从服务器完全相当于用于容灾，而无其他作用，且作为容灾手段，他还可能丢失数据，因为采用的是最终一致性方案。

假如一个服务器上放三个db，那么损失的是存储空间，每个服务器存储大小仅为原服务器的三分之一。但是每个服务器都还承担着读写任务，只不过因为存储量为三分之一，总体的读写量也降低为三分之一



三个db放到一个dbserver中，以vector存储（vec_）。`vec_[0]`为主db，`vec_[1]`和`vec[2]`为从db1和从db2.





目前写了lruMap，这是最底层用于存储的数据结构



Db保存lruMap和相关的基础信息？

这个类有必要吗？



Sever存有三个Db





Db0: 0 1 2->0 1 3

Db1: 1 2 0

Db2: 2 0 1->2 3 1

Db3: 3 0 2

这个数据迁移问题字母解决：目前的办法，扩容时，各个db0把数据转给心dbserver，再由dbserver去传递数据给其db1、db2





问题：master作为系统的管理者，如何去协调？

打个比方，系统扩容了，master重新分配slot，并发送命令给server。他如何确定各个server是否执行了对应的操作呢？（难道说master也不管，下一个请求直接把新的slot的dbserverid传递出去，而不管这个slot的数据是否已经传出？）

redis又是怎么做的呢？难道停机来完成扩容？





serverdb中存主db的slots（bitset）【我建议先搞这个】，还是在db中存（这样的话一个server三个db，每个db都存自己的slot，有必要吗？）？

master存`vector<int>slotsvec` 存每个slot分配的服务器的编号。（这个编号，master需要存个表），编号id对应一个node数据结构

master还需保存每个server的slots吗？以什么方式存呢？bitset还是int表示范围？还要存每个server拥有的slot的个数

slots完全由master发送给server。注意server更换slots的时候需要把统计出哪些slots改变了，并且把需要转发的数据转发出去



怎么进行范围bitset置位和复位？

slotsnum怎么存储呢？Dbserver需要存吗？（先存一下吧）

master肯定要存。而且都要存，存储的数据结构请再做规划



master与主从服务器沟通还是只与主服务器沟通，很关键。关系是否每个db都有slots



遍历map有点太蠢了，建议再搞一个map存储key==看看redis怎么做这一点的？==



两个线程都要对DbSerever进行操作的，怎么办呢？

创建一个状态，给他对应的mutex，哪怎么锁呢？unique 的wait那个吗？notify？ 怎么弄呢？



现在先把DbServer设计成，扩容时。。。。。注意是slots需要互斥吗？

能不能设置一个标志为表示正在使用某个需要保证线程安全的对象，从而不用去保护这个对象，这样会不会好一点？

一般情况下不会有优化，但是我这里是可以的，为什么呢？因为基本都是服务客户的线程在工作，而且这个线程工作时，只读slots，不进行修改，若对其加锁反而会拖慢进度。
是不是可以用atomic的bool？假如用bool 那么提前进去的那部分怎么办？？看看muduo怎么设置的，我这里建议先不管了，反正也就一个请求进去了，而且不是修改（slots）请求而是读取请求。==不管的话，有个小问题。假如这个client线程是setKV操作（这个本来是先发生的），那边server线程是扩容操作，并且set的KV是对呀的slot是要发送的slot，会不会因为cpu的执行速度问题导致扩容那边先完成，但是setKV后完成，这样的话新的KV会留下来，造成数据丢失。（丢失一个要紧吗？）==



扩容发生前，来自客户端的指令可能需要重定向。

所以信息需要带slot，然后在DbServer这里检查。



消息 serverid slot key （value） 



client访问slots和master访问slot，现在



tcpserver

dbserver



分析一下memcache的server

memcachedserver ：onconnection->tcpserver.setConnectionCallback()	//设置tcpserver的connectionCallback_数据成员



tcpserver

数据成员：

messageCallback_					//这两个默认default函数在tcpconnection里

connectionCallback_				//这个和上面那个有初始化的函数的，是个default的

writeCompleteCallback_		//这几个函数是给新的TCP连接安排的回调，给对应channel

threadInitCallback_

函数成员：

newConnection()->acceptor->setConnectionCallback()//acceptor的连接回调函数

removeConnection()	//这个是tcpconnection的，然后给channel，用于断开连接



断开链接和消息回复的回调看下echo
其他多看看memcahced那块



master和server都是准备工作完成之后再开启client端io线程