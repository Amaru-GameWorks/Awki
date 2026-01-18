#pragma once

#define CONCAT_IMPL(x,y) x##y
#define CONCAT(x,y) CONCAT_IMPL(x, y)

#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

#define STRING_COUNTER STRINGIFY(__COUNTER__)
#define COUNTER_CONCAT(x) CONCAT(x, __COUNTER__)