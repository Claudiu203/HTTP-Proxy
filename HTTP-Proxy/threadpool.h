#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <functional>

class ThreadPool : public QObject {
    Q_OBJECT
public:
    explicit ThreadPool(int threadCount, QObject *parent = nullptr);
    ~ThreadPool();

    void enqueueTask(const std::function<void()>& task);

private:
    QVector<QThread*> threads;
    QQueue<std::function<void()>> taskQueue;
    QMutex mutex;
    QWaitCondition condition;
    bool stop;

    void workerThread();
};

#endif // THREADPOOL_H
