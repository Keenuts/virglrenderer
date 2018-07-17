#ifndef UTIL_MACRO_H
#define UTIL_MACRO_H

#define UNUSED_PARAMETER(Param) (void)(Param)
#define ARRAY_SIZE(Array) (sizeof(Array) / sizeof((Array)[0]))

#ifdef DEBUG

#define TRACE_IN() fprintf(stderr, "server: --> %s\n", __func__)

#define TRACE_OUT(Value)                                             \
   do {                                                              \
      fprintf(stderr, "%s: server: <-- (%p)\n", __func__, Value);    \
   } while (0)
#else

#define TRACE_IN()
#define TRACE_OUT(...)

#endif

#define RETURN(Value)         \
   TRACE_OUT(Value);  \
   return Value;

#endif
