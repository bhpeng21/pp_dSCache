/*
 * TcpClient.h - client客户端类定义
 * 作者：徐轶哲 
 * 版本 1.0
 * 2021/12/17
 */
#pragma once


#ifndef _LRU_H_
#define _LRU_H_
#include <iostream>
#include <unordered_map>
#include <list>
#include <boost/make_shared.hpp>

//boost::shared_ptr<Net<float> > net_;
using namespace std;

//template<typename string, typename string>
class LRUCache
{
private:
	int m_iMaxSize;
	list<string> m_listLru; // 保存Key的链表，维护访问顺序

	typedef pair<list<string>::iterator, string> MPair;
	typedef shared_ptr<MPair> PairPtr; //利用智能指针来避免内存泄漏
	//MPair* PairPtr;
	unordered_map<string, PairPtr> m_mapPair; // 保存Key在链表中的位置和Key对应的Value

public:

	// 默认构造
	LRUCache();

	// 有参构造
	LRUCache(int iMaxSize);

	// 判断双向链表中是否有待查询key
	bool Contain(string& key);

	// 获取key对应value值
	bool Get(string& key, string& val);

	// 更新或者将新K-V对插入到链表中
	bool Set(string key, string val);

	// 移除最久未被访问的K-V对
	bool Remove(string& key);

};

#endif