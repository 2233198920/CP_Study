
#ifndef _MARK_MPSC_QUEUE_H
#define _MARK_MPSC_QUEUE_H

#include <atomic>
#include <utility>

template<typename T>
class MPSCQueueNonIntrusive
{
public:
    MPSCQueueNonIntrusive() : _head(new Node()), _tail(_head.load(std::memory_order_relaxed))
    {
        Node* front = _head.load(std::memory_order_relaxed);
        front->Next.store(nullptr, std::memory_order_relaxed);
    }

    ~MPSCQueueNonIntrusive()
    {
        T* output;
        while (Dequeue(output))
            delete output;

        Node* front = _head.load(std::memory_order_relaxed);
        delete front;
    }

// wait-free
    void Enqueue(T* input)
    {
        Node* node = new Node(input);
        // preHead is the old head
        // head is the new head
        Node* prevHead = _head.exchange(node, std::memory_order_acq_rel);
        prevHead->Next.store(node, std::memory_order_release);
    }

    bool Dequeue(T*& result)
    {
        Node* tail = _tail.load(std::memory_order_relaxed);
        Node* next = tail->Next.load(std::memory_order_acquire);
        if (!next)
            return false;

        result = next->Data;
        _tail.store(next, std::memory_order_release);
        delete tail;
        return true;
    }

private:
    struct Node
    {
        Node() = default;
        explicit Node(T* data) : Data(data)
        {
            Next.store(nullptr, std::memory_order_relaxed);
        }

        T* Data;
        std::atomic<Node*> Next;
    };

    std::atomic<Node*> _head;
    std::atomic<Node*> _tail;

    MPSCQueueNonIntrusive(MPSCQueueNonIntrusive const&) = delete;
    MPSCQueueNonIntrusive& operator=(MPSCQueueNonIntrusive const&) = delete;
};

template<typename T, std::atomic<T*> T::* IntrusiveLink>
class MPSCQueueIntrusive
{
public:
    MPSCQueueIntrusive() : _dummyPtr(reinterpret_cast<T*>(std::addressof(_dummy))), _head(_dummyPtr), _tail(_dummyPtr)
    {
        // _dummy is constructed from aligned_storage and is intentionally left uninitialized (it might not be default constructible)
        // so we init only its IntrusiveLink here
        std::atomic<T*>* dummyNext = new (&(_dummyPtr->*IntrusiveLink)) std::atomic<T*>();
        dummyNext->store(nullptr, std::memory_order_relaxed);
    }

    ~MPSCQueueIntrusive()
    {
        T* output;
        while (Dequeue(output))
            delete output;
    }

    void Enqueue(T* input)
    {
        (input->*IntrusiveLink).store(nullptr, std::memory_order_release);
        T* prevHead = _head.exchange(input, std::memory_order_acq_rel);
        (prevHead->*IntrusiveLink).store(input, std::memory_order_release);
    }

    bool Dequeue(T*& result)
    {
        T* tail = _tail.load(std::memory_order_relaxed);
        T* next = (tail->*IntrusiveLink).load(std::memory_order_acquire);
        if (tail == _dummyPtr)
        {
            
            if (!next)
                return false;

            _tail.store(next, std::memory_order_release);
            tail = next;
            next = (next->*IntrusiveLink).load(std::memory_order_acquire);
        }

        if (next)
        {
            _tail.store(next, std::memory_order_release);
            result = tail;
            return true;
        }

        T* head = _head.load(std::memory_order_acquire);
        if (tail != head)
            return false;

        Enqueue(_dummyPtr);
        next = (tail->*IntrusiveLink).load(std::memory_order_acquire);
        if (next)
        {
            _tail.store(next, std::memory_order_release);
            result = tail;
            return true;
        }
        return false;
    }

private:
    std::aligned_storage_t<sizeof(T), alignof(T)> _dummy;
    T* _dummyPtr;
    std::atomic<T*> _head;
    std::atomic<T*> _tail;

    MPSCQueueIntrusive(MPSCQueueIntrusive const&) = delete;
    MPSCQueueIntrusive& operator=(MPSCQueueIntrusive const&) = delete;
};

template<typename T, std::atomic<T*> T::* IntrusiveLink = nullptr>
using MPSCQueue = std::conditional_t<IntrusiveLink != nullptr, MPSCQueueIntrusive<T, IntrusiveLink>, MPSCQueueNonIntrusive<T>>;

#endif // MPSCQueue_h__
