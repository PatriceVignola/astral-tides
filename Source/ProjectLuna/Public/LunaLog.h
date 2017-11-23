#pragma once

#ifdef _MSC_VER
#define FUNC_NAME TEXT(__FUNCTION__)
#else
#define FUNC_NAME __func__
#endif

#define LUNA_LOG(Category, f, ...) UE_LOG(Category, Log, TEXT("%s():%d: " f), FUNC_NAME, __LINE__, ##__VA_ARGS__)
#define LUNA_WARN(Category, f, ...) UE_LOG(Category, Warning, TEXT("%s():%d: " f), FUNC_NAME, __LINE__, ##__VA_ARGS__)
#define LUNA_ERROR(Category, f, ...) UE_LOG(Category, Error, TEXT("%s():%d: " f), FUNC_NAME, __LINE__, ##__VA_ARGS__)
#define LUNA_FATAL(Category, f, ...) UE_LOG(Category, Fatal, TEXT("%s():%d: " f), FUNC_NAME, __LINE__, ##__VA_ARGS__)

#define DECLARE_LUNA_LOG(Category) DECLARE_LOG_CATEGORY_EXTERN(Category, Log, All)
#define DEFINE_LUNA_LOG(Category) DEFINE_LOG_CATEGORY(Category)