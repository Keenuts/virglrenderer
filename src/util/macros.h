#ifndef UTIL_MACRO_H
#define UTIL_MACRO_H

#define UNUSED_PARAMETER(Param) (void)(Param)
#define ARRAY_SIZE(Array) (sizeof(Array) / sizeof((Array)[0]))

#ifdef DEBUG
#define TRACE_IN() fprintf(stderr, "server: --> %s\n", __func__)
#define TRACE_OUT(...) \
        do {    \
            char *fmt = "server: <-- %s (%d)\n"; \
            fprintf(stderr, fmt, __func__ __VA_OPT__(,) __VA_ARGS__ , 0); \
        } while (0)
#else
#define TRACE_IN()
#define TRACE_OUT(...)
#endif

#define RETURN(...)         \
    TRACE_OUT(__VA_ARGS__); \
    return __VA_ARGS__

#endif
