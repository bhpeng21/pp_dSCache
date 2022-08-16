#include "wuhuServer.h"


wuhuServer::wuhuServer()
{
    timeval = time(0);
}

wuhuServer::~wuhuServer()
{
}

//分配槽
void wuhuServer::allocateSlot()
{
    int num = slots.size();
    int averageCnt = numslots / num;
    int count = 0;
    int rounds = 1;
    for (auto iter = slots.begin(); iter != slots.end(); iter ++)
    {
        (*iter).second.first = count;
        count += averageCnt;
        (*iter).second.second = count;
        if (rounds == num)
        {
            (*iter).second.second = numslots - 1;
        }
        count ++;
        rounds ++;
    }
    return;
}

//增加槽
void wuhuServer::addSlot(const string &ipandport)
{
    auto inpair = make_pair(0,0);
    auto outpair = make_pair(ipandport, inpair);
    slots.push_back(outpair);
    allocateSlot();
    return;
}

//减少槽
void wuhuServer::cutSlot(const string &ipandport)
{
    for (auto iter = slots.begin(); iter != slots.end(); iter++)
    {
        if (iter->first == ipandport)
        {
            slots.erase(iter);
			break;
        }
    }
    allocateSlot();
    return;
}

//返回hash值所在槽代表的cache server的名称
string wuhuServer::compareSlot(const int hashVal)
{
   for (auto iter = slots.begin(); iter != slots.end(); iter++)
   {
       if (hashVal >= (*iter).second.first && hashVal <= (*iter).second.second)
       {
           return (*iter).first;
       }
   }
}

//增加cache server索引表，名称为ipandport
void wuhuServer::addListDcNode(const string &ipandport)
{
    wuhuDc listDcNode(ipandport, timeval);
    listDc.push_back(listDcNode);
    return;
}

//根据ipandport查找索引表节点，返回指向节点的迭代器
list<wuhuDc>::iterator wuhuServer::findListDcNode(const string &ipandport)
{
    for (auto iter = listDc.begin(); iter != listDc.end(); iter ++)
    {
        if ((*iter).getip() == ipandport)
        {
            return iter;
        }
    }  
}

//减少指定ipandport的cache server索引表
void wuhuServer::cutListDcNode(list<wuhuDc>::iterator iter)
{
    listDc.erase(iter);
    return;
}

//getkey操作
string wuhuServer::getKey(const string &key)
{
    int hashval = MD5(key).s2i() % numslots;
    string ipandport = compareSlot(hashval);
	//test
	/*cout << ipandport << " ";*/
    auto iter = findListDcNode(ipandport);
    if ((*iter).indexExist(key) == 1)
    {
        return ipandport;       //这个语句需要优化
    }
    return "cant find key";
}

//setkey操作
string wuhuServer::setKey(const string &key)
{
    int hashval = MD5(key).s2i() % numslots;
    string ipandport = compareSlot(hashval);
    auto iter = findListDcNode(ipandport);
    if ((*iter).indexExist(key) == 1)
    {
        return ipandport;
    }
    (*iter).indexAdd(key);
    return ipandport;
}

//容量变化时调整位置/socket回复
void wuhuServer::adjustAndReply(const string &cmdStr)
{
    for (auto iterOut = listDc.begin(); iterOut != listDc.end(); iterOut++)
    {
        map<string, string> change;
        vector<string> toNode;
        vector<string> keylist;
        for (auto iterIn = iterOut->indexTable.begin(); iterIn != iterOut->indexTable.end(); iterIn++)
        {
            if (compareSlot(iterIn->second) != iterOut->getip())
            {
                findListDcNode(compareSlot(iterIn->second))->moveIndex(*iterIn);
				change.insert({ iterIn->first,compareSlot(iterIn->second) });
            }
        }
        for (int i = 0; i < nodeNum; i ++)
        {
            toNode.push_back(slots[i].first);
            keylist.push_back("");
        }
        for (auto iterIn = change.begin(); iterIn != change.end(); iterIn ++)
        {
            for (int i = 0; i < nodeNum; i ++)
            {
                if (iterIn->second == toNode[i])
                {
                    keylist[i].append(iterIn->first);
                    iterOut->indexTable.erase(iterIn->first);
                }
            }
        }
        //socket程序

        for (int i = 0; i < nodeNum; i ++)
        {
			if (keylist[i] == "")
			{
				continue;
			}
			string ipStr = iterOut->ip.substr(0, iterOut->ip.size() - 5);
			/*char ipChar[20];
			strcpy(ipChar, ipStr.c_str());*/
			 const char *ipChar = ipStr.c_str();
			 int port = iterOut->port;

			//socket连接
			CTcpClient TcpClient;
			if (TcpClient.ConnectToServer(ipChar, port) == false)
			{
				printf("connect(%s:%d) failed.\n", ipChar, port);
				close(TcpClient.m_sockfd);
			}
			else {
				printf("connect ok.\n");
			}

			while (keylist[i].size() > 0)
			{
				// char sendbuffer[1024];
				// memset(sendbuffer, 0, sizeof(sendbuffer));
				string sendStr = cmdStr;
				sendStr.append(" ");
				sendStr.append(toNode[i]);
				char buf[1024];

				if (keylist[i].size() > 800)
				{
					sendStr.append(" ");
					sendStr.append(keylist[i], 0, 800);
					keylist[i] = keylist[i].substr(800);
					// buf = sendStr.c_str();
					strcpy(buf, sendStr.c_str());
					// strcpy(sendbuffer,sendStr.c_str());
					// write (eventfd, sendbuffer, strlen(sendbuffer));
				}
				else {
					sendStr.append(" ");
					sendStr.append(keylist[i]);
					// buf = sendStr.c_str();
					strcpy(buf, sendStr.c_str());
					keylist[i] = "";
					// strcpy(sendbuffer,sendStr.c_str());
					// write (eventfd, sendbuffer, strlen(sendbuffer));
				}
				if (write(TcpClient.m_sockfd, buf, strlen(buf)) <= 0) // 将客户端输入发送到远程服务器中
				{
					printf("write() failed.\n");  close(TcpClient.m_sockfd); // 发送失败则关闭客户端
				}

				//char buf1[1024];
				//memset(buf1, 0, sizeof(buf));
				//if (read(TcpClient.m_sockfd, buf, sizeof(buf)) <= 0)  // 读取远程服务器响应
				//{
				//	printf("read() failed.\n");  close(TcpClient.m_sockfd);  // 读取远程服务器响应失败
				//}
				//// printf("recv:%s\n",buf);
			}
			close(TcpClient.m_sockfd);
        }
    }
}

//扩容：增加cache server节点
void wuhuServer::expand(const string &ipandport, const string &cmdStr)
{
    nodeNum ++;
    addSlot(ipandport);
    addListDcNode(ipandport);
    adjustAndReply(cmdStr);
}


void wuhuServer::expand(const string &ipandport)
{
    nodeNum ++;
    addSlot(ipandport);
    addListDcNode(ipandport);
    // adjustAndReply(cmdStr);
}



//缩容：减少cache server节点
void wuhuServer::narrow(const string &ipandport, const string &cmdStr)
{
    nodeNum --;
	cutSlot(ipandport);
    adjustAndReply(cmdStr);
	cutListDcNode(findListDcNode(ipandport));
}

//心跳时钟更新
void wuhuServer::updateTimeval()
{
    timeval = time(0);
}

//心跳监测机制
void wuhuServer::heartbeatMonitor()
{
	
	vector<list<wuhuDc>::iterator> cutNode;
    
    for (auto iter = listDc.begin(); iter != listDc.end(); iter ++)
    // for (int i = 0; i < listDc.size(); i++)
    {
        if (timeval >= iter->expireTime)
        {
            //socket连接
            string ipStr = iter->ip.substr(0,iter->ip.size() - 5);
            char ipChar[50];
            // const char *ipChar = ipStr.c_str();
            strcpy(ipChar, ipStr.c_str());
            int port = iter->port;

            CTcpClient TcpClient;
            if (TcpClient.ConnectToServer(ipChar, port) == false)
            {
                printf("connect(%s:%d) failed.\n",ipChar,port); close(TcpClient.m_sockfd);  
                nodeNum --;
                cutSlot(iter->ip);
				cutNode.push_back(iter);
                //cutListDcNode(findListDcNode(iter->ip));
                //adjustAndReply("move");
                continue;
            }
            else{
                printf("connect ok.\n");
            }

            char buf[1024] = "isAlive";
            if (write(TcpClient.m_sockfd,buf,strlen(buf)) <=0) // 将客户端输入发送到远程服务器中
            { 
                printf("write() failed.\n");  close(TcpClient.m_sockfd);// 发送失败则关闭客户端
            }
		
            memset(buf,0,sizeof(buf));
            if (read(TcpClient.m_sockfd,buf,sizeof(buf)) <=0)  // 读取远程服务器响应
            { 
                printf("read() failed.\n");  close(TcpClient.m_sockfd);  // 读取远程服务器响应失败
            }
            printf("recv:%s\n",buf);
            close(TcpClient.m_sockfd);

			updateHeartbeat(iter);
        }
    }
	for (int i = 0; i < cutNode.size(); i ++)
    {
        listDc.erase(cutNode[i]);
    }
    adjustAndReply("move");
}

//更新过期时间及下次心跳时间
void wuhuServer::updateHeartbeat(list<wuhuDc>::iterator iter)
{
	iter->expireTime = timeval + iter->heartbeatLen;

}

//test
 // void wuhuServer::show()
 // {
 // 	cout << slots.front().second.first << " " << slots.front().second.second << endl;
	//cout << slots.back().second.first << " " << slots.back().second.second << endl;
 // }
 // int wuhuServer::showhashval(const string &key)
 // {
	//int hashval = MD5(key).s2i() % numslots;
	//return hashval;
 // }
 // int wuhuServer::shownum()
 // {
	//  return nodeNum;
 // }


// test
// int main()
// {
//     wuhuServer master;
//     master.expand("192.168.2.21 7000");
//   	master.expand("47.35.21.8 5005");
// 	master.expand("47.35.21.8 5000");
//   	master.show();
// 	cout << master.setKey("wuhuqifi") << " " << master.showhashval("wuhuqifi") << endl;
// 	cout << master.setKey("wuhuqifq") << " " << master.showhashval("wuhuqifq") << endl;
// 	cout << master.setKey("wuhuqifw") << " " << master.showhashval("wuhuqifw") << endl;
// 	cout << master.setKey("wuhuqife") << " " << master.showhashval("wuhuqife") << endl;
// 	cout << master.setKey("wuhuqifr") << " " << master.showhashval("wuhuqifr") << endl;
// 	cout << master.setKey("wuhuqift") << " " << master.showhashval("wuhuqift") << endl;
// 	cout << master.setKey("wuhuqify") << " " << master.showhashval("wuhuqify") << endl;
// 	cout << master.setKey("wuhuqifu") << " " << master.showhashval("wuhuqifu") << endl;
// 	cout << master.setKey("wuhuqifo") << " " << master.showhashval("wuhuqifo") << endl;
// 	cout << master.setKey("wuhuqifp") << " " << master.showhashval("wuhuqifp") << endl;
// 	cout << master.setKey("wuhuqifa") << " " << master.showhashval("wuhuqifa") << endl;
// 	cout << master.setKey("wuhuqifs") << " " << master.showhashval("wuhuqifs") << endl;///
// 	cout << master.setKey("wuhuqifd") << " " << master.showhashval("wuhuqifd") << endl;
// 	cout << master.setKey("wuhuqiff") << " " << master.showhashval("wuhuqiff") << endl;
// 	cout << master.setKey("wuhuqifg") << " " << master.showhashval("wuhuqifg") << endl;
// 	cout << master.setKey("wuhuqifh") << " " << master.showhashval("wuhuqifh") << endl;
// 	cout << master.setKey("wuhuqifj") << " " << master.showhashval("wuhuqifj") << endl;
// 	cout << master.setKey("wuhuqifk") << " " << master.showhashval("wuhuqifk") << endl;
// 	cout << master.setKey("wuhuqifl") << " " << master.showhashval("wuhuqifl") << endl;
// 	cout << master.setKey("wuhuqifz") << " " << master.showhashval("wuhuqifz") << endl;
// 	cout << master.setKey("wuhuqifx") << " " << master.showhashval("wuhuqifx") << endl;
// 	cout << master.setKey("wuhuqifc") << " " << master.showhashval("wuhuqifc") << endl;
// 	cout << master.setKey("wuhuqifv") << " " << master.showhashval("wuhuqifv") << endl;
// 	cout << master.setKey("wuhuqifb") << " " << master.showhashval("wuhuqifb") << endl;
// 	cout << master.setKey("wuhuqifn") << " " << master.showhashval("wuhuqifn") << endl;
// 	cout << master.getKey("yiqihapi") << " " << master.showhashval("yiqihapi") << endl;//
// 	cout << master.setKey("wuhuqify") << " " << master.showhashval("wuhuqify") << endl;
// 	cout << master.setKey("wuhuqifu") << " " << master.showhashval("wuhuqifu") << endl;
// 	cout << master.setKey("wuhuqifo") << " " << master.showhashval("wuhuqifo") << endl;
// 	cout << master.setKey("wuhuqifp") << " " << master.showhashval("wuhuqifp") << endl;
// 	cout << master.getKey("wuhuqifa") << " " << master.showhashval("wuhuqifa") << endl;
// 	cout << master.getKey("wuhuqifs") << " " << master.showhashval("wuhuqifs") << endl;
// 	cout << master.getKey("wuhuqifd") << " " << master.showhashval("wuhuqifd") << endl;
// 	cout << master.getKey("wuhuqiff") << " " << master.showhashval("wuhuqiff") << endl;
// 	cout << master.getKey("wuhuqifg") << " " << master.showhashval("wuhuqifg") << endl;
// 	cout << master.getKey("wuhuqifh") << " " << master.showhashval("wuhuqifh") << endl;
// 	cout << master.getKey("wuhuqifj") << " " << master.showhashval("wuhuqifj") << endl;
// 	cout << master.getKey("wuhuqifk") << " " << master.showhashval("wuhuqifk") << endl;
// 	cout << master.getKey("wuhuqifl") << " " << master.showhashval("wuhuqifl") << endl;
// 	cout << master.getKey("wuhuqifz") << " " << master.showhashval("wuhuqifz") << endl;
// 	cout << master.getKey("wuhuqifx") << " " << master.showhashval("wuhuqifx") << endl;
// 	cout << master.getKey("wuhuqifc") << " " << master.showhashval("wuhuqifc") << endl;
// 	cout << master.getKey("wuhuqifv") << " " << master.showhashval("wuhuqifv") << endl;
// 	cout << master.getKey("wuhuqifb") << " " << master.showhashval("wuhuqifb") << endl;
// 	cout << master.getKey("wuhuqifn") << " " << master.showhashval("wuhuqifn") << endl;

// 	master.narrow("47.35.21.8 5005");
// 	cout << master.shownum() << endl;
// 	master.show();

// 	cout << master.getKey("wuhuqifa") << " " << master.showhashval("wuhuqifa") << endl;
// 	cout << master.getKey("wuhuqifs") << " " << master.showhashval("wuhuqifs") << endl;//
// 	cout << master.getKey("wuhuqifd") << " " << master.showhashval("wuhuqifd") << endl;
// 	cout << master.getKey("wuhuqiff") << " " << master.showhashval("wuhuqiff") << endl;
// 	cout << master.getKey("wuhuqifg") << " " << master.showhashval("wuhuqifg") << endl;
// 	cout << master.getKey("wuhuqifh") << " " << master.showhashval("wuhuqifh") << endl;
// 	cout << master.getKey("wuhuqifj") << " " << master.showhashval("wuhuqifj") << endl;
// 	cout << master.getKey("wuhuqifk") << " " << master.showhashval("wuhuqifk") << endl;
// 	cout << master.getKey("wuhuqifl") << " " << master.showhashval("wuhuqifl") << endl;
// 	cout << master.getKey("wuhuqifz") << " " << master.showhashval("wuhuqifz") << endl;
// 	cout << master.getKey("wuhuqifx") << " " << master.showhashval("wuhuqifx") << endl;
// 	cout << master.getKey("wuhuqifc") << " " << master.showhashval("wuhuqifc") << endl;
// 	cout << master.getKey("wuhuqifv") << " " << master.showhashval("wuhuqifv") << endl;
// 	cout << master.getKey("wuhuqifb") << " " << master.showhashval("wuhuqifb") << endl;
// 	cout << master.getKey("wuhuqifn") << " " << master.showhashval("wuhuqifn") << endl;

// 	master.expand("47.35.21.8 5005");
// 	master.show();
// 	cout << master.getKey("wuhuqifa") << " " << master.showhashval("wuhuqifa") << endl;
// 	cout << master.getKey("wuhuqifs") << " " << master.showhashval("wuhuqifs") << endl;//
// 	cout << master.getKey("wuhuqifd") << " " << master.showhashval("wuhuqifd") << endl;
// 	cout << master.getKey("wuhuqiff") << " " << master.showhashval("wuhuqiff") << endl;
// 	cout << master.getKey("wuhuqifg") << " " << master.showhashval("wuhuqifg") << endl;
// 	cout << master.getKey("wuhuqifh") << " " << master.showhashval("wuhuqifh") << endl;
// 	cout << master.getKey("wuhuqifj") << " " << master.showhashval("wuhuqifj") << endl;
// 	cout << master.getKey("wuhuqifk") << " " << master.showhashval("wuhuqifk") << endl;
// 	cout << master.getKey("wuhuqifl") << " " << master.showhashval("wuhuqifl") << endl;
// 	cout << master.getKey("wuhuqifz") << " " << master.showhashval("wuhuqifz") << endl;
// 	cout << master.getKey("wuhuqifx") << " " << master.showhashval("wuhuqifx") << endl;
// 	cout << master.getKey("wuhuqifc") << " " << master.showhashval("wuhuqifc") << endl;
// 	cout << master.getKey("wuhuqifv") << " " << master.showhashval("wuhuqifv") << endl;
// 	cout << master.getKey("wuhuqifb") << " " << master.showhashval("wuhuqifb") << endl;
// 	cout << master.getKey("wuhuqifn") << " " << master.showhashval("wuhuqifn") << endl;

//   	system("pause");
//     return 0;
//   }
