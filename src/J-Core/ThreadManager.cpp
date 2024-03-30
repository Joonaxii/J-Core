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

        bool isSkipping() {
            return CURRENT_TASK.isRunning() && (CURRENT_TASK.flags & THREAD_FLAG_SKIP) != 0;
        }

        bool markForSkip() {
            if (CURRENT_TASK.isRunning()) {
                if (!PROGRESS_MUTEX.try_lock()) { return false; }
                CURRENT_TASK.flags |= THREAD_FLAG_SKIP;
                PROGRESS_MUTEX.unlock();
                return true;
            }
            return false;
        }
        bool performSkip() {
            if (CURRENT_TASK.isRunning() && (CURRENT_TASK.flags & THREAD_FLAG_SKIP)) {
                if (!PROGRESS_MUTEX.try_lock()) { return false; }
                CURRENT_TASK.flags &= ~THREAD_FLAG_SKIP;
                PROGRESS_MUTEX.unlock();
                return true;
            }
            return false;
        }

        void clearPreview() {
            CURRENT_TASK.showPreview = false;
        }

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

        bool isCancelling() {
            return CURRENT_TASK.isRunning() && (CURRENT_TASK.flags & THREAD_FLAG_CANCEL);
        }

        bool cancelCurTask()  {
            if (!CURRENT_TASK.isRunning()) {
                JCORE_WARN("[J-Core - TaskManager] No task in progress, didn't cancel anything.");
                return false;
            }
            CURRENT_TASK.cancel();
            clearPreview();
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
            else if (CURRENT_TASK.flags & THREAD_FLAG_RUNNING) {
                if (PROGRESS_MUTEX.try_lock()) {
                    if (PROGRESS.preview) {
                        auto prevIm = PROGRESS.preview;
                        CURRENT_TASK.showPreview = CURRENT_TASK.previewTex->create(prevIm->data, prevIm->format, prevIm->paletteSize, prevIm->width, prevIm->height, prevIm->flags);
                        PROGRESS.preview = nullptr;
                    }
                    PROGRESS_MUTEX.unlock();
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
        showPreview = false;

        flags = THREAD_FLAG_RUNNING | (requireOpenGL ? THREAD_FLAG_OPENGL : 0);
        initAndRun(method);
    }

    void Task::cancel() {
        if ((flags & THREAD_FLAG_CANCEL) != 0) { return; }
        flags |= THREAD_FLAG_CANCEL;
    }

    void Task::join()  {
        if (_thread.joinable()) {
            _thread.join();
        }
    }

    void Task::initAndRun(std::function<void(void)> method) {
        _thread = std::thread([this, method]() {
            method();
            flags = (flags & ~(THREAD_FLAG_RUNNING | THREAD_FLAG_CANCEL | THREAD_FLAG_SKIP)) | THREAD_FLAG_DONE;
            });
    }
}