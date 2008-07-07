/**
 * @file  QdecSubject.cpp
 * @brief Stores all data associated with a subject.
 *
 * This is one row from the input data table file (qdec.table.dat).
 */
/*
 * Original Author: Nick Schmansky
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2007/05/20 23:15:23 $
 *    $Revision: 1.2 $
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

#include <cstring>
#include <stdexcept>

#include "QdecSubject.h"


// Constructors/Destructors
//

QdecSubject::QdecSubject ( string isId, vector< QdecFactor* > iFactors )
{
  msId = isId;
  mFactors = iFactors;
}

QdecSubject::~QdecSubject ( )
{
  while (mFactors.size() != 0)
  {
    delete mFactors.back();
    mFactors.pop_back();
  }
}

/**
 * Get the value of msId the subject identifier, as found in the
 * 'fsid' column of the table.dat input file.
 * @return the value of msId
 * @return string
 */
string QdecSubject::GetId ( )
{
  return msId;
}


/**
 * @return string
 * @param  isFactorName
 */
string QdecSubject::GetDiscreteFactor (const char* isFactorName )
{
  for (unsigned int i=0; i < mFactors.size(); i++)
  {
    if (mFactors[i]->IsDiscrete())
    {
      if ( 0 == strcmp( mFactors[i]->GetFactorName().c_str(), isFactorName ) )
      {
        return mFactors[i]->GetDiscreteValue();
      }
    }
  }

  throw runtime_error( string("ERROR: QdecSubject::GetDiscreteFactor failure, cannot find factor ") + string(isFactorName) );
  return NULL;
}


/**
 * @return double
 * @param  isFactorName
 */
double QdecSubject::GetContinuousFactor (const char* isFactorName )
{
  for (unsigned int i=0; i < mFactors.size(); i++)
  {
    if (mFactors[i]->IsContinuous())
    {
      if ( 0 == strcmp( mFactors[i]->GetFactorName().c_str(), isFactorName ) )
      {
        return mFactors[i]->GetContinuousValue();
      }
    }
  }

  throw runtime_error( string("ERROR: QdecSubject::GetContinuousFactor failure, cannot find factor: ") + string(isFactorName));
  return 0.0;
}


/**
 * @return vector < QdecFactor* >
 */
vector < QdecFactor* > QdecSubject::GetContinuousFactors ( )
{
  vector < QdecFactor* > factors;
  for (unsigned int i=0; i < mFactors.size(); i++)
  {
    if (mFactors[i]->IsContinuous())
    {
      factors.push_back( mFactors[i] );
    }
  }

  return factors;
}


/**
 * @return vector < QdecFactor* >
 */
vector < QdecFactor* > QdecSubject::GetFactors ( )
{
  return mFactors;
}


