/*

   Copyright (c) 2006-2010, The Scripps Research Institute

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Author: Dr. Oleg Trott <ot14@columbia.edu>, 
           The Olson Lab, 
           The Scripps Research Institute

*/

#ifndef VINA_PARALLEL_MC_H
#define VINA_PARALLEL_MC_H

#include "monte_carlo.h"
#include "parallel_progress.h"

struct parallel_mc {
	monte_carlo mc;
	sz num_tasks;
	sz num_threads;
	bool display_progress;
	parallel_mc() : num_tasks(8), num_threads(1), display_progress(true) {}
	void operator()(const model& m, output_container& out, const precalculate& p, const igrid& ig, const precalculate& p_widened, const igrid& ig_widened, const vec& corner1, const vec& corner2, rng& generator) const;
};

// move from parallel_mc.cpp

struct parallel_mc_task {
	model m;
	output_container out; // used to write output, every task keep one. will be merged when search done
	rng generator;
	parallel_mc_task(const model& m_, int seed) : m(m_), generator(static_cast<rng::result_type>(seed)) {}
};

typedef boost::ptr_vector<parallel_mc_task> parallel_mc_task_container;

struct parallel_mc_aux {
	const monte_carlo* mc;
	const precalculate* p;
	const igrid* ig;
	const precalculate* p_widened;
	const igrid* ig_widened;
	const vec* corner1;
	const vec* corner2;
	parallel_progress* pg;
	parallel_mc_aux(const monte_carlo* mc_, const precalculate* p_, const igrid* ig_, const precalculate* p_widened_, const igrid* ig_widened_, const vec* corner1_, const vec* corner2_, parallel_progress* pg_)
		: mc(mc_), p(p_), ig(ig_), p_widened(p_widened_), ig_widened(ig_widened_), corner1(corner1_), corner2(corner2_), pg(pg_) {}
	void operator()(parallel_mc_task& t) const {
		(*mc)(t.m, t.out, *p, *ig, *p_widened, *ig_widened, *corner1, *corner2, pg, t.generator);
	}
};

void merge_output_containers(const output_container& in, output_container& out, fl min_rmsd, sz max_size);
void merge_output_containers(const parallel_mc_task_container& many, output_container& out, fl min_rmsd, sz max_size);

#endif
