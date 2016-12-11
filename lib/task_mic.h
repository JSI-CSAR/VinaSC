#ifndef TASKMIC_H_
#define TASKMIC_H_


#include <pthread.h>
#include "tbb/concurrent_queue.h"
#include <iostream>
#include <stdlib.h>


template<typename task_t>
class TaskWorkerGroup {
public:
	TaskWorkerGroup(tbb::concurrent_bounded_queue<task_t *> &taskQueue, int numWorkers):
			taskQueue(taskQueue),
			numWorkers(numWorkers) {
//		michandler = NULL;
		int i = 0;
		wids = new pthread_t[numWorkers];
		for (; i < numWorkers; i++) {
			pthread_create(wids + i, NULL, run, (void *)this);
		}
	}

	void waitAllWorkerExit() {
		int i = 0;

		std::cout << "join all workers in group begin \n"; std::cout.flush();
		for (; i < numWorkers; i++) {
			pthread_join(wids[i], NULL);
		}

		std::cout << "join all workers in group end \n"; std::cout.flush();
	}

	void notifyAllWorkerDone() {
		for (int i = 0; i < numWorkers; i++) {
			taskQueue.push(NULL);
		}
	}

//	TaskWorkerGroup(tbb::concurrent_bounded_queue<task_t *> &taskQueue, int numWorkers, TaskTransferMIC<task_t> *michandler):
//			taskQueue(taskQueue),
//			numWorkers(numWorkers),
//			michandler(michandler) {
//		int i = 0;
//		wids = new pthread_t[numWorkers];
//		for (; i < numWorkers; i++) {
//			pthread_create(wids + i, NULL, run, (void *)this);
//		}
//	}

	virtual ~TaskWorkerGroup() {
		// std::cout << "EEEEEEEEEEEEEEE ~TaskWorkerGroup()\n";
	}

	static void* run(void *args) {
		TaskWorkerGroup *thisp = (TaskWorkerGroup *)args;
		std::cout << "worker run ... \n"; std::cout.flush();
		while (true) {
			task_t *task;
			thisp->taskQueue.pop(task);

			if (task == NULL) {
				break;
			}

			task->run();
		}

		std::cout << "worker exit ... \n"; std::cout.flush();

		return NULL;
	}

private:
	tbb::concurrent_bounded_queue<task_t *> &taskQueue;
	int numWorkers;
	pthread_t *wids;
	// TaskTransferMIC<task_t> *michandler;
};


template<typename task_t>
class TaskTransferMIC {
public:
	TaskTransferMIC(int numWorkers): numWorkers(numWorkers) {
		workerGroup = new TaskWorkerGroup<task_t>(taskQueue, numWorkers);

	}

	virtual ~TaskTransferMIC() {
		// std::cout << "EEEEEEEEEEEEEEE TaskTransferMIC delete\n";
		delete workerGroup;
	}

	void enqueue(task_t *task) {
		taskQueue.push(task);
	}

	void waitOneTaskDone(task_t* &doneTask) {
		taskDoneQueue.pop(doneTask);
	}

	void notifyOneTaskDone(task_t* doneTask) {
		taskDoneQueue.push(doneTask);
	}

	void notifyAllTasksDone() {
		workerGroup->notifyAllWorkerDone();
	}

	void waitAllTaskDone() {
		workerGroup->waitAllWorkerExit();
		taskDoneQueue.push(NULL);
	}

private:
	tbb::concurrent_bounded_queue<task_t *> taskQueue;
	tbb::concurrent_bounded_queue<task_t *> taskDoneQueue;

	int numWorkers;
	TaskWorkerGroup<task_t> *workerGroup;

};

template<typename task_t, int micno>
class TaskTransferCPU {
public:
	TaskTransferCPU(int numReceiversOnMIC, int maxTaskTransferOneTime):
			numReceiversOnMIC(numReceiversOnMIC),
			maxTaskTransferOneTime(maxTaskTransferOneTime) {

		TaskTransferMIC<task_t>**handler = (TaskTransferMIC<task_t>**)malloc(sizeof(TaskTransferMIC<task_t>*));

		totatlNumTask = 2147483647;
		#pragma offload target(mic:micno) inout(handler: length(1))
		{
			*handler = new TaskTransferMIC<task_t>(numReceiversOnMIC);
		}

		micHandler = handler;

		// std::cout << "create mic handler " << *micHandler << std::endl;

		for (int i = 0; i < maxTaskTransferOneTime; i++) {
			transferSlotQueue.push(TaskTransferCPU::slot);
		}

		pthread_create(&transfer_tid, NULL, transferExec, (void *)this);
		pthread_create(&waitToDone_tid, NULL, waitToDoneExec, (void *)this);
	}

	virtual ~TaskTransferCPU() {
		// std::cout << "~TaskTransferCPU()\n";

		long handlerLong = (long)(*micHandler);

		#pragma offload target(mic:micno)
		{
			TaskTransferMIC<task_t> * tmpHandler = (TaskTransferMIC<task_t> *)handlerLong;
			delete tmpHandler;
		}

		free(micHandler);
	}

	void push(task_t *task) {
		taskSourceQueue.push(task);
	}

	static void* transferExec(void *args) {
		int tmp;
		TaskTransferCPU *thisp = (TaskTransferCPU *)args;
		long handlerLong = (long)(*thisp->micHandler);

		while (true) {
			thisp->transferSlotQueue.pop(tmp);
			task_t *task = NULL;

			thisp->taskSourceQueue.pop(task);

			if (task != NULL) {
				int i = task->i;

				// std::cout << "transfer task "<< i <<" ...\n";
				#pragma offload target(mic:micno)
				{
					TaskTransferMIC<task_t> *tmpHandler = (TaskTransferMIC<task_t> *)handlerLong;
					task_t *newTask = new task_t();
					newTask->setTaskTransferMIC(tmpHandler);
					newTask->i = i;
					tmpHandler->enqueue(newTask);
				}
			} else {
				#pragma offload target(mic:micno)
				{
					TaskTransferMIC<task_t> *tmpHandler = (TaskTransferMIC<task_t> *)handlerLong;
					tmpHandler->notifyAllTasksDone();
				}

				break;
			}
		}

		return NULL;
	}

	static void* waitToDoneExec(void *args) { /*only one instance */
		int tmp;
		TaskTransferCPU *thisp = (TaskTransferCPU *)args;
		bool isDone = false;
		long handlerLong = (long)(*thisp->micHandler);
		while (!isDone) {
			int i = -1;

			#pragma offload target(mic:micno)
			{
				TaskTransferMIC<task_t> * tmpHandler = (TaskTransferMIC<task_t> *)handlerLong;

				task_t *doneTask = NULL;
				tmpHandler->waitOneTaskDone(doneTask);

				if (doneTask == NULL) {
					isDone = true;
				} else {
					i = doneTask->i;
				}
			}

			if (!isDone) {
				std::cout << "-------- receive task done on cpu "<< i <<"\n";
				thisp->transferSlotQueue.push(TaskTransferCPU::slot);
			} else {
				std::cout << "task done waiting thread exits\n"; std::cout.flush();
			}
		}

		return NULL;
	}

	TaskTransferMIC<task_t>* getTaskTransferHandlerOnMIC() {
		return *micHandler;
	}

	void notifyAllTaskDone() {
		taskSourceQueue.push(NULL);
	}

	void waitAllTaskDone() {
		long handlerLong = (long)(*micHandler);

		#pragma offload target(mic:micno)
		{
			TaskTransferMIC<task_t> * tmpHandler = (TaskTransferMIC<task_t> *)handlerLong;
			tmpHandler->waitAllTaskDone();
		}

		pthread_join(waitToDone_tid, NULL);
	}

private:
	int numReceiversOnMIC;
	int maxTaskTransferOneTime;
	tbb::concurrent_bounded_queue<task_t *> taskSourceQueue;
	tbb::concurrent_bounded_queue<int> transferSlotQueue;

	TaskTransferMIC<task_t> **micHandler;

	static const int slot;
	pthread_t transfer_tid;
	pthread_t waitToDone_tid;

	int totatlNumTask;
};

template<typename task_t, int micno>
const int TaskTransferCPU<task_t, micno>::slot = 1;

class MTask {
public:
	MTask() { i = 0;}
	MTask(int i): i(i) { }
	virtual ~MTask() {
		std::cout << "XXXXXXXXXXXXXXX  ~MTask()\n";
	}
	void run() {
		sleep(1);
		notifyTaskDone();
	}

	void notifyTaskDone() {
		#ifdef __MIC__
		michandler->notifyOneTaskDone(this);
		#endif
	}

	int i;
	void setTaskTransferMIC(TaskTransferMIC<MTask> *handler) {
		michandler = handler;
	}

	TaskTransferMIC<MTask> * michandler;
};

#endif /* TASKMIC_H_ */



