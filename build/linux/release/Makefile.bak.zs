BASE=/usr/local
#BASE=/tmp/autodock
#BOOST_VERSION=1_41
#BOOST_INCLUDE = $(BASE)/include
#BOOST_INCLUDE = /usr/include/boost
BOOST_INCLUDE = /home/zs/boost_1_49_0_cpu
C_PLATFORM= -pthread
#GPP=/usr/bin/g++
GPP=/opt/intel/composerxe/bin/icpc
C_OPTIONS= -O3 -DNDEBUG -g -xAVX -vec-report6 
BOOST_LIB_VERSION=

include ../../makefile_common

