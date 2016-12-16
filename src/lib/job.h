#ifndef VINA_JOB_POOL_H
#define VINA_JOB_POOL_H

#include <vector>
#include <string>
#include "parallel_mc.h"
//#include "taskq.h"

struct task; //forward declaration.

#define NO_TASK 0
#define RMN_TASK 1

struct aux_data {

	aux_data(const precalculate& p_, const cache& ig_, const precalculate& p_widened_, const cache& ig_widened_, const vec& corner1_, const vec& corner2_) : p(p_), ig(ig_), p_widened(p_widened_), ig_widened(ig_widened_), corner1(corner1_), corner2(corner2_) {}

	precalculate p;
	cache ig;
	precalculate p_widened;
	cache ig_widened;
	vec corner1;
	vec corner2;
	//parallel_proress pg;
};

struct extra_data {

	extra_data(const model m_, const vec authentic_v_, sz max_modes_sz_, fl energy_range_) : m(m_), authentic_v(authentic_v_), max_modes_sz(max_modes_sz_), energy_range(energy_range_) {}

	//non_cache nc;
	model m;	//different from the one kept by parallel_mc_task. those might be modified. this one is the original one.
	vec authentic_v;
	//weighted_terms sf; //inherit scoring function
	sz max_modes_sz; //num_modes
	fl energy_range;
};

struct job {
	//void do_search(model& m, const boost::optional<model>& ref, const scoring_function& sf, const precalculate& prec, const igrid& ig, const precalculate& prec_widened, const igrid& ig_widened, non_cache& nc, // nc.slope is changed
	//			   const std::string& out_name,
	//			   const vec& corner1, const vec& corner2,
	//			   const parallel_mc& par, fl energy_range, sz num_modes,
	//			   int seed, int verbosity, bool score_only, bool local_only, tee& log, const terms& t, const flv& weights) {

	job(model m_, vec authentic_v_, sz max_modes_sz_, fl energy_range_,
		int task_num_, std::string& lig_name_, std::string& rcp_name_, std::string& out_name_, parallel_mc& par_,
		const precalculate* p_, const cache* ig_, const precalculate* p_widened_,
		const cache* ig_widened_, const vec* corner1_, const vec* corner2_, parallel_progress* pg_) : task_num(task_num_), rmn_task(task_num_), lig_name(lig_name_), rcp_name(rcp_name_), out_name(out_name_), par(par_),
				e_data(m_, authentic_v_, max_modes_sz_, energy_range_),
				data(*p_, *ig_, *p_widened_, *ig_widened_, *corner1_, *corner2_),
				//par_aux(&par.mc, p_, ig_, p_widened_, ig_widened_, corner1_, corner2_, pg_)
				par_aux(&par.mc, &data.p, &data.ig, &data.p_widened, &data.ig_widened, &data.corner1, &data.corner2, pg_) { pthread_mutex_init(&cnt_lock, NULL); }

	//FIXME memory leak??
	//~job() {
	//	if(par_aux.pg != NULL)
	//		delete(par_aux.pg);
	//
	//}
	int task_done() {

		int rst;
		pthread_mutex_lock(&cnt_lock);
		rmn_task--;
		if(rmn_task == 0)
			rst = NO_TASK;
		else
			rst = RMN_TASK;
		pthread_mutex_unlock(&cnt_lock);

		return rst;
	}

	int task_num;
	std::string lig_name;
	std::string rcp_name;
	std::string out_name;

	aux_data data;
	extra_data e_data;
	parallel_mc par;
	parallel_mc_aux par_aux;
	//model m;
	//non_cache nc;

	std::vector<task*> slaves;
	output_container out;


private:
	pthread_mutex_t cnt_lock;
	int rmn_task;
};


#endif
