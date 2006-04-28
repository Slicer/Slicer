#
# If being build as part of Slicer3, we know where to find tclap include files
IF(Slicer3_SOURCE_DIR)
SET(TCLAP_DIR ${Slicer3_SOURCE_DIR}/Libs/tclap)
ELSE(Slicer3_SOURCE_DIR)
FIND_PATH(TCLAP_DIR include ../tclap DOC "Location of tclap/include")
ENDIF(Slicer3_SOURCE_DIR)

INCLUDE_DIRECTORIES (${TCLAP_DIR}/include)

SET(Slicer3_SOURCE_DIR d:/lorensen/Projects/Slicer3)


UTILITY_SOURCE(GENERATECLP_EXE GenerateCLP ./ GenerateCLP.cxx)
# create the .clp files
# usage: GENERATE_CLP(foo_SRCS ${XML_FILES})
MACRO(GENERATECLP SOURCES XML)
  # For each XML file
  FOREACH(FILE ${XML})
    # what is the filename without the extension
    GET_FILENAME_COMPONENT(TMP_FILENAME ${FILE} NAME_WE)
        
    # the input file might be full path so handle that
    GET_FILENAME_COMPONENT(TMP_FILEPATH ${FILE} PATH)

    # compute the input filename
    IF (TMP_FILEPATH)
      SET(TMP_INPUT ${TMP_FILEPATH}/${TMP_FILENAME}.xml) 
    ELSE (TMP_FILEPATH)
      SET(TMP_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/${TMP_FILENAME}.xml)
    ENDIF (TMP_FILEPATH)

    # add custom command to output
    ADD_CUSTOM_COMMAND(
      OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${TMP_FILENAME}CLP.h
      DEPENDS GenerateCLP ${TMP_INPUT}
      COMMAND ${GENERATECLP_EXE}
        ${TMP_INPUT} ${CMAKE_CURRENT_SOURCE_DIR}/${TMP_FILENAME}CLP.h
      )
    # mark the .clp file as a header file
    SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/${TMP_FILENAME}CLP.h PROPERTIES HEADER_FILE_ONLY TRUE)
    SET_SOURCE_FILES_PROPERTIES(${TMP_FILENAME}.cxx PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${TMP_FILENAME}CLP.h)


    SET(${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/${TMP_FILENAME}CLP.h ${${SOURCES}}) 
    ENDFOREACH(FILE)
ENDMACRO(GENERATECLP)
