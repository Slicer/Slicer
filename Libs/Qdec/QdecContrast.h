/**
 * @file  QdecContrast.h
 * @brief Stores a GLM contrast vector
 *
 * Stores a GLM contrast vector associated with a particular design (being
 * based on user selected factors, formulating a hypothesis to test).
 * Stores the name associated with it (to go in the .fsgd file), the
 * human-readable question (for use by GUI), and the vector itself (as an
 * int array).
 */
/*
 * Original Author: Nick Schmansky
 */

#ifndef QDECCONTRAST_H
#define QDECCONTRAST_H

#include <QdecConfigure.h>
#include "vtkQdecWin32Header.h"

#include <string>
#include <vector>
#ifndef _WIN32
#include <sys/param.h>
#else
#include <direct.h>
#endif
//#include "vtkQdec.h"

using namespace std;

class  VTK_Qdec_EXPORT QdecContrast
{
public:

  /// Constructors/Destructors
  //

  QdecContrast ( vector< double > iaVector,
                 string isName,
                 string isQuestion );

  virtual ~QdecContrast ( );

  /**
   * @return string
   */
  string GetName ( );


  /**
   * @return string
   */
  string GetQuestion ( );


  /**
   * @return string
   */
  string GetContrastStr ( );


  /**
   * Writes the contrast vector to a .mat file, which is readable by matlab,
   * and mri_glmfit.
   * @return int
   * @param string ifnWorkingDir
   */
  int WriteDotMatFile ( string ifnWorkingDir );


  /**
   * @return string
   */
  string GetDotMatFileName ( );

private:

  /// private attributes
  //

  vector< double > maVector;
  string msName;
  string msQuestion;
  string mfnDotMatFileName;

};

#endif /// QDECCONTRAST_H
