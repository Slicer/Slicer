
#-----------------------------------------------------------------------------
## The FORCE_BUILD_CHECK macro adds a forecebuild step that will cause the
## external project build process to be checked for updates each time
## a dependent project is built.  It MUST be called AFTER the ExternalProject_Add
## step for the project that you want to force building on.
macro(FORCE_BUILD_CHECK  proj)
    ExternalProject_Add_Step(${proj} forcebuild
      COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BUILD_DIR}/${proj}-prefix/src/${proj}-stamp/${proj}-build
      DEPENDEES configure
      DEPENDERS build
      ALWAYS 1
    )
endmacro()

#-----------------------------------------------------------------------------
## empty until ITK is brought into here as an ExternalProject
macro(PACKAGE_NEEDS_ITK LOCAL_CMAKE_BUILD_OPTIONS gen)
  set(packageToCheck ITK)
  OPTION(OPT_USE_SYSTEM_${packageToCheck} "Use the system's ${packageToCheck} library." OFF)
  #  MARK_AS_ADVANCED(OPT_USE_SYSTEM_${packageToCheck})
  if(OPT_USE_SYSTEM_ITK)
    find_package(ITK REQUIRED)
    include(${ITK_USE_FILE})
    set(ITK_DEPEND "") ## Set the external depandancy for ITK
  else()
    set(proj Insight)
    ExternalProject_Add(${proj}
    GIT_REPOSITORY "http://itk.org/ITK.git"
    # This tag fixes two items:  #1) ImageDuplicator for vector images, #2) respect metric->NumberOfThreads
    GIT_TAG "0219c81682779b7cc51166578b5e3af5a1b887d1"
      UPDATE_COMMAND ""
      SOURCE_DIR ${proj}
      BINARY_DIR ${proj}-build
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${LOCAL_CMAKE_BUILD_OPTIONS}
        -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
        -DBUILD_TESTING:BOOL=OFF
        -DITK_USE_REVIEW:BOOL=ON
        -DITK_USE_REVIEW_STATISTICS:BOOL=ON
        -DITK_USE_TRANSFORM_IO_FACTORIES:BOOL=ON
        -DITK_USE_ORIENTED_IMAGE_DIRECTION:BOOL=ON
        -DITK_IMAGE_BEHAVES_AS_ORIENTED_IMAGE:BOOL=ON
        -DITK_USE_OPTIMIZED_REGISTRATION_METHODS:BOOL=ON
        -DITK_USE_PORTABLE_ROUND:BOOL=ON
        -DITK_USE_CENTERED_PIXEL_COORDINATES_CONSISTENTLY:BOOL=ON
        -DITK_USE_TRANSFORM_IO_FACTORIES:BOOL=ON
        -DITK_LEGACY_REMOVE:BOOL=ON
        INSTALL_COMMAND ""
        #    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}
    )
    FORCE_BUILD_CHECK(${proj})
    set(ITK_DIR ${CMAKE_CURRENT_BINARY_DIR}/Insight-build)
    set (ITK_DEPEND ${proj}) ## Set the internal dependancy for ITK
  endif()
endmacro()

#-----------------------------------------------------------------------------
# Get and build VTK
#
macro(PACKAGE_NEEDS_VTKWITHQT LOCAL_CMAKE_BUILD_OPTIONS gen)
  set(packageToCheck VTK)
  OPTION(OPT_USE_SYSTEM_${packageToCheck} "Use the system's ${packageToCheck} library." OFF)
  #  MARK_AS_ADVANCED(OPT_USE_SYSTEM_${packageToCheck})
  if(OPT_USE_SYSTEM_VTK)
    find_package(VTK 5.6 REQUIRED)
    include(${VTK_USE_FILE})
    set(VTK_DEPEND "") ## Set the external depandancy for ITK
  else()
    set(proj vtk-5-6)
    set(vtk_tag -r VTK-5-6)
    set(vtk_module VTK)

    set(vtk_WRAP_TCL OFF)
    set(vtk_WRAP_PYTHON OFF)

    find_package(Qt4 REQUIRED)
    if(QT_USE_FILE)
      include(${QT_USE_FILE})
    endif(QT_USE_FILE)
    set(QT_ARGS
        -DDESIRED_QT_VERSION:STRING=4
        -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}
      )

    set(vtk_QT_ARGS)
    set(vtk_QT_ARGS
        ${QT_ARGS}
        -DVTK_USE_GUISUPPORT:BOOL=ON
        -DVTK_USE_QVTK:BOOL=ON
        -DVTK_USE_QT:BOOL=ON
    )
    if(APPLE)
      # Qt 4.6 binary libs are built with empty OBJCXX_FLAGS for mac Cocoa
      set(vtk_QT_ARGS
        ${vtk_QT_ARGS}
        -DVTK_USE_CARBON:BOOL=OFF
        -DVTK_USE_COCOA:BOOL=ON
        -DVTK_REQUIRED_OBJCXX_FLAGS:STRING=
        )
    endif(APPLE)

    ExternalProject_Add(${proj}
      GIT_REPOSITORY "git://vtk.org/VTK.git"
      GIT_TAG "v5.6.1"
      # CVS_REPOSITORY ":pserver:anonymous:vtk@public.kitware.com:/cvsroot/VTK"
      # CVS_MODULE "${vtk_module}"
      # CVS_TAG ${vtk_tag}
      UPDATE_COMMAND ""
      SOURCE_DIR ${proj}
      BINARY_DIR ${proj}-build
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${LOCAL_CMAKE_BUILD_OPTIONS}
        -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
        -DVTK_USE_PARALLEL:BOOL=ON
        -DVTK_DEBUG_LEAKS:BOOL=OFF
        -DVTK_WRAP_TCL:BOOL=${vtk_WRAP_TCL}
        -DVTK_WRAP_PYTHON:BOOL=${vtk_WRAP_PYTHON}
        ${vtk_QT_ARGS}
      INSTALL_COMMAND ""
      #  INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
    )
    FORCE_BUILD_CHECK(${proj})

    set(VTK_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)
    set(VTK_DEPEND ${proj})
    MESSAGE(STATUS "Setting VTK_DIR to -DVTK_DIR:PATH=${VTK_DIR}")
    set(VTK_CMAKE
       -DVTK_DIR:PATH=${VTK_DIR}
        ${QT_ARGS}
    )
  endif()
endmacro()

macro(PACKAGE_NEEDS_VTK_NOGUI LOCAL_CMAKE_BUILD_OPTIONS gen)
  set(packageToCheck VTK)
  OPTION(OPT_USE_SYSTEM_${packageToCheck} "Use the system's ${packageToCheck} library." OFF)
  #  MARK_AS_ADVANCED(OPT_USE_SYSTEM_${packageToCheck})
  if(OPT_USE_SYSTEM_VTK)
    find_package(VTK 5.6 REQUIRED)
    include(${VTK_USE_FILE})
    set(VTK_DEPEND "") ## Set the external depandancy for ITK
  else()
    set(proj vtk-5-6)
    set(vtk_tag -r VTK-5-6)
    set(vtk_module VTK)

    set(vtk_WRAP_TCL OFF)
    set(vtk_WRAP_PYTHON OFF)

    set(vtk_GUI_ARGS
        -DVTK_USE_GUISUPPORT:BOOL=OFF
        -DVTK_USE_QVTK:BOOL=OFF
        -DVTK_USE_QT:BOOL=OFF
        -DVTK_USE_X:BOOL=OFF
        -DVTK_USE_CARBON:BOOL=OFF
        -DVTK_USE_COCOA:BOOL=OFF
        -DVTK_USE_RENDERING:BOOL=OFF
    )
    if(APPLE)
      # Qt 4.6 binary libs are built with empty OBJCXX_FLAGS for mac Cocoa
      set(vtk_GUI_ARGS
        ${vtk_GUI_ARGS}
        -DVTK_REQUIRED_OBJCXX_FLAGS:STRING=
        )
    endif(APPLE)

    ExternalProject_Add(${proj}
      CVS_REPOSITORY ":pserver:anonymous:vtk@public.kitware.com:/cvsroot/VTK"
      CVS_MODULE "${vtk_module}"
      CVS_TAG ${vtk_tag}
      UPDATE_COMMAND ""
      SOURCE_DIR ${proj}
      BINARY_DIR ${proj}-build
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${LOCAL_CMAKE_BUILD_OPTIONS}
        -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
        -DVTK_USE_PARALLEL:BOOL=ON
        -DVTK_DEBUG_LEAKS:BOOL=OFF
        -DVTK_WRAP_TCL:BOOL=${vtk_WRAP_TCL}
        -DVTK_WRAP_PYTHON:BOOL=${vtk_WRAP_PYTHON}
        ${vtk_GUI_ARGS}
      INSTALL_COMMAND ""
      #  INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
    )
    FORCE_BUILD_CHECK(${proj})

    set(VTK_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)
    set(VTK_DEPEND ${proj})
    MESSAGE(STATUS "Setting VTK_DIR to -DVTK_DIR:PATH=${VTK_DIR}")
    set(VTK_CMAKE
       -DVTK_DIR:PATH=${VTK_DIR}
    )
  endif()
endmacro()

macro(PACKAGE_NEEDS_VTK_NOGUI LOCAL_CMAKE_BUILD_OPTIONS gen)
  set(packageToCheck VTK)
  OPTION(OPT_USE_SYSTEM_${packageToCheck} "Use the system's ${packageToCheck} library." OFF)
  #  MARK_AS_ADVANCED(OPT_USE_SYSTEM_${packageToCheck})
  if(OPT_USE_SYSTEM_VTK)
    find_package(VTK 5.6 REQUIRED)
    include(${VTK_USE_FILE})
    set(VTK_DEPEND "") ## Set the external depandancy for ITK
  else()
    set(proj vtk-5-6)
    set(vtk_tag -r VTK-5-6)
    set(vtk_module VTK)

    set(vtk_WRAP_TCL OFF)
    set(vtk_WRAP_PYTHON OFF)

    set(vtk_GUI_ARGS
        -DVTK_USE_GUISUPPORT:BOOL=OFF
        -DVTK_USE_QVTK:BOOL=OFF
        -DVTK_USE_QT:BOOL=OFF
        -DVTK_USE_X:BOOL=OFF
        -DVTK_USE_CARBON:BOOL=OFF
        -DVTK_USE_COCOA:BOOL=OFF
        -DVTK_USE_RENDERING:BOOL=OFF
    )
    if(APPLE)
      # Qt 4.6 binary libs are built with empty OBJCXX_FLAGS for mac Cocoa
      set(vtk_GUI_ARGS
        ${vtk_GUI_ARGS}
        -DVTK_REQUIRED_OBJCXX_FLAGS:STRING=
        )
    endif(APPLE)

    ExternalProject_Add(${proj}
      CVS_REPOSITORY ":pserver:anonymous:vtk@public.kitware.com:/cvsroot/VTK"
      CVS_MODULE "${vtk_module}"
      CVS_TAG ${vtk_tag}
      UPDATE_COMMAND ""
      SOURCE_DIR ${proj}
      BINARY_DIR ${proj}-build
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${LOCAL_CMAKE_BUILD_OPTIONS}
        -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
        -DVTK_USE_PARALLEL:BOOL=ON
        -DVTK_DEBUG_LEAKS:BOOL=OFF
        -DVTK_WRAP_TCL:BOOL=${vtk_WRAP_TCL}
        -DVTK_WRAP_PYTHON:BOOL=${vtk_WRAP_PYTHON}
        ${vtk_GUI_ARGS}
      INSTALL_COMMAND ""
      #  INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
    )
    FORCE_BUILD_CHECK(${proj})

    set(VTK_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)
    set(VTK_DEPEND ${proj})
    MESSAGE(STATUS "Setting VTK_DIR to -DVTK_DIR:PATH=${VTK_DIR}")
    set(VTK_CMAKE
       -DVTK_DIR:PATH=${VTK_DIR}
    )
  endif()
endmacro()

macro(PACKAGE_NEEDS_VTK_WITHR LOCAL_CMAKE_BUILD_OPTIONS gen)
  set(packageToCheck VTK)
  OPTION(OPT_USE_SYSTEM_${packageToCheck} "Use the system's ${packageToCheck} library." OFF)
  #  MARK_AS_ADVANCED(OPT_USE_SYSTEM_${packageToCheck})
  if(OPT_USE_SYSTEM_VTK)
    find_package(VTK 5.7 REQUIRED)
    include(${VTK_USE_FILE})
    set(VTK_DEPEND "") ## Set the external depandancy for ITK
  else()
    set(proj vtk-head)

    set(vtk_WRAP_TCL OFF)
    set(vtk_WRAP_PYTHON OFF)

    set(vtk_GUI_ARGS
        -DVTK_USE_GUISUPPORT:BOOL=OFF
        -DVTK_USE_QVTK:BOOL=OFF
        -DVTK_USE_QT:BOOL=OFF
        -DVTK_USE_X:BOOL=OFF
        -DVTK_USE_CARBON:BOOL=OFF
        -DVTK_USE_COCOA:BOOL=OFF
        -DVTK_USE_RENDERING:BOOL=OFF
    )
    if(APPLE)
      # Qt 4.6 binary libs are built with empty OBJCXX_FLAGS for mac Cocoa
      set(vtk_GUI_ARGS
        ${vtk_GUI_ARGS}
        -DVTK_REQUIRED_OBJCXX_FLAGS:STRING=
        )
    endif(APPLE)

    ExternalProject_Add(${proj}
      GIT_REPOSITORY "git://vtk.org/VTK.git"
      SOURCE_DIR ${proj}
      BINARY_DIR ${proj}-build
      DEPENDS ${R_DEPEND}
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${LOCAL_CMAKE_BUILD_OPTIONS}
        -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
        -DVTK_USE_PARALLEL:BOOL=OFF
        -DVTK_DEBUG_LEAKS:BOOL=OFF
        -DVTK_WRAP_TCL:BOOL=${vtk_WRAP_TCL}
        -DVTK_WRAP_PYTHON:BOOL=${vtk_WRAP_PYTHON}
        -DVTK_USE_GNU_R:BOOL=ON
        -DR_COMMAND:FILEPATH=${R_DIR}/bin/R
        -DR_INCLUDE_DIR:PATH=${R_DIR}/include                                                                       
        -DR_LIBRARY_BASE=${R_DIR}/lib                                                                                 
        -DR_LIBRARY_BLAS:FILEPATH=${R_DIR}/lib/libRblas${CMAKE_SHARED_LIBRARY_SUFFIX}                                                   
        -DR_LIBRARY_LAPACK:FILEPATH=${R_DIR}/lib/libRlapack${CMAKE_SHARED_LIBRARY_SUFFIX}                                                  
        #-DVTK_R_HOME:PATH=${CMAKE_CURRENT_BINARY_DIR}/R-build
        ${vtk_GUI_ARGS}
      INSTALL_COMMAND ""
      #  INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
    )
    FORCE_BUILD_CHECK(${proj})

    set(VTK_DIR ${CMAKE_CURRENT_BINARY_DIR}/${proj}-build)
    set(VTK_DEPEND ${proj})
    MESSAGE(STATUS "Setting VTK_DIR to -DVTK_DIR:PATH=${VTK_DIR}")
    set(VTK_CMAKE
       -DVTK_DIR:PATH=${VTK_DIR}
    )
  endif()
endmacro()
#-----------------------------------------------------------------------------
# Get and build SlicerExecutionModel
##  Build the SlicerExecutionModel Once, and let all derived project use the same version
macro(PACKAGE_NEEDS_SlicerExecutionModel LOCAL_CMAKE_BUILD_OPTIONS gen)
  set(packageToCheck SlicerExecutionModel)
  OPTION(OPT_USE_SYSTEM_${packageToCheck} "Use the system's ${packageToCheck} library." OFF)
  #  MARK_AS_ADVANCED(OPT_USE_SYSTEM_${packageToCheck})
  if(OPT_USE_SYSTEM_SlicerExecutionModel)
    find_package(GenerateCLP NO_MODULE REQUIRED)
    include(${GenerateCLP_USE_FILE})
    set(GenerateCLP_DEPEND "")
    set(SlicerExecutionModel_DEPEND "")
  else()
    #### ALWAYS BUILD WITH STATIC LIBS
    set(proj SlicerExecutionModel)
    ExternalProject_Add(${proj}
      SVN_REPOSITORY "http://svn.slicer.org/Slicer3/trunk/Libs/SlicerExecutionModel"
      SOURCE_DIR ${proj}
      BINARY_DIR ${proj}-build
      DEPENDS ${ITK_DEPEND}
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${LOCAL_CMAKE_BUILD_OPTIONS}
        -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
        -DITK_DIR:PATH=${ITK_DIR}
        INSTALL_COMMAND ""
        # INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}
    )
    FORCE_BUILD_CHECK(${proj})
    set(GenerateCLP_DIR ${CMAKE_CURRENT_BINARY_DIR}/SlicerExecutionModel-build/GenerateCLP)
    set(GenerateCLP_DEPEND "${proj}")
    set(SlicerExecutionModel_DEPEND "${proj}")
  endif()
endmacro()

#-----------------------------------------------------------------------------
# Get and build BRAINSCommonLib
macro(PACKAGE_NEEDS_BRAINSCommonLib LOCAL_CMAKE_BUILD_OPTIONS gen)
  set(packageToCheck BRAINSCommonLib)
  OPTION(OPT_USE_SYSTEM_${packageToCheck} "Use the system's ${packageToCheck} library." OFF)
  #  MARK_AS_ADVANCED(OPT_USE_SYSTEM_${packageToCheck})
  if(OPT_USE_SYSTEM_BRAINSCommonLib)
    find_package(BRAINSCommonLib NO_MODULE REQUIRED)
    include(${BRAINSCommonLib_USE_FILE})
    set(BRAINSCommonLib_DEPEND "")
  else()
    set(proj BRAINSCommonLib)
    ExternalProject_Add(${proj}
      SVN_REPOSITORY "http://www.nitrc.org/svn/brains/BRAINSCommonLib/trunk"
      SVN_USERNAME "slicerbot"
      SVN_PASSWORD "slicer"
      SOURCE_DIR ${proj}
      BINARY_DIR ${proj}-build
      DEPENDS ${ITK_DEPEND}
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${LOCAL_CMAKE_BUILD_OPTIONS}
        -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
        -DITK_DIR:PATH=${ITK_DIR}
        INSTALL_COMMAND ""
        # INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}
    )
    FORCE_BUILD_CHECK(${proj})
    set(BRAINSCommonLib_DIR ${CMAKE_CURRENT_BINARY_DIR}/BRAINSCommonLib-build)
    set(BRAINSCommonLib_DEPEND "${proj}")
  endif()
endmacro()

#-----------------------------------------------------------------------------
# Get and build R statistical library
macro(PACKAGE_NEEDS_RSTATS LOCAL_CMAKE_BUILD_OPTIONS gen)
  set(packageToCheck R)
  OPTION(OPT_USE_SYSTEM_${packageToCheck} "Use the system's ${packageToCheck} library." OFF)
  #  MARK_AS_ADVANCED(OPT_USE_SYSTEM_${packageToCheck})
  if(OPT_USE_SYSTEM_R)
    find_package(R NO_MODULE REQUIRED)
    include(${R_USE_FILE})
    set(R_DEPEND "")
  else()
    set(proj R)
    ExternalProject_Add(${proj}
      URL "http://streaming.stat.iastate.edu/CRAN/src/base/R-2/R-2.12.0.tar.gz"
      SOURCE_DIR ${proj}
      BINARY_DIR ${proj}-build
      DEPENDS ""
      CONFIGURE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/R/configure --prefix=${CMAKE_CURRENT_BINARY_DIR} --enable-BLAS-shlib
      #CMAKE_ARGS
      #  ${LOCAL_CMAKE_BUILD_OPTIONS}
      #  -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
      #  -DITK_DIR:PATH=${ITK_DIR}
      #  INSTALL_COMMAND ""
        # INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}
    )
    FORCE_BUILD_CHECK(${proj})
    set(R_DIR ${CMAKE_CURRENT_BINARY_DIR}/R-build)
    set(R_DEPEND "${proj}")
  endif()
endmacro()
