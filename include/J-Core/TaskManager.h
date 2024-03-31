#pragma once 
#include <cstdint>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstdarg>
#include <functional>
#include <type_traits>
#include <J-Core/Rendering/Window.h>
#include <J-Core/Util/EnumUtils.h>
#include <J-Core/Util/StringUtils.h>
#include <J-Core/Rendering/Texture.h>

using TaskCB = std::function<void(void)>;
#define NO_TASK TaskCB{}

namespace JCore {
    struct TaskProgress {
        struct Progress {
            double value;
            double target;
            double stepValue;

            double getPercent(double step) const {
                double v;
                if (target > 0) {
                    v = (value + step) / target;
                }
                else {
                    v = value;
                }
                return Math::clamp01(v);
            }

            void clear() {
                value = 0;
                target = 0;
                stepValue = 0;
            }
        };

        struct State {
            std::string message{};
            Progress progress{};
            size_t users{};

            void clear() {
                message.clear();
                progress.clear();
            }
        };

        std::string title{};
        State main{};
        std::vector<State> subStates{};

        void reset() {
            title.clear();
            main.clear();
            subStates.clear();
        }

        double getPercent(uint8_t level, int32_t originalLevel = -1) const {
            if (originalLevel == level) { return 0.0; }
            originalLevel = originalLevel < 0 ? level : originalLevel;

            const State* state = nullptr;
            double step = 0.0;

            if (level == 0) {
                state = &main;
            }
            else {
                level--;
                state = level >= subStates.size() ? nullptr : &subStates[level];
            }

            if (state) {     
                step = state->progress.stepValue;
                step = step < 0 ? getPercent(uint8_t(-step), originalLevel) : step;
                return state->progress.getPercent(step);
            }
            return 0.0;
        }

        void regLevel(size_t level) {
            if (level >= subStates.size()) {
                subStates.resize(level + 1);

                subStates[level].clear();
                subStates[level].users = 0;
            }
            subStates[level].users++;
        }

        void unregLevel(size_t level) {
            if (level >= subStates.size()) {
                return;
            }
            auto& state = subStates[level];

            if (state.users) {
                --state.users;
            }

            if (state.users == 0) {
                for (size_t i = level + 1; i < subStates.size(); i++) {
                    if (subStates[i].users) { return; }
                }
                subStates.resize(level);
            }
        }

        std::string_view getMessage(size_t state) const {
            if (state == 0) {
                return main.message;
            }
            state--;
            return state >= subStates.size() ? std::string{} : subStates[state].message;
        }
    };

    class Task {
    public:
        enum : uint8_t {
            F_HAS_CANCEL  = 0x1,
            F_HAS_SKIP    = 0x2,
            F_HAS_PREVIEW = 0x4,

            F_SHOULD_PREVIEW = 0x80,
        };

        enum : uint8_t {
            P_HAS_CANCEL = 0x1,
            P_HAS_SKIP = 0x2,
            P_HAS_PREVIEW = 0x4,
        };

        enum : uint8_t {
            STATE_RUNNING = 0x1,
            STATE_CANCELLING = 0x2,
            STATE_SKIPPING = 0x4,
            STATE_DONE = 0x8,
        };

        uint8_t state{ 0 };
        uint8_t flags{ 0 };

        TaskProgress progress{};
        const ImageData* buffer{};
        bool copying{};
        std::shared_ptr<Texture> previewTex = std::make_shared<Texture>();
        TaskCB onCancel;
        TaskCB onComplete;

        ~Task() {
            if (_thread.joinable()) {
                _thread.join();
            }
            _thread = std::thread();
        }

        void join();
        bool cancel();
        bool markForSkip();
        void run(TaskCB method, TaskCB onComplete, TaskCB onCancel, uint8_t flags);

        bool isRunning() const { return (state & STATE_RUNNING) != 0; }
        bool isCancelling() const { return (state & STATE_CANCELLING) != 0; }
        bool isSkipping() const { return (state & STATE_SKIPPING) != 0; }
        bool isDone() const { return (state & STATE_DONE) != 0; }

        void clearPreview() {
            flags &= ~F_SHOULD_PREVIEW;
        }

        bool performSkip() {
            if (isSkipping()) {
                state &= ~STATE_SKIPPING;
                return true;
            }
            return false;
        }
    private:
        friend class TaskManager;
        std::thread _thread;
        void initAndRun(TaskCB method);

        void clearProgress() {
            progress.reset();
        }

        bool doCancel() {
            if (isCancelling()) {
                join();
                if (onCancel) {
                    onCancel();
                }

                clearProgress();
                state = 0x00;
                return true;
            }
            return false;
        }

        bool doComplete() {
            if (isDone()) {
                join();
                if (onComplete) {
                    onComplete();
                }

                clearProgress();
                state = 0x00;
                return true;
            }
            return false;
        }
    };

    class TaskManager {
    public:
        static TaskManager* get();
        static TaskManager* init();
        static void deinit();

        static bool tryLockState();
        static void unlockState();

        const Task& getTask() const {
            return _task;
        }

        template<typename ...Args>
        static void setTaskTitle(const char* format, Args... args) {
            if (checkIfCanReport()) {
                std::string& title = _instance->_task.progress.title;
                title.clear();
                Utils::appendFormat(title, format, args...);
            }
        }

        static void reportPreview(const ImageData* frameData) {
            if (_instance && frameData && !_instance->_task.copying) {
                _instance->_task.buffer = frameData;
            }
        }

        template<typename TP = double, typename TS = double, typename TT = double>
        static void reportProgress(uint8_t level, TP progress, TS stepValue, TT target) {
            auto state = getProgressState(level, false);
            if (state) {
                state->progress.value = double(progress);
                state->progress.stepValue = double(stepValue);
                state->progress.target = double(target);
            }
        }


        static void reportProgress(uint8_t level, double progress) {
            auto state = getProgressState(level, false);
            if (state) {
                state->progress.value = progress;
            }
        }

        static void reportIncrement(uint8_t level, double amount = 1.0) {
            auto state = getProgressState(level, false);
            if (state) {
                state->progress.value += amount;
            }
        }

        static void reportStep(uint8_t level, double step) {
            auto state = getProgressState(level, false);
            if (state) {
                state->progress.stepValue = step;
            }
        }

        static void reportStep(uint8_t level, double progress, double step) {
            auto state = getProgressState(level, false);
            if (state) {
                state->progress.value = progress;
                state->progress.stepValue = step;
            }
        }

        static void reportStepFrom(uint8_t level, uint8_t other) {
            auto stateA = getProgressState(level, false);
            if (stateA) {
                stateA->progress.stepValue = -double(other);
            }
        }

        static void reportStepIncrement(uint8_t level, double step, double amount = 1.0) {
            auto state = getProgressState(level, false);
            if (state) {
                state->progress.value += amount;
                state->progress.stepValue = step;
            }
        }
        
        template<typename TT = double>
        static void reportTarget(uint8_t level, TT target) {
            auto state = getProgressState(level, false);
            if (state) {
                state->progress.target = double(target);
            }
        }
        
        template<typename TP = double, typename TT = double>
        static void reportTarget(uint8_t level, TP progress, TT target) {
            auto state = getProgressState(level, false);
            if (state) {
                state->progress.value = double(progress);
                state->progress.target = double(target);
            }
        }

        template<typename ...Args>
        static void report(uint8_t level, const char* format, Args... args) {
            auto state = getProgressState(level, false);
            if (state) {
                state->message.clear();
                Utils::appendFormat(state->message, format, args...);
            }
        }

        static void regLevel(uint8_t subLevel) {
            if (subLevel == 0) { return; }
            subLevel--;
            if (checkIfCanReport()) {
                _instance->_task.progress.regLevel(subLevel);
            }
        }
        static void unregLevel(uint8_t subLevel) {
            if (subLevel == 0) { return; }
            subLevel--;
            if (checkIfCanReport()) {
                _instance->_task.progress.unregLevel(subLevel);
            }
        }

        static bool shouldWaitForBuffer() {
            return _instance && (!_instance->_task.copying || _instance->_task.buffer);
        }

        static void waitForBuffer() {
            while(shouldWaitForBuffer()){}
        }

        static bool isRunning() {
            return _instance && _instance->_task.isRunning();
        }

        static bool isCanceling() {
            return _instance && _instance->_task.isCancelling();
        }

        static bool isSkipping() {
            return _instance && _instance->_task.isSkipping();
        }

        static bool performSkip() {
            return _instance && _instance->_task.performSkip();
        }

        static bool cancelCurrentTask() {
            return _instance && _instance->_task.cancel();
        }

        static bool markSkipForTask() {
            return _instance && _instance->_task.markForSkip();
        }

        static bool canPreview() {
            return _instance && (_instance->_task.flags & (Task::F_HAS_PREVIEW | Task::F_SHOULD_PREVIEW)) == Task::F_HAS_PREVIEW;
        }

        static bool canCancel() {
            return _instance && (_instance->_task.flags & Task::F_HAS_CANCEL) != 0;
        }

        static bool canSkip() {
            return _instance && (_instance->_task.flags & Task::F_HAS_SKIP) != 0;
        }

        static std::shared_ptr<Texture> getPreview() {
            if (_instance && _instance->canPreview()) {
                return _instance->_task.previewTex;
            }
            return std::shared_ptr<Texture>{};
        }

        static void clearPreview() {
            if (checkIfCanReport()) {
                _instance->_task.clearPreview();
            }
        }

        static void beginTask(TaskCB method, TaskCB onComplete, TaskCB onCancel, uint8_t flags);

        static void updateTasks() {
            if (_instance) {
                _instance->update();
            }
        }

    private:
        friend class Application;

        std::atomic_bool _isLocked{false};
        std::mutex _mutex{};
        static TaskManager* _instance;
        Task _task;

        TaskManager();
        void update();

        void* operator new(size_t size) {
            return malloc(size);
        }

        void operator delete(void* ptr) {
            if (ptr) {
                free(ptr);
            }
        }

        static bool checkIfCanReport();

        static TaskProgress::State* getProgressState(uint8_t level, bool doReg) {
            if (!checkIfCanReport()) { return nullptr; }

            Task& task = _instance->_task;
            if (level == 0) {
                return &task.progress.main;
            }
            level--;

            if (level >= task.progress.subStates.size()) {
                if (!doReg) { return nullptr; }

            }
            auto state = &task.progress.subStates[level];
            if (doReg) {
                task.progress.regLevel(level);
            }
            return state->users ? state : nullptr;
        }
    };
}

#define REPORT_PROGRESS(X)\
{ \
  if(JCore::TaskManager::tryLockState()){\
      X \
    JCore::TaskManager::unlockState();\
  } \
}
