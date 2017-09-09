#ifndef __STL_VECTOR__
#define __STL_VECTOR__

#include <new>

namespace std
{

template <typename T, unsigned int N>
class vector
{
	public:
	vector (void) : end_ (begin_) {}

	public:
	typedef T* iterator;
	typedef const T* const_iterator;

	public:
	void push_back (const T& o) { new (end_++) T (o); }
	void pop_back (void) {delete (--end_);}

	public:
	iterator begin (void) {return begin_;}
	iterator end   (void) {return end_;  }
	unsigned int size (void) const {return end() - begin();}
	bool empty (void) const {return (end() == begin());}

	public:
	const_iterator begin (void) const {return begin_;}
	const_iterator end   (void) const {return end_;  }

	private:
	T begin_[N];
	T* end_;
};

};

#endif
