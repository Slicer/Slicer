/*=========================================================================
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile$
 *  Language:  C++
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even
 *  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/
#ifndef __itkInverseConsistentLandmarks_txx
#define __itkInverseConsistentLandmarks_txx
#include "itkInverseConsistentLandmarks.h"
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string>
#include <iostream>

namespace itk
{
template< typename PointStorageType, typename PointSetType >
InverseConsistentLandmarks< PointStorageType, PointSetType >
::InverseConsistentLandmarks(void)
{
  ImageDims[0] = 12345;
  ImageDims[1] = 12345;
  ImageDims[2] = 12345;
  ImageDims[3] = 12345;
}

template< typename PointStorageType, typename PointSetType >
InverseConsistentLandmarks< PointStorageType, PointSetType >
::InverseConsistentLandmarks(const int XDim,
                             const int YDim,
                             const int ZDim,
                             const int TDim)
{
  ImageDims[0] = XDim;
  ImageDims[1] = YDim;
  ImageDims[2] = ZDim;
  ImageDims[3] = TDim;
}

template< typename PointStorageType, typename PointSetType >
InverseConsistentLandmarks< PointStorageType, PointSetType > &
InverseConsistentLandmarks< PointStorageType, PointSetType >
::operator=(const InverseConsistentLandmarks & rhs)
{
  this->std::map< std::string, PointType >::operator=(rhs);
  ImageDims[0] = rhs.ImageDims[0];
  ImageDims[1] = rhs.ImageDims[1];
  ImageDims[2] = rhs.ImageDims[2];
  ImageDims[3] = rhs.ImageDims[3];
  return *this;
}

template< typename PointStorageType, typename PointSetType >
bool InverseConsistentLandmarks< PointStorageType, PointSetType >
::ReadPointTypes(const std::string lmrkfilename)
{
  const Landmark_File_Format lmkff = QueryLandmarkFile(lmrkfilename);
  bool                       check = false;

  switch ( lmkff )
    {
    case GEC_LANDMARKS:
      check = InverseConsistentLandmarks::ReadGECPointTypes(lmrkfilename);
      break;
    case INTELLX_LANDMARKS:
      check = InverseConsistentLandmarks::ReadIntellXPointTypes(lmrkfilename);
      break;
    case ANALYZE_LANDMARKS:
      check = InverseConsistentLandmarks::ReadAnalyzePointTypes(lmrkfilename);
      break;
    case IPL_LANDMARKS:
      check = InverseConsistentLandmarks::ReadIPLPointTypes(lmrkfilename);
      break;
    case IPL_TALAIRACH_BOUNDS_LANDMARKS:
      check = InverseConsistentLandmarks::ReadIPLTalairachPointTypes(
        lmrkfilename);
      break;
    case IPL_CEREBELLAR_BOUNDS_LANDMARKS:
      check = InverseConsistentLandmarks::ReadIPLCerebellarPointTypes(
        lmrkfilename);
      break;
    case UNKNOWN_LANDMARKS:
      std::cout << "Unknown landmark file format!" << std::endl;
      return false;
    default:
      std::cout << "Error. Invalid Landmark_File_Format type!" << std::endl;
      exit(-1);
    }
  return check;
}

template< typename PointStorageType, typename PointSetType >
bool InverseConsistentLandmarks< PointStorageType, PointSetType >
::ReadPointTypes(const std::string lmrkfilename, const int XDim,
                 const int YDim, const int ZDim, const int TDim)
{
  Landmark_File_Format lmkff = QueryLandmarkFile(lmrkfilename);

  switch ( lmkff )
    {
    case GEC_LANDMARKS:
      InverseConsistentLandmarks::ReadGECPointTypes(lmrkfilename,
                                                    XDim,
                                                    YDim,
                                                    ZDim,
                                                    TDim);
      break;
    case INTELLX_LANDMARKS:
      InverseConsistentLandmarks::ReadIntellXPointTypes(lmrkfilename,
                                                        XDim,
                                                        YDim,
                                                        ZDim,
                                                        TDim);
      break;
    case ANALYZE_LANDMARKS:
      InverseConsistentLandmarks::ReadAnalyzePointTypes(lmrkfilename,
                                                        XDim,
                                                        YDim,
                                                        ZDim,
                                                        TDim);
      break;
    case IPL_LANDMARKS:
      InverseConsistentLandmarks::ReadIPLPointTypes(lmrkfilename,
                                                    XDim,
                                                    YDim,
                                                    ZDim,
                                                    TDim);
      break;
    case IPL_TALAIRACH_BOUNDS_LANDMARKS:
      InverseConsistentLandmarks::ReadIPLTalairachPointTypes(lmrkfilename,
                                                             XDim,
                                                             YDim,
                                                             ZDim,
                                                             TDim);
      break;
    case IPL_CEREBELLAR_BOUNDS_LANDMARKS:
      InverseConsistentLandmarks::ReadIPLCerebellarPointTypes(lmrkfilename,
                                                              XDim,
                                                              YDim,
                                                              ZDim,
                                                              TDim);
      break;
    case UNKNOWN_LANDMARKS:
      std::cout << "Unknown landmark file format!" << std::endl;
      return false;
    default:
      std::cout << "Error. Invalid Landmark_File_Format type!" << std::endl;
      exit(-1);
    }
  return true;
}

template< typename PointStorageType, typename PointSetType >
bool InverseConsistentLandmarks< PointStorageType, PointSetType >
::WritePointTypes(const std::string lmrkfilename,
                  const Landmark_File_Format lmkff)
{
  bool writeresult;

  switch ( lmkff )
    {
    case GEC_LANDMARKS:
      writeresult = InverseConsistentLandmarks::WriteGECPointTypes(
        lmrkfilename);
      break;
    case INTELLX_LANDMARKS:
      writeresult = InverseConsistentLandmarks::WriteIntellXPointTypes(
        lmrkfilename);
      break;
    case ANALYZE_LANDMARKS:
      writeresult = InverseConsistentLandmarks::WriteAnalyzePointTypes(
        lmrkfilename);
      break;
    case IPL_LANDMARKS:
    case IPL_TALAIRACH_BOUNDS_LANDMARKS:
    case IPL_CEREBELLAR_BOUNDS_LANDMARKS:
      // writeresult = InverseConsistentLandmarks::WriteIPLPointTypes(
      // lmrkfilename );
      std::cout << "IPL_LANDMARKS NOT IMPLEMENTED FOR WRITING" << std::endl;
      break;
    case UNKNOWN_LANDMARKS:
      std::cout << "Unknown output landmark file format!" << std::endl;
      writeresult = false;
    }
  return writeresult;
}

template< typename PointStorageType, typename PointSetType >
bool InverseConsistentLandmarks< PointStorageType, PointSetType >
::PrintPointTypes(void) const
{
  printf("#IMAGEDIMS %6hu %6hu %6hu %6hu\n",
         ImageDims[0],
         ImageDims[1],
         ImageDims[2],
         ImageDims[3]);
  printf("#%11s %12s %12s %12s %12s %12s\n",
         "Name",
         "X",
         "Y",
         "Z",
         "T",
         "Weighting");
  for ( typename InverseConsistentLandmarks::const_iterator mapiter =
          this->begin();
        mapiter != this->end();
        mapiter++ )
    {
    printf( "%12s %12.6f %12.6f %12.6f %12.5f %12.6f\n", mapiter->first.c_str(),
            mapiter->second[0],
            mapiter->second[1],
            mapiter->second[2],
            mapiter->second.GetT(),
            mapiter->second.GetWeighting() );
    }
  if ( this->size() > 0 )
    {
    return true;
    }
  else
    {
    return false;
    }
}

template< typename PointStorageType, typename PointSetType >
typename InverseConsistentLandmarks< PointStorageType,
                                     PointSetType >::Landmark_File_Format
InverseConsistentLandmarks< PointStorageType, PointSetType >
::QueryLandmarkFile(const std::string lmrkfilename)
{
  /* These are the header formats for GEC, IntellX, and Analyze Landmark files
    * respectively
    * GEC Landmark File Format (typical extension .lmks)
    *
    *--------------------------------------------------------------------------------------
    * #GECLANDMARKS-v1.0
    * #IMAGEDIMS   XDIM   YDIM   ZDIM   TDIM
    * #       Name            X            Y            Z            T
    *    Weighting
    * AC_Point 128 184 128 0 0
    * PC_Point 100 160 130 0 0
    * etc.
    *
    *--------------------------------------------------------------------------------------
    * IntellX Landmark File Format (typical extension .lmk)
    *
    *--------------------------------------------------------------------------------------
    * Landmarks-1.0
    * Number of points
    * "landmark identifier/description"
    * XPOINT YPOINT ZPOINT 1 1
    * "landmark identifier/description"
    * XPOINT YPOINT ZPOINT 1 1
    * etc.
    *
    *--------------------------------------------------------------------------------------
    * Analyze Landmark File Format (typical extension .log)
    *
    *--------------------------------------------------------------------------------------
    * # System Time
    * # Volume=name of volume
    * #
    * #
    * #     Point
    * #   X    Y    Z     Value       Label
    * # ==== ==== ====  ==========  ==========
    *   129  277  128          14        Label
    *   150  100  164          32        Label
    * etc.
    *
    *--------------------------------------------------------------------------------------
    * For specializing which file format it is, the firstline #GECLANDMARKS-v1.0
    * will
    * specify a type GEC_LANDMARKS.  The tag Landmarks-1.0 will specify a
    * landmark
    * file of the type INTELLX_LANDMARKS.  The lines
    */
  // #     Point
  // #   X    Y    Z     Value       Label
  // will be used to specify a landmark file generated from an Analyze point log
  // session as the type ANALYZE_LANDMARKS.  Any other format which doesn't
  // match
  // will return UNKNOWN_LANDMARKS.

  // Open the files
  FILE *tempfile = fopen(lmrkfilename.c_str(), "r");

  if ( tempfile == NULL )
    {
    std::cout << "ERROR: can not open " << lmrkfilename << std::endl;
    return UNKNOWN_LANDMARKS;
    }

  const unsigned short int FILE_BUFFER_SIZE = 256;
  char                     buffer[FILE_BUFFER_SIZE]; // Dummy Varible
  char *                   status = 0;
  // Read Header Information
  status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
  if ( status == NULL )
    {
    return UNKNOWN_LANDMARKS;
    }

  if ( strncmp(buffer, "#GECLANDMARKS-v1.0", 18) == 0 )
    {
    return GEC_LANDMARKS;
    }

  if ( strncmp(buffer, "Landmarks-1.0", 13) == 0 )
    {
    return INTELLX_LANDMARKS;
    }

  if ( strncmp(buffer, "IPL_HEADER_BEGIN", 16) == 0 )
    {
    if ( lmrkfilename.find("Talairach.bnd") != std::string::npos )
      {
      return IPL_TALAIRACH_BOUNDS_LANDMARKS;
      }
    else if ( lmrkfilename.find("Cerebellum.bnd") != std::string::npos )
      {
      return IPL_CEREBELLAR_BOUNDS_LANDMARKS;
      }
    else
      {
      return IPL_LANDMARKS;
      }
    }

  // If it is not either of these two, try to read the next 4 lines
  // and check to see if it matches the Analyze point log file format
  for ( int i = 0; i < 4; i++ )
    {
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      return UNKNOWN_LANDMARKS;
      }
    }

  if ( strncmp(buffer, "#     Point", 11) == 0 )
    {
    // Try to match next line
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      return UNKNOWN_LANDMARKS;
      }
    if ( strncmp(buffer, "#   X    Y    Z     Value       Label", 37) == 0 )
      {
      return ANALYZE_LANDMARKS;
      }
    }
  return UNKNOWN_LANDMARKS;
}

template< typename PointStorageType, typename PointSetType >
bool InverseConsistentLandmarks< PointStorageType, PointSetType >
::ReadGECPointTypes(const std::string lmrkfilename)
{
  // std::cout << "Reading GEC Landmark File " << lmrkfilename << std::endl;

  this->erase( this->begin(), this->end() );
  FILE *tempfile = fopen(lmrkfilename.c_str(), "r");
  if ( tempfile == NULL )
    {
    // std::cout << "ERROR: can not open " << lmrkfilename << std::endl;
    // exit( -1 );
    return false;
    }
  const unsigned short int FILE_BUFFER_SIZE = 256;
  const unsigned short int LANDMARK_NAME_SIZE = 40;
  char                     buffer[FILE_BUFFER_SIZE]; // Dummy Varible
  char                     CurrentLandmarkName[LANDMARK_NAME_SIZE];
  char *                   status = 0;
  // Read Header Information
  status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
  if ( status == NULL )
    {
    return false;
    }
  if ( strncmp(buffer, "#GECLANDMARKS-v1.0", 18) != 0 )
    {
    // Only files with a header of #GECLANDMARKS-v1.0 will be accepted
    return false;
    }
  // Read dimensions of image that landmarks are associated with
  status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
  if ( status == NULL )
    {
    return false;
    }
  // Note CurrentLandmarkName is just a placeholder here!.
  // Note the hu is an indicator that this is a unsigned short int

  sscanf( buffer, "%s %hu %hu %hu %hu", CurrentLandmarkName,
          &( ImageDims[0] ),
          &( ImageDims[1] ),
          &( ImageDims[2] ),
          &( ImageDims[3] ) );

  while ( fgets(buffer, FILE_BUFFER_SIZE, tempfile) != NULL )
    {
    if ( buffer[0] == '#' )          // This allows for comments to be added
                                     // anywhere in the file
      {
      continue;
      }
    PointType TempPnt;
      {
      double       val0;
      double       val1;
      double       val2;
      double       valT;
      double       valW;
      unsigned int numread = sscanf(buffer, "%s %lf %lf %lf %lf %lf",
                                    CurrentLandmarkName,
                                    &val0, &val1, &val2, &valT, &valW);
      if ( numread < 6 )
        {
        return false;
        }
      TempPnt[0] = static_cast< double >( val0 );
      TempPnt[1] = static_cast< double >( val1 );
      TempPnt[2] = static_cast< double >( val2 );
      TempPnt.SetT( static_cast< double >( valT ) );
      TempPnt.SetWeighting( static_cast< double >( valW ) );
      }
    if ( ImageDims[0] <= TempPnt[0] )
      {
      // std::cout << "Error: Point "<< CurrentLandmarkName << " has invalid x
      // coordinate.  Max value is: " << ImageDims[0]-1<<std::endl;
      // exit( -1 );
      return false;
      }
    if ( ImageDims[1] <= TempPnt[1] )
      {
      // std::cout << "Error: Point "<< CurrentLandmarkName << " has invalid y
      // coordinate.  Max value is: " << ImageDims[1]-1<<std::endl;
      // exit( -1 );
      return false;
      }
    if ( ImageDims[2] <= TempPnt[2] )
      {
      // std::cout << "Error: Point "<< CurrentLandmarkName << " has invalid z
      // coordinate.  Max value is: " << ImageDims[2]-1<<std::endl;
      // exit( -1 );
      return false;
      }
    if ( ImageDims[3] <= TempPnt.GetT() )
      {
      // std::cout << "Error: Point "<< CurrentLandmarkName << " has invalid t
      // coordinate.  Max value is: " << ImageDims[3]-1<<std::endl;
      // exit( -1 );
      return false;
      }
    ( *this )[CurrentLandmarkName] = TempPnt;
    }
  fclose(tempfile);
  if ( this->size() > 0 )
    {
    return true;
    }
  else
    {
    return false;
    }
}

template< typename PointStorageType, typename PointSetType >
bool InverseConsistentLandmarks< PointStorageType, PointSetType >
::ReadGECPointTypes(const std::string lmrkfilename,
                    const int XDim, const int YDim,
                    const int ZDim, const int TDim)
{
  const bool testread = this->InverseConsistentLandmarks::ReadGECPointTypes(
    lmrkfilename);

  this->InverseConsistentLandmarks::rescale(XDim, YDim, ZDim, TDim);
  return testread;
}

template< typename PointStorageType, typename PointSetType >
bool InverseConsistentLandmarks< PointStorageType, PointSetType >
::ReadIntellXPointTypes(const std::string lmrkfilename)
{
  //  std::cout << "Reading IntellX Landmark File " << lmrkfilename <<
  // std::endl;
  this->erase( this->begin(), this->end() );
  // Open the files
  FILE *tempfile = fopen(lmrkfilename.c_str(), "r");
  if ( tempfile == NULL )
    {
    // std::cout << "ERROR: can not open " << lmrkfilename << std::endl;
    // exit( -1 );
    return false;
    }
  const unsigned short int FILE_BUFFER_SIZE = 128;
  char                     buffer[FILE_BUFFER_SIZE]; // Dummy Varible
  std::string              CurrentLandmarkName;
  char *                   status = 0;
  // Read Header Information
  status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
  if ( status == NULL )
    {
    return false;
    }
  if ( strncmp(buffer, "Landmarks-1.0", 13) != 0 )
    {
    // Only files with a header of Landmarks-1.0 will be accepted
    return false;
    }
  // Read Number of Landmark points
  status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
  if ( status == NULL )
    {
    return false;
    }
  const int NumberOfPoints = atoi(buffer);
  for ( int p = 0; p < NumberOfPoints; p++ )
    {
    // Extract Landmark Name
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      return false;
      }
    CurrentLandmarkName = std::string(buffer);
    // CurrentLandmarkName.replace(CurrentLandmarkName.find("\""),1,"");
    // CurrentLandmarkName.replace(CurrentLandmarkName.find("\""),1,"");
    std::string NewLandmarkName;
    for ( unsigned int iter = 0; iter < CurrentLandmarkName.length(); iter++ )
      {
      // std::cout << "CurrentLandmarkName[" << iter << "] = " <<
      // CurrentLandmarkName[iter] << std::endl;
      if ( CurrentLandmarkName[iter] == '\n' || CurrentLandmarkName[iter] ==
           '\r' || CurrentLandmarkName[iter] == '\t'
           || CurrentLandmarkName[iter] == '\"' )
        {
        continue;
        }
      else if ( CurrentLandmarkName[iter] == ' ' )
        {
        NewLandmarkName += "_";
        }
      else
        {
        NewLandmarkName += CurrentLandmarkName[iter];
        }
      // std::cout << "NewLandmarkName = " << NewLandmarkName << std::endl;
      }

    // Extract Coordinates
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      return false;
      }
    PointType TempPnt;
      {
      unsigned int numread = sscanf(buffer, "%lf %lf %lf",
                                    &TempPnt[0], &TempPnt[1], &TempPnt[2]);
      if ( numread < 3 )
        {
        return false;
        }
      }
    // Convert to Analyze coordinate system...Note the -1 is needed for correct
    // coord
    // Ex. 0,0,0 in Warp = 127,127,127 in a 128x128x128 analyze image.
    // Assumes the landmark dimensions have been set before used in this
    // function
    TempPnt[0] = static_cast< PointStorageType >( ImageDims[0] ) - TempPnt[0];
    TempPnt[1] = static_cast< PointStorageType >( ImageDims[1] ) - TempPnt[1];
    TempPnt[2] = static_cast< PointStorageType >( ImageDims[2] ) - TempPnt[2];
    // DDEBUG, this is set to one when dimension is one, which goofs when you
    // write out
    // a lmk file as a ilmks and try to read the ilmks back in.  Since no t
    // coordinates
    // will be used in the short run, this will be implemented
    // TempPnt.setT() = static_cast<PointStorageType>( ImageDims[3] ) -
    // TempPnt[3];
    TempPnt.SetT(0.0F);

    // Set Weighting to default value of 1.0F;
    TempPnt.SetWeighting(1.0F);
    ( *this )[NewLandmarkName] = TempPnt;
    }
  fclose(tempfile);
  if ( this->size() > 0 )
    {
    return true;
    }
  else
    {
    return false;
    }
}

template< typename PointStorageType, typename PointSetType >
bool InverseConsistentLandmarks< PointStorageType, PointSetType >
::ReadIntellXPointTypes(const std::string lmrkfilename,
                        const int XDim, const int YDim,
                        const int ZDim, const int TDim)
{
  // std::cout << "Reading IntellX Landmark File " << lmrkfilename << std::endl;
  // std::cout << "Specifying dimensions " << XDim << ", " << YDim << ", " <<
  // ZDim << ", " << TDim << "." << std::endl;

  if ( XDim < 0 )
    {
    std::cout << "Error. X dimension " << XDim << " given is less than 0"
              << std::endl;
    return false;
    }
  if ( YDim < 0 )
    {
    std::cout << "Error. Y dimension " << YDim << " given is less than 0"
              << std::endl;
    return false;
    }
  if ( ZDim < 0 )
    {
    std::cout << "Error. Z dimension " << ZDim << " given is less than 0"
              << std::endl;
    return false;
    }
  if ( TDim < 0 )
    {
    std::cout << "Error. T dimension " << TDim << " given is less than 0"
              << std::endl;
    return false;
    }

  // Delete all old landmarks
  this->erase( this->begin(), this->end() );
  // Open the files
  FILE *tempfile = fopen(lmrkfilename.c_str(), "r");
  if ( tempfile == NULL )
    {
    // std::cout << "ERROR: can not open " << lmrkfilename << std::endl;
    // exit( -1 );
    return false;
    }
  const unsigned short int FILE_BUFFER_SIZE = 128;
  char                     buffer[FILE_BUFFER_SIZE]; // Dummy Varible
  std::string              CurrentLandmarkName;
  char *                   status = 0;
  // Read Header Information
  status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
  if ( status == NULL )
    {
    return false;
    }
  if ( strncmp(buffer, "Landmarks-1.0", 13) != 0 )
    {
    // Only files with a header of Landmarks-1.0 will be accepted
    return false;
    }
  // Read Number of Landmark points
  status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
  if ( status == NULL )
    {
    return false;
    }
  const int NumberOfPoints = atoi(buffer);

  // Assigning the dimensions from the parameters
  ImageDims[0] = XDim;
  ImageDims[1] = YDim;
  ImageDims[2] = ZDim;
  ImageDims[3] = TDim;

  for ( int p = 0; p < NumberOfPoints; p++ )
    {
    // Extract Landmark Name
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      return false;
      }

    CurrentLandmarkName = std::string(buffer);

    std::string NewLandmarkName;
    for ( unsigned int iter = 0; iter < CurrentLandmarkName.length(); iter++ )
      {
      // std::cout << "CurrentLandmarkName[" << iter << "] = " <<
      // CurrentLandmarkName[iter] << std::endl;
      if ( CurrentLandmarkName[iter] == '\n' || CurrentLandmarkName[iter] ==
           '\r' || CurrentLandmarkName[iter] == '\t'
           || CurrentLandmarkName[iter] == '\"' )
        {
        continue;
        }
      else if ( CurrentLandmarkName[iter] == ' ' )
        {
        NewLandmarkName += "_";
        }
      else
        {
        NewLandmarkName += CurrentLandmarkName[iter];
        }
      }

    // Extract Coordinates
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      return false;
      }
    PointType TempPnt;
      {
      double       val0;
      double       val1;
      double       val2;
      unsigned int numread = sscanf(buffer, "%lf %lf %lf",
                                    &val0, &val1, &val2);
      if ( numread < 3 )
        {
        return false;
        }
      TempPnt[0] = static_cast< double >( val0 );
      TempPnt[1] = static_cast< double >( val1 );
      TempPnt[2] = static_cast< double >( val2 );
      }
    // Convert to Analyze coordinate system...Note the -1 is needed for correct
    // coord
    // Ex. 0,0,0 in Warp = 127,127,127 in a 128x128x128 analyze image.
    TempPnt[0] = static_cast< PointStorageType >( ImageDims[0] ) - TempPnt[0];
    TempPnt[1] = static_cast< PointStorageType >( ImageDims[1] ) - TempPnt[1];
    TempPnt[2] = static_cast< PointStorageType >( ImageDims[2] ) - TempPnt[2];

    TempPnt.SetT(0.0F);

    // Set Weighting to default value of 1.0F;
    TempPnt.SetWeighting(1.0F);
    ( *this )[NewLandmarkName] = TempPnt;
    }
  fclose(tempfile);
  if ( this->size() > 0 )
    {
    return true;
    }
  else
    {
    return false;
    }
}

template< typename PointStorageType, typename PointSetType >
bool InverseConsistentLandmarks< PointStorageType, PointSetType >
::ReadAnalyzePointTypes(const std::string lmrkfilename)
{
  std::cout << "Reading Analyze Landmark File " << lmrkfilename << std::endl;

  // Delete all old landmarks
  this->erase( this->begin(), this->end() );

  // Open the files
  FILE *tempfile = fopen(lmrkfilename.c_str(), "r");
  if ( tempfile == NULL )
    {
    return false;
    }
  const unsigned short int FILE_BUFFER_SIZE = 256;
  const unsigned short int LANDMARK_NAME_SIZE = 40;
  char                     buffer[FILE_BUFFER_SIZE]; // Dummy Varible
  char                     CurrentLandmarkName[LANDMARK_NAME_SIZE];
  char *                   status = 0;

  // Read Header Information
  for ( int i = 0; i < 5; i++ )
    {
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      return false;
      }
    }

  if ( strncmp(buffer, "#     Point", 11) == 0 )
    {
    // Try to match next line
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      return false;
      }
    if ( strncmp(buffer, "#   X    Y    Z     Value       Label", 37) != 0 )
      {
      return false;
      }
    }
  else
    {
    return false;
    }

  // Reading the line with " ====  ====  ==== ..etc"
  status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
  if ( status == NULL )
    {
    return false;
    }

  int mycount = 1;
  while ( fgets(buffer, FILE_BUFFER_SIZE, tempfile) != NULL )
    {
    int       grayvalue;
    PointType TempPnt;
      {
      double val0;
      double val1;
      double val2;
      int    numread = sscanf(buffer, "%lf %lf %lf %d %s", &val0,
                              &val1, &val2, &grayvalue, CurrentLandmarkName);
      TempPnt[0] = static_cast< double >( val0 );
      TempPnt[1] = static_cast< double >( val1 );
      TempPnt[2] = static_cast< double >( val2 );
      if ( numread == -1 )
        {
        continue;
        }

      if ( numread < 4 )         // Need at least an X,Y,Z coordinate
        {
        return false;
        }
      if ( numread == 4 )         // No label was found
        {
        memset(CurrentLandmarkName, 0, LANDMARK_NAME_SIZE);
        }
      }

    if ( ImageDims[0] <= TempPnt[0] )
      {
      return false;
      }
    if ( ImageDims[1] <= TempPnt[1] )
      {
      return false;
      }
    if ( ImageDims[2] <= TempPnt[2] )
      {
      return false;
      }

    // Here is how the naming definition is defined, if the label is not found,
    // the
    // landmark identifier is Landmark_N where N is the current number of
    // landmarks
    // read in.  If the landmark label is found in the current map, then the
    // label is renamed with the old label plus a "_1".  If that tag exists,
    // then
    // the next label is the old label with "_2".  This continues until a
    // unique name is found for the landmark.

    std::string CurrentName(CurrentLandmarkName);

    if ( CurrentName.size() == 0 )
      {
      char numberbuffer[16];
      sprintf(numberbuffer, "%d", mycount);
      CurrentName = "Landmark_" + std::string(numberbuffer);
      }

    // Checking to make sure no elements in the map of landmark points are
    // duplicated.
      {
      int         label = 1;
      std::string OriginalLabel(CurrentName);

      char labelbuffer[16];
      while ( ( *this ).find(CurrentName) != ( *this ).end() )
        {
        sprintf(labelbuffer, "%d", label);
        CurrentName = OriginalLabel + "_" + std::string(labelbuffer);
        label++;
        }
      }
    TempPnt.SetWeighting(1.0F);
    ( *this )[CurrentName] = TempPnt;
    mycount++;
    }
  fclose(tempfile);
  if ( this->size() > 0 )
    {
    return true;
    }
  else
    {
    return false;
    }
}

template< typename PointStorageType, typename PointSetType >
bool InverseConsistentLandmarks< PointStorageType, PointSetType >
::ReadAnalyzePointTypes(const std::string lmrkfilename,
                        const int XDim, const int YDim,
                        const int ZDim, const int TDim)
{
  std::cout << "Reading Analyze Landmark File " << lmrkfilename << std::endl;
  std::cout << "Specifying dimensions " << XDim << ", " << YDim << ", "
            << ZDim << ", " << TDim << "." << std::endl;

  if ( XDim < 0 )
    {
    std::cout << "Error. X dimension " << XDim << " given is less than 0"
              << std::endl;
    return false;
    }
  if ( YDim < 0 )
    {
    std::cout << "Error. Y dimension " << YDim << " given is less than 0"
              << std::endl;
    return false;
    }
  if ( ZDim < 0 )
    {
    std::cout << "Error. Z dimension " << ZDim << " given is less than 0"
              << std::endl;
    return false;
    }
  if ( TDim < 0 )
    {
    std::cout << "Error. T dimension " << TDim << " given is less than 0"
              << std::endl;
    return false;
    }

  // Delete all old landmarks
  this->erase( this->begin(), this->end() );

  // Open the files
  FILE *tempfile = fopen(lmrkfilename.c_str(), "r");
  if ( tempfile == NULL )
    {
    return false;
    }
  const unsigned short int FILE_BUFFER_SIZE = 256;
  const unsigned short int LANDMARK_NAME_SIZE = 40;
  char                     buffer[FILE_BUFFER_SIZE]; // Dummy Varible
  char                     CurrentLandmarkName[LANDMARK_NAME_SIZE];
  char *                   status = 0;

  // Read Header Information
  for ( int i = 0; i < 5; i++ )
    {
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      return false;
      }
    }

  if ( strncmp(buffer, "#     Point", 11) == 0 )
    {
    // Try to match next line
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      return false;
      }
    if ( strncmp(buffer, "#   X    Y    Z     Value       Label", 37) != 0 )
      {
      return false;
      }
    }
  else
    {
    return false;
    }

  // Reading the line with " ====  ====  ==== ..etc"
  status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
  if ( status == NULL )
    {
    return false;
    }

  // Using the function to assign the landmark image dimensions
  ImageDims[0] = XDim;
  ImageDims[1] = YDim;
  ImageDims[2] = ZDim;
  ImageDims[3] = TDim;

  int count = 1;
  while ( fgets(buffer, FILE_BUFFER_SIZE, tempfile) != NULL )
    {
    int       grayvalue;
    PointType TempPnt;
      {
      double val0;
      double val1;
      double val2;
      int    numread = sscanf(buffer, "%lf %lf %lf %d %s", &val0,
                              &val1, &val2, &grayvalue, CurrentLandmarkName);
      TempPnt[0] = static_cast< double >( val0 );
      TempPnt[1] = static_cast< double >( val1 );
      TempPnt[2] = static_cast< double >( val2 );

      if ( numread == -1 )
        {
        // No values read in from sscanf so continue
        continue;
        }

      if ( numread < 4 )           // Need at least an X,Y,Z coordinate
        {
        return false;
        }

      if ( numread == 4 )            // No label was found
        {
        memset(CurrentLandmarkName, 0, LANDMARK_NAME_SIZE);
        }

      // std::cout << "Number read = " << numread << std::endl;
      // std::cout << "CurrentLandmarkName = " << CurrentLandmarkName <<
      // std::endl;
      }

    if ( ImageDims[0] <= TempPnt[0] )
      {
      // std::cout << "Error: Point "<< CurrentLandmarkName << " has invalid x
      // coordinate.  Max value is: " << ImageDims[0]-1<<std::endl;
      // exit( -1 );
      return false;
      }
    if ( ImageDims[1] <= TempPnt[1] )
      {
      // std::cout << "Error: Point "<< CurrentLandmarkName << " has invalid y
      // coordinate.  Max value is: " << ImageDims[1]-1<<std::endl;
      // exit( -1 );
      return false;
      }
    if ( ImageDims[2] <= TempPnt[2] )
      {
      // std::cout << "Error: Point "<< CurrentLandmarkName << " has invalid z
      // coordinate.  Max value is: " << ImageDims[2]-1<<std::endl;
      // exit( -1 );
      return false;
      }

    // Here is how the naming definition is defined, if the label is not found,
    // the
    // landmark identifier is Landmark_N where N is the current number of
    // landmarks
    // read in.  If the landmark label is found in the current map, then the
    // label is renamed with the old label plus a "_1".  If that tag exists,
    // then
    // the next label is the old label with "_2".  This continues until a
    // unique name is found for the landmark.

    std::string CurrentName(CurrentLandmarkName);

    if ( CurrentName.size() == 0 )
      {
      char numberbuffer[16];
      sprintf(numberbuffer, "%d", count);
      CurrentName = "Landmark_" + std::string(numberbuffer);
      // std::cout << "Landmark label = " << CurrentName << std::endl;
      }

    // Checking to make sure no elements in the map of landmark points are
    // duplicated.
      {
      int         label = 1;
      std::string OriginalLabel(CurrentName);

      char labelbuffer[16];
      while ( ( *this ).find(CurrentName) != ( *this ).end() )
        {
        sprintf(labelbuffer, "%d", label);
        CurrentName = OriginalLabel + "_" + std::string(labelbuffer);
        label++;
        }
      // std::cout << "CurrentName = " << CurrentName << std::endl;
      }
    TempPnt.SetWeighting() = 1.0F;
    ( *this )[CurrentName] = TempPnt;
    count++;
    }
  fclose(tempfile);
  if ( this->size() > 0 )
    {
    return true;
    }
  else
    {
    return false;
    }
}

template< typename PointStorageType, typename PointSetType >
bool
InverseConsistentLandmarks< PointStorageType, PointSetType >
::WriteGECPointTypes(const std::string lmrkfilename) const
{
  // Open the files
  FILE *tempfile = fopen(lmrkfilename.c_str(), "w");

  if ( tempfile == NULL )
    {
    // std::cout << "ERROR: can not open " << lmrkfilename << std::endl;
    // exit( -1 );
    return false;
    }
  fprintf(tempfile, "#GECLANDMARKS-v1.0\n");
  fprintf(tempfile,
          "#IMAGEDIMS %6hu %6hu %6hu %6hu\n",
          ImageDims[0],
          ImageDims[1],
          ImageDims[2],
          ImageDims[3]);
  // Note: This is just a comment
  fprintf(tempfile,
          "#%11s %12s %12s %12s %12s %12s\n",
          "Name",
          "X",
          "Y",
          "Z",
          "T",
          "Weighting");
  for ( typename InverseConsistentLandmarks::const_iterator mapiter =
          this->begin();
        mapiter != this->end();
        mapiter++ )
    {
    fprintf( tempfile, "%12s %12.6f %12.6f %12.6f %12.5f %12.6f\n",
             mapiter->first.c_str(),
             mapiter->second[0],
             mapiter->second[1],
             mapiter->second[2],
             mapiter->second.GetT(),
             mapiter->second.GetWeighting() );
    }
  fclose(tempfile);
  if ( this->size() > 0 )
    {
    return true;
    }
  else
    {
    return false;
    }
}

template< typename PointStorageType, typename PointSetType >
bool
InverseConsistentLandmarks< PointStorageType, PointSetType >
::WriteIntellXPointTypes(const std::string lmrkfilename) const
{
  // Open the files
  FILE *tempfile = fopen(lmrkfilename.c_str(), "w");

  if ( tempfile == NULL )
    {
    // std::cout << "ERROR: can not open " << lmrkfilename << std::endl;
    // exit( -1 );
    return false;
    }
  fprintf(tempfile, "Landmarks-1.0\n");
  fprintf( tempfile, "%d\n", static_cast< int >( this->size() ) );
  for ( typename InverseConsistentLandmarks::const_iterator mapiter =
          this->begin();
        mapiter != this->end();
        mapiter++ )
    {
    std::string temp("\"");

    temp += mapiter->first;
    temp += "\"";
    // Notice the conversion back to IntellX coordinate system.
    // Note: -1 is needed here to get correct coordinate.
    fprintf(tempfile, "%s\n%f\t%f\t%f\t1\t1\n", temp.c_str(),
            static_cast< PointStorageType >( ImageDims[0] ) - mapiter->second[0],
            static_cast< PointStorageType >( ImageDims[1] ) - mapiter->second[1],
            static_cast< PointStorageType >( ImageDims[2] ) - mapiter->second[2]);
    }
  fclose(tempfile);
  if ( this->size() > 0 )
    {
    return true;
    }
  else
    {
    return false;
    }
}

template< typename PointStorageType, typename PointSetType >
bool
InverseConsistentLandmarks< PointStorageType, PointSetType >
::WriteAnalyzePointTypes(const std::string lmrkfilename) const
{
  // Open the files
  FILE *tempfile = fopen(lmrkfilename.c_str(), "w");

  if ( tempfile == NULL )
    {
    // std::cout << "ERROR: can not open " << lmrkfilename << std::endl;
    // exit( -1 );
    return false;
    }

  // Getting the current time information
  time_t     curtime;
  struct tm *loctime;
  curtime = time(NULL);
  loctime = localtime(&curtime);
  std::string date( asctime(loctime) );

  date = std::string("# ") + date;
  fprintf( tempfile, date.c_str() );
  fprintf(tempfile, "# Volume=\n");
  fprintf(tempfile, "#\n");
  fprintf(tempfile, "#\n");
  fprintf(tempfile, "#     Point\n");
  fprintf(tempfile, "#   X    Y    Z     Value       Label\n");
  fprintf(tempfile, "# ==== ==== ====  ==========  ==========\n");

  for ( typename InverseConsistentLandmarks::const_iterator mapiter =
          this->begin();
        mapiter != this->end();
        mapiter++ )
    {
    // Graylevel value is assumed to not be necessary so it's set to 0
    fprintf( tempfile, "%5d%5d%5d%12d    %s\n",
             static_cast< int >( mapiter->second[0] ),
             static_cast< int >( mapiter->second[1] ),
             static_cast< int >( mapiter->second[2] ),
             0, mapiter->first.c_str() );
    }
  fclose(tempfile);
  if ( this->size() > 0 )
    {
    return true;
    }
  else
    {
    return false;
    }
}

template< typename PointStorageType, typename PointSetType >
bool
InverseConsistentLandmarks< PointStorageType, PointSetType >
::ConcatLandmarks(InverseConsistentLandmarks & newlmks)
{
  // Copy landmarks from newlmks to the current landmark file, but
  // ensuring that the keys are not identical

  if ( this == &newlmks )
    {
    std::cout << "Error. Can not concatenate a landmark object with itself!"
              << std::endl;
    return false;
    }

  // Check to make sure the dimensions match
  bool sizematch = true;
  if ( this->getXDim() != newlmks.getXDim() )
    {
    std::cout
    <<
    "Error. X dimensions of landmark objects do not match in ConcatLandmarks!"
    << std::endl;
    sizematch = false;
    }
  if ( this->getYDim() != newlmks.getYDim() )
    {
    std::cout
    <<
    "Error. Y dimensions of landmark objects do not match in ConcatLandmarks!"
    << std::endl;
    sizematch = false;
    }
  if ( this->getZDim() != newlmks.getZDim() )
    {
    std::cout
    <<
    "Error. Z dimensions of landmark objects do not match in ConcatLandmarks!"
    << std::endl;
    sizematch = false;
    }
  if ( this->getTDim() != newlmks.getTDim() )
    {
    std::cout
    <<
    "Error. T dimensions of landmark objects do not match in ConcatLandmarks!"
    << std::endl;
    sizematch = false;
    }
  if ( sizematch == false )
    {
    return sizematch;
    }

  // This will store all the labels
  std::map< std::string, int > labelcount;
  std::string                  CurrentName;
  char                         labelbuffer[16];

  if ( newlmks.size() == 0 )
    {
    return false;
    }

  // Initialize the map with the labels found in this landmark object
  for ( typename InverseConsistentLandmarks::iterator iter = this->begin();
        iter != this->end();
        iter++ )
    {
    labelcount[iter->first] = 0;
    }

  for ( typename InverseConsistentLandmarks::iterator iter = newlmks.begin();
        iter != newlmks.end();
        iter++ )
    {
    if ( labelcount.find(iter->first) != labelcount.end() )
      {
      // Increment the counter and apply that name with the new tag
      labelcount[iter->first] = labelcount[iter->first] + 1;
      sprintf(labelbuffer, "%d", labelcount.find(iter->first)->second);
      CurrentName = iter->first + "_" + std::string(labelbuffer);
      }
    else
      {
      labelcount[iter->first] = 0;
      CurrentName = iter->first;
      }
    // std::cout << "CurrentName = " << CurrentName << std::endl;
    ( *this )[CurrentName] = iter->second;
    }

  return true;
}

template< typename PointStorageType, typename PointSetType >
bool
InverseConsistentLandmarks< PointStorageType, PointSetType >
::ConcatLandmarks(const std::string lmrkfilename)
{
  // Copy landmarks from newlmks to the current landmark file
  InverseConsistentLandmarks temp;

  temp.ReadPointTypes(lmrkfilename);
  return this->ConcatLandmarks(temp);
}

template< typename PointStorageType, typename PointSetType >
bool
InverseConsistentLandmarks< PointStorageType, PointSetType >
::ConcatLandmarks(const std::string lmrkfilename, const int XDim,
                  const int YDim, const int ZDim, const int TDim)
{
  // Copy landmarks from newlmks to the current landmark file
  InverseConsistentLandmarks temp;

  temp.ReadPointTypes(lmrkfilename, XDim, YDim, ZDim, TDim);
  return this->ConcatLandmarks(temp);
}

template< typename PointStorageType, typename PointSetType >
bool
InverseConsistentLandmarks< PointStorageType, PointSetType >
::RemoveClosePoints(const PointStorageType distance)
{
  bool removedpoint = false;

  typename InverseConsistentLandmarks::iterator iter;
  typename InverseConsistentLandmarks::iterator jter;

  for ( iter = this->begin(); iter != this->end(); iter++ )
    {
    // std::cout << "iter->first = " << iter->first << std::endl;
    typename InverseConsistentLandmarks::iterator ipp = iter;
    ipp++;
    jter = ipp;

    while ( jter != this->end() )
      {
      // std::cout << "jter->first = " << jter->first << std::endl;
      PointStorageType ix = iter->second[0];
      PointStorageType iy = iter->second[1];
      PointStorageType iz = iter->second[2];
      PointStorageType jx = jter->second[0];
      PointStorageType jy = jter->second[1];
      PointStorageType jz = jter->second[2];

      PointStorageType value =
        sqrt( ( jx
                - ix )
              * ( jx - ix ) + ( jy - iy ) * ( jy - iy ) + ( jz - iz ) * ( jz - iz ) );
      if ( value < distance )
        {
        // You lose the iterator when you issue an erase, so saving jter
        typename InverseConsistentLandmarks::iterator dter = jter;
        removedpoint = true;
        this->erase(dter->first);

        // The iterator has stayed the same, but the list has moved back one, so
        // an
        // extra increment is needed.
        jter++;
        }
      jter++;
      }
    }

  return removedpoint;
}

template< typename PointStorageType, typename PointSetType >
bool
InverseConsistentLandmarks< PointStorageType, PointSetType >
::RemoveUnmatchedPoints(InverseConsistentLandmarks & tempmap1)
{
  for ( typename InverseConsistentLandmarks::iterator mapiter = this->begin();
        mapiter != this->end();
        mapiter++ )
    {
    typename InverseConsistentLandmarks::iterator mapiter1 =
      tempmap1.find(mapiter->first);
    // Item NOT Found
    if ( mapiter1 == tempmap1.end() )
      {
      this->erase(mapiter->first);
      }
    }

  if ( this->size() > 0 )
    {
    return true;
    }
  else
    {
    return false;
    }
}

template< typename PointStorageType, typename PointSetType >
bool
InverseConsistentLandmarks< PointStorageType, PointSetType >
::rescale(const int newx, const int newy, const int newz, const int newt)
{
  assert(newx > 0);
  assert(newy > 0);
  assert(newz > 0);
  assert(newt > 0);

  const PointStorageType scalex = static_cast< PointStorageType >( newx )
                                  / static_cast< PointStorageType >( ImageDims[0] );
  const PointStorageType scaley = static_cast< PointStorageType >( newy )
                                  / static_cast< PointStorageType >( ImageDims[1] );
  const PointStorageType scalez = static_cast< PointStorageType >( newz )
                                  / static_cast< PointStorageType >( ImageDims[2] );
  const PointStorageType scalet = static_cast< PointStorageType >( newt )
                                  / static_cast< PointStorageType >( ImageDims[3] );

  ImageDims[0] = static_cast< unsigned short >( newx );
  ImageDims[1] = static_cast< unsigned short >( newy );
  ImageDims[2] = static_cast< unsigned short >( newz );
  ImageDims[3] = static_cast< unsigned short >( newt );
  std::cout << "Dims: " << ImageDims[0] << " " << ImageDims[1] << " "
            << ImageDims[2] << " " << ImageDims[3] << "\n";
  std::cout << "scale: " << scalex << " " << scaley << " " << scalez << " "
            << scalet << "\n";

  for ( typename InverseConsistentLandmarks::iterator mapiter = this->begin();
        mapiter != this->end(); mapiter++ )
    {
    mapiter->second[0] = mapiter->second[0] * scalex;
    mapiter->second[1] = mapiter->second[1] * scaley;
    mapiter->second[2] = mapiter->second[2] * scalez;
    mapiter->second.SetT(mapiter->second.GetT() * scalet);
    }

  return true;
}

template< typename PointStorageType, typename PointSetType >
bool InverseConsistentLandmarks< PointStorageType, PointSetType >
::ReadIPLPointTypes(const std::string lmrkfilename)
{
  std::cout << "Reading IPL Landmark File " << lmrkfilename << std::endl;
  // Delete all old landmarks
  this->erase( this->begin(), this->end() );
  // Open the files
  FILE *tempfile = fopen(lmrkfilename.c_str(), "r");
  if ( tempfile == NULL )
    {
    // std::cout << "ERROR: can not open " << lmrkfilename << std::endl;
    // exit( -1 );
    return false;
    }
  const unsigned short int FILE_BUFFER_SIZE = 256;
  const unsigned short int LANDMARK_NAME_SIZE = 40;
  char                     buffer[FILE_BUFFER_SIZE]; // Dummy Varible
  char                     CurrentLandmarkName[LANDMARK_NAME_SIZE];
  char *                   status = 0;
  // Read Header Information
  status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
  if ( status == NULL )
    {
    return false;
    }
  if ( strncmp(buffer, "IPL_HEADER_BEGIN", 16) != 0 )
    {
    // Only files with a header of IPL_HEADER_BEGIN will be accepted
    return false;
    }
  // Read dimensions of image that landmarks are associated with
  // skip a head to landmark definitions
  status = buffer;
  while ( ( strncmp(buffer, "LANDMARK_HEADER_BEGIN",
                    21) != 0 ) && ( status != NULL ) )
    {
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      std::cout
      << "ERROR: End of file reached before LANDMARK_HEADER_BEGIN found"
      << std::endl;
      }
    }
  bool xisset = false;
  bool yisset = false;
  bool zisset = false;
  ImageRes[0] = 1.0;
  ImageRes[1] = 1.0;
  ImageRes[2] = 1.0;
  ImageRes[3] = 1.0;

  // process looking for dims
  status = buffer;
  while ( ( strncmp(buffer, "LANDMARK_HEADER_END",
                    19) != 0 ) && ( status != NULL ) )
    {
    // Look for x
    if ( strncmp(buffer, "LANDMARK_X_SIZE", 15) == 0  )
      {
      sscanf( buffer, "%s %hu", CurrentLandmarkName, &( ImageDims[0] ) );
      xisset = true;
      }
    // Look for y
    if ( strncmp(buffer, "LANDMARK_Y_SIZE", 15) == 0  )
      {
      sscanf( buffer, "%s %hu", CurrentLandmarkName, &( ImageDims[1] ) );
      yisset = true;
      }
    // Look for z
    if ( strncmp(buffer, "LANDMARK_Z_SIZE", 15) == 0  )
      {
      sscanf( buffer, "%s %hu", CurrentLandmarkName, &( ImageDims[2] ) );
      zisset = true;
      }
    if ( strncmp(buffer, "LANDMARK_X_RESOLUTION", 20) == 0  )
      {
      sscanf( buffer, "%s %f", CurrentLandmarkName, &( ImageRes[0] ) );
      }
    if ( strncmp(buffer, "LANDMARK_Y_RESOLUTION", 20) == 0  )
      {
      sscanf( buffer, "%s %f", CurrentLandmarkName, &( ImageRes[1] ) );
      }
    if ( strncmp(buffer, "LANDMARK_Z_RESOLUTION", 20) == 0  )
      {
      sscanf( buffer, "%s %f", CurrentLandmarkName, &( ImageRes[2] ) );
      }
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      std::cout
      << "ERROR: End of file reached before LANDMARK_HEADER_END found"
      << std::endl;
      }
    }
  ImageDims[3] = 1;
  if ( !( xisset && yisset && zisset ) )
    {
    std::cout << "ERROR must set all LANDMARK_[XYZ]_SIZE values" << std::endl;
    }

  // Skip ahead to data
  status = buffer;
  while ( ( strncmp(buffer, "IPL_HEADER_END", 14) != 0 ) && ( status != NULL ) )
    {
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      std::cout << "ERROR: End of file reached before IPL_HEADER_END found"
                << std::endl;
      }
    }
  // Actually read the data
  while ( fgets(buffer, FILE_BUFFER_SIZE, tempfile) != NULL )
    {
    if ( buffer[0] == '#' )          // This allows for comments to be added
                                     // anywhere in the file
      {
      continue;
      }
    PointType TempPnt;
      {
      // DO I NEED TO INVERT Y HERE? SMP
      // const double invertY = static_cast<double> (ImageDims[1]-1);
      unsigned int numread = sscanf(buffer,
                                    "%s %lf %lf %lf ",
                                    CurrentLandmarkName,
                                    &TempPnt[0],
                                    &TempPnt[1],
                                    &TempPnt[2]);
      if ( numread != 4 )
        {
        std::cout << "ERROR invalid number of args read at landmark "
                  << CurrentLandmarkName << std::endl;
        return false;
        }
      }
    TempPnt.SetT(0.0F);
    TempPnt.SetWeighting(1.0F);
    if ( ( ImageDims[0] <= TempPnt[0] * ImageRes[0] )
         || ( ImageDims[1] <= TempPnt[1] * ImageRes[1] )
         || ( ImageDims[2] <= TempPnt[2] * ImageRes[2] )
         || ( ImageDims[3] <= TempPnt.GetT() * ImageRes[3] ) )
      {
      std::cout << "Point Set Outside of Image Dimensions." << std::endl;
      return false;
      }
    ( *this )[CurrentLandmarkName] = TempPnt;
    }
  fclose(tempfile);
  if ( this->size() > 0 )
    {
    return true;
    }
  else
    {
    return false;
    }
}

template< typename PointStorageType, typename PointSetType >
bool
InverseConsistentLandmarks< PointStorageType, PointSetType >
::ReadIPLPointTypes(const std::string lmrkfilename,
                    const int XDim, const int YDim,
                    const int ZDim, const int TDim)
{
  const bool testread = this->InverseConsistentLandmarks::ReadIPLPointTypes(
    lmrkfilename);

  this->InverseConsistentLandmarks::rescale(XDim, YDim, ZDim, TDim);
  return testread;
}

// ==================Talairach Bounds to Landmark Points===================
template< typename PointStorageType, typename PointSetType >
bool
InverseConsistentLandmarks< PointStorageType, PointSetType >
::process_bnd_point(const std::string & CurrentLandmarkName,
                    const char *buffer,
                    const unsigned short local_ImageDims[4],
                    const float /*local_ImageRes */[4],
                    PointType & ModifiedPoint)
{
  PointStorageType tmp[3];

    {
    double       val0;
    double       val1;
    double       val2;
    unsigned int numread = sscanf(buffer, "%lf %lf %lf ", &val0, &val1, &val2);
    if ( numread != 3 )
      {
      std::cout << "ERROR invalid number of args read at landmark "
                << CurrentLandmarkName << std::endl;
      return false;
      }
    tmp[0] = val0;
    tmp[1] = val1;
    tmp[2] = val2;
    }
  for ( int i = 0; i < 3; i++ )
    {
    ModifiedPoint[i] = tmp[i];
    }

  // Invert Y Readings for our coordinate system.
  ModifiedPoint[1] = local_ImageDims[1] - 1 - ModifiedPoint[1];
  ModifiedPoint.SetT(0.0F);
  ModifiedPoint.SetWeighting(1.0F);
  if ( local_ImageDims[0] <= ModifiedPoint[0] )
    {
    // std::cout << "Error: Point "<< CurrentLandmarkName << " has invalid x
    // coordinate.  Max value is: " << local_ImageDims[0]-1<<std::endl;
    // exit( -1 );
    return false;
    }
  if ( local_ImageDims[1] <= ModifiedPoint[1] )
    {
    // std::cout << "Error: Point "<< CurrentLandmarkName << " has invalid y
    // coordinate.  Max value is: " << local_ImageDims[1]-1<<std::endl;
    // exit( -1 );
    return false;
    }
  if ( local_ImageDims[2] <= ModifiedPoint[2] )
    {
    // std::cout << "Error: Point "<< CurrentLandmarkName << " has invalid z
    // coordinate.  Max value is: " << local_ImageDims[2]-1<<std::endl;
    // exit( -1 );
    return false;
    }
  if ( local_ImageDims[3] <= ModifiedPoint.GetT() )
    {
    // std::cout << "Error: Point "<< CurrentLandmarkName << " has invalid t
    // coordinate.  Max value is: " << local_ImageDims[3]-1<<std::endl;
    // exit( -1 );
    return false;
    }
  for ( int i = 0; i < 3; i++ )
    {
    // ModifiedPoint[i]*=local_ImageRes[i];
    }
  return true;
}

template< typename PointStorageType, typename PointSetType >
bool
InverseConsistentLandmarks< PointStorageType, PointSetType >
::ReadIPLTalairachPointTypes(const std::string lmrkfilename)
{
  // std::cout << "Reading IPL Talairach Bounds as  Landmark File " <<
  // lmrkfilename << std::endl;

  // Delete all old landmarks
  this->erase( this->begin(), this->end() );
  // Open the files
  std::cout << "  File: " << lmrkfilename.c_str() << std::endl;
  FILE *tempfile = fopen(lmrkfilename.c_str(), "r");
  if ( tempfile == NULL )
    {
    // std::cout << "ERROR: can not open " << lmrkfilename << std::endl;
    // exit( -1 );
    return false;
    }
  const unsigned short int FILE_BUFFER_SIZE = 256;
  char                     buffer[FILE_BUFFER_SIZE]; // Dummy Varible
  char *                   status = 0;
  // Read Header Information
  status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
  if ( status == NULL )
    {
    return false;
    }
  if ( strncmp(buffer, "IPL_HEADER_BEGIN", 16) != 0 )
    {
    // Only files with a header of IPL_HEADER_BEGIN will be accepted
    return false;
    }
  // Read dimensions of image that landmarks are associated with
  // skip a head to landmark definitions
  status = buffer;
  while ( ( strncmp(buffer, "TALAIRACH_PARAMETER_HEADER_BEGIN",
                    32) != 0 ) && ( status != NULL ) )
    {
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    // std::cout << "LINE: " << buffer << std::endl;
    if ( status == NULL )
      {
      std::cout
      <<
      "ERROR: End of file reached before TALAIRACH_PARAMETER_HEADER_BEGIN found"
      << std::endl;
      }
    }

  bool xisset = false;
  bool yisset = false;
  bool zisset = false;
  // process looking for dims
  status = buffer;
  while ( ( strncmp(buffer, "TALAIRACH_PARAMETER_HEADER_END",
                    30) != 0 ) && ( status != NULL ) )
    {
    const unsigned short int TALAIRACH_PARAMETER_NAME_SIZE = 40;
    char                     Dummy[TALAIRACH_PARAMETER_NAME_SIZE];
    // Look for x
    if ( strncmp(buffer, "TALAIRACH_PARAMETER_X_SIZE", 26) == 0  )
      {
      sscanf( buffer, "%s %hu", Dummy, &( ImageDims[0] ) );
      xisset = true;
      }
    // Look for y
    if ( strncmp(buffer, "TALAIRACH_PARAMETER_Y_SIZE", 26) == 0  )
      {
      sscanf( buffer, "%s %hu", Dummy, &( ImageDims[1] ) );
      yisset = true;
      }
    // Look for z
    if ( strncmp(buffer, "TALAIRACH_PARAMETER_Z_SIZE", 26) == 0  )
      {
      sscanf( buffer, "%s %hu", Dummy, &( ImageDims[2] ) );
      zisset = true;
      }
    if ( strncmp(buffer, "TALAIRACH_PARAMETER_X_RESOLUTION", 26) == 0  )
      {
      sscanf( buffer, "%s %f", Dummy, &( ImageRes[0] ) );
      }
    // Look for y
    if ( strncmp(buffer, "TALAIRACH_PARAMETER_Y_RESOLUTION", 26) == 0  )
      {
      sscanf( buffer, "%s %f", Dummy, &( ImageRes[1] ) );
      }
    // Look for z
    if ( strncmp(buffer, "TALAIRACH_PARAMETER_Z_RESOLUTION", 26) == 0  )
      {
      sscanf( buffer, "%s %f", Dummy, &( ImageRes[2] ) );
      }
    // if(!(xisset && yisset && zisset))
    // {
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    // }
    if ( status == NULL )
      {
      std::cout
      <<
      "ERROR: End of file reached before TALAIRACH_PARAMETER_HEADER_END found"
      << std::endl;
      }
    }
  ImageDims[3] = 1;
  ImageRes[3] = 1.0;
  if ( !( xisset && yisset && zisset ) )
    {
    std::cout << "ERROR must set all TALAIRACH_PARAMETER_[XYZ]_SIZE values"
              << std::endl;
    }

  // Skip ahead to data
  status = buffer;
  while ( ( strncmp(buffer, "IPL_HEADER_END", 14) != 0 ) && ( status != NULL ) )
    {
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      std::cout << "ERROR: End of file reached before IPL_HEADER_END found"
                << std::endl;
      }
    }
  // Actually read the data
  // First read the SLA point
    {
    PointType AC_Point;    // Antieror Commisure
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      std::cout
      <<
      "ERROR: End of file reached before TALAIRACH_PARAMETER_HEADER_END found"
      << std::endl;
      }
    if ( process_bnd_point("AC_Point", buffer, ImageDims, ImageRes,
                           AC_Point) == false )
      {
      return false;
      }

    PointType PC_Point;    // Posterior Commisure
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      std::cout
      <<
      "ERROR: End of file reached before TALAIRACH_PARAMETER_HEADER_END found"
      << std::endl;
      }
    if ( process_bnd_point("PC_Point", buffer, ImageDims, ImageRes,
                           PC_Point) == false )
      {
      return false;
      }

    PointType SLAPoint;   // Superior Left Anterior
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      std::cout
      <<
      "ERROR: End of file reached before TALAIRACH_PARAMETER_HEADER_END found"
      << std::endl;
      }
    if ( process_bnd_point("SLAPoint", buffer, ImageDims, ImageRes,
                           SLAPoint) == false )
      {
      return false;
      }

    PointType IRPPoint;   // Inferior Right Posterior
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      std::cout
      <<
      "ERROR: End of file reached before TALAIRACH_PARAMETER_HEADER_END found"
      << std::endl;
      }
    if ( process_bnd_point("IRPPoint", buffer, ImageDims, ImageRes,
                           IRPPoint) == false )
      {
      return false;
      }

    // Process to extend 4 points to all possible combinations
    PointType TempPnt = SLAPoint;
      {
      std::cout << "  Coordinates:: SLA " << SLAPoint << "   IRP "
                << IRPPoint << "   PC " << PC_Point << "   AC " << AC_Point
                << std::endl;
      std::string CurrentLandmarkName = "AC-Point";    TempPnt[0] = AC_Point[0];
      TempPnt[1] = AC_Point[1]; TempPnt[2] = AC_Point[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "PC-Point";    TempPnt[0] = PC_Point[0];
      TempPnt[1] = PC_Point[1]; TempPnt[2] = PC_Point[2];
      ( *this )[CurrentLandmarkName] = TempPnt;

      /* Inferior Points */
      CurrentLandmarkName = "IRP";    TempPnt[0] = IRPPoint[0]; TempPnt[1] =
        IRPPoint[1]; TempPnt[2] = IRPPoint[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "IRA";    TempPnt[0] = IRPPoint[0]; TempPnt[1] =
        IRPPoint[1]; TempPnt[2] = SLAPoint[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "ILP";    TempPnt[0] = SLAPoint[0]; TempPnt[1] =
        IRPPoint[1]; TempPnt[2] = IRPPoint[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "ILA";    TempPnt[0] = SLAPoint[0]; TempPnt[1] =
        IRPPoint[1]; TempPnt[2] = SLAPoint[2];
      ( *this )[CurrentLandmarkName] = TempPnt;

      /* Superior Points */
      CurrentLandmarkName = "SLA";    TempPnt[0] = SLAPoint[0]; TempPnt[1] =
        SLAPoint[1]; TempPnt[2] = SLAPoint[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "SLP";    TempPnt[0] = SLAPoint[0]; TempPnt[1] =
        SLAPoint[1]; TempPnt[2] = IRPPoint[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "SRA";    TempPnt[0] = IRPPoint[0]; TempPnt[1] =
        SLAPoint[1]; TempPnt[2] = SLAPoint[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "SRP";    TempPnt[0] = IRPPoint[0]; TempPnt[1] =
        SLAPoint[1]; TempPnt[2] = IRPPoint[2];
      ( *this )[CurrentLandmarkName] = TempPnt;

      /*
        *  CurrentLandmarkName="Tal_SLAPoint";    TempPnt[0]=SLAPoint[0];
        *  TempPnt[1]=SLAPoint[1]; TempPnt[2]=SLAPoint[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_SLPPoint";    TempPnt[0]=SLAPoint[0];
        *  TempPnt[1]=SLAPoint[1]; TempPnt[2]=IRPPoint[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_SRAPoint";    TempPnt[0]=IRPPoint[0];
        *  TempPnt[1]=SLAPoint[1]; TempPnt[2]=SLAPoint[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_SRPPoint";    TempPnt[0]=IRPPoint[0];
        *  TempPnt[1]=SLAPoint[1]; TempPnt[2]=IRPPoint[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_S_PC_APoint"; TempPnt[0]=PC_Point[0];
        *  TempPnt[1]=SLAPoint[1]; TempPnt[2]=SLAPoint[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_S_PC_PPoint"; TempPnt[0]=PC_Point[0];
        *  TempPnt[1]=SLAPoint[1]; TempPnt[2]=IRPPoint[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_SL_AC_Point"; TempPnt[0]=SLAPoint[0];
        *  TempPnt[1]=SLAPoint[1]; TempPnt[2]=AC_Point[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_SR_AC_Point"; TempPnt[0]=IRPPoint[0];
        *  TempPnt[1]=SLAPoint[1]; TempPnt[2]=AC_Point[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_SL_PC_Point"; TempPnt[0]=SLAPoint[0];
        *  TempPnt[1]=SLAPoint[1]; TempPnt[2]=PC_Point[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_SR_PC_Point"; TempPnt[0]=IRPPoint[0];
        *  TempPnt[1]=SLAPoint[1]; TempPnt[2]=PC_Point[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_S_AC_Point";  TempPnt[0]=PC_Point[0];
        *  TempPnt[1]=SLAPoint[1]; TempPnt[2]=AC_Point[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_S_PC_Point";  TempPnt[0]=PC_Point[0];
        *  TempPnt[1]=SLAPoint[1]; TempPnt[2]=PC_Point[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *
        *  CurrentLandmarkName="Tal_PC_LAPoint";    TempPnt[0]=SLAPoint[0];
        *  TempPnt[1]=PC_Point[1]; TempPnt[2]=SLAPoint[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_PC_LPPoint";    TempPnt[0]=SLAPoint[0];
        *  TempPnt[1]=PC_Point[1]; TempPnt[2]=IRPPoint[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_PC_RAPoint";    TempPnt[0]=IRPPoint[0];
        *  TempPnt[1]=PC_Point[1]; TempPnt[2]=SLAPoint[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_PC_RPPoint";    TempPnt[0]=IRPPoint[0];
        *  TempPnt[1]=PC_Point[1]; TempPnt[2]=IRPPoint[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_PC_PC_APoint"; TempPnt[0]=PC_Point[0];
        *  TempPnt[1]=PC_Point[1]; TempPnt[2]=SLAPoint[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_PC_PC_PPoint"; TempPnt[0]=PC_Point[0];
        *  TempPnt[1]=PC_Point[1]; TempPnt[2]=IRPPoint[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_PC_L_AC_Point"; TempPnt[0]=SLAPoint[0];
        *  TempPnt[1]=PC_Point[1]; TempPnt[2]=AC_Point[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_PC_R_AC_Point"; TempPnt[0]=IRPPoint[0];
        *  TempPnt[1]=PC_Point[1]; TempPnt[2]=AC_Point[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_PC_L_PC_Point"; TempPnt[0]=SLAPoint[0];
        *  TempPnt[1]=PC_Point[1]; TempPnt[2]=PC_Point[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_PC_R_PC_Point"; TempPnt[0]=IRPPoint[0];
        *  TempPnt[1]=PC_Point[1]; TempPnt[2]=PC_Point[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_AC_Point";      TempPnt[0]=PC_Point[0];
        *  TempPnt[1]=AC_Point[1]; TempPnt[2]=AC_Point[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_PC_Point";      TempPnt[0]=PC_Point[0];
        *  TempPnt[1]=PC_Point[1]; TempPnt[2]=PC_Point[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *
        *  CurrentLandmarkName="Tal_ILAPoint";    TempPnt[0]=SLAPoint[0];
        *  TempPnt[1]=IRPPoint[1]; TempPnt[2]=SLAPoint[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_ILPPoint";    TempPnt[0]=SLAPoint[0];
        *  TempPnt[1]=IRPPoint[1]; TempPnt[2]=IRPPoint[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_IRAPoint";    TempPnt[0]=IRPPoint[0];
        *  TempPnt[1]=IRPPoint[1]; TempPnt[2]=SLAPoint[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_IRPPoint";    TempPnt[0]=IRPPoint[0];
        *  TempPnt[1]=IRPPoint[1]; TempPnt[2]=IRPPoint[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_I_PC_APoint"; TempPnt[0]=PC_Point[0];
        *  TempPnt[1]=IRPPoint[1]; TempPnt[2]=SLAPoint[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_I_PC_PPoint"; TempPnt[0]=PC_Point[0];
        *  TempPnt[1]=IRPPoint[1]; TempPnt[2]=IRPPoint[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_IL_AC_Point"; TempPnt[0]=SLAPoint[0];
        *  TempPnt[1]=IRPPoint[1]; TempPnt[2]=AC_Point[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_IR_AC_Point"; TempPnt[0]=IRPPoint[0];
        *  TempPnt[1]=IRPPoint[1]; TempPnt[2]=AC_Point[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_IL_PC_Point"; TempPnt[0]=SLAPoint[0];
        *  TempPnt[1]=IRPPoint[1]; TempPnt[2]=PC_Point[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_IR_PC_Point"; TempPnt[0]=IRPPoint[0];
        *  TempPnt[1]=IRPPoint[1]; TempPnt[2]=PC_Point[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_I_AC_Point";  TempPnt[0]=PC_Point[0];
        *  TempPnt[1]=IRPPoint[1]; TempPnt[2]=AC_Point[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        *  CurrentLandmarkName="Tal_I_PC_Point";  TempPnt[0]=PC_Point[0];
        *  TempPnt[1]=IRPPoint[1]; TempPnt[2]=PC_Point[2];
        *     (*this)[CurrentLandmarkName]=TempPnt;
        */
      }
    }
  fclose(tempfile);
  if ( this->size() > 0 )
    {
    return true;
    }
  else
    {
    return false;
    }
}

template< typename PointStorageType, typename PointSetType >
bool
InverseConsistentLandmarks< PointStorageType, PointSetType >
::ReadIPLTalairachPointTypes(const std::string lmrkfilename,
                             const int XDim, const int YDim,
                             const int ZDim, const int TDim)
{
  const bool testread = this->InverseConsistentLandmarks::ReadIPLPointTypes(
    lmrkfilename);

  this->InverseConsistentLandmarks::rescale(XDim, YDim, ZDim, TDim);
  return testread;
}

//
// ======================================Cerebellum Bounds
template< typename PointStorageType, typename PointSetType >
bool
InverseConsistentLandmarks< PointStorageType, PointSetType >
::ReadIPLCerebellarPointTypes(const std::string lmrkfilename)
{
  //  std::cout << "Reading IPL Cerebellar Bounds as  Landmark File " <<
  // lmrkfilename << std::endl;

  // Delete all old landmarks
  // DEBUG:  NOTE we can not delete after being called from Talairach.bnd!!!
  //        this->erase( this->begin(  ), this->end(  ) );
  // Open the files
  FILE *tempfile = fopen(lmrkfilename.c_str(), "r");

  if ( tempfile == NULL )
    {
    // std::cout << "ERROR: can not open " << lmrkfilename << std::endl;
    // exit( -1 );
    return false;
    }
  const unsigned short int FILE_BUFFER_SIZE = 256;
  char                     buffer[FILE_BUFFER_SIZE]; // Dummy Varible
  char *                   status = 0;
  // Read Header Information
  status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
  if ( status == NULL )
    {
    return false;
    }
  if ( strncmp(buffer, "IPL_HEADER_BEGIN", 16) != 0 )
    {
    // Only files with a header of IPL_HEADER_BEGIN will be accepted
    return false;
    }
  // Read dimensions of image that landmarks are associated with
  // skip a head to landmark definitions
  status = buffer;
  while ( ( strncmp(buffer, "TALAIRACH_PARAMETER_HEADER_BEGIN",
                    32) != 0 ) && ( status != NULL ) )
    {
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      std::cout
      <<
      "ERROR: End of file reached before TALAIRACH_PARAMETER_HEADER_BEGIN found"
      << std::endl;
      }
    }

  bool xisset = false;
  bool yisset = false;
  bool zisset = false;
  // process looking for dims
  status = buffer;
  while ( ( strncmp(buffer, "TALAIRACH_PARAMETER_HEADER_END",
                    30) != 0 ) && ( status != NULL ) )
    {
    const unsigned short int TALAIRACH_PARAMETER_NAME_SIZE = 40;
    char                     Dummy[TALAIRACH_PARAMETER_NAME_SIZE];
    // Look for x
    if ( strncmp(buffer, "TALAIRACH_PARAMETER_X_SIZE", 26) == 0  )
      {
      sscanf( buffer, "%s %hu", Dummy, &( ImageDims[0] ) );
      xisset = true;
      }
    // Look for y
    if ( strncmp(buffer, "TALAIRACH_PARAMETER_Y_SIZE", 26) == 0  )
      {
      sscanf( buffer, "%s %hu", Dummy, &( ImageDims[1] ) );
      yisset = true;
      }
    // Look for z
    if ( strncmp(buffer, "TALAIRACH_PARAMETER_Z_SIZE", 26) == 0  )
      {
      sscanf( buffer, "%s %hu", Dummy, &( ImageDims[2] ) );
      zisset = true;
      }
    // if(!(xisset && yisset && zisset))
    // {
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    // }
    if ( status == NULL )
      {
      std::cout
      <<
      "ERROR: End of file reached before TALAIRACH_PARAMETER_HEADER_END found"
      << std::endl;
      }
    }
  ImageDims[3] = 1;
  if ( !( xisset && yisset && zisset ) )
    {
    std::cout << "ERROR must set all TALAIRACH_PARAMETER_[XYZ]_SIZE values"
              << std::endl;
    }

  // Skip ahead to data
  status = buffer;
  while ( ( strncmp(buffer, "IPL_HEADER_END", 14) != 0 ) && ( status != NULL ) )
    {
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      std::cout << "ERROR: End of file reached before IPL_HEADER_END found"
                << std::endl;
      }
    }
  // Actually read the data
  // First read the SLA point
    {
    PointType SLAPoint;   // Superior Left Anterior
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      std::cout
      <<
      "ERROR: End of file reached before TALAIRACH_PARAMETER_HEADER_END found"
      << std::endl;
      }
    if ( process_bnd_point("SLAPoint", buffer, ImageDims, ImageRes,
                           SLAPoint) == false )
      {
      return false;
      }
      {                    // NOTE: for Cerebellum AC_Point is meaningless
      PointType AC_Point;  // Antieror Commisure
      status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
      if ( status == NULL )
        {
        std::cout
        <<
        "ERROR: End of file reached before TALAIRACH_PARAMETER_HEADER_END found"
        << std::endl;
        }
      // if(process_bnd_point("AC_Point", buffer, ImageDims, ImageRes,
      // AC_Point)==false) {return false;}
      }

    PointType FV_Point;    // Posterior Commisure
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      std::cout
      <<
      "ERROR: End of file reached before TALAIRACH_PARAMETER_HEADER_END found"
      << std::endl;
      }
    if ( process_bnd_point("FV_Point", buffer, ImageDims, ImageRes,
                           FV_Point) == false )
      {
      return false;
      }

    PointType IRPPoint;   // Inferior Right Posterior
    status = fgets(buffer, FILE_BUFFER_SIZE, tempfile);
    if ( status == NULL )
      {
      std::cout
      <<
      "ERROR: End of file reached before TALAIRACH_PARAMETER_HEADER_END found"
      << std::endl;
      }
    if ( process_bnd_point("IRPPoint", buffer, ImageDims, ImageRes,
                           IRPPoint) == false )
      {
      return false;
      }
    // Process to extend 4 points to all possible combinations
    PointType TempPnt = SLAPoint;
      {
      std::string CurrentLandmarkName;
      //                                            R/L
      //                             S/I                           A/P
      // Next 4 may conflict with PC Point and center plane
      // CurrentLandmarkName="Cbl_SLAPoint";    TempPnt[0]=SLAPoint[0];
      // TempPnt[1]=SLAPoint[1]; TempPnt[2]=SLAPoint[2];
      // (*this)[CurrentLandmarkName]=TempPnt;
      // CurrentLandmarkName="Cbl_SLPPoint";    TempPnt[0]=SLAPoint[0];
      // TempPnt[1]=SLAPoint[1]; TempPnt[2]=IRPPoint[2];
      // (*this)[CurrentLandmarkName]=TempPnt;
      // CurrentLandmarkName="Cbl_SRAPoint";    TempPnt[0]=IRPPoint[0];
      // TempPnt[1]=SLAPoint[1]; TempPnt[2]=SLAPoint[2];
      // (*this)[CurrentLandmarkName]=TempPnt;
      // CurrentLandmarkName="Cbl_SRPPoint";    TempPnt[0]=IRPPoint[0];
      // TempPnt[1]=SLAPoint[1]; TempPnt[2]=IRPPoint[2];
      // (*this)[CurrentLandmarkName]=TempPnt;
      CurrentLandmarkName = "Cbl_S_FV_APoint"; TempPnt[0] = FV_Point[0];
      TempPnt[1] = SLAPoint[1]; TempPnt[2] = SLAPoint[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "Cbl_S_FV_PPoint"; TempPnt[0] = FV_Point[0];
      TempPnt[1] = SLAPoint[1]; TempPnt[2] = IRPPoint[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "Cbl_SL_FV_Point"; TempPnt[0] = SLAPoint[0];
      TempPnt[1] = SLAPoint[1]; TempPnt[2] = FV_Point[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "Cbl_SR_FV_Point"; TempPnt[0] = IRPPoint[0];
      TempPnt[1] = SLAPoint[1]; TempPnt[2] = FV_Point[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "Cbl_S_FV_Point";  TempPnt[0] = FV_Point[0];
      TempPnt[1] = SLAPoint[1]; TempPnt[2] = FV_Point[2];
      ( *this )[CurrentLandmarkName] = TempPnt;

      CurrentLandmarkName = "Cbl_FV_LAPoint";    TempPnt[0] = SLAPoint[0];
      TempPnt[1] = FV_Point[1]; TempPnt[2] = SLAPoint[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "Cbl_FV_LPPoint";    TempPnt[0] = SLAPoint[0];
      TempPnt[1] = FV_Point[1]; TempPnt[2] = IRPPoint[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "Cbl_FV_RAPoint";    TempPnt[0] = IRPPoint[0];
      TempPnt[1] = FV_Point[1]; TempPnt[2] = SLAPoint[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "Cbl_FV_RPPoint";    TempPnt[0] = IRPPoint[0];
      TempPnt[1] = FV_Point[1]; TempPnt[2] = IRPPoint[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "Cbl_FV_FV_APoint"; TempPnt[0] = FV_Point[0];
      TempPnt[1] = FV_Point[1]; TempPnt[2] = SLAPoint[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "Cbl_FV_FV_PPoint"; TempPnt[0] = FV_Point[0];
      TempPnt[1] = FV_Point[1]; TempPnt[2] = IRPPoint[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "Cbl_FV_L_FV_Point"; TempPnt[0] = SLAPoint[0];
      TempPnt[1] = FV_Point[1]; TempPnt[2] = FV_Point[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "Cbl_FV_R_FV_Point"; TempPnt[0] = IRPPoint[0];
      TempPnt[1] = FV_Point[1]; TempPnt[2] = FV_Point[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "Cbl_FV_Point";      TempPnt[0] = FV_Point[0];
      TempPnt[1] = FV_Point[1]; TempPnt[2] = FV_Point[2];
      ( *this )[CurrentLandmarkName] = TempPnt;

      // Next 4 may conflict with inferior plane
      // CurrentLandmarkName="Cbl_ILAPoint";    TempPnt[0]=SLAPoint[0];
      // TempPnt[1]=IRPPoint[1]; TempPnt[2]=SLAPoint[2];
      // (*this)[CurrentLandmarkName]=TempPnt;
      // CurrentLandmarkName="Cbl_ILPPoint";    TempPnt[0]=SLAPoint[0];
      // TempPnt[1]=IRPPoint[1]; TempPnt[2]=IRPPoint[2];
      // (*this)[CurrentLandmarkName]=TempPnt;
      // CurrentLandmarkName="Cbl_IRAPoint";    TempPnt[0]=IRPPoint[0];
      // TempPnt[1]=IRPPoint[1]; TempPnt[2]=SLAPoint[2];
      // (*this)[CurrentLandmarkName]=TempPnt;
      // CurrentLandmarkName="Cbl_IRPPoint";    TempPnt[0]=IRPPoint[0];
      // TempPnt[1]=IRPPoint[1]; TempPnt[2]=IRPPoint[2];
      // (*this)[CurrentLandmarkName]=TempPnt;
      CurrentLandmarkName = "Cbl_I_FV_APoint"; TempPnt[0] = FV_Point[0];
      TempPnt[1] = IRPPoint[1]; TempPnt[2] = SLAPoint[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "Cbl_I_FV_PPoint"; TempPnt[0] = FV_Point[0];
      TempPnt[1] = IRPPoint[1]; TempPnt[2] = IRPPoint[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "Cbl_IL_FV_Point"; TempPnt[0] = SLAPoint[0];
      TempPnt[1] = IRPPoint[1]; TempPnt[2] = FV_Point[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "Cbl_IR_FV_Point"; TempPnt[0] = IRPPoint[0];
      TempPnt[1] = IRPPoint[1]; TempPnt[2] = FV_Point[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      CurrentLandmarkName = "Cbl_I_FV_Point";  TempPnt[0] = FV_Point[0];
      TempPnt[1] = IRPPoint[1]; TempPnt[2] = FV_Point[2];
      ( *this )[CurrentLandmarkName] = TempPnt;
      }
    }
  fclose(tempfile);
  if ( this->size() > 0 )
    {
    return true;
    }
  else
    {
    return false;
    }
}

template< typename PointStorageType, typename PointSetType >
bool
InverseConsistentLandmarks< PointStorageType, PointSetType >
::ReadIPLCerebellarPointTypes(const std::string lmrkfilename,
                              const int XDim, const int YDim,
                              const int ZDim, const int TDim)
{
  const bool testread = this->InverseConsistentLandmarks::ReadIPLPointTypes(
    lmrkfilename);

  this->InverseConsistentLandmarks::rescale(XDim, YDim, ZDim, TDim);
  return testread;
}

template< typename PointStorageType, typename PointSetType >
void
InverseConsistentLandmarks< PointStorageType, PointSetType >
::AddExtendedPointTypes3D_UnitCube(const InverseConsistentLandmarks & input)
{
  // First create the center slice replicated in +-x and +-y directions
  InverseConsistentLandmarks centersliceLM = input;

  ( *this ) = input;                  // Used to reset image Dims
  bool is3d = false;
  for ( typename InverseConsistentLandmarks::const_iterator iter = input.begin();
        iter != input.end();
        iter++ )
    {
    const PointStorageType xpoint = iter->second[0];
    const PointStorageType ypoint = iter->second[1];
    const PointStorageType zpoint = iter->second[2];
    if ( zpoint > 1e-5 )             // This assumes image sizes less than 10000
                                     // slices
      {
      is3d = true;
      }
    const PointStorageType tpoint = iter->second.GetT();
    const PointStorageType wpoint = iter->second.GetWeighting();
    std::string            base(iter->first);

    std::string pl;                  // pointlable
    // Center
    centersliceLM[base] = iter->second;
    pl = "Left";
    pl += base;
    // Left
    centersliceLM[pl] =
      PointType( ( xpoint - 1.0F ), ( ypoint - 0.0F ), zpoint, tpoint,
                 wpoint );
    pl = "TopLeft";
    pl += base;
    // TopLeft
    centersliceLM[pl] = PointType(xpoint - 1.0F,
                                  ypoint + 1.0F,
                                  zpoint,
                                  tpoint,
                                  wpoint);
    pl = "Top";
    pl += base;
    // Top
    centersliceLM[pl] = PointType(xpoint - 0.0F,
                                  ypoint + 1.0F,
                                  zpoint,
                                  tpoint,
                                  wpoint);
    pl = "TopRight";
    pl += base;
    // TopRight
    centersliceLM[pl] = PointType(xpoint + 1.0F,
                                  ypoint + 1.0F,
                                  zpoint,
                                  tpoint,
                                  wpoint);
    pl = "Right";
    pl += base;
    // Right
    centersliceLM[pl] = PointType(xpoint + 1.0F,
                                  ypoint - 0.0F,
                                  zpoint,
                                  tpoint,
                                  wpoint);
    pl = "BottomRight";
    pl += base;
    // BottomRight
    centersliceLM[pl] = PointType(xpoint + 1.0F,
                                  ypoint - 1.0F,
                                  zpoint,
                                  tpoint,
                                  wpoint);
    pl = "Bottom";
    pl += base;
    // Bottom
    centersliceLM[pl] = PointType(xpoint - 0.0F,
                                  ypoint - 1.0F,
                                  zpoint,
                                  tpoint,
                                  wpoint);
    pl = "BottomLeft";
    pl += base;
    // BottomLeft
    centersliceLM[pl] = PointType(xpoint - 1.0F,
                                  ypoint - 1.0F,
                                  zpoint,
                                  tpoint,
                                  wpoint);
    }
  for ( typename InverseConsistentLandmarks::const_iterator iter =
          centersliceLM.begin();
        iter != centersliceLM.end();
        iter++ )
    {
    const PointStorageType xpoint = iter->second[0];
    const PointStorageType ypoint = iter->second[1];
    const PointStorageType zpoint = iter->second[2];
    const PointStorageType tpoint = iter->second.GetT();
    const PointStorageType wpoint = iter->second.GetWeighting();
    std::string            base(iter->first);

    std::string pl;                   // pointlable
    ( *this )[base] = iter->second;   // Center

    if ( is3d )                      // Only Do this if it is a 3D image
      {
      pl = "Previous";
      pl += base;
      ( *this )[pl] =
        PointType( ( xpoint ), ( ypoint ), ( zpoint - 1.0F ), tpoint,
                   wpoint );

      pl = "Next";
      pl += base;
      ( *this )[pl] =
        PointType( ( xpoint ), ( ypoint ), ( zpoint + 1.0F ), tpoint,
                   wpoint );
      }
    }
  return;
}

template< typename PointStorageType, typename PointSetType >
void
InverseConsistentLandmarks< PointStorageType, PointSetType >
::AddExtendedPointTypes3D_OnN(const InverseConsistentLandmarks & input,
                              const int nx, const int ny, const int nz)
{
  // First create the center slice replicated in +-x and +-y directions
  InverseConsistentLandmarks centersliceLM = input;

  ( *this ) = input;                  // Used to reset image Dims
  bool is3d = false;
  for ( typename InverseConsistentLandmarks::const_iterator iter = input.begin();
        iter != input.end();
        iter++ )
    {
    const PointStorageType xpoint = iter->second[0];
    const PointStorageType ypoint = iter->second[1];
    const PointStorageType zpoint = iter->second[2];
    if ( zpoint > 1e-5 )             // This assumes image sizes less than 10000
                                     // slices
      {
      is3d = true;
      }
    const PointStorageType tpoint = iter->second.GetT();
    const PointStorageType wpoint = iter->second.GetWeighting();
    std::string            base(iter->first);

    std::string pl;                  // pointlable
    // Center
    centersliceLM[base] = iter->second;
    pl = "Left";
    pl += base;
    // Left
    centersliceLM[pl] =
      PointType( ( xpoint - nx ), ( ypoint - 0.0F ), zpoint, tpoint,
                 wpoint );
    pl = "TopLeft";
    pl += base;
    // TopLeft
    centersliceLM[pl] = PointType(xpoint - nx,
                                  ypoint + ny,
                                  zpoint,
                                  tpoint,
                                  wpoint);
    pl = "Top";
    pl += base;
    // Top
    centersliceLM[pl] = PointType(xpoint - 0.0F,
                                  ypoint + ny,
                                  zpoint,
                                  tpoint,
                                  wpoint);
    pl = "TopRight";
    pl += base;
    // TopRight
    centersliceLM[pl] = PointType(xpoint + nx,
                                  ypoint + ny,
                                  zpoint,
                                  tpoint,
                                  wpoint);
    pl = "Right";
    pl += base;
    // Right
    centersliceLM[pl] = PointType(xpoint + nx,
                                  ypoint - 0.0F,
                                  zpoint,
                                  tpoint,
                                  wpoint);
    pl = "BottomRight";
    pl += base;
    // BottomRight
    centersliceLM[pl] = PointType(xpoint + nx,
                                  ypoint - ny,
                                  zpoint,
                                  tpoint,
                                  wpoint);
    pl = "Bottom";
    pl += base;
    // Bottom
    centersliceLM[pl] = PointType(xpoint - 0.0F,
                                  ypoint - ny,
                                  zpoint,
                                  tpoint,
                                  wpoint);
    pl = "BottomLeft";
    pl += base;
    // BottomLeft
    centersliceLM[pl] = PointType(xpoint - nx,
                                  ypoint - ny,
                                  zpoint,
                                  tpoint,
                                  wpoint);
    }
  for ( typename InverseConsistentLandmarks::const_iterator iter =
          centersliceLM.begin();
        iter != centersliceLM.end();
        iter++ )
    {
    const PointStorageType xpoint = iter->second[0];
    const PointStorageType ypoint = iter->second[1];
    const PointStorageType zpoint = iter->second[2];
    const PointStorageType tpoint = iter->second.GetT();
    const PointStorageType wpoint = iter->second.GetWeighting();
    std::string            base(iter->first);

    std::string pl;                   // pointlable
    ( *this )[base] = iter->second;   // Center

    if ( is3d )                      // Only Do this if it is a 3D image
      {
      pl = "Previous";
      pl += base;
      ( *this )[pl] =
        PointType( ( xpoint ), ( ypoint ), ( zpoint - nz ), tpoint,
                   wpoint );

      pl = "Next";
      pl += base;
      ( *this )[pl] =
        PointType( ( xpoint ), ( ypoint ), ( zpoint + nz ), tpoint,
                   wpoint );
      }
    }
  return;
}
}
#endif
