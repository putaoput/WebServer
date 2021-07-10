//@Author: Lin Tao
//@Email: putaopu@qq.com
#pragma once

//该头文件定义了堆的算法相关的函数

namespace LT {
	//--------------------------------------堆的相关的算法函数------------------------------------
	template<class RandomIterator, class Distance, class Comp>//该函数自上向下调整堆
	void __adjust(RandomIterator _itBegin, Distance _index, Distance _len, Comp _cmp) {
		int left = 2 * _index + 1;//左右子节点
		int right = 2 * _index + 2;

		int maxIdx = _index;
		if (left < _len && _cmp(*(_itBegin + left), *(_itBegin + maxIdx))) { maxIdx = left; }
		if (left < _len && _cmp(*(_itBegin + right), *(_itBegin + maxIdx))) { maxIdx = right; }
		if (maxIdx != _index) {
			iter_swap(_itBegin + _index, _itBegin + maxIdx);
			__adjust(_itBegin, maxIdx, _len, _cmp);
		}
	}

	template<class RandomIterator, class Comp>
	inline void __make_heap(RandomIterator _itBegin, RandomIterator _itEnd, Comp _cmp) {
		int len = _itEnd - _itBegin;
		for (int i = (_itEnd - _itBegin) / 2 - 1; i >= 0; --i) {
			LT::__adjust(_itBegin, i, len, _cmp);
		}
	}

	//push_heap
	//该函数提供堆尾压入一个元素之后调整堆。自下向上调整。
	template<class RandomIterator, class Distance, class T, class Comp>
	inline void __push_heap(RandomIterator _itBegin, Distance _holeIndex, Distance _topIndex, T _value, Comp _cmp) {
		//该函数是实际执行push_heap堆调整的函数
		Distance parent = (_holeIndex - 1) / 2; //holeIndex是压入的元素，找出该元素的父节点，然进行调整。
		while (_holeIndex < _topIndex && _cmp(*(_itBegin + parent), _value))//说明需要调整，不满足_cmp(child,parent) == true;
		{
			*(_itBegin + _holeIndex) = *(_itBegin + parent);
			_holeIndex = parent;
			parent = (_holeIndex - 1) / 2;
		}
		*(_itBegin + _holeIndex) = _value;//调整完成。
	}
	template<class RandomIterator, class Distance, class T, class Comp>
	inline void __push_heap_aux(RandomIterator _itBegin, RandomIterator _itEnd, Distance*, T*, Comp _cmp) {
		__push_heap(_itBegin, static_cast<Distance>(_itEnd - _itBegin - 1), Distance(0), T(*(_itEnd - 1)), _cmp);
	}

	template<class RandomIterator, class Comp>
	inline void push_heap(RandomIterator _itBegin, RandomIterator _itEnd, Comp _cmp) {
		__push_heap_aux(_itBegin, _itEnd, distance_type(_itBegin), value_type(_itBegin), _cmp);
	}


	//pop_heap
	//该函数提供pop一个元素之后，对堆的调整。自上向下
	template<class RandomIterator, class Distance, class Comp>
	inline void __pop_heap(RandomIterator _itBegin, Distance _holeIndex, Distance _tailIndex, Comp _cmp) {
		Distance left = _holeIndex * 2 + 1;
		Distance right = _holeIndex * 2 + 2;

		while (_holeIndex <= (_tailIndex - 1) / 2) {
			Distance tmpIndex = _holeIndex;
			if (left <= _tailIndex && _cmp(*(_itBegin + tmpIndex), *(_itBegin + left))) {
				tmpIndex = left;
			}
			if (right <= _tailIndex && _cmp(*(_itBegin + tmpIndex), *(_itBegin + right))) {
				tmpIndex = right;
			}
			if (tmpIndex == _holeIndex) {
				break;
			}
			else {
				LT::iter_swap(_itBegin + tmpIndex, _itBegin + _holeIndex);
				_holeIndex = tmpIndex;
			}
		}
	}
	template<class RandomIterator, class Distance, class T, class Comp>
	inline void __pop_heap_aux(RandomIterator _itBegin, RandomIterator _itEnd, Distance*, Comp _cmp) {
		__pop_heap(_itBegin, static_cast<Distance>(0), static_cast<Distance>(_itEnd - _itBegin - 1), _cmp);
	}

	template<class RandomIterator, class Comp >
	inline void pop_heap(RandomIterator _itBegin, RandomIterator _itEnd, Comp _cmp) {
		__pop_heap_aux(_itBegin, _itEnd, distance_type(*_itBegin), _cmp);
	}

}