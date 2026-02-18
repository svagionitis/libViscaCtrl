#pragma once

#if defined(_WIN32) || defined(_WIN64)
#if defined(VISCA_BUILD_SHARED)
#define VISCA_EXPORT __declspec(dllexport)
#elif defined(VISCA_USE_SHARED)
#define VISCA_EXPORT __declspec(dllimport)
#else
#define VISCA_EXPORT
#endif
#else
#if defined(VISCA_BUILD_SHARED)
#define VISCA_EXPORT __attribute__((visibility("default")))
#else
#define VISCA_EXPORT
#endif
#endif
