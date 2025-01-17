#include "threadpool.h"
#include <QDebug>

ThreadPool::ThreadPool(int threadCount, QObject *parent)
    : QObject(parent), stop(false) {
    for (int i = 0; i < threadCount; ++i) {
        QThread *thread = new QThread(this);
        connect(thread, &QThread::started, [this]() { workerThread(); });
        threads.push_back(thread);
        thread->start();
    }
}


ThreadPool::~ThreadPool() {
    {
        QMutexLocker locker(&mutex);
        stop = true;
        condition.wakeAll();
    }

    for (QThread *thread : threads) {
        thread->quit();
        thread->wait();
        delete thread;
    }
}

void ThreadPool::enqueueTask(const std::function<void()>& task) {
    {
        QMutexLocker locker(&mutex);
        taskQueue.enqueue(task);
        qDebug() << "Task adăugat în coadă. Număr de task-uri în coadă:" << taskQueue.size();
    }
    condition.wakeOne();
}

void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task;
        {
            QMutexLocker locker(&mutex);
            if (stop && taskQueue.isEmpty()) break;

            if (taskQueue.isEmpty()) {
                qDebug() << "Thread-ul așteaptă noi task-uri...";
                condition.wait(&mutex);
                continue;
            }

            task = taskQueue.dequeue();
            qDebug() << "Executăm un task în thread-ul:" << QThread::currentThread();
        }

        try {
            task();
        } catch (const std::exception &e) {
            qDebug() << "Eroare în task:" << e.what();
        } catch (...) {
            qDebug() << "Eroare necunoscută în task.";
        }
    }
}

