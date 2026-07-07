// hamu_engine/src/core.cpp
#include "test.h"

#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"

namespace hamu {

    bool initialize() {
        spdlog::info("Engine initialized");
        return true;
    }

    void shutdown() {
        spdlog::info("Engine shutdown");
    }

    void update() {
        spdlog::info("Engine update");
    }

}