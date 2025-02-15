# The name of our project is “ATTPC”.  CMakeLists files in this project can
# refer to the root source directory of the project as ${ATTPC_SOURCE_DIR}
# or as ${CMAKE_SOURCE_DIR} and to the root binary directory of the project as
# ${ATTPC_BINARY_DIR} or ${CMAKE_BINARY_DIR}.
# This difference is important for the base classes which are in FAIRROOT
# and the experiment part.

if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(ColourBold  "${Esc}[1m")
  set(Red         "${Esc}[31m")
  set(Green       "${Esc}[32m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Magenta     "${Esc}[35m")
  set(Cyan        "${Esc}[36m")
  set(White       "${Esc}[37m")
  set(BoldRed     "${Esc}[1;31m")
  set(BoldGreen   "${Esc}[1;32m")
  set(BoldYellow  "${Esc}[1;33m")
  set(BoldBlue    "${Esc}[1;34m")
  set(BoldMagenta "${Esc}[1;35m")
  set(BoldCyan    "${Esc}[1;36m")
  set(BoldWhite   "${Esc}[1;37m")
endif()

# Check if cmake has the required version
CMAKE_MINIMUM_REQUIRED(VERSION  3.11.0 FATAL_ERROR)
cmake_policy(VERSION 3.11...3.14)

### CMP0025   Compiler id for Apple Clang is now AppleClang.
### CMP0042   MACOSX_RPATH is enabled by default.

foreach(p
  CMP0025 # CMake 3.0
  CMP0028 # double colon for imported and alias targets
  CMP0042 # CMake 3.0
  )
  if(POLICY ${p})
  cmake_policy(SET ${p} NEW)
  endif()
endforeach()

# Set name of our project to “ATTPC". Has to be done
# after check of cmake version since this is a new feature
project(ATTPC)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#In case you need Fortran
#ENABLE_LANGUAGE(Fortran)

# Check for needed environment variables
IF(NOT DEFINED ENV{FAIRROOTPATH})
  MESSAGE(FATAL_ERROR "You did not define the environment variable FAIRROOTPATH which is needed to find FairRoot. Please set this variable and execute cmake again.")
ENDIF(NOT DEFINED ENV{FAIRROOTPATH})

IF(NOT DEFINED ENV{SIMPATH})
   MESSAGE(FATAL_ERROR "You did not define the environment variable SIMPATH which is nedded to find the external packages. Please set this variable and execute cmake again.")
ENDIF(NOT DEFINED ENV{SIMPATH})

SET(SIMPATH $ENV{SIMPATH})
SET(FAIRROOTPATH $ENV{FAIRROOTPATH})

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/
# is checked
#Remove defaults becuse we want to fource config loading over FindModules
#set(CMAKE_MODULE_PATH "${FAIRROOTPATH}/share/fairbase/cmake/modules"  ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

Set(CheckSrcDir "${FAIRROOTPATH}/share/fairbase/cmake/checks")

if(SIMPATH)
  set(CMAKE_PREFIX_PATH ${SIMPATH} ${CMAKE_PREFIX_PATH})
  set(LD_LIBRARY_PATH ${LD_LIBRARY_PATH} ${SIMPATH}/lib)
endif()


find_package(FairRoot REQUIRED)

# Load some basic macros which are needed later on
include(FairMacros)
include(WriteConfigFile)
include(CTest)
include(CheckCompiler)
include(AtROOTMacros)
include(FairRootTargetRootDictionary)
include(GNUInstallDirs)
  
#include(CheckFortran)

#set(CMAKE_CXX_FLAGS -std=c++14)

#Check the compiler and set the compile and link flags
If(NOT CMAKE_BUILD_TYPE)
  Message("Set BuildType to RELWITHDEBINFO")
  set(CMAKE_BUILD_TYPE RELWITHDEBINFO)
EndIf(NOT CMAKE_BUILD_TYPE)

Check_Compiler()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++14)
# If we are building debug then
message("Building in ${CMAKE_BUILD_TYPE}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message("Suppressing warning from precompiler conditions because of FairLogger!")
  add_compile_options(-Wno-cpp "-Wno-#warnings")
endif()

#String(FIND "${CMAKE_CXX_FLAGS}" "-std=c++14" POS_C++11)
#If(${POS_C++11} EQUAL -1)
#      Message(FATAL_ERROR "FairSoft wasn't compiled with c++11, c++14 or c++17 support. Please recompile FairSoft with a compiler which supports at least c++11.")
# EndIf()


set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(INCLUDE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/include")
Set(VMCWORKDIR ${CMAKE_SOURCE_DIR})
Option(USE_PATH_INFO "Information from PATH and LD_LIBRARY_PATH are
used." OFF
)
If(USE_PATH_INFO)
  Set(PATH "$PATH")
  If (APPLE)
    Set(LD_LIBRARY_PATH $ENV{DYLD_LIBRARY_PATH})
  Else (APPLE)
    Set(LD_LIBRARY_PATH $ENV{LD_LIBRARY_PATH})
  EndIf (APPLE)
Else(USE_PATH_INFO)
  STRING(REGEX MATCHALL "[^:]+" PATH $ENV{PATH})
EndIf(USE_PATH_INFO)


# Check if the user wants to build the project in the source
# directory
CHECK_OUT_OF_SOURCE_BUILD()

# Check if we are on an UNIX system. If not stop with an error
# message
IF(NOT UNIX)
  MESSAGE(FATAL_ERROR "You're not on an UNIX system. The project was up to now only tested on UNIX systems, so we break here. If you want to go on please edit the CMakeLists.txt in the source directory.")
ENDIF(NOT UNIX)

# Check if the external packages are installed into a separate install
# directory
CHECK_EXTERNAL_PACKAGE_INSTALL_DIR()

# searches for needed packages
# REQUIRED means that cmake will stop if this packages are not found
# For example the framework can run without GEANT4, but ROOT is
# mandatory


message("Looking for config files in : ${CMAKE_PREFIX_PATH}")
find_package2(PUBLIC ROOT 6.16 CONFIG)
if(NOT ROOT_FOUND)
  find_package2(PUBLIC ROOT 6.0 REQUIRED)
  message("Looked for ROOT using modules and found ${ROOT_VERSION}")
endif()

include(ROOTMacros)
if(ROOT_vmc_FOUND)
  set(VMCLIB ROOT::VMC)
else()
  message("NO VMC")
  find_package2(PUBLIC VMC REQUIRED)
  set(VMCLIB VMCLibrary)
endif()

#Look for target
#get_target_property(dirs ROOT::Core INTERFACE_LINK_DIRECTORIES)
#message("base: ${dirs}")


find_package2(PUBLIC FairLogger  VERSION 1.2.0 REQUIRED)
foreach(dep IN LISTS FairLogger_PACKAGE_DEPENDENCIES)
  if(NOT dep STREQUAL "Boost")
    find_package2(PUBLIC ${dep} REQUIRED VERSION ${FairLogger_${dep}_VERSION})
    set(PROJECT_${dep}_VERSION ${FairLogger_${dep}_VERSION})
  endif()
endforeach()

find_package2(PUBLIC Pythia6 REQUIRED)
find_package2(PUBLIC Pythia8)
find_package2(PUBLIC Geant3)
find_package2(PUBLIC Geant4)
Find_Package2(PUBLIC VGM)
find_package2(PUBLIC Geant4VMC)
find_package2(PUBLIC CLHEP)
find_package2(PUBLIC HEPMC)

Set(Boost_NO_SYSTEM_PATHS TRUE)
Set(Boost_NO_BOOST_CMAKE TRUE)
If(${ROOT_LIBRARY_DIR} MATCHES /lib/root)
  set(BOOST_ROOT ${SIMPATH})
  set(GSL_DIR ${SIMPATH})
Else(${ROOT_LIBRARY_DIR} MATCHES /lib/root)
  set(BOOST_ROOT ${SIMPATH}/basics/boost)
  set(GSL_DIR ${SIMPATH}/basics/gsl)
EndIf(${ROOT_LIBRARY_DIR} MATCHES /lib/root)


if(NOT DEFINED Boost_NO_SYSTEM_PATHS)
  Set(Boost_NO_SYSTEM_PATHS TRUE)
endif()
if(NOT DEFINED Boost_NO_BOOST_CMAKE AND CMAKE_VERSION VERSION_LESS 3.15)
  Set(Boost_NO_BOOST_CMAKE TRUE)
endif()
if(Boost_NO_BOOST_CMAKE)
  # If an older version of boost is found both of the variables below are
  # cached and in a second cmake run, a good boost version is found even
  # if the version is to old.
  # To overcome this problem both variables are cleared before checking
  # for boost.
  Unset(Boost_INCLUDE_DIR CACHE)
  Unset(Boost_LIBRARY_DIRS CACHE)
endif()

find_package2(PUBLIC Boost
  VERSION 1.67
  ADD_REQUIREMENTS_OF FairRoot FairMQ FairLogger
)

If (Boost_FOUND)
  Set(Boost_Avail 1)
Else (Boost_FOUND)
  Set(Boost_Avail 0)
EndIf (Boost_FOUND)

## Add additional packages beyond the initial required for FairRoot
find_package2(PUBLIC HDF5 REQUIRED CONFIG)
find_package2(PUBLIC PCL 1.8 CONFIG)
find_package2(PUBLIC GEANT4DATA)
find_package2(PUBLIC GENFIT2)
find_package2(PUBLIC OpenMP)
find_package2(PUBLIC CUDA)
find_package2(PUBLIC Eigen3 CONFIG)
find_package2(PUBLIC FLANN)
find_package2(PUBLIC VTK)
find_package2(PUBLIC HiRAEVT)
message("Found HiRAEVT: ${HiRAEVT_FOUND}")

# Possibly unused packages? #
#find_package(GENERATORS REQUIRED) this was just looking for pythia6...
#It looks like cernlib stopped development while ago and is not used anymore
#find_package(CERNLIB)

# set a variable which should be used in all CMakeLists.txt
# Defines all basic include directories from fairbase
SetBasicVariables()

# Set the library version in the main CMakeLists.txt
SET(ATTPCROOT_MAJOR_VERSION 0)
SET(ATTPCROOT_MINOR_VERSION 0)
SET(ATTPCROOT_PATCH_VERSION 0)
SET(ATTPCROOT_VERSION "${ATTPCROOT_MAJOR_VERSION}.${ATTPCROOT_MINOR_VERSION}.${ATTPCROOT_PATCH_VERSION}")

set(ATTPCROOT_LIBRARY_PROPERTIES ${ATTPCROOT_LIBRARY_PROPERTIES}
  VERSION "${ATTPCROOT_VERSION}"
  SOVERSION "${ATTPCROOT_MAJOR_VERSION}.${ATTPCROOT_MINOR_VERSION}"
)
set(PROJECT_LIBRARY_PROPERTIES ${ATTPCROOT_LIBRARY_PROPERTIES})

configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/FairRootConfigVersion.cmake.in "${CMAKE_BINARY_DIR}/FairRootConfigVersion.cmake" @ONLY)

Generate_Version_Info()

SET(_LIBDIR ${CMAKE_BINARY_DIR}/lib)
SET(LD_LIBRARY_PATH  ${_LIBDIR} ${LD_LIBRARY_PATH})


# Check if the compiler support specific C++11 features
# Up to now this is only a check since the code does not use
# any of the features of the new standard
include(CheckCXX11Features)

IF(HAS_CXX11_SHAREDPOINTER)
  Add_Definitions(-DHAS_SHAREDPOINTER)
ENDIF(HAS_CXX11_SHAREDPOINTER)

# Recurse into the given subdirectories.  This does not actually
# cause another cmake executable to run.  The same process will walk through
# the project's entire directory structure.

# Make sure that we always return something when we say we are
# I have seen this cause all sorts of errors in the code from segfaults to
# infinite loops. 

add_subdirectory (AtData)
add_subdirectory (AtGenerators)
add_subdirectory (AtTpc)
add_subdirectory (AtSiArray)
add_subdirectory (AtPassive)
add_subdirectory (AtField)
add_subdirectory (AtMap)
add_subdirectory (AtReconstruction)
add_subdirectory (AtParameter)
add_subdirectory (AtDigitization)
add_subdirectory (AtS800)
add_subdirectory (AtApollo)
add_subdirectory (AtAnalysis)

if (PCL_FOUND)
  add_subdirectory (AtEventDisplay)
endif()

WRITE_CONFIG_FILE(config.sh)

#### Summary of what was found #####

MESSAGE("${Green} ROOT_FOUND_VERSION = ${ROOT_VERSION}${ColourReset}")

if (OPENMP_CXX_FOUND)
 # MESSAGE("${Green} OpenMP Support Found ${OpenMP_CXX_SPEC_DATE} ${ColourReset}")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  # set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS_DEBUG} ${OpenMP_C_FLAGS}")
  # set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} ${OpenMP_CXX_FLAGS}")
  
endif()
 
#if (PCL_FOUND)
#  MESSAGE("${Green} PCL Support Found ${ColourReset}")
  #MESSAGE(${PCL_INCLUDE_DIRS})
  #MESSAGE(${PCL_DEFINITIONS})
#Else()
#  MESSAGE("${BoldRed} PCL Support NOT Found: Classes and modules depending on PCL will not be built. ${ColourReset}")
#endif()

if(GENFIT2_FOUND)
  MESSAGE("${Green} GENFIT FOUND ${ColourReset}")
  MESSAGE("${Green} ${GENFIT2_INCLUDE_DIR} ${ColourReset}")
  MESSAGE("${Green} ${GENFIT2_LIBRARY_DIR} ${ColourReset}")
  MESSAGE("${Green} ${GENFIT2_LDFLAGS} ${ColourReset}")
endif()
#if(VTK_FOUND)
#  MESSAGE("${Green} VTK Support Found ${ColourReset}")
  #MESSAGE(${VTK_INCLUDE_DIRS})
#endif()

configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake
  ${CMAKE_BINARY_DIR}/CTestCustom.cmake
  )

# Print nice summary


if(CMAKE_CXX_FLAGS)
  message(STATUS "  ")
  message(STATUS "  ${Cyan}GLOBAL CXX FLAGS${CR}  ${BGreen}${CMAKE_CXX_FLAGS}${ColourReset}")
endif()
if(CMAKE_CXX_STANDARD)
  message(STATUS "  ")
  message(STATUS "  ${Cyan}GLOBAL CXX STANDARD${CR}  ${BGreen}c++${CMAKE_CXX_STANDARD}${CR}")
endif()

if(CMAKE_CONFIGURATION_TYPES)
  message(STATUS "  ")
  message(STATUS "  ${Cyan}BUILD TYPE         CXX FLAGS$${ColourReset}")
  string(TOUPPER "${CMAKE_BUILD_TYPE}" selected_type)
  foreach(type IN LISTS CMAKE_CONFIGURATION_TYPES)
   string(TOUPPER "${type}" type_upper)
   if(type_upper STREQUAL selected_type)
     pad("${type}" 18 " " type_padded)
     message(STATUS "${BGreen}* ${type_padded}${CMAKE_CXX_FLAGS_${type_upper}}${ColourReset}")
   else()
     pad("${type}" 18 " " type_padded)
     message(STATUS "  ${BWhite}${type_padded}${CR}${CMAKE_CXX_FLAGS_${type_upper}}")
   endif()
   unset(type_padded)
   unset(type_upper)
 endforeach()
message(STATUS "  ")
message(STATUS "  (Change the build type with ${BMagenta}-DCMAKE_BUILD_TYPE=...${ColourReset})")
endif()

if(PROJECT_PACKAGE_DEPENDENCIES)
  message(STATUS "  ")
  message(STATUS "  ${Cyan}DEPENDENCY FOUND     VERSION                   PREFIX${CR}")
  foreach(dep IN LISTS PROJECT_PACKAGE_DEPENDENCIES)

    if(${dep}_VERSION)
      if(${dep} STREQUAL Boost)
        if(Boost_VERSION_MAJOR)
          set(version_str "${BGreen}${${dep}_VERSION_MAJOR}.${${dep}_VERSION_MINOR}${CR}")
        else()
          set(version_str "${BGreen}${${dep}_MAJOR_VERSION}.${${dep}_MINOR_VERSION}${CR}")
        endif()
      else()
        set(version_str "${BGreen}${${dep}_VERSION}${CR}")
      endif()
    else()
      set(version_str "${BYellow}unknown${CR}")
    endif()
    
    if(PROJECT_${dep}_VERSION)
      set(version_req_str " (>= ${PROJECT_${dep}_VERSION})")
    endif()
    pad(${dep} 20 " " dep_padded)

    if(DISABLE_COLOR)
      pad("${version_str}${version_req_str}" 25 " " version_padded)
    else()
      pad("${version_str}${version_req_str}" 25 " " version_padded COLOR 1)
    endif()

    set(prefix ${${dep}_PREFIX})

    # Search through and get the library locations
    if(${dep} STREQUAL Boost)
      if(TARGET Boost::headers)
        get_target_property(boost_include Boost::headers INTERFACE_INCLUDE_DIRECTORIES)
      else()
        get_target_property(boost_include Boost::boost INTERFACE_INCLUDE_DIRECTORIES)
      endif()
      get_filename_component(prefix ${boost_include}/.. ABSOLUTE)
    elseif(${dep} STREQUAL Pythia6)
      get_filename_component(prefix ${PYTHIA6_LIBRARY_DIR}/.. ABSOLUTE)
    elseif(${dep} STREQUAL Pythia8)
      get_filename_component(prefix ${PYTHIA8_LIB_DIR}/.. ABSOLUTE)
    elseif(${dep} STREQUAL FairLogger)
      if(FairLogger_PREFIX)
        set(prefix ${FairLogger_PREFIX})
      else()
        set(prefix ${FairLogger_ROOT})
      endif()
    elseif(${dep} STREQUAL FairMQ)
      if(FairMQ_PREFIX)
        set(prefix ${FairMQ_PREFIX})
      else()
        set(prefix ${FairMQ_ROOT})
      endif()
    elseif(${dep} STREQUAL Geant4VMC)
      string(REPLACE ":" ";" geant4vmc_include ${Geant4VMC_INCLUDE_DIRS})
      list(GET geant4vmc_include 0 geant4vmc_include)
      get_filename_component(prefix ${geant4vmc_include}/../.. ABSOLUTE)
    elseif(${dep} STREQUAL Geant3)
      get_target_property(geant3_include geant321 INTERFACE_INCLUDE_DIRECTORIES)
      string(REPLACE ":" ";" geant3_include ${geant3_include})
      list(GET geant3_include 0 geant3_include)
      get_filename_component(prefix ${geant3_include}/../.. ABSOLUTE)
    elseif(${dep} STREQUAL Geant4)
      list(GET Geant4_INCLUDE_DIRS 0 geant4_include)
      get_filename_component(prefix ${geant4_include}/../.. ABSOLUTE)
    elseif(${dep} STREQUAL VGM)
      string(REPLACE ":" ";" vgm_include ${VGM_INCLUDE_DIRS})
      list(GET vgm_include 0 vgm_include)
      get_filename_component(prefix ${vgm_include}/.. ABSOLUTE)
    elseif(${dep} STREQUAL ROOT)
      set(prefix ${ROOT_INSTALL_DIR})
    elseif(${dep} STREQUAL VMC)
      get_target_property(vmc_include VMCLibrary INTERFACE_INCLUDE_DIRECTORIES)
      get_filename_component(prefix ${vmc_include}/.. ABSOLUTE)
    elseif(${dep} STREQUAL HDF5)
      get_target_property(hdf5_include hdf5::hdf5-shared INTERFACE_INCLUDE_DIRECTORIES)
      list(GET hdf5_include 0 hdf5_include)
      get_filename_component(prefix ${hdf5_include}/.. ABSOLUTE)
    elseif(${dep} STREQUAL PCL)
      #string(REPLACE ":" ";" pcl_include ${PCL_INCLUDE_DIRS})
      list(GET pcl_include 0 PCL_INCLUDE_DIRS)
      get_filename_component(prefix "${pcl_include}" ABSOLUTE)
    elseif(${dep} STREQUAL Eigen)
      get_filename_component(prefix "${EIGEN_INCLUDE_DIR}" ABSOLUTE)
    elseif(${dep} STREQUAL FLANN)
      get_filename_component(prefix "${FLANN_INCLUDE_DIR}" ABSOLUTE)
    elseif(${dep} STREQUAL GENFIT2)
      get_filename_component(prefix ${GENFIT2_INCLUDE_DIR}/.. ABSOLUTE)
    elseif(${dep} STREQUAL CLHEP)
      get_filename_component(prefix ${CLHEP_INCLUDE_DIR}/.. ABSOLUTE) 
    elseif(${dep} STREQUAL HEPMC)
      get_filename_component(prefix ${HEPMC_INCLUDE_DIR}/.. ABSOLUTE)
    elseif(${dep} STREQUAL OpenMP)
      get_filename_component(prefix "${OPENMP_CXX_INCLUDE_DIRS}/.." ABSOLUTE)
    endif()
    
    message(STATUS "  ${BWhite}${dep_padded}${CR}${version_padded}${prefix}")

    if(${dep} STREQUAL Geant3)
      message(STATUS "                                          G3SYS: ${Geant3_SYSTEM_DIR}")
    endif()
    if(${dep} STREQUAL Geant4)
      foreach(dataset IN LISTS Geant4_DATASETS)
        pad(${Geant4_DATASET_${dataset}_ENVVAR} 20 " " envvar_padded LEFT)
        message(STATUS "                          ${envvar_padded}: ${Geant4_DATASET_${dataset}_PATH}")
      endforeach()
    endif()

    unset(version_str)
    unset(version_padded)
    unset(version_req_str)
  endforeach()
endif()
