#pragma once 
#include <cstdint>
#include <thread>
#include <atomic>
#include <functional>
#include <J-Core/Rendering/Window.h>
#include <cstdarg>

namespace JCore {

    enum : uint32_t {
        THREAD_FLAG_CANCEL    = 0x1,
        THREAD_FLAG_RUNNING   = 0x2,
        THREAD_FLAG_DONE      = 0x4,

        THREAD_FLAG_STATE_MASK = 0xF,

        THREAD_FLAG_OPENGL    = 0x1000,
    };

    enum : uint32_t {
        TASK_COPY_TITLE = 0x1,
        TASK_COPY_MESSAGE = 0x2,
        TASK_COPY_SUBMESSAGE = 0x4,
        TASK_COPY_PROGRESS = 0x8,
        TASK_COPY_SUBPROGRESS = 0x10,
        TASK_COPY_FLAGS = 0x20,
        TASK_COPY_TARGET_PROGRESS = 0x20,
    };

    struct TaskProgress {

        enum : uint8_t {
            HAS_SUB_TASK = 0x1,
            HAS_SUB_RELATIVE = 0x2,
        };

        char title[64]{ 0 };
        char message[64]{ 0 };
        char subMessage[64]{ 0 };
        float progress{0};
        float targetProg{0};
        float subProgress{0};
        uint8_t flags{0};

        TaskProgress() : title{ 0 }, message{ 0 }, subMessage{0}, progress(0), subProgress(0), flags(0) {}

        TaskProgress(const char* title, const char* message) {
            size_t len = std::min<size_t>(strlen(title), 63);
            memcpy(this->title, title, len);

            len = std::min<size_t>(strlen(message), 63);
            memcpy(this->message, message, len);
        }

        TaskProgress(const char* title, const char* message, const char* subMessage) {
            size_t len = std::min<size_t>(strlen(title), 63);
            memcpy(this->title, title, len);

            len = std::min<size_t>(strlen(message), 63);
            memcpy(this->message, message, len);

            len = std::min<size_t>(strlen(subMessage), 63);
            memcpy(this->subMessage, subMessage, len);
        }

        float getProgress() const {
            if ((flags & (HAS_SUB_RELATIVE | HAS_SUB_TASK)) == (HAS_SUB_RELATIVE | HAS_SUB_TASK)) {
                return progress + (targetProg * subProgress);
            }
            return progress;
        }

        void setSubMessage(const char* fmt, ...) {
            va_list args;
            va_start(args, fmt);
            vsprintf_s(subMessage, fmt, args);
            va_end(args);
        }

        void setMessage(const char* fmt, ...) {
            va_list args;
            va_start(args, fmt);
            vsprintf_s(message, fmt, args);
            va_end(args);
        }

        void setTitle(const char* fmt, ...) {
            va_list args;
            va_start(args, fmt);
            vsprintf_s(title, fmt, args);
            va_end(args);
        }

        void copyFrom(const TaskProgress& other, uint8_t copyFlags) {
            if (copyFlags & TASK_COPY_TITLE) {
                memcpy(title, other.title, sizeof(title));
            }
                        
            if (copyFlags & TASK_COPY_MESSAGE) {
                memcpy(message, other.message, sizeof(message));
            }

            if (copyFlags & TASK_COPY_SUBMESSAGE) {
                memcpy(subMessage, other.subMessage, sizeof(subMessage));
            }

            if (copyFlags & TASK_COPY_SUBPROGRESS) {
                subProgress = other.subProgress;
            }

            if (copyFlags & TASK_COPY_PROGRESS) {
                progress = other.progress;
            }  
                  
            if (copyFlags & TASK_COPY_TARGET_PROGRESS) {
                targetProg = other.targetProg;
            }  
            
            if (copyFlags & TASK_COPY_FLAGS) {
                flags = other.flags;
            }
        }

        void clear() {
            flags = 0;
            progress = 0;
            subProgress = 0;
            targetProg = 0;
            title[0] = 0;
            message[0] = 0;
            subMessage[0] = 0;
        }
    };

    class Task {
    public:
        uint32_t flags{0};
        TaskProgress progress{};
        ~Task() {
            if (_thread.joinable()) {
                _thread.join();
            }
            _thread = std::thread();
        }

        void run(std::function<void(void)> method, bool requireOpenGL);
        void cancel();
        void join();

        bool isRunning() const { return bool(flags & THREAD_FLAG_RUNNING); }
        bool shouldCancel() const { return bool(flags & THREAD_FLAG_CANCEL); }
    private:
        std::thread _thread;

        void initAndRun(std::function<void(void)> method);
    };

    namespace TaskManager {
        const TaskProgress& getProgress();
        bool isDirty();
        void markProgressNotDirty();
        GLContext& getTaskContext();

        bool beginTask(std::function<void(void)> method, std::function<void(void)> onComplete);
        bool cancelCurTask();

        void updateTask();

        void reportProgress(const TaskProgress& progress, uint8_t copyFlags = 0xFF);
        const Task& getCurrentTask();
    }
}