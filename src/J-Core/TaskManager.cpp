#include <J-Core/TaskManager.h>
#include <J-Core/Log.h>
#include <GLFW/glfw3.h>

namespace JCore {
    TaskManager* TaskManager::_instance = { nullptr };

    void TaskManager::beginTask(TaskCB method, TaskCB onComplete, TaskCB onCancel, uint8_t flags) {
        if (_instance) {
            if (_instance->_task.isRunning()) {
                // TODO: Implement task queue
                JCORE_WARN("Task already running! (In the future tasks may be queued)");
                return;
            }
            _instance->_task.run(method, onComplete, onCancel, flags);
            return;
        }
        JCORE_ERROR("Task could not be started running! (TaskManager is not Initialized!)");
    }

    TaskManager::TaskManager() : _mutex{}, _isLocked(false) {}

    TaskManager* TaskManager::init() {
        if (_instance) { return _instance; }
        _instance = new TaskManager();
        return _instance;
    }
    void TaskManager::deinit() {
        if (!_instance) { return; }
        delete _instance;
        _instance = nullptr;
    }

    bool TaskManager::tryLockState() {
        if (_instance && !_instance->_isLocked.load() && _instance->_mutex.try_lock()) {
            _instance->_isLocked.store(true);
            return true;
        }
        return false;
    }

    void TaskManager::unlockState() {
        if (_instance) {
            _instance->_mutex.unlock();
            _instance->_isLocked.store(false);
        }
    }

    bool TaskManager::checkIfCanReport() {
        if (_instance) {
            if (!_instance->_isLocked.load()) {
                if (TaskManager::isRunning()) {
                    JCORE_WARN("Cannot report progress! (You have to lock the Task before modifying progress values! See the " STR(REPORT_PROGRESS) " Macro )");
                }
                return false;
            }
            return true;
        }

        return false;
    }

    TaskManager* TaskManager::get() {
        return _instance;
    }

    void TaskManager::update() {
        if (_task.isRunning()) {
            if (_task.doCancel()) { return; }
            else if (_task.doComplete()) { return; }

            if (!_task.copying && _task.buffer) {
                _task.copying = true;
                auto fData = _task.buffer;
                _task.previewTex->create(fData->data, fData->format, fData->paletteSize, fData->width, fData->height, fData->flags);
                _task.copying = false;
                _task.buffer = nullptr;
            }
        }
    }

    void Task::run(TaskCB method, TaskCB onComplete, TaskCB onCancel, uint8_t flags) {
        join();
        this->onComplete = onComplete;
        this->onCancel = onCancel;
        this->flags = flags;
        this->state = STATE_RUNNING;
        buffer = nullptr;
        copying = false;
        clearProgress();
        initAndRun(method);
    }

    bool Task::cancel() {
        if (isCancelling()) { return false; }
        state |= STATE_CANCELLING;
        return true;
    }

    bool Task::markForSkip() {
        if (isSkipping()) { return false; }
        state |= STATE_SKIPPING;
        return true;
    }

    void Task::join()  {
        if (_thread.joinable()) {
            _thread.join();
        }
    }

    void Task::initAndRun(TaskCB method) {
        _thread = std::thread([this, method]() {
            method();
            state |= STATE_DONE;
            });
    }
}