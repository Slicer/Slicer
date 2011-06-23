message(STATUS "Looking for OpenCV")

macro(DBGMSG MSG)
    message(STATUS ${MSG})
endmacro(DBGMSG)

#
# Windows registry key for installation root.
# インストールルートパス取得用 Windows レジストリキー
#
get_filename_component(OpenCV_ROOT_DIR_CANDIDATES
    "[HKEY_CURRENT_USER\\Software\\Intel\\OpenCV\\Settings;Path]"
    ABSOLUTE CACHE
)

#
# Installation root candidate list.
# インストールルートパス調査対象
#

set(OpenCV_ROOT_DIR_CANDIDATES ${OpenCV_ROOT_DIR_CANDIDATES}
    ${OpenCV_DIR}
    $ENV{ProrgramFiles}/OpenCV
)

set(OpenCV_INCLUDE_DIRS ${OpenCV_DIR}/include/opencv)
set(OpenCV_BIN_DIR ${OpenCV_DIR}/bin)
DBGMSG("OpenCV_INC_DIR: ${OpenCV_INC_DIR}")
DBGMSG("OpenCV_BIN_DIR: ${OpenCV_BIN_DIR}")

if(OpenCV_BIN_DIR)
  set(OpenCV_FOUND ON)
else(OpenCV_BIN_DIR)
  set(OpenCV_FOUND 0)
  message(FATAL_ERROR "Failed to find OpenCV lib.")
endif(OpenCV_BIN_DIR)

#
# Sub directory candidate list for including.
# インクルードディレクトリ調査対象
#
set(OpenCV_INCDIR_SUFFIXES
    include
    include/cv
    include/opencv
    cv/include
    cxcore/include
    cvaux/include
    otherlibs/cvcam/include
    otherlibs/highgui
    otherlibs/highgui/include
    otherlibs/_graphics/include
)

#
# Find sub directories for including.
# インクルードディレクトリ探索
#
find_path(OpenCV_CV_INCLUDE_DIR
    NAMES cv.h
    PATHS ${OpenCV_DIR}
    PATH_SUFFIXES ${OpenCV_INCDIR_SUFFIXES}
)
DBGMSG("OpenCV_CV_INCLUDE_DIR: ${OpenCV_CV_INCLUDE_DIR}")

find_path(OpenCV_CXCORE_INCLUDE_DIR
    NAMES cxcore.h
    PATHS ${OpenCV_DIR}
    PATH_SUFFIXES ${OpenCV_INCDIR_SUFFIXES}
)
DBGMSG("OpenCV_CXCORE_INCLUDE_DIR: ${OpenCV_CXCORE_INCLUDE_DIR}")

find_path(OpenCV_CVAUX_INCLUDE_DIR
    NAMES cvaux.h
    PATHS ${OpenCV_DIR}
    PATH_SUFFIXES ${OpenCV_INCDIR_SUFFIXES}
)
DBGMSG("OpenCV_CVAUX_INCLUDE_DIR: ${OpenCV_CVAUX_INCLUDE_DIR}")

find_path(OpenCV_HIGHGUI_INCLUDE_DIR
    NAMES highgui.h
    PATHS ${OpenCV_DIR}
    PATH_SUFFIXES ${OpenCV_INCDIR_SUFFIXES}
)
DBGMSG("OpenCV_HIGHGUI_INCLUDE_DIR: ${OpenCV_HIGHGUI_INCLUDE_DIR}")

find_path(OpenCV_CVCAM_INCLUDE_DIR
    NAMES cvcam.h
    PATHS ${OpenCV_DIR}
    PATH_SUFFIXES ${OpenCV_INCDIR_SUFFIXES}
)
DBGMSG("OpenCV_CVCAM_INCLUDE_DIR: ${OpenCV_CVCAM_INCLUDE_DIR}")



#
# Sub directory candidate list for libraries
# ライブラリディレクトリ調査対象
#
set(OpenCV_LIBDIR_SUFFIXES
    lib
    OpenCV/lib
    otherlibs/_graphics/lib
)

#
# Find sub directories for libraries.
# ライブラリディレクトリ探索
#
find_library(OpenCV_CV_LIBRARY
    NAMES cv opencv
    PATHS ${OpenCV_DIR}
    PATH_SUFFIXES ${OpenCV_LIBDIR_SUFFIXES}
)
DBGMSG("OpenCV_CV_LIBRARY: ${OpenCV_CV_LIBRARY}")

find_library(OpenCV_CVAUX_LIBRARY
    NAMES cvaux
    PATHS ${OpenCV_DIR}
    PATH_SUFFIXES ${OpenCV_LIBDIR_SUFFIXES}
)
DBGMSG("OpenCV_CVAUX_LIBRARY: ${OpenCV_CVAUX_LIBRARY}")

find_library(OpenCV_CVCAM_LIBRARY
    NAMES cvcam
    PATHS ${OpenCV_DIR}
    PATH_SUFFIXES ${OpenCV_LIBDIR_SUFFIXES}
)
DBGMSG("OpenCV_CVCAM_LIBRARY: ${OpenCV_CVCAM_LIBRARY}")

find_library(OpenCV_CVHAARTRAINING_LIBRARY
    NAMES cvhaartraining
    PATHS ${OpenCV_DIR}
    PATH_SUFFIXES ${OpenCV_LIBDIR_SUFFIXES}
)
DBGMSG("OpenCV_CVHAARTRAINING_LIBRARY: ${OpenCV_CVHAARTRAINING_LIBRARY}")

find_library(OpenCV_CXCORE_LIBRARY
    NAMES cxcore
    PATHS ${OpenCV_DIR}
    PATH_SUFFIXES ${OpenCV_LIBDIR_SUFFIXES}
)
DBGMSG("OpenCV_CXCORE_LIBRARY: ${OpenCV_CXCORE_LIBRARY}")

find_library(OpenCV_CXTS_LIBRARY
    NAMES cxts
    PATHS ${OpenCV_DIR}
    PATH_SUFFIXES ${OpenCV_LIBDIR_SUFFIXES}
)
DBGMSG("OpenCV_CXTS_LIBRARY: ${OpenCV_CXTS_LIBRARY}")

find_library(OpenCV_HIGHGUI_LIBRARY
    NAMES highgui
    PATHS ${OpenCV_DIR}
    PATH_SUFFIXES ${OpenCV_LIBDIR_SUFFIXES}
)
DBGMSG("OpenCV_HIGHGUI_LIBRARY: ${OpenCV_HIGHGUI_LIBRARY}")

find_library(OpenCV_ML_LIBRARY
    NAMES ml
    PATHS ${OpenCV_DIR}
    PATH_SUFFIXES ${OpenCV_LIBDIR_SUFFIXES}
)
DBGMSG("OpenCV_ML_LIBRARY: ${OpenCV_ML_LIBRARY}")

find_library(OpenCV_TRS_LIBRARY
    NAMES trs
    PATHS ${OpenCV_DIR}
    PATH_SUFFIXES ${OpenCV_LIBDIR_SUFFIXES}
)
DBGMSG("OpenCV_TRS_LIBRARY: ${OpenCV_TRS_LIBRARY}")


#
# SET CONFIGURATION VARIABLES
# 変数設定
#
set(OpenCV_REQUIRED_COMPONENTS CV CXCORE CVAUX HIGHGUI)
foreach(NAME ${OpenCV_REQUIRED_COMPONENTS})
    if(OpenCV_${NAME}_INCLUDE_DIR AND OpenCV_${NAME}_LIBRARY)
        list(APPEND OpenCV_INCLUDE_DIRS ${OpenCV_${NAME}_INCLUDE_DIR})
        list(APPEND OpenCV_LIBRARIES ${OpenCV_${NAME}_LIBRARY})
    else(OpenCV_${NAME}_INCLUDE_DIR AND OpenCV_${NAME}_LIBRARY)
        set(OpenCV_FOUND OFF)
    endif(OpenCV_${NAME}_INCLUDE_DIR AND OpenCV_${NAME}_LIBRARY)
endforeach(NAME)

if(OpenCV_FOUND)
    message(STATUS "Found OpenCV")
endif(OpenCV_FOUND)

if(OpenCV_CV_LIBRARY)
    get_filename_component(OpenCV_LINK_DIRECTORIES ${OpenCV_CV_LIBRARY} PATH)
endif(OpenCV_CV_LIBRARY)

mark_as_advanced(
    OpenCV_ROOT_DIR
    OpenCV_INCLUDE_DIRS
    OpenCV_CV_INCLUDE_DIR
    OpenCV_CXCORE_INCLUDE_DIR
    OpenCV_CVAUX_INCLUDE_DIR
    OpenCV_CVCAM_INCLUDE_DIR
    OpenCV_HIGHGUI_INCLUDE_DIR
    OpenCV_LIBRARIES
    OpenCV_CV_LIBRARY
    OpenCV_CXCORE_LIBRARY
    OpenCV_CVAUX_LIBRARY
    OpenCV_CVCAM_LIBRARY
    OpenCV_CVHAARTRAINING_LIBRARY
    OpenCV_CXTS_LIBRARY
    OpenCV_HIGHGUI_LIBRARY
    OpenCV_ML_LIBRARY
    OpenCV_TRS_LIBRARY
)

