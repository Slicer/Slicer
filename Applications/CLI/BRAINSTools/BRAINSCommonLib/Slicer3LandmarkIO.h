/*
 * Author: Wei Lu
 * at Psychiatry Imaging Lab,
 * University of Iowa Health Care 2010
 */

#ifndef __Slicer3LandmarkIO__h
#define __Slicer3LandmarkIO__h

#include "itkPoint.h"
#include <itksys/SystemTools.hxx>

#include <fstream>
#include <map>
#include <string>

/*
 * This IO utility program transforms between ITK landmarks (in LPS coordinate
 * system) to Slicer3 landmarks (in RAS coordinate system).
 */

typedef itk::Point< double, 3 >             PointType;
typedef std::map< std::string, PointType >  LandmarksMapType;


/*
 * Write ITK landmarks to a Slicer3 landmark list file (.fcsv)
 * Input:
 * landmarksFilename  - the filename of the output Slicer landmark list file
 * landmarks          - a map of landmarks (itkPoints) to be written into file
 *
 * Output:
 * NONE
 */
extern void
WriteITKtoSlicer3Lmk( const std::string landmarksFilename,
                      const LandmarksMapType &landmarks );


/*
 * Read Slicer3 landmark list file (.fcsv) into a map of ITK points
 * Input:
 * landmarksFilename  - the filename of the input Slicer landmark list file
 *
 * Output:
 * landmarks          - a map of itkPoints to save the landmarks in ITK
 */
extern LandmarksMapType
ReadSlicer3toITKLmk( const std::string landmarksFilename );

#endif
