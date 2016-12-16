BASE=/usr/local
BOOST_VERSION=1_41
BOOST_INCLUDE = /usr/include/
MIC_BOOST_INCLUDE = /root/xsun23/workspace/boost/mic/include

TBB_VERSION=2_2
C_PLATFORM=-pthread -tbb #-static 
#GPP=icpc
GPP=mpiicpc 
MICGPP=icpc 

#GPP=mpiicpc -offload-option,mic,ld,"-L/usr/linux-k1om-4.7/linux-k1om/lib64/ -lpthread"
#GPP=mpiicpc -offload-option,mic,ld,"-L/usr/linux-k1om-4.7/linux-k1om/lib64/ -lpthread -ltbb -L/root/xsun23/workspace/autodock_proj/boost/mic/lib64 -lboost_system -lboost_thread  -lboost_filesystem -lboost_program_options"
#GPP=mpiicpc -offload-option,mic,ld,"-L/root/xsun23/workspace/autodock_proj/boost/mic/lib64 -lboost_system -lboost_thread  -lboost_filesystem -lboost_program_options -tbb "
#C_OPTIONS= -O3 -g -qopt-report=5 -qopt-report-phase=vec -DNDEBUG
C_OPTIONS= -O3 -g -DNDEBUG
BOOST_LIB_VERSION=

include ../../makefile_common


