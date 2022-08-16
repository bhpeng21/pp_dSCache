/*
 * TcpClient.h - client客户端类定义
 * 作者：徐轶哲 
 * 版本 1.0
 * 2021/12/17
 */
#include "LRU.h"

// 默认构造
LRUCache::LRUCache()
{

}


// 有参构造
LRUCache::LRUCache(int iMaxSize)
{
	m_iMaxSize = iMaxSize;
}

// 功能：判断双向链表中是否有待查询key
//输入：
//  待查询string字符串key
//输出：
//  返回是否存在
bool LRUCache::Contain(string& key)
{
	auto iterFind = m_mapPair.find(key);
	if (iterFind == m_mapPair.end())
		return false;
	return true;
}

//功能：获取key对应value值
//输入：
//  待查询的string类型key值，要存储的string类型value值
//输出：
//  返回是否获取成功
bool LRUCache::Get(string& key, string& val)
{
	auto iterFind = m_mapPair.find(key);
	if (iterFind == m_mapPair.end())
		return false;

	val = iterFind->second->second;

	//	访问后移至链表头部
	auto iterList = iterFind->second->first;
	m_listLru.erase(iterList);
	m_listLru.push_front(iterFind->first);
	iterFind->second->first = m_listLru.begin();

	return true;
}

//功能：将K-V对插入到双向链表中
//输入：
//  待插入的string类型key值，待插入的string类型value值
//输出：
//  返回是否插入成功
bool LRUCache::Set(string key, string val)
{
	if (Contain(key))
	{
		auto iterFind = m_mapPair.find(key);
		iterFind->second->second = val;
		auto iterList = iterFind->second->first;
		m_listLru.erase(iterList);
		m_listLru.push_front(iterFind->first);
		iterFind->second->first = m_listLru.begin();
		//Move2Head(szKey);
		//cout << "Modify the value of key:" << szKey << endl;
		return true;
	}

	//	在链表的头部插入
	m_listLru.push_front(key);
	auto iterFront = m_listLru.begin();
	PairPtr pairPtr = make_shared<MPair>(iterFront, val);
	m_mapPair.insert(make_pair(key, pairPtr));

	//	判断缓存容量是否超过最大值
	if (m_listLru.size() > m_iMaxSize)
	{
		//	移除最久未被访问元素
		auto myKey = m_listLru.back();
		this->Remove(myKey);
	}

	return true;
}

//功能：移除最久未被访问的K-V对
//输入：
//  待删除K-V对中的string类型key值
//输出：
//  返回是否删除成功
bool LRUCache::Remove(string& key)
{
	auto iterFind = m_mapPair.find(key);
	if (iterFind == m_mapPair.end()) return false;
	auto iterList = iterFind->second->first;
	m_listLru.erase(iterList);
	m_mapPair.erase(iterFind);

	//cout << "Remove key:" << szKey << endl;

	return true;
}
