#ifndef THREADPOOL_HPP_
#define THREADPOOL_HPP_

#include <queue>
#include <mutex>
#include <functional>
#include <condition_variable>



class ThreadPool {
  class Worker {
    private:
    ThreadPool &m_tpool;

    public:
    Worker(ThreadPool& tp) : m_tpool{tp}
    {
    }

    void operator()() 
    {
      std::function<void()> job;
      while(m_tpool.m_running) {
        {
          std::unique_lock<std::mutex> lock{m_tpool.m_tasks_mutex};
          while(m_tpool.m_running && m_tpool.m_tasks.empty())
            m_tpool.m_cond_var.wait(lock);
          
          if(!m_tpool.m_running)
            return;

          job = m_tpool.m_tasks.front();
          m_tpool.m_tasks.pop();

        }
        job();
      }
    }
  };
  private:
  std::vector<std::thread> m_threads;
  std::queue<std::function<void()>> m_tasks;
  std::mutex m_tasks_mutex;
  std::condition_variable m_cond_var;
  bool m_running{true};

  public:
  ThreadPool(std::size_t threadCount = 1)
  {
    if(threadCount <= 0)
      throw std::invalid_argument("Thread count must be > 0");

    m_threads.reserve(threadCount);
    for(auto i{0u}; i!=threadCount; i++)
      m_threads.emplace_back(std::thread{Worker{*this}});

  }

  ~ThreadPool() {
    {
      std::lock_guard<std::mutex> lock{m_tasks_mutex};
      m_running = false;
    }
    m_cond_var.notify_all();
    for(auto &t : m_threads)
      t.join();
  }

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(const ThreadPool &) = delete;
  ThreadPool& operator=(ThreadPool &&) = delete;


  template<typename Task>
  void addTask(Task task)
  {
    {
      std::lock_guard<std::mutex> lock{m_tasks_mutex};
      m_tasks.emplace(std::function<void()>(task));
    }
    m_cond_var.notify_one();
  }
};

#endif
