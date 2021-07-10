//@Author: Lin Tao
//@Email: putaopu@qq.com

//内存池：未完成，未使用
#pragma once
//这个头文件负责分配内存，是默认的内存配置器。管理内存池，主要用于维护16个自由链表。对于大内存，转而调用第一级配置器
#include <cstdlib> //分配内存的库函数malloc和free在这里

namespace LT {
	class alloc {
		//设置一些常量
	private:
		static const int ALIGN = 8;
		static const int MAXSIZE = 128;//
		static const int NUM = 16;//自由链表个数
		static const int NODBJS= 20; //每次扩容增加的节点数

		//定义一个联合作为只有链表节点
		union obj {
			union obj* next;
			char client[1];
		};
		static obj* freeList[NUM];

		//构造内存池
		static char* startFree;
		static char* endFree;
		static size_t heapSize;


		//padding
		static size_t round_up(size_t _bytes) {
			if (_bytes % ALIGN) {
				return ALIGN + (_bytes / ALIGN) * ALIGN;
			} else {
				return _bytes;
			}
		}

		static size_t list_index(size_t _bytes) {
			return (_bytes / ALIGN) - 1;
		}

		//该函数的输入大小一定是8的倍数，用于返回一个大小为n的区块，
		//有时候会给自由链表增加节点。
		static void* refill(size_t _n) {

		}

		//在空间足够的情况下，配置一块可以容纳nobjs个大小为_bytes的区块
		//如果空间不足，分配的区块可能达不到要求
		static char* chunk_alloc(size_t _bytes, size_t& _nobjs) {
			char* result = 0;
			size_t totalBytes = _bytes * _nobjs;
			size_t left = endFree - startFree;

			if (left >= totalBytes) {//剩余内存充足
				result = startFree;
				startFree += totalBytes;
				return result;
			}
			else if (left >= _nobjs) {
				//剩余内存空间可以满足一个区块以上，则分配一个及以上的区块
				_nobjs = left / _bytes;
				result = startFree;
				startFree += _nobjs * _bytes;
				return result;
			}

			//到此，说明当前内存池剩余空间连一个区块都分配不出来了，需要想办法
			size_t needBytes = 2 * totalBytes + round_up(heapSize >> 4);
			if (left > 0) {
				obj** myFreeList = freeList + list_index(left);//总有一个区块可以容纳剩余的这一点点空间 **指向指针的指针
				((obj*)startFree)->next = *myFreeList;
				*myFreeList = (obj*)startFree;//把剩余的空间添加到合适的自由链表尾端
			}
			startFree = (char*)malloc(needBytes);//分配定额空间
			if (!startFree) {
				//分配失败,尽量分配别的
				obj** myFreeList = 0, * p = 0;
				for (int i = 0; i <= MAXSIZE; i += ALIGN)
				{
					//从每一个自由链表中取出空闲块进行分配
					myFreeList = freeList + list_index(i);
					p = *myFreeList;
					if (p) {
						*myFreeList = p->next;
						startFree = (char*)p;
						endFree = startFree + 1;//又挤出了一点空间
						return chunk_alloc(_bytes, _nobjs);//进行递归分配
					}
				}
				endFree = 0;
			}
			heapSize += needBytes;
			endFree = startFree + needBytes;
			return chunk_alloc(_bytes, _nobjs);
		}

	};
}


