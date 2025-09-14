#ifndef INCLUDED_commonsense_singlelist_singlelist_H
#define INCLUDED_commonsense_singlelist_singlelist_H

#include <stddef.h>

namespace CommonSense {
    namespace SingleList {

// ----------------------------------------------
struct DefaultPolicy
{
    struct Allocator
    {
        static void* Alloc( size_t size ) { return new char[ size ]; }
        static void Free( void* ptr ) { delete [] (char *)ptr; }
    };
};

template <class T, class Policy>
class SingleList
{
    struct Node
    {
        typedef typename Policy::Allocator Allocator;
        Node(const T &x, Node *y = 0) : mData(x), mNext(y) {}
        T mData;
        Node *mNext;

        void *operator new(size_t s) { return Allocator::Alloc(s); }
        void operator delete(void *p, size_t s) { return Allocator::Free(p, s); }
    };

    Node *mHead;
public:

    class Iterator
    {
        Node *mRep;

    public:
        friend class ConstIterator;
        friend class SingleList;

        Iterator(Node *x = 0) : mRep(x){}
        Iterator(const Iterator& x) : mRep(x.mRep) {}
        Iterator& operator=(const Iterator& x)
        {
            mRep = x.mRep;
            return *this;
        }
        Iterator& operator++()
        {
            mRep = mRep->mNext;
            return *this;
        }
        Iterator operator++(int)
        {
            Iterator tmp(*this);
            mRep = mRep->mNext;
            return tmp;
        }
        T& operator*() const { return mRep->mData; }
        T* operator->() const { return mRep; }

        bool operator==(const Iterator& x) const
        {
            return mRep == x.mRep;
        }

        bool operator!=(const Iterator& x) const
        {
            return mRep != x.mRep;
        }
    };

    class ConstIterator
    {
        const Node* mRep;
    public:
        friend class Iterator;
        friend class SingleList;

        ConstIterator(const Node* x = 0) : mRep(x) {}
        ConstIterator(const ConstIterator& x) : mRep(x.mRep) {}
        ConstIterator(const Iterator& x) : mRep(x.mRep) {}
        ConstIterator& operator=(const ConstIterator& x)
        {
            mRep = x.mRep;
            return *this;
        }
        ConstIterator& operator=(const Iterator& x)
        {
            mRep = x.mRep;
            return *this;
        }
        ConstIterator& operator++()
        {
            mRep = mRep->mNext;
            return *this;
        }
        ConstIterator operator++(int)
        {
            ConstIterator tmp(*this);
            mRep = mRep->mNext;
            return tmp;
        }
        const T &operator*() const { return mRep->mData; }
        T *operator->() const { return mRep; }
        bool operator==(const ConstIterator& x) const
        {
            return mRep == x.mRep;
        }
        bool operator!=(const ConstIterator& x) const
        {
            return mRep != x.mRep;
        }
    };



    SingleList() : mHead(0) {}

    SingleList(const SingleList &l) : mHead(0)
    {
        for (ConstIterator i = l.Begin(); i != l.End(); ++i) PushFront(*i);
        Reverse();
    }

    void Reverse()
    {
        Node *p = 0; Node *i = mHead; Node *n;
        while (i)
        {
            n = i->mNext;
            i->mNext = p;
            p = i; i = n;
        }
        mHead = p;
    }

    void Swap(SingleList &x) { Node *tmp = mHead; mHead = x.mHead; x.mHead = tmp; }

    SingleList &operator=(const SingleList &x)
    {
        SingleList tmp(x);
        Swap(tmp);
        return *this;
    }

    ~SingleList() { Clear(); }
    void Clear() { while (!Empty()) PopFront(); }

    void PushFront(const T&x)
    {
        Node *tmp = new Node(x);
        tmp->mNext = mHead;
        mHead = tmp;
    }

    void PopFront()
    {
        if (mHead)
        {
            Node *newhead = mHead->mNext;
            delete mHead;
            mHead = newhead;
        }
    }

    bool Empty() { return mHead == 0; }

    Iterator Begin() { return Iterator(mHead); }
    Iterator End() { return Iterator(); }
    ConstIterator Begin() const { return mHead; }
    ConstIterator End() const { return ConstIterator(); }

    void EraseAfter(Iterator& x)
    {
        Node *tmp = x.mRep->mNext;
        if (x.mRep->mNext) x.mRep->mNext = x.mRep->mNext->mNext;
        delete tmp;
    }

    // Expensive: prefer EraseAfter
    void Erase(Iterator &x)
    {
        if (x.mRep == mHead)
        {
            PopFront();
        }
        else
        {
            Node *tmp = mHead;
            Node *kill = x.mRep;
            while (tmp && tmp->mNext != kill)
            {
                tmp = tmp->mNext;
            }
            // JP 11/21/2007 The Item that we want to delete could not be in the list, so we need to check if tmp is not null before linking the previous and next items
            if (tmp)
            {
                tmp->mNext = kill->mNext;
            }
            delete kill;
        }
    }

    void InsertAfter(Iterator& x, const T& y)
    {
        Node *tmp = new Node(y, x.mRep->mNext);
        x.mRep->mNext = tmp;
    }
};


    };
};

#endif
