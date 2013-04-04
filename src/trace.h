/*
 * trace.h
 *
 *  Created on: Mar 30, 2013
 *      Author: malvi
 */

#ifndef TRACE_H_
#define TRACE_H_

#include <cstdio>

/*
 * The tracing macros are expanded only if TRACE_OFF is not defined.
 * The tracing structures are defined only if TRACE_OFF is not defined.
 */
#ifdef TRACE_OFF
#   define trace( type, level, msg, ... )
#   define traceIf( type, level, condition, msg, ... )
#   define hasTraceLevel( type, level ) false
#   define setTraceLevel( type, level )
#else

/**
 * This structure contains an unsigned integer for each kind of trace.
 */
struct TraceLevels
{
    friend struct Options;

public:
    unsigned std;

    TraceLevels() : std(0) {}
};

#include "options.h"

#   define trace( type, level, msg, ... ) \
    if( __options__.traceLevels.type >= level ) \
    { \
        for( unsigned __indent_Level__ = 1; __indent_Level__ < level; ++__indent_Level__ ) \
            fprintf( stderr, " " ); \
        fprintf( stderr, msg, ##__VA_ARGS__ ); \
    }
#   define traceIf( type, level, condition, msg, ... ) \
    if( condition ) \
        trace( type, level, msg, ##__VA_ARGS__ )
#   define setTraceLevel( type, level ) \
        __options__.traceLevels.type = level
#   define hasTraceLevel( type, level ) \
        ( __options__.traceLevels.type >= level )

#endif

#endif /* TRACE_H_ */
