#pragma once 
#include <cstdint>
#include <thread>
#include <atomic>
#include <functional>
#include <J-Core/Rendering/Window.h>
#include <cstdarg>
#include <J-Core/Util/EnumUtils.h>

namespace JCore {
    enum : uint32_t {
        THREAD_FLAG_CANCEL = 0x1,
        THREAD_FLAG_RUNNING = 0x2,
        THREAD_FLAG_DONE = 0x4,

        THREAD_FLAG_STATE_MASK = 0xF,

        THREAD_FLAG_OPENGL = 0x1000,
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
    enum TaskProgressType : uint8_t {
        PROG_ValueFloat,
        PROG_ValueInt,

        PROG_RangeFloat,
        PROG_RangeInt,

        PROG_IsStepped = 0x20,
        PROG_ShowRange = 0x40,
        PROG_IsInclusive = 0x80,

        PROG_VALUE_MASK = 0x1F,
        PROG_FLAG_MASK = 0xE0,
    };

    struct TaskProgress {
        struct ProgressValue {
            TaskProgressType type[2];

            union Data {
                uint32_t uintV;
                float floatV;
            } value[2];

            Data step[2];

            static float getNormalized(const Data data[2], TaskProgressType pType) {
                switch (pType & PROG_VALUE_MASK) {
                    case PROG_ValueFloat:
                        return data[0].floatV;
                    case PROG_RangeInt: {
                        float max = std::max((pType & PROG_IsInclusive) ? float(data[1].uintV) : float(data[1].uintV) - 1.0f, 1.0f);
                        return float(data[0].uintV) / max;
                    }
                    case PROG_RangeFloat: {
                        float max = std::max((pType & PROG_IsInclusive) ? data[1].floatV : data[1].floatV - 1.0f, 1.0f);
                        return data[0].floatV / max;
                    }
                }
                return 0;
            }

            float getNormalized() const {
                bool isStepped = bool(type[0] & PROG_IsStepped);

                float mainP = getNormalized(value, type[0]);
                return isStepped ? mainP * getNormalized(step, type[1]) : mainP;
            }

            void clear() {
                type[0] = PROG_ValueFloat;
                type[1] = PROG_ValueFloat;
                value[0].uintV = 0;
                value[1].uintV = 0;
            }

            uint32_t getValueI(int32_t t) const { return value[t & 0x1].uintV; }
            float getValueF(int32_t t)  const { return value[t & 0x1].floatV; }

            constexpr bool isFloat() const { return (type[0] & PROG_VALUE_MASK) == PROG_ValueFloat || (type[0] & PROG_VALUE_MASK) == PROG_RangeFloat; }
            constexpr bool isRange() const { return (type[0] & PROG_VALUE_MASK) == PROG_RangeInt || (type[0] & PROG_VALUE_MASK) == PROG_RangeFloat; }

            void setType(int32_t tI, TaskProgressType type, TaskProgressType flags) {
                this->type[tI & 0x1] = TaskProgressType((type & PROG_VALUE_MASK) | (flags & PROG_FLAG_MASK));
            }

            void setType(TaskProgressType type, int32_t tI) {
                this->type[tI & 0x1] = TaskProgressType((type & PROG_VALUE_MASK) | (this->type[tI & 0x1] & PROG_FLAG_MASK));
            }

            void setFlags(int32_t tI, TaskProgressType flags, bool state) {
                flags = TaskProgressType(flags & PROG_FLAG_MASK);
                type[tI & 0x1] = TaskProgressType((type[tI & 0x1] & ~flags) | TaskProgressType(state ? flags : 0x00));
            }

            void setProgress(float value) {
                this->value[0].floatV = value;
            }

            void setProgress(uint32_t value) {
                this->value[0].uintV = value;
            }

            void setProgress(float min, float max) {
                value[0].floatV = min;
                value[1].floatV = max;
            }

            void setProgress(uint32_t min, uint32_t max) {
                value[0].uintV = min;
                value[1].uintV = max;
            }

            void setStep(float value) {
                this->step[0].floatV = value;
            }

            void setStep(uint32_t value) {
                this->step[0].uintV = value;
            }

            void setStep(float min, float max) {
                step[0].floatV = min;
                step[1].floatV = max;
            }

            void setStep(uint32_t min, uint32_t max) {
                step[0].uintV = min;
                step[1].uintV = max;
            }
        };

        enum : uint8_t {
            HAS_SUB_TASK = 0x1,
            HAS_SUB_RELATIVE = 0x2,
        };

        char title[48]{ 0 };
        char message[48]{ 0 };
        char subMessage[96]{ 0 };
        ProgressValue progress{};
        ProgressValue targetProg{};
        ProgressValue subProgress{};
        uint8_t flags{ 0 };

        TaskProgress() : title{ 0 }, message{ 0 }, subMessage{ 0 }, progress(), subProgress(), flags(0), targetProg() {}

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
                return progress.getNormalized() + (targetProg.getNormalized() * subProgress.getNormalized());
            }
            return progress.getNormalized();
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
            progress.clear();
            subProgress.clear();
            targetProg.clear();
            title[0] = 0;
            message[0] = 0;
            subMessage[0] = 0;
        }
    };

    class Task {
    public:
        uint32_t flags{ 0 };
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
ENUM_BIT_OPERATORS(JCore::TaskProgressType)
