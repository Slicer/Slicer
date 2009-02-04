/*=auto========================================================================

  (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  Project:      ZLinAlg
  Description:  General Linear Algebra functions and operators required by 
                ZTrackerTransform.
  Author:       Simon DiMaio, simond@bwh.harvard.edu
  Version:      $Id: ZLinAlg.h,v 1.2 2007/01/17 12:44:08 simond Exp $

=========================================================================auto=*/


/*= INCLUDE ==================================================================*/

#ifndef _ZLINALG_H
#define _ZLINALG_H

#include "math.h"
#include <iostream>
//#include "ZLogger.h"


/*= DEFINE ===================================================================*/

// Tolerance for vector operations.
#define VEPSILON  (1e-10)


/*= GLOBAL ===================================================================*/

//extern ZLogger ZLog;


/*= CLASS: Column2Vector =====================================================*/

/** \class Column2Vector
  * \brief A class of 2-vectors and a number of linear algebra methods and 
  *        operators.
  */
class Column2Vector
{
  public:
    Column2Vector();
    ~Column2Vector();

    float           getX();
    float           getY();

    void            setvalues(float X, float Y);
    void            setX(float X);
    void            setY(float Y);

    bool            normalize();
    float           norm();

    Column2Vector   operator+(Column2Vector Vright);
    Column2Vector   operator-(Column2Vector Vright);
    Column2Vector   operator*(float);
    Column2Vector   operator/(float);
    Column2Vector&  operator=(Column2Vector);

    void            PrintSelf();

  private:
    float           VectorArray[2];
};


/*= CLASS: Column3Vector =====================================================*/

/** \class Column3Vector
  * \brief A class of 3-vectors and a number of linear algebra methods and 
  *        operators.
  */
class Column3Vector
{
  public:
    Column3Vector();
    ~Column3Vector();

    float           getX();
    float           getY();
    float           getZ();

    void            setvalues(float X, float Y, float Z);
    void            setX(float X);
    void            setY(float Y);
    void            setZ(float Z);

    bool            normalize();
    float           norm();

    Column3Vector   operator+(Column3Vector Vright);
    Column3Vector   operator-(Column3Vector Vright);
    Column3Vector   operator*(Column3Vector Vright);
    Column3Vector   operator*(float);
    Column3Vector   operator/(float);
    Column3Vector&  operator=(Column3Vector);

    void            PrintSelf();

  private:
    float           VectorArray[3];
};


/*= CLASS: Quaternion ========================================================*/

/** \class Quaternion
  * \brief A class of quaternions and a number of linear algebra methods and 
  *        operators.
  */
class Quaternion
{
  public:
    Quaternion();
    ~Quaternion();

    float         getX();
    float         getY();
    float         getZ();
    float         getW();

    void          setvalues(float X, float Y, float Z, float W);
    void          setX(float X);
    void          setY(float Y);
    void          setZ(float Z);
    void          setW(float W);

    float         norm();
    bool          normalize();
    bool          conjugate();
    bool          ComputeFromRotationMatrix(Column3Vector C0, Column3Vector C1, 
                                            Column3Vector C2);

    Column3Vector RotateVector(Column3Vector P1);

    Quaternion    operator*(Quaternion Qright);
    Quaternion    operator/(Quaternion Qright);
    Quaternion&   operator=(Quaternion Qnew);

    void          PrintSelf();

  private:
    float         QuaternionArray[4];
};

#endif

/*= END ZLinAlg.h ============================================================*/
