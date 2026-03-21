#pragma once
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>

// 有界线程安全队列，用于媒体帧与数据包跨线程传递
template <typename T>
class ThreadSafeQueue
{
public:
    explicit ThreadSafeQueue(int maxSize = 30) : m_maxSize(maxSize) {}

    void enqueue(const T &item)
    {
        QMutexLocker locker(&m_mutex);
        if (m_queue.size() >= m_maxSize)
            m_queue.dequeue(); // 丢弃最旧帧，防止积压
        m_queue.enqueue(item);
        m_cond.wakeOne();
    }

    bool dequeue(T &item, unsigned long timeoutMs = 100)
    {
        QMutexLocker locker(&m_mutex);
        if (m_queue.isEmpty())
            m_cond.wait(&m_mutex, timeoutMs);
        if (m_queue.isEmpty())
            return false;
        item = m_queue.dequeue();
        return true;
    }

    int size() const
    {
        QMutexLocker locker(&m_mutex);
        return m_queue.size();
    }

private:
    mutable QMutex  m_mutex;
    QWaitCondition  m_cond;
    QQueue<T>      m_queue;
    int             m_maxSize;
};
