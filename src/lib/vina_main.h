#ifndef VINA_MAIN_H
#define VINA_MAIN_H

#include <iostream>
#include <pthread.h>
#include <string>
#include <exception>
#include <vector> // ligand paths
#include <cmath> // for ceila
#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp> // filesystem::basename
#include <boost/thread/thread.hpp> // hardware_concurrency // FIXME rm ?
#include "parse_pdbqt.h"
#include "parallel_mc.h"
#include "file.h"
#include "cache.h"
#include "non_cache.h"
#include "naive_non_cache.h"
#include "parse_error.h"
#include "everything.h"
#include "weighted_terms.h"
#include "current_weights.h"
#include "quasi_newton.h"
#include "tee.h"
#include "coords.h" // add_to_output_container

#ifndef __MIC__
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <intel-coi/source/COIProcess_source.h>
#include <intel-coi/source/COIEngine_source.h>
#include <intel-coi/source/COIPipeline_source.h>
#include <intel-coi/source/COIEvent_source.h>
#include <intel-coi/source/COIBuffer_source.h>

#define CHECK_RESULT(_COIFUNC) \
{ \
    COIRESULT result = _COIFUNC; \
    if (result != COI_SUCCESS) \
    { \
        printf("%s returned %s\n", #_COIFUNC, COIResultGetName(result));\
        return -1; \
    }\
}
#else 
#include <stdio.h>
#include <unistd.h>

#include <intel-coi/sink/COIPipeline_sink.h>
#include <intel-coi/sink/COIProcess_sink.h>
#include <intel-coi/common/COIMacros_common.h>
#include <intel-coi/common/COISysInfo_common.h>
#include <intel-coi/common/COIEvent_common.h>
#endif

//#pragma offload_attribute(push, target(mic))
struct config {
	config() {}
	config(fl center_x, fl center_y, fl center_z, fl size_x, fl size_y, fl size_z, int cpu, int seed, int exhaustiveness, int verbosity, int num_modes,
			fl energy_range, fl weight_gauss1, fl weight_gauss2, fl weight_repulsion, fl weight_hydrophobic, fl weight_hydrogen, fl weight_rot) : center_x(center_x),
					center_y(center_y), center_z(center_z), size_x(size_x), size_y(size_y), size_z(size_z), cpu(cpu), seed(seed), exhaustiveness(exhaustiveness),
					verbosity(verbosity), num_modes(num_modes), energy_range(energy_range), weight_gauss1(weight_gauss1), weight_gauss2(weight_gauss2),
					weight_repulsion(weight_repulsion), weight_hydrophobic(weight_hydrophobic), weight_hydrogen(weight_hydrogen), weight_rot(weight_rot) {}
	fl center_x, center_y, center_z, size_x, size_y, size_z;
	int cpu, seed, exhaustiveness, verbosity, num_modes;
	fl energy_range;
	fl weight_gauss1;
	fl weight_gauss2;
	fl weight_repulsion;
	fl weight_hydrophobic;
	fl weight_hydrogen;
	fl weight_rot;

	std::string rigid_name;
	std::string ligand_name;
	std::string out_name;

	friend std::istream& operator >> (std::istream& in, config& c);
	friend std::ostream& operator << (std::ostream& out, config& c);

	void read_buf() {
		//read order: rigid_name, ligand_name, out_name
		std::string in_str(buffer);
		int loc = 0;
		int i = 0;
		while(true) {
			std::string tmp;
			loc = in_str.find(",");
			if(loc == -1) {
				tmp = in_str.substr(0, in_str.length());
				in_str.erase(0, in_str.length());
				out_name = tmp;
				//out.push_back(tmp);
				break;
			}
			tmp = in_str.substr(0, loc);
			in_str.erase(0, loc + 1);
			//out.push_back(tmp);
			if(i == 0) {
				rigid_name = tmp;
				++i;
			} else
				ligand_name = tmp;
		}
	}
	char buffer[500];
};

//#pragma offload_attribute(pop)

int boot_up(int argc, char* argv[]);

int vina_main(std::vector<std::string>& lig_list, int job_offset, int job_num, int num_workers,
			  boost::optional<std::string> rigid_name_opt, boost::optional<std::string> flex_name_opt, std::string& out_name, std::string& out_path,
		      int exhaustiveness, int verbosity, int seed, bool score_only, bool local_only, bool randomize_only,
			  grid_dims& gd, flv& weights, sz max_modes_sz, fl energy_range, int cpu, tee& log, config& trans);

void done(int verbosity, tee& log);

void doing(int verbosity, const std::string& str, tee& log);

path make_path(const std::string& str);
class vina_main_t {
public:
	
	vina_main_t(strv& lig_list, int job_offset, int job_num, int worker_num,
		boost::optional<std::string> &rigid_name_opt, boost::optional<std::string>& flex_name_opt, std::string& out_name, std::string& out_path,
		int exhaustiveness, int verbosity, int seed, bool score_only, bool local_only, bool randomize_only,
		grid_dims& gd, flv& weights, sz max_modes_sz, fl energy_range, int cpu, tee& log, config& trans);

	int boot();
	int mic_boot();


 pthread_t pro_tid;
 pthread_t *cus_tid;

#ifndef __MIC__
 //----------------- cpu side -----------------
 pthread_t trans_tid;
 pthread_t wait_tid; // threads that wait for job done on mic
#endif

 strv& lig_list;
 int job_offset;
 int job_num;
 int worker_num;

 boost::optional<std::string> &rigid_name_opt;
 boost::optional<std::string>& flex_name_opt;
 std::string& out_name;
 std::string& out_path;

 int exhaustiveness;
 int verbosity;
 int seed;
 bool score_only;
 bool local_only;
 bool randomize_only;

 grid_dims& gd;
 flv& weights;
 sz max_modes_sz;
 fl energy_range;
 int cpu;
 tee& log;
 config& trans;
};

void serial_strings(char *buf, int len, int argc, char **argv);
void deserial_strings(char *buf, int len, int &argc, char** &argv);

extern vina_main_t *global_vina_main;
#ifndef __MIC__
	extern COIPROCESS          proc;
	extern COIENGINE           engine;
	extern COIFUNCTION         func;
	extern COIPIPELINE         pipeline;
	extern uint32_t            num_engines;
	extern tbb::concurrent_bounded_queue<int> *slot_q;
#else
	extern tbb::concurrent_bounded_queue<int> *rcv_q;
	extern tbb::concurrent_bounded_queue<int> *done_q;
#endif
//typedef tbb::concurrent_bounded_queue<config*> config_q;

#endif
