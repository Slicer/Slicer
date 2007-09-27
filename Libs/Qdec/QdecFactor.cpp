/**
 * @file  QdecFactor.cpp
 * @brief Stores a factor, which can be either discrete or continuous
 *
 * An example of a discrete factor is gender (male or female) or
 * diagnosis (demented or nondemented).  An example continuous factor is
 * age, or volume of a subcortical structure.
 */
/*
 * Original Author: Nick Schmansky
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2007/05/20 23:15:23 $
 *    $Revision: 1.3 $
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

#include "QdecFactor.h"


// Constructors/Destructors
//

QdecFactor::QdecFactor ( const char* isName,
                         int iType /* ==1 discrete or ==2 continuous */ )
{
  msName = isName;

  // if ==1, continuous
  // if ==2, discrete
  mType = iType;
  assert( (mType == 1) || (mType == 2) );

  mHaveDotLevelsFile = false;

}

QdecFactor::QdecFactor ( const char* isName,
                         int iType, // ==1 discrete
                         const char* iValue )
{
  msName = isName;

  // if ==1, continuous
  // if ==2, discrete
  mType = iType;
  assert( mType == 1 );

  msDiscreteValue = iValue;

  mHaveDotLevelsFile = false;
}


QdecFactor::QdecFactor ( const char* isName,
                         int iType, // ==2 continuous
                         double iValue )
{
  msName = isName;

  // if ==1, continuous
  // if ==2, discrete
  mType = iType;
  assert( mType == 2 );

  mContinuousValue = iValue;

  mHaveDotLevelsFile = false;
}


QdecFactor::~QdecFactor ( )
{ }

//
// Methods
//

/**
 * @return bool
 */
bool QdecFactor::IsDiscrete ( )
{
  if ( mType == 1 ) return true;
  return false;
}


/**
 * @return bool
 */
bool QdecFactor::IsContinuous ( )
{
  if ( mType == 2 ) return true;
  return false;
}


/**
 * @return string
 */
string QdecFactor::GetFactorName ( )
{
  return msName;
}


/**
 * GetFactorTypeName() - returns the string name of the
 * type of the given factor: 'continuous' or 'discrete'
 * @return string
 */
string QdecFactor::GetFactorTypeName ( )
{
  if (this->IsContinuous())return("continuous");
  if (this->IsDiscrete())  return("discrete");
  return("type-error");
}

/**
 * @return int
 * @param  isLevelName
 */
void QdecFactor::AddLevelName ( string isLevelName )
{
  assert( mType == 1 );

  // check if already in our list:
  if (this->ValidLevelName( isLevelName.c_str() ))
    {
    return;
    }

  mLevelNames.push_back( isLevelName );
}


/**
 * @return vector< string >
 */
vector< string > QdecFactor::GetLevelNames ( )
{
  return mLevelNames;
}


/**
 * Returns true if the given levelName is in our list of known level names
 * @return bool
 */
bool QdecFactor::ValidLevelName ( const char* iLevelName )
{
  for ( unsigned int i=0; i < mLevelNames.size(); i++ )
  {
    if ( strcmp(iLevelName, mLevelNames[i].c_str() ) == 0 ) return true;
  }
  return false;
}

/**
 * Returns the value of the discrete factor stored in this instance
 * (null if this is not a discrete factor).
 * @return string
 */
string QdecFactor::GetDiscreteValue ( )
{
  assert( mType == 1 );
  return msDiscreteValue;
}


/**
 * Returns the value of the continous factor stored in this instance
 * (null if this is not a continuous factor).
 * @return double
 */
double QdecFactor::GetContinuousValue ( )
{
  assert( mType == 2 );
  return mContinuousValue;
}

