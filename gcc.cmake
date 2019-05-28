set(CTEST_SITE "YETI")

set(CTEST_SOURCE_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}")

# set binary directory
if (OPEN_MPI)
  set(CTEST_BINARY_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}/_cmake-${CMAKE_VER}-${OPEN_MPI}")
else()
  set(_SUFFIX "-${GCC_VER}")
  if(BUILD_MPI)
    set(_SUFFIX "-${GCC_VER}_mpi")
  elseif(BUILD_OPENMP AND NOT BUILD_MPI)
    set(_SUFFIX "-${GCC_VER}_openmp")
  endif()
  set(CTEST_BINARY_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}/_cmake-${CMAKE_VER}-gcc${_SUFFIX}")
endif()
file(REMOVE_RECURSE "${CTEST_BINARY_DIRECTORY}")

set(CTEST_BUILD_NAME "gcc${_SUFFIX}")
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
set(CTEST_PROJECT_NAME "vs2drt")

set(BUILD_OPTIONS
  -DCMAKE_INSTALL_PREFIX:PATH=${CTEST_BINARY_DIRECTORY}/INSTALL
  -DCMAKE_C_COMPILER:FILEPATH=gcc
  -DCMAKE_CXX_COMPILER:FILEPATH=g++
  -DCMAKE_Fortran_COMPILER:FILEPATH=gfortran
  )

if(BUILD_MPI)
  set(BUILD_OPTIONS
    ${BUILD_OPTIONS}
    -DVS2DRT_BUILD_MPI:BOOL=ON
    -DVS2DRT_BUILD_OPENMP:BOOL=OFF
    )
elseif(BUILD_OPENMP)
  set(BUILD_OPTIONS
    ${BUILD_OPTIONS}
    -DVS2DRT_BUILD_MPI:BOOL=OFF
    -DVS2DRT_BUILD_OPENMP:BOOL=ON
    )
else()
  set(BUILD_OPTIONS
    ${BUILD_OPTIONS}
    -DVS2DRT_BUILD_MPI:BOOL=OFF
    -DVS2DRT_BUILD_OPENMP:BOOL=OFF
    )
endif()


CTEST_START("Experimental")
#ctest_update([SOURCE source] [RETURN_VALUE res])
CTEST_CONFIGURE(BUILD "${CTEST_BINARY_DIRECTORY}"
                OPTIONS "${BUILD_OPTIONS}")
CTEST_BUILD(BUILD "${CTEST_BINARY_DIRECTORY}" FLAGS -j8)
#CTEST_TEST(BUILD "${CTEST_BINARY_DIRECTORY}")
CTEST_BUILD(BUILD "${CTEST_BINARY_DIRECTORY}" TARGET install)
#CTEST_BUILD(BUILD "${CTEST_BINARY_DIRECTORY}" TARGET PACKAGE)
###CTEST_SUBMIT()
