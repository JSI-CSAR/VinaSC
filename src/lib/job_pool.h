/*
 * job_pool.h
 *
 *  Created on: 2015Äê5ÔÂ29ÈÕ
 *      Author: langyu
 */

#ifndef SRC_LIB_JOB_POOL_H_
#define SRC_LIB_JOB_POOL_H_

#include "tbb/concurrent_queue.h"

struct job_pool {

	job_pool(int offset, int num) {
		threshold = 1;
		for(int i = 0; i < num; ++i )
			pool.push(i + offset);
	}
	void set_threshold(int rnl) {threshold = rnl;}
	int get_job(bool is_cpu) {
		//FIXME? concurrent access will cause bug?
		int rst;
		if(pool.size() <= 0)
			return -1;
		else {
			if(is_cpu || pool.size() > threshold) {
				pool.pop(rst);
				return rst;
			} else
				return -1;
		}
	}
private:
	tbb::concurrent_bounded_queue<int> pool;
	int threshold;
};



#endif /* SRC_LIB_JOB_POOL_H_ */
