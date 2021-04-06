cmake_minimum_required(VERSION 3.10)
project(SubgraphMatryoshka)
message( STATUS "CONDA PREFIX is $ENV{CONDA_PREFIX}" )

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_LINK_DIRECTORIES_BEFORE 14)

include_directories(./src/include/
        ./lib/bliss-0.73/
        ./lib/SFMT-src-1.5.1/
        ./lib/pcg-cpp-0.98/include/
        $ENV{CONDA_PREFIX}/include/
        /usr/local/include/)
link_directories(
        /usr/lib/
        ./lib/bliss-0.73/
        ./lib/SFMT-src-1.5.1/
        $ENV{CONDA_PREFIX}/lib/
        /usr/local/lib/)


add_definitions(-DBOOST_LOG_DYN_LINK -DBOOST_ERROR_CODE_HEADER_ONLY -DSFMT_MEXP=19937)
add_compile_options(-Wall)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
message("CMAKE_CXX_FLAGS_DEBUG = ${CMAKE_CXX_FLAGS_DEBUG}")
message("CMAKE_CXX_FLAGS_RELEASE = ${CMAKE_CXX_FLAGS_RELEASE}")

add_custom_target(external_bliss COMMAND make || make clean all
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/bliss-0.73/)
add_custom_target(external_pcg COMMAND make || make clean all
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/pcg-cpp-0.98/)
add_custom_target(external_SFMT COMMAND make || make clean all
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/SFMT-src-1.5.1/)

add_library(librgpm
        src/BasicEmbedding.cpp
        src/BasicEmbeddingSpace.cpp
        src/canonical.cpp
        src/dataReader.cpp
        src/edge.cpp
        src/EmbeddingUtils.cpp
        src/graph.cpp
        src/graphSetReader.cpp
        src/logging.cpp
        src/MatrioskaEmbeddingSpace.cpp
        src/node.cpp
        src/random.cpp
        src/RGPMEmbeddingSpace.cpp
        src/strmisc.cpp
        src/SuperEmbedding.cpp
        src/timemisc.cpp
        src/utils.cpp
        src/VertexInducedEmbedding.cpp
        src/include/Aggregator.h
        src/include/AggregatorPatternCounter.h
        src/include/BasicEmbedding.h
        src/include/BasicEmbeddingSpace.h
        src/include/EdgeInducedEmbedding.h
        src/include/EmbeddingUtils.h
        src/include/MatrioskaEmbeddingSpace.h
        src/include/RGPMEmbeddingSpace.h
        src/include/Stats.h
        src/include/SuperEmbedding.h
        src/include/TourStats.h
        src/include/VertexInducedEmbedding.h
        src/include/cache.h
        src/include/canonical.h
        src/include/constants.h
        src/include/dataReader.h
        src/include/edge.h
        src/include/graph.h
        src/include/graphSetReader.h
        src/include/include_nauty.h
        src/include/logging.h
        src/include/node.h
        src/include/patternCode.h
        src/include/random.h
        src/include/reservoir.h
        src/include/strmisc.h
        src/include/timemisc.h
        src/include/utils.h
        )
add_dependencies(librgpm external_bliss external_pcg external_SFMT)
find_package(Threads)

find_package(Boost COMPONENTS program_options log thread system filesystem REQUIRED)
target_link_libraries(librgpm
        sfmt bliss
        gsl gslcblas
        m stdc++
        tbb ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES}
        )



add_executable(matrioska
        ./apps/matrioska/rwQuery.cpp)
target_link_libraries(matrioska
        librgpm
        sfmt bliss
        gsl gslcblas
        m stdc++
        tbb ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES}
)

add_executable(rgpm
        ./apps/rgpm/rwQuery.cpp)
target_link_libraries(rgpm
        librgpm
        sfmt bliss
        gsl gslcblas
        m stdc++
        tbb ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES}
        )

add_executable(psrw
        ./apps/srw-psrw/rwQuery.cpp)
target_link_libraries(psrw
        librgpm
        sfmt bliss
        gsl gslcblas
        m stdc++
        tbb ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES}
        )