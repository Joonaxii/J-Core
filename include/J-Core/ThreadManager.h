#pragma once 
#include <cstdint>
#include <thread>
#include <atomic>
#include <type_traits>
#include <functional>
#include <J-Core/Rendering/Window.h>
#include <cstdarg>
#include <J-Core/Util/EnumUtils.h>
#include <J-Core/Util/StringUtils.h>
#include <J-Core/Rendering/Texture.h>

namespace JCore {
    enum : uint32_t {
        THREAD_FLAG_CANCEL = 0x1,
        THREAD_FLAG_RUNNING = 0x2,
        THREAD_FLAG_DONE = 0x4,
        THREAD_FLAG_SKIP = 0x8,

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
        TASK_COPY_PREVIEW = 0x40,
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
            HAS_SUB_RELATIVE = 0x2
        };

        std::string title{};
        std::string message{};
        std::string subMessage{};
        ProgressValue progress{};
        ProgressValue targetProg{};
        ProgressValue subProgress{};

        ImageData* preview{};
        uint8_t flags{ 0 };

        TaskProgress() : title{ }, message{ }, subMessage{ }, progress(), subProgress(), flags(0), targetProg(), preview{} {}

        TaskProgress(const char* title, const char* message) : TaskProgress(){
            this->title = std::string{ title };
            this->message = std::string{ message };
        }

        TaskProgress(const char* title, const char* message, const char* subMessage) : TaskProgress() {
            this->title = std::string{ title };
            this->message = std::string{ message };
            this->subMessage = std::string{ subMessage };
        }

        float getProgress() const {
            if ((flags & (HAS_SUB_RELATIVE | HAS_SUB_TASK)) == (HAS_SUB_RELATIVE | HAS_SUB_TASK)) {
                return progress.getNormalized() + (targetProg.getNormalized() * subProgress.getNormalized());
            }
            return progress.getNormalized();
        }

        template <class Arg>
        static decltype(auto) prepare(const Arg& arg)
        {
            if constexpr (std::is_same_v<Arg, std::string>) { return arg.c_str(); }
            return arg;
        }

        template<typename ...Args>
        void setSubMessage(const char* format, Args... args) {
            subMessage.clear();
            Utils::appendFormat(subMessage, format, args...);
        }

        template<typename ...Args>
        void setMessage(const char* format, Args... args) {
            message.clear();
            Utils::appendFormat(message, format, args...);
        }

        template<typename ...Args>
        void setTitle(const char* format, Args... args) {
            title.clear();
            Utils::appendFormat(title, format, args...);
        }

        void copyFrom(const TaskProgress& other, uint8_t copyFlags) {
            if (copyFlags & TASK_COPY_TITLE) {
                title = other.title;
            }

            if (copyFlags & TASK_COPY_MESSAGE) {
                message = other.message;
            }

            if (copyFlags & TASK_COPY_SUBMESSAGE) {
                subMessage = other.subMessage;
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

            if (copyFlags & TASK_COPY_PREVIEW) {
                preview = other.preview;
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
        bool showPreview{};
        std::shared_ptr<Texture> previewTex = std::make_shared<Texture>();

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

        bool isCancelling();

        bool markForSkip();
        bool performSkip();
        bool isSkipping();

        void clearPreview();
        void updateTask();

        void reportProgress(const TaskProgress& progress, uint8_t copyFlags = (0xFF & ~TASK_COPY_PREVIEW));
        const Task& getCurrentTask();
    }
}
ENUM_BIT_OPERATORS(JCore::TaskProgressType)
