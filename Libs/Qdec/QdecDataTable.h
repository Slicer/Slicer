/**
 * @file  QdecDataTable.h
 * @brief Container for the text-input file to QDEC.
 *
 * Implements loading/saving the white-space delimited data file containing
 * the list of subjects with their discrete and continuous factors.
 */
/*
 * Original Author: Nick Schmansky
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2007/05/23 23:05:35 $
 *    $Revision: 1.4 $
 *
 * Copyright (C) 2007,
 * The General Hospital Corporation (Boston, MA).
 * All rights reserved.
 *
 * Distribution, usage and copying of this software is covered under the
 * terms found in the License Agreement file named 'COPYING' found in the
 * FreeSurfer source code root directory, and duplicated here:
 * https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferOpenSourceLicense
 *
 * General inquiries: freesurfer@nmr.mgh.harvard.edu
 * Bug reports: analysis-bugs@nmr.mgh.harvard.edu
 *
 */

#ifndef QDECDATATABLE_H
#define QDECDATATABLE_H

#include <QdecConfigure.h>
#include "vtkQdecWin32Header.h"

#include <string>
#include <vector>

#include "QdecFactor.h"
#include "QdecSubject.h"

//#include "vtkQdec.h"

using namespace std;

class VTK_Qdec_EXPORT QdecDataTable
{
public:

  // Constructors/Destructors
  //

  QdecDataTable ( );

  virtual ~QdecDataTable ( );

  /**
   * Load white-space delimited file containing subject ids and their
   * discrete and continuous factors.
   * @return int
   * @param  isFileName
   * @param  osNewSubjDir
   */
  int Load (const char* isFileName, char* osNewSubjDir );


  /**
   * @return int
   * @param  isFileName
   */
  int Save (const char* isFileName );


  /**
   * @return string
   */
  string GetFileName ( );


  /**
   * @return vector< string >
   */
  vector< string > GetSubjectIDs ( );


  /**
   * @return vector< QdecSubject* >
   */
  vector< QdecSubject* > GetSubjects ( );


  /**
   * @return QdecFactor*
   * @param isFactorName
   */
  QdecFactor* GetFactor ( const char* isFactorName );


  /**
   * @return vector< string >
   */
  vector< string > GetDiscreteFactors ( );


  /**
   * @return vector< string >
   */
  vector< string > GetContinuousFactors ( );


  /**
   * GetNumberOfClasses( ) - returns the number of classes for the design.
   * The number of classes is just all the combinations of all
   * the levels for the discrete factors.
   */
  int GetNumberOfClasses ( );

  /**
   * GetNumberOfRegressors() - returns the number of regressors for the
   * given design.
   */
  int GetNumberOfRegressors ( );

  /**
   * dumps factors and inputs to filepointer (stdout, or file)
   * @param  iFilePointer
   */
  void Dump (  FILE* iFilePointer );

  /**
   * GetMeanAndStdDev() - computes the average and stddev of continuous factor
   * @return vector< double > - first element is mean, second is the stddev
   * @param isFactorName
   */
  vector< double > GetMeanAndStdDev ( const char* isFactorName );

private:

  // private attributes
  //

  string mfnFileName;

  vector < QdecFactor* > mFactors;

  // Stores subject data (id and factors) as read from the
  // table.dat input file.
  vector < QdecSubject* > mSubjects;

  /**
   * Check that all subjects exist in the specified subjects_dir (including the
   * specified average subject).  Print to stderr and ErrorMessage any errors
   * found (one message for each error).  Also check that thickness, sulc,
   * curv, area and jacobian_white files exist, and that their vertex
   * numbers equal their inflated surface (and that surfaces all have the
   * same number of vertices).
   * @return int
   */
  int VerifySubjects ( );

};

#endif // QDECDATATABLE_H
