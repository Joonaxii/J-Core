#include <J-Core/ThreadManager.h>
#include <mutex>
#include <J-Core/Log.h>
#include <GLFW/glfw3.h>

namespace JCore {
    namespace TaskManager {
        Task CURRENT_TASK{};
        TaskProgress PROGRESS{};
        std::atomic<bool> PROGRESS_DIRTY{};
        std::mutex PROGRESS_MUTEX{};
        GLContext TASK_CONTEXT{};
        std::function<void(void)> ON_COMPLETE{};

        GLContext& getTaskContext() {
            return TASK_CONTEXT;
        }

        const TaskProgress& getProgress() { return PROGRESS; }
        bool isDirty() { return PROGRESS_DIRTY.load(); }

        void markProgressNotDirty() {
            PROGRESS_DIRTY.store(false);
        }

        bool beginTask(std::function<void(void)> method, std::function<void(void)> onComplete) {
            if (CURRENT_TASK.isRunning()) {
                JCORE_WARN("[J-Core - TaskManager] Task already in progress! Couldn't being a new task!");
                return false;
            }
            ON_COMPLETE = onComplete;
            PROGRESS.clear();
            PROGRESS_DIRTY.store(true);
            CURRENT_TASK.run(method, false);
            return true;
        }

        bool cancelCurTask()  {
            if (!CURRENT_TASK.isRunning()) {
                JCORE_WARN("[J-Core - TaskManager] No task in progress, didn't cancel anything.");
                return false;
            }
            CURRENT_TASK.cancel();
            return true;
        }

        void updateTask() {
            if (CURRENT_TASK.flags & THREAD_FLAG_DONE) {
                CURRENT_TASK.join();
                CURRENT_TASK.flags &= ~THREAD_FLAG_DONE;
                if (ON_COMPLETE) {
                    ON_COMPLETE();
                    ON_COMPLETE = nullptr;
                }
            }
        }

        const Task& getCurrentTask() { return CURRENT_TASK; }
        void reportProgress(const TaskProgress& progress, uint8_t copyFlags) {
            if (PROGRESS_MUTEX.try_lock()) {
                PROGRESS_DIRTY.store(true);
                PROGRESS.copyFrom(progress, copyFlags);
                PROGRESS_MUTEX.unlock();
            }
        }
    }

    void Task::run(std::function<void(void)> method, bool requireOpenGL) {
        join();

        flags = THREAD_FLAG_RUNNING | (requireOpenGL ? THREAD_FLAG_OPENGL : 0);
        initAndRun(method);
    }

    void Task::cancel() {
        if ((flags & THREAD_FLAG_CANCEL) != 0) { return; }
        flags = (flags & ~THREAD_FLAG_STATE_MASK) | THREAD_FLAG_CANCEL;
        _thread.join();
        flags = 0;
    }

    void Task::join()  {
        if (_thread.joinable()) {
            _thread.join();
        }
    }

    void Task::initAndRun(std::function<void(void)> method) {
        _thread = std::thread([this, method]() {
            method();
            flags = (flags & ~THREAD_FLAG_RUNNING) | THREAD_FLAG_DONE;
            });
    }
}