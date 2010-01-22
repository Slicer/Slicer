/**
 * @file  QdecGlmFit.h
 * @brief Wrapper for mri_glmfit.
 *
 * Run mri_glmfit, given its input data, and puts the output in the specified
 * working directory.
 */
/*
 * Original Author: Nick Schmansky
 */

#ifndef QDECGLMFIT_H
#define QDECGLMFIT_H

#include <QdecConfigure.h>
#include "vtkQdecWin32Header.h"

#include "QdecGlmDesign.h"
#include "QdecGlmFitResults.h"

//#include "vtkQdec.h"

using namespace std;

class  VTK_Qdec_EXPORT QdecGlmFit
{
public:

  /// Constructors/Destructors
  //

  QdecGlmFit ( );

  virtual ~QdecGlmFit ( );

  /// public attribute accessor methods
  //


  /**
   * @return int
   * @param  iGlmDesign
   */
  int Run (QdecGlmDesign* iGlmDesign );

  /**
   * Creates the contrast, stddef, coefficients, and fsgdf file names
   * using the working directory from the design. Creates the results
   * object with these values. Does not generate any new data.
   * @return int
   * @param iGlmDesign
   */
  int CreateResultsFromCachedData ( QdecGlmDesign* iGlmDesign );

  
   /**
   * @return int
   * @param  iGlmDesign
   */
  //int Load (QdecGlmDesign* iGlmDesign );

  

  /**
   * @return QdecGlmFitResults
   */
  QdecGlmFitResults* GetResults ( );

private:

  /// private attributes
  //

  QdecGlmFitResults* mGlmFitResults;

};

#endif /// QDECGLMFIT_H
