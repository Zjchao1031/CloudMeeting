#pragma once
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>

/**
 * @file ThreadSafeQueue.h
 * @brief 声明有界线程安全队列模板。
 */

/**
 * @class ThreadSafeQueue
 * @brief 在线程间安全传递数据的有界队列。
 * @tparam T 队列中存储的数据类型。
 */
template <typename T>
class ThreadSafeQueue
{
public:
    /**
     * @brief 构造线程安全队列。
     * @param[in] maxSize 队列允许保存的最大元素数量。
     */
    explicit ThreadSafeQueue(int maxSize = 30) : m_maxSize(maxSize) {}

    /**
     * @brief 向队列尾部插入一个元素。
     * @param[in] item 待插入的元素。
     * @note 当队列已满时，会丢弃最早插入的元素以避免积压。
     */
    void enqueue(const T &item)
    {
        QMutexLocker locker(&m_mutex);
        if (m_queue.size() >= m_maxSize)
            m_queue.dequeue(); // 丢弃最旧元素，防止积压
        m_queue.enqueue(item);
        m_cond.wakeOne();
    }

    /**
     * @brief 从队列头部取出一个元素。
     * @param[out] item 接收出队元素的变量。
     * @param[in] timeoutMs 等待队列可读的超时时间，单位：毫秒。
     * @return 若成功取出元素则返回 `true`，否则返回 `false`。
     */
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

    /**
     * @brief 获取当前队列中的元素数量。
     * @return 当前队列长度。
     */
    int size() const
    {
        QMutexLocker locker(&m_mutex);
        return m_queue.size();
    }

private:
    mutable QMutex  m_mutex;   ///< 保护队列访问的互斥量。
    QWaitCondition  m_cond;    ///< 用于等待新数据到达的条件变量。
    QQueue<T>       m_queue;   ///< 实际存储队列元素的容器。
    int             m_maxSize; ///< 队列允许保存的最大元素数量。
};
