#ifndef PROCESSING_TASK_QUEUE_H
#define PROCESSING_TASK_QUEUE_H

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <utility>
#include <optional>
#include <fmt/core.h>
#include <typeinfo>
#include <cxxabi.h>

class TaskQueue {
public:
    // Modes: debugMode for immediate execution, blockingMode for synchronous execution in worker thread
    enum class Mode {
        Debug,
        Blocking,
        Async // Default mode is Async (worker thread)
    };

    TaskQueue(Mode mode = Mode::Blocking) : mode(mode) {
        if (mode != Mode::Debug) {
            worker = std::thread(&TaskQueue::workerThread, this);
        }
    }

    ~TaskQueue() {
        if (mode != Mode::Debug) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                stop = true;
            }
            cv.notify_one();
            worker.join();
        }
        // TODO: If desturctore is run again just ignore?
        mode = Mode::Debug;
    }

    // Dispatch a task; depends on mode (debugMode, blockingMode, or asyncMode)
   template <typename Func, typename... Args>
   void dispatch(Func&& func, Args&&... args) {
      dispatch(mode, std::forward<Func>(func), std::forward<Args>(args)... );
   }

    // Dispatch a task; depends on mode (debugMode, blockingMode, or asyncMode)
   template <typename Func, typename... Args>
   void dispatch(Mode mode, Func&& func, Args&&... args) {
      // if current thread is already worker then abort
      if (std::this_thread::get_id() == worker.get_id())  {
         // int s;
         // fmt::print(stderr,"Warning: dispatch of {} from render thread, just calling direct.\n", abi::__cxa_demangle(typeid(func).name(), nullptr,nullptr, &s));
         std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
         return;
      }
      if (mode == Mode::Debug) {
         // Run the task in the current thread (debug mode)
         std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
      } else {
         // Run the task on the worker thread but block until it's complete (blocking mode)
         std::unique_lock<std::mutex> lock(mtx);
         cv.wait(lock, [this]() { return !task.has_value(); }); // Wait if a task is running
         task.emplace([f = std::forward<Func>(func), ...params = std::forward<Args>(args)] {
            std::invoke(f, params...);
         });
         cv.notify_one();

         if (mode == Mode::Blocking) {
            // Now wait for the task to complete
            cv.wait(lock, [this]() { return !task.has_value(); });
         }
      }
   }

private:
    std::optional<std::function<void()>> task;
    std::mutex mtx;
    std::condition_variable cv;
    std::thread worker;
    bool stop = false;
    Mode mode;

    void workerThread() {
        while (true) {
            std::function<void()> currentTask;
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this]() { return stop || task.has_value(); });

                if (stop) return;
                currentTask = std::move(*task);
                task.reset();
            }

            currentTask(); // Execute the task outside of the lock

            cv.notify_one();
        }
    }
};

extern TaskQueue renderThread;
#endif
