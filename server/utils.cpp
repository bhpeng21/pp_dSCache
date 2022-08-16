/*
 * TcpClient.cpp - client客户端类成员函数实现
 * 作者：徐轶哲 
 * 版本 1.0
 * 2021/12/17
 */
#include "utils.h"

//功能：判断输入字符串是否为正确格式
//输入：
//  待检查string类型字符串s
//输出：
//  返回输入字符串是否为正确格式
bool IsCommand(const string s) 
{
	//cout << "输入字符串为：" << s << endl;
	if (s.size() < 8) return false; // set或get占3个字符 两个空格占2个字符 k-v至少为4个字符
	if (s[3] != ' ') return false;

	string str_command = s.substr(0, 3);
	
	if (str_command == "set")
	{
		if(s.size()!=13) return false; // 3+1+4+1+4
		if(s[3]==' '&&s[8]==' ') return true;
		else return false;
	}
	else if (str_command == "get")
	{
		
		if (s.find(' ') != 3 || s.size()!=8)
		{
			return false;
		}
		else return true;
	}
	
}

//功能：根据字符串提取命令
//输入：
//  提取字符串前3个字符
//输出：
//  指定字符串前3个字符
string GetCommand(const string s)
{
	return s.substr(0, 3);
}


//功能：根据字符串提取KV信息
//输入：
//  输入string类型字符串命令s, 待存储的key值， 待存储的val值
//输出：
//  无
void GetKV(const string s, string& key, string& val)
{
	string str_command = s.substr(0, 3);

	if (str_command == "set")
	{
		key = s.substr(4, 4);  // 这里默认key的长度为4
		val = s.substr(9, 4);  // 这里默认val的长度为4
	}
	else if (str_command == "get")
	{
		// key = s.substr(4, s.length() - 4);
		key = s.substr(4, 4);  // 这里默认key的长度为4
	}
}

//功能：处理来自客户端字符串数据包
//输入：
//  输入string类型字符串命令s, cache节点
//输出：
//  string类型处理结果
string PreClient(const string s, LRUCache& cache)
{
	string res;
	string key,val;
	if(!IsCommand(s))
	{
		// strcpy(sendbuffer, "发送字符串数据包格式错误，请重新发送");
		res = "发送字符串数据包格式错误，请重新发送";
	}
	else
	{
		if (GetCommand(s) == "set")
		{
			GetKV(s, key, val);
			if (!cache.Set(key, val))
			{
				res = "加入到缓存中失败！";
			}
			else
			{
				// cout << "加入到缓存成功！" << endl;
				cache.Set(key, val);
				res = "加入到缓存中成功！";
			}
			
		}
		else if (GetCommand(s) == "get")
		{
			GetKV(s, key, val);
			if (cache.Get(key, val))
			{
				res = "获取成功，值为：" + val;

			}
			else
			{
				res = "获取失败！";
			}
		}
	}
	return res;
}

//功能：处理来自Master端字符串数据包
//输入：
//  输入string类型字符串命令s, cache节点，client端socket类
//输出：
//  string类型处理结果
string PreMaster(const string s, LRUCache& cache, CTcpClient& TcpClient)
{
	char buf[1024];
	string strKV = "updateCache";
	string key;
	string val;
	string res;
	int num = (s.size()-26)/4; // k-v对中每个k/v长度为4
	for(int i=0;i<num;i++)
	{
		key = s.substr(26+i*4, 4); // 关键字前字符串长度为：expand/narrow 6 + ip地址13 + 端口号4 + 3个空格= 26
		if(cache.Get(key, val))
		{
			strKV = strKV + key + val;
			// 删除当前存储的KV对
			cache.Remove(key);
		}
	}
	strcpy(buf, strKV.c_str());
	if (write(TcpClient.m_sockfd,buf,strlen(buf)) <=0)
    { 
      printf("write() failed.\n");
	  res = "cache发送给另一个节点kV键值信息失败！";
	  close(TcpClient.m_sockfd);
	  return res;
    }
		
    memset(buf,0,sizeof(buf));
    if (read(TcpClient.m_sockfd,buf,sizeof(buf)) <=0) 
    { 
      printf("read() failed.\n");
	  res = "另一个节点kV返回cache确定信息失败！";
	  close(TcpClient.m_sockfd);
	  return res;
    }
	res = "扩容缩容成功！";
    // printf("recv:%s\n",buf);
	close(TcpClient.m_sockfd);
	return res;
}

//功能：处理来自cache server端字符串数据包
//输入：
//  输入string类型字符串命令s, cache节点
//输出：
//  string类型处理结果
string PreCacheServer(const string s, LRUCache& cache) // 输入的s格式比如为 updateCachekey1val1key2val2
{
	string res, key, val;
	int num = (s.size()-11)/8; // 11为updateCache字符串长度 8为k-v对长度 
	for(int i=0;i<num;i++)
	{
		key = s.substr(11+i*8, 4); // 提取出相应的Key值
		val = s.substr(15+i*8, 4); // 提取出相应的val值
		if (!cache.Set(key, val))
		{
			res = "加入到缓存中失败！";
		}
		else
		{
			// cout << "加入到缓存成功！" << endl;
			cache.Set(key, val);
			res = "加入到缓存中成功！";
		}
	}
	return res;
}

