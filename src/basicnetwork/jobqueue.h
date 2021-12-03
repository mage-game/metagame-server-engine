
#ifndef JOBQUEUE_H
#define JOBQUEUE_H

#include "common/platform/thread/threadqueue.h"
class Job;

typedef _ThreadQueue<Job *> JobQueue;

#endif

