//@Author: Lin Tao
//@Email: putaopu@qq.com

//�ڴ�أ�δ��ɣ�δʹ��
#pragma once
//���ͷ�ļ���������ڴ棬��Ĭ�ϵ��ڴ��������������ڴ�أ���Ҫ����ά��16�������������ڴ��ڴ棬ת�����õ�һ��������
#include <cstdlib> //�����ڴ�Ŀ⺯��malloc��free������

namespace LT {
	class alloc {
		//����һЩ����
	private:
		static const int ALIGN = 8;
		static const int MAXSIZE = 128;//
		static const int NUM = 16;//�����������
		static const int NODBJS= 20; //ÿ���������ӵĽڵ���

		//����һ��������Ϊֻ������ڵ�
		union obj {
			union obj* next;
			char client[1];
		};
		static obj* freeList[NUM];

		//�����ڴ��
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

		//�ú����������Сһ����8�ı��������ڷ���һ����СΪn�����飬
		//��ʱ���������������ӽڵ㡣
		static void* refill(size_t _n) {

		}

		//�ڿռ��㹻������£�����һ���������nobjs����СΪ_bytes������
		//����ռ䲻�㣬�����������ܴﲻ��Ҫ��
		static char* chunk_alloc(size_t _bytes, size_t& _nobjs) {
			char* result = 0;
			size_t totalBytes = _bytes * _nobjs;
			size_t left = endFree - startFree;

			if (left >= totalBytes) {//ʣ���ڴ����
				result = startFree;
				startFree += totalBytes;
				return result;
			}
			else if (left >= _nobjs) {
				//ʣ���ڴ�ռ��������һ���������ϣ������һ�������ϵ�����
				_nobjs = left / _bytes;
				result = startFree;
				startFree += _nobjs * _bytes;
				return result;
			}

			//���ˣ�˵����ǰ�ڴ��ʣ��ռ���һ�����鶼���䲻�����ˣ���Ҫ��취
			size_t needBytes = 2 * totalBytes + round_up(heapSize >> 4);
			if (left > 0) {
				obj** myFreeList = freeList + list_index(left);//����һ�������������ʣ�����һ���ռ� **ָ��ָ���ָ��
				((obj*)startFree)->next = *myFreeList;
				*myFreeList = (obj*)startFree;//��ʣ��Ŀռ���ӵ����ʵ���������β��
			}
			startFree = (char*)malloc(needBytes);//���䶨��ռ�
			if (!startFree) {
				//����ʧ��,����������
				obj** myFreeList = 0, * p = 0;
				for (int i = 0; i <= MAXSIZE; i += ALIGN)
				{
					//��ÿһ������������ȡ�����п���з���
					myFreeList = freeList + list_index(i);
					p = *myFreeList;
					if (p) {
						*myFreeList = p->next;
						startFree = (char*)p;
						endFree = startFree + 1;//�ּ�����һ��ռ�
						return chunk_alloc(_bytes, _nobjs);//���еݹ����
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


