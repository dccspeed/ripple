src="./../../src"
lib="./../../lib"
g++ -std=c++14 -DBOOST_LOG_DYN_LINK -fPIC -DSFMT_MEXP=19937 -Wall  -fext-numeric-literals -O3 -I./$lib/bliss-0.73/  -I./$lib/SFMT-src-1.5.1/ -I./$lib/pcg-cpp-0.98/include/ -I./$src/include/ -L/usr/lib/ -L./$src/ -L./$lib/bliss-0.73/ -L./$lib/SFMT-src-1.5.1/ -L./$lib/pcg-cpp-0.98/src/ rwQuery.cpp -o app  -lrgpm -lsfmt -lm -lboost_program_options -lstdc++ -lbliss  -lboost_log -lboost_system -lquadmath -ltbb -lpthread #-pg -g #-lefence
