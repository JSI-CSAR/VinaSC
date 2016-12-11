/*
 * taskq.h
 *
 *  Created on: 2015Äê4ÔÂ2ÈÕ
 *      Author: langyu
 */

#ifndef SRC_LIB_TASKQ_H_
#define SRC_LIB_TASKQ_H_

#include "tbb/concurrent_queue.h"
#include "monte_carlo.h"
#include <pthread.h>
#include "job.h"
#include "parallel_mc.h"

#define MAX_TASK 64

struct task {

	task(job* j, model& m, int seed) : owner(j) { par_task = new parallel_mc_task(m, seed);}
	task() : owner(NULL), par_task(NULL) {}
	//FIXME memory leak?
	~task() {
		if(par_task != NULL)
			delete par_task;
	}

	job* owner;
	struct parallel_mc_task* par_task;

	/*
	model* m;
	output_container* out;
	const precalculate* p;
	const igrid* ig;
	const precalculate* p_widened;
	const igrid* ig_widened;
	const vec* corner1;
	const vec* corner2;
	rng* generator;
	task(job* owner_, model* m_, output_container* out_, const precalculate* p_, const igrid* ig_, const precalculate* p_widened_, const igrid* ig_widened_, const vec* corner1_, const vec* corner2_, rng* generator_)
		: owner(owner_), m(m_), out(out_), p(p_), ig(ig_), p_widened(p_widened_), ig_widened(ig_widened_), corner1(corner1_), corner2(corner2_), generator(generator_) {}*/
};

//typedef tbb::concurrent_bounded_queue<task> task_q;
typedef tbb::concurrent_bounded_queue<task*> task_q;

#endif /* SRC_LIB_TASKQ_H_ */
