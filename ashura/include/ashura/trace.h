
#pragma once

#define ASH_DECLARE_TRACE_CATEGORY()
#define ASH_DEFINE_TRACE_CATEGORY()
#define ASH_TRACE()
#define ASH_TRACE_FUNC()
#define ASH_TRACE_BEGIN()
#define ASH_TRACE_END()

typedef struct Tracer_T       *Tracer;
typedef struct TracerInterface TracerInterface;
typedef struct TracerImpl      TracerImpl;

#define DEFINE_TRACE
#define TRACE_PARAM

// log to console, log to json file, log to whatever
// use for regression testing
struct TracerInterface
{
  void (*begin_event)(char const *event);
  void (*end_event)(char const *event);
};