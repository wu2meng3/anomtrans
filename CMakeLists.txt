project(anomtrans)
cmake_minimum_required(VERSION 2.6.4) # minimum for googletest

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})

# Dependencies inside the project directory.
set(JSON_INCLUDE_DIR "dependencies/json/src")
set(GOOGLETEST_DIR "dependencies/googletest")
set(GOOGLETEST_INCLUDE_DIR "dependencies/googletest/googletest/include")
set(EIGEN_INCLUDE_DIR "dependencies/Eigen")

# System-provided dependencies.
set(PETSC_INCLUDE_DIR "$ENV{PETSC_DIR}/include")
set(BOOST_INCLUDE_DIR $ENV{BOOST_INC})
set(PETSC_ARCH_INCLUDE_DIR "$ENV{PETSC_DIR}/$ENV{PETSC_ARCH}/include")

# Build gtest.
add_subdirectory(${GOOGLETEST_DIR})

# Set include directories for dependencies.
include_directories(${GOOGLETEST_INCLUDE_DIR})
include_directories(${BOOST_INCLUDE_DIR})
include_directories(${PETSC_INCLUDE_DIR})
include_directories(${PETSC_ARCH_INCLUDE_DIR})
include_directories(SYSTEM ${JSON_INCLUDE_DIR})
include_directories(SYSTEM ${EIGEN_INCLUDE_DIR})

# anomtrans include directory.
include_directories("include/anomtrans")

# Libraries to link.
add_library(petsc SHARED IMPORTED)
set_target_properties(petsc PROPERTIES IMPORTED_LOCATION "$ENV{PETSC_LIB}/libpetsc.so")
set(ANOMTRANS_LIBS petsc)
set(ANOMTRANS_TEST_LIBS ${ANOMTRANS_LIBS} gtest gtest_main anomtrans)

if(DEFINED ENV{ANOMTRANS_TEST_LINK_SAN})
  set(ANOMTRANS_LIBS ${ANOMTRANS_LIBS} ubsan)
  set(ANOMTRANS_TEST_LIBS ${ANOMTRANS_TEST_LIBS} ubsan)
endif()

# Enable OpenMP. Need to disable some warnings due to FindOpenMP:
# https://gitlab.kitware.com/cmake/cmake/issues/17476
# https://github.com/Microsoft/CMake/commit/dd5324ab1e68d745e0e83e0d0fe75fd5c9774bfa
cmake_policy(SET CMP0012 NEW)
cmake_policy(SET CMP0054 NEW)
find_package(OpenMP)
if (OPENMP_FOUND)
  set (ANOMTRANS_OpenMP_CXX_FLAGS "${OpenMP_CXX_FLAGS}")
else()
  set (ANOMTRANS_OpenMP_CXX_FLAGS "")
endif()

enable_testing()

# OpenMPI will not run as the root user without `--allow-run-as-root`.
# Conventionally, Docker runs as root inside the container.
# Set this flag to simplify running `anomtrans` from Docker.
if (DEFINED ENV{ANOMTRANS_MPI_ALLOW_ROOT})
  set (ANOMTRANS_MPI_ALLOW_ROOT_FLAG "--allow-run-as-root")
else()
  set (ANOMTRANS_MPI_ALLOW_ROOT_FLAG "")
endif()

# Function to set up a test target.
# Given a test program `some_test_name_test.cpp`, call `add_mpi_test(some_test_name)`
# to register the test.
function(add_mpi_test name)
  add_executable(${name} ${name}_test.cpp)
  set_target_properties(${name} PROPERTIES
    COMPILE_FLAGS "${CMAKE_CXX_FLAGS} ${ANOMTRANS_OpenMP_CXX_FLAGS} $ENV{ANOMTRANS_TEST_COMPILE_FLAGS}"
    LINK_FLAGS "${ANOMTRANS_OpenMP_CXX_FLAGS}"
  )
  target_link_libraries(${name} ${ANOMTRANS_TEST_LIBS})
  set(test_parameters ${ANOMTRANS_MPI_ALLOW_ROOT_FLAG} $ENV{ANOMTRANS_TEST_MPI_NUMPROCS_PREFIX} $ENV{ANOMTRANS_TEST_MPI_NUMPROCS} $ENV{ANOMTRANS_TEST_AFFINITY} "./${name}")
  add_test(NAME ${name} COMMAND $ENV{ANOMTRANS_TEST_MPI_CMD} ${test_parameters})
  set_property(TEST ${name} PROPERTY
    ENVIRONMENT "ANOMTRANS_TEST_DATA_DIR=$ENV{ANOMTRANS_TEST_DATA_DIR}")
  set_property(TEST ${name} APPEND PROPERTY
    ENVIRONMENT "OMP_NUM_THREADS=$ENV{ANOMTRANS_TEST_OMP_NUM_THREADS}")
endfunction(add_mpi_test)

add_subdirectory(src)
add_subdirectory(test)
