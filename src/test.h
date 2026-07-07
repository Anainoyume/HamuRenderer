// hamu_engine/include/hamu_engine/api.h
#pragma once

#ifdef HAMU_ENGINE_BUILD
    #define HAMU_API __declspec(dllexport)
#else
    #define HAMU_API __declspec(dllimport)
#endif

namespace hamu {

    // 导出函数，初始化引擎
    HAMU_API bool initialize();
    HAMU_API void shutdown();
    HAMU_API void update();

}