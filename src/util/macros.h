#ifndef UTIL_MACRO_H
#define UTIL_MACRO_H

#define UNUSED_PARAMETER(Param) (void)(Param)
#define ARRAY_SIZE(Array) (sizeof(Array) / sizeof((Array)[0]))

#ifdef DEBUG

#define TRACE_IN() fprintf(stderr, "server: --> %s\n", __func__)

#define TRACE_OUT(...)                                                     \
   do {                                                                    \
      const char *fmt = "server: <-- %s (%d)\n";                               \
      fprintf(stderr, fmt, __func__ __VA_OPT__(,) __VA_ARGS__ , 0);     \
   } while (0)


#define DEBUG_ERR(Fmt, ...) \
   fprintf(stderr, (Fmt) __VA_OPT__(,) __VA_ARGS__);

#else

#define TRACE_IN()
#define TRACE_OUT(...)

#endif

#define RETURN(Value)         \
   TRACE_OUT(Value);          \
   return Value;

#endif
