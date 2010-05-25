/*=auto========================================================================

  (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  Project:      ZLinAlg
  Description:  General Linear Algebra functions and operators required by 
                ZTrackerTransform.
  Author:       Simon DiMaio, simond@bwh.harvard.edu
  Version:      $Id: ZLinAlg.cxx,v 1.3 2007/02/12 15:52:02 ahans Exp $

=========================================================================auto=*/

/*= INCLUDE ==================================================================*/

#include "ZLinAlg.h"
#include <stdio.h>

/*= METHODS: Column2Vector ===================================================*/

/**
 * Constructor method.
 * The vector elements are initialized to zero.
 * @return none
 */
Column2Vector::Column2Vector()
{
  this->VectorArray[0] = 0.0;
  this->VectorArray[1] = 0.0;
}

/*----------------------------------------------------------------------------*/

/**
 * Destructor method.
 * Nothing to do here.
 * @return none 
 */
Column2Vector::~Column2Vector()
{
  // Nothing to do.
}

/*----------------------------------------------------------------------------*/

/**
 * Get the first element of the 2-vector.
 * @return first element of the 2-vector.
 */
float Column2Vector::getX()
{
  return( this->VectorArray[0] );
}

/*----------------------------------------------------------------------------*/

/**
 * Get the second element of the 2-vector.
 * @return second element of the 2-vector.
 */
float Column2Vector::getY()
{
  return( this->VectorArray[1] );
}

/*----------------------------------------------------------------------------*/

/**
 * Set the vector elements using the X, Y and Z values provided.
 * @param X The first scalar element of the 2-vector.
 * @param Y The second scalar element of the 2-vector.
 */
void  Column2Vector::setvalues(float X, float Y)
{
  this->VectorArray[0] = X;
  this->VectorArray[1] = Y;
}

/*----------------------------------------------------------------------------*/

/**
 * Get the specified element of the 2-vector.
 * @param X The new value of the first element of the 2-vector. 
 */
void Column2Vector::setX(float X)
{
  this->VectorArray[0] = X;
}

/*----------------------------------------------------------------------------*/

/**
 * Get the specified element of the 2-vector.
 * @param Y The new value of the second element of the 2-vector.
 */
void Column2Vector::setY(float Y)
{
  this->VectorArray[1] = Y;
}

/*----------------------------------------------------------------------------*/

/**
 * Normalize the 2-vector.
 * @return True if successfully normalized, false if vector cannot be normalized.
 */
bool  Column2Vector::normalize()
{
  float vnorm;

  vnorm = this->norm();

  //  RISK: the vector norm may be close to zero, resulting in a divide-by-zero 
  //  when normalizing. Return false if close to zero.
  if(vnorm<VEPSILON)
  {
    printf("Column2Vector::normalize() - vector norm close \
                            to zero.");
    return(false);
  }

  this->VectorArray[0] /= vnorm;
  this->VectorArray[1] /= vnorm;
  
  return(true);
}

/*----------------------------------------------------------------------------*/

/**
 * Compute the norm of the 2-vector.
 * @return The norm of the 2-vector.
 */
float Column2Vector::norm()
{
  float vnorm, sqnorm;

  // Inner product of vector with itself.
  sqnorm =  this->VectorArray[0]*this->VectorArray[0] +
            this->VectorArray[1]*this->VectorArray[1];

  //  RISK: the squared norm may be negative if overflow occurs, resulting in 
  //  failure of the square root. Return zero in this case. 
  if(sqnorm<0.0)
  {
    printf("Column2Vector::norm() - negative sqrt() argument.");
    return(0.0);
  }
  
  // Take the square root for the vector norm.
  vnorm = sqrt((float)sqnorm);

  return(vnorm);
}

/*----------------------------------------------------------------------------*/

/**
 * An overloaded operator for summing two 2-vectors.
 * @param   vector2 2-vector that is to be added to this one.
 * @return  The sum of the two 2-vectors.
 */
Column2Vector Column2Vector::operator+(Column2Vector Vright)
{
  Column2Vector result;

  result.setX( this->getX() + Vright.getX() );
  result.setY( this->getY() + Vright.getY() );

  return(result);
}

/*----------------------------------------------------------------------------*/

/**
 * An overloaded operator for subtracting a 2-vector.
 * @param   vector2 The 2-vector that is to be subtracted from this one.
 * @return  The difference of the two 2-vectors.
 */
Column2Vector Column2Vector::operator-(Column2Vector Vright)
{
  Column2Vector result;

  result.setX( this->getX() - Vright.getX() );
  result.setY( this->getY() - Vright.getY() );

  return(result);
}

/*----------------------------------------------------------------------------*/

/**
 * An overloaded operator for multiplying a 2-vector by a scalar value.
 * @param scalefactor A scalar coefficient.
 * @return The resulting scaled 2-vector.
 */
Column2Vector Column2Vector::operator*(float scalefactor)
{
  Column2Vector product;

  product.setX( this->getX() * scalefactor );
  product.setY( this->getY() * scalefactor );

  return(product);
}

/*----------------------------------------------------------------------------*/

/**
 * An overloaded operator for dividing a 2-vector by a scalar value.
 * @param divisor A scalar value to divide the 2-vector.
 * @return The resulting 2-vector quotient.
 */
Column2Vector Column2Vector::operator/(float divisor)
{
  Column2Vector quotient;

  // RISK:  Divide by zero.
  if(divisor<VEPSILON)
  {
    printf("Column2Vector::operator/(float) - divide by zero.");
    quotient.setX( 0.0 );
    quotient.setY( 0.0 );
  } else
    {
      quotient.setX( this->getX() / divisor );
      quotient.setY( this->getY() / divisor );
    }

  return(quotient);
}

/*----------------------------------------------------------------------------*/

/**
 * An overloaded operator to assign a 2-vector value.
 * @param AssignedVector The 2-vector to be assigned to this one. 
 * @return true
 */
Column2Vector& Column2Vector::operator=(Column2Vector Vnew)
{
  this->setX( Vnew.getX() );
  this->setY( Vnew.getY() );
  return(*this);
}

/*----------------------------------------------------------------------------*/

/**
 * Print the contents of the 2-vector to the console.
 */
void Column2Vector::PrintSelf()
{
  std::cout << "Vector: [" << this->getX() << ", " << this->getY() << "]" 
            << std::endl;
}


/*= METHODS: Column3Vector ===================================================*/

/**
 * Constructor method.
 * The vector elements are initialized to zero.
 * @return none
 */
Column3Vector::Column3Vector()
{
  this->VectorArray[0] = 0.0;
  this->VectorArray[1] = 0.0;
  this->VectorArray[2] = 0.0;
}

/*----------------------------------------------------------------------------*/

/**
 * Destructor method.
 * Nothing to do here.
 * @return none 
 */
Column3Vector::~Column3Vector()
{
}

/*----------------------------------------------------------------------------*/

/**
 * Get the first element of the 3-vector.
 * @return first element of the 3-vector.
 */
float Column3Vector::getX()
{
  return( this->VectorArray[0] );
}

/*----------------------------------------------------------------------------*/

/**
 * Get the second element of the 3-vector.
 * @return second element of the 3-vector.
 */
float Column3Vector::getY()
{
  return( this->VectorArray[1] );
}

/*----------------------------------------------------------------------------*/

/**
 * Get the third element of the 3-vector.
 * @return third element of the 3-vector.
 */
float Column3Vector::getZ()
{
  return( this->VectorArray[2] );
}

/*----------------------------------------------------------------------------*/

/**
 * Set the vector elements using the X, Y and Z values provided.
 * @param X The first scalar element of the 3-vector.
 * @param Y The second scalar element of the 3-vector.
 * @param Z The third scalar element of the 3-vector.
 */
void  Column3Vector::setvalues(float X, float Y, float Z)
{
  this->VectorArray[0] = X;
  this->VectorArray[1] = Y;
  this->VectorArray[2] = Z;
}

/*----------------------------------------------------------------------------*/

/**
 * Get the specified element of the 3-vector.
 * @param X The new value of the first element of the 3-vector. 
 */
void Column3Vector::setX(float X)
{
  this->VectorArray[0] = X;
}

/*----------------------------------------------------------------------------*/

/**
 * Get the specified element of the 3-vector.
 * @param Y The new value of the second element of the 3-vector. 
 */
void Column3Vector::setY(float Y)
{
  this->VectorArray[1] = Y;
}

/*----------------------------------------------------------------------------*/

/**
 * Get the specified element of the 3-vector.
 * @param Z The new value of the third element of the 3-vector. 
 */
void Column3Vector::setZ(float Z)
{
  this->VectorArray[2] = Z;
}

/*----------------------------------------------------------------------------*/

/**
 * Normalize the 3-vector.
 * @return True if successfully normalized, false if vector cannot be normalized.
 */
bool  Column3Vector::normalize()
{
  float vnorm;

  vnorm = this->norm();

  //  RISK: the vector norm may be close to zero, resulting in a divide-by-zero 
  //  when normalizing. Return false if close to zero.
  if(vnorm<VEPSILON)
  {
    printf("Column3Vector::normalize() - \
                            vector norm close to zero.");
    return(false);
  }

  this->VectorArray[0] /= vnorm;
  this->VectorArray[1] /= vnorm;
  this->VectorArray[2] /= vnorm;
  
  return(true);
}

/*----------------------------------------------------------------------------*/

/**
 * Compute the norm of the 3-vector.
 * @return The norm of the 3-vector.
 */
float Column3Vector::norm()
{
  float vnorm, sqnorm;

  // Inner product of vector with itself.
  sqnorm =  this->VectorArray[0]*this->VectorArray[0] +
      this->VectorArray[1]*this->VectorArray[1] +
      this->VectorArray[2]*this->VectorArray[2];

  //  RISK: the squared norm may be negative if overflow occurs, resulting in 
  //  failure of the square root. Return zero in this case. 
  if(sqnorm<0.0)
  {
    printf( "Column2Vector::norm() - negative sqrt() argument.");
    return(0.0);
  }
  
  // Take the square root for the vector norm.
  vnorm = sqrt((float)sqnorm);

  return(vnorm);
}

/*----------------------------------------------------------------------------*/

/**
 * An overloaded operator for summing two 3-vectors.
 * @param   vector2 3-vector that is to be added to this one.
 * @return  The sum of the two 3-vectors.
 */
Column3Vector Column3Vector::operator+(Column3Vector Vright)
{
  Column3Vector result;

  result.setX( this->getX() + Vright.getX() );
  result.setY( this->getY() + Vright.getY() );
  result.setZ( this->getZ() + Vright.getZ() );

  return(result);
}

/*----------------------------------------------------------------------------*/

/**
 * An overloaded operator for subtracting a 3-vector.
 * @param   vector2 The 3-vector that is to be subtracted from this one.
 * @return  The difference of the two 3-vectors.
 */
Column3Vector Column3Vector::operator-(Column3Vector Vright)
{
  Column3Vector result;

  result.setX( this->getX() - Vright.getX() );
  result.setY( this->getY() - Vright.getY() );
  result.setZ( this->getZ() - Vright.getZ() );

  return(result);
}

/*----------------------------------------------------------------------------*/

/**
 * An overloaded operator for computing the NORMALIZED cross product of two 
 * 3-vectors.
 * @param   vector2 The 3-vector that multiplies this one in the cross product. 
 * @return  The resulting normalized cross product 3-vector.
 */
Column3Vector Column3Vector::operator*(Column3Vector Vright)
{
  Column3Vector result;

  result.setX(  this->getY()*Vright.getZ() - 
                this->getZ()*Vright.getY()   );
  result.setY(  this->getZ()*Vright.getX() - 
                this->getX()*Vright.getZ()   );
  result.setZ(  this->getX()*Vright.getY() - 
                this->getY()*Vright.getX()   );

  // RISK:  Vector normalization may fail if cross product is near zero.
  //        Return a zero vector in this case.
  if(result.normalize()==false)
  {
    printf( "Column3Vector::operator*(Column3Vector) - \
                            vector norm close to zero");
    result.setvalues(0.0, 0.0, 0.0);
  }

  return(result);
}

/*----------------------------------------------------------------------------*/

/**
 * An overloaded operator for multiplying a 3-vector by a scalar value.
 * @param scalefactor A scalar coefficient.
 * @return The resulting scaled 3-vector.
 */
Column3Vector Column3Vector::operator*(float scalefactor)
{
  Column3Vector product;

  product.setX( this->getX() * scalefactor );
  product.setY( this->getY() * scalefactor );
  product.setZ( this->getZ() * scalefactor );

  return(product);
}

/*----------------------------------------------------------------------------*/

/**
 * An overloaded operator for dividing a 3-vector by a scalar value.
 * @param divisor A scalar value to divide the 3-vector.
 * @return The resulting 3-vector quotient.
 */
Column3Vector Column3Vector::operator/(float divisor)
{
  Column3Vector quotient;

  // RISK:  Divide by zero.
  if(divisor<VEPSILON)
  {
    printf( "Column3Vector::operator/(float) - divide by zero.");
    quotient.setX( 0.0 );
    quotient.setY( 0.0 );
    quotient.setZ( 0.0 );
  } else
    {
      quotient.setX( this->getX() / divisor );
      quotient.setY( this->getY() / divisor );
      quotient.setZ( this->getZ() / divisor );
    }

  return(quotient);
}

/*----------------------------------------------------------------------------*/

/**
 * An overloaded operator to assign a 3-vector value.
 * @param AssignedVector The 3-vector to be assigned to this one. 
 * @return true
 */
Column3Vector& Column3Vector::operator=(Column3Vector Vnew)
{
  this->setX( Vnew.getX() );
  this->setY( Vnew.getY() );
  this->setZ( Vnew.getZ() );
  return(*this);
}

/*----------------------------------------------------------------------------*/

/**
 * Print the contents of the 3-vector to the console.
 */
void Column3Vector::PrintSelf()
{
  std::cout << "Vector: [" << this->getX() << ", " << this->getY() << ", " 
            << this->getZ() << "]" << std::endl;
}


/*= CLASS: Quaternion =========================================================*/

/**
 * Constructor Class.
 * @return none.
 */
Quaternion::Quaternion()
{
  // Initialize the quaternion elements to [qx,qy,qz,qw] = [0,0,0,1].
  this->QuaternionArray[0] = 0.0;
  this->QuaternionArray[1] = 0.0;
  this->QuaternionArray[2] = 0.0;
  this->QuaternionArray[3] = 1.0;
}

/*----------------------------------------------------------------------------*/

/**
 * Destructor Class
 * @return 
 */
Quaternion::~Quaternion()
{
  // Nothing to do.
}

/*----------------------------------------------------------------------------*/

/**
 * Get first element of the quaternion.
 * @return The x-coefficient for the quaternion.
 */
float Quaternion::getX()
{
  return(this->QuaternionArray[0]);
}

/*----------------------------------------------------------------------------*/

/**
 * Get second element of the quaternion.
 * @return The y-coefficient for the quaternion.
 */
float Quaternion::getY()
{
  return(this->QuaternionArray[1]);
}

/*----------------------------------------------------------------------------*/

/**
 * Get third element of the quaternion.
 * @return The z-coefficient for the quaternion.
 */
float Quaternion::getZ()
{
  return(this->QuaternionArray[2]);
}

/*----------------------------------------------------------------------------*/

/**
 * Get fourth element of the quaternion.
 * @return The scalar constant for the quaternion.
 */
float Quaternion::getW()
{
  return(this->QuaternionArray[3]);
}

/*----------------------------------------------------------------------------*/

/**
 * Set all the quaternion vector elements.
 * @param X X element.
 * @param Y Y element.
 * @param Z Z element.
 * @param W Real element.
 */
void Quaternion::setvalues(float X, float Y, float Z, float W)
{
  this->setX(X);
  this->setY(Y);
  this->setZ(Z);
  this->setW(W);

  // Check that quaternion is normalized and log.
}

/*----------------------------------------------------------------------------*/

/**
 * Set the X element.
 * @param X Desired value of X element.
 */
void Quaternion::setX(float X)
{
  this->QuaternionArray[0] = X;
}

/*----------------------------------------------------------------------------*/

/**
 * Set the Y element.
 * @param Y Desired value of Y element.
 */
void Quaternion::setY(float Y)
{
  this->QuaternionArray[1] = Y;
}

/*----------------------------------------------------------------------------*/

/**
 * Set the Z element.
 * @param Z Desired value of Z element.
 */
void Quaternion::setZ(float Z)
{
  this->QuaternionArray[2] = Z;
}

/*----------------------------------------------------------------------------*/

/**
 * Set the W element (real).
 * @param W Desired value of W element (real).
 */
void Quaternion::setW(float W)
{
  this->QuaternionArray[3] = W;
}

/*----------------------------------------------------------------------------*/

/**
 * Compute the norm of the 3-vector.
 * @return The norm of the 3-vector.
 */
float Quaternion::norm()
{
  float qnorm, sqnorm;

  // Inner product of vector with itself.
  sqnorm =  this->QuaternionArray[0]*this->QuaternionArray[0] +
            this->QuaternionArray[1]*this->QuaternionArray[1] +
            this->QuaternionArray[2]*this->QuaternionArray[2] +
            this->QuaternionArray[3]*this->QuaternionArray[3];

  //  RISK: the squared norm may be negative if overflow occurs, resulting in 
  //  failure of the square root. Return zero in this case. 
  if(sqnorm<0.0)
  {
    printf( "Quaternion::norm() - negative sqrt() argument.");
    return(0.0);
  }
  
  // Take the square root for the vector norm.
  qnorm = sqrt((float)sqnorm);

  return(qnorm);
}

/*----------------------------------------------------------------------------*/

/**
 * Normalize the quaternion.
 * @return True if successfully normalized, false if quaternion cannot 
 * be normalized.
 */
bool  Quaternion::normalize()
{
  float qnorm;

  qnorm = this->norm();

  //  RISK: the norm may be close to zero, resulting in a divide-by-zero 
  //  when normalizing. Return false if close to zero.
  if(qnorm<VEPSILON)
  {
    printf( "Quaternion::normalize() - quaternion norm \
                            close to zero.");
    return(false);
  }

  this->QuaternionArray[0] /= qnorm;
  this->QuaternionArray[1] /= qnorm;
  this->QuaternionArray[2] /= qnorm;
  this->QuaternionArray[3] /= qnorm;

  return(true);
}

/*----------------------------------------------------------------------------*/

bool  Quaternion::conjugate()
{
  this->setX( -this->getX() );
  this->setY( -this->getY() );
  this->setZ( -this->getZ() );
  this->setW(  this->getW() );

        return true;
}
 
/*----------------------------------------------------------------------------*/

/**
 * Compute quaternion elements from a rotation matrix. NOTE: The matrix columns
 * should be orthonormal. This is not verified prior to computing the quaternion.
 * @param C0 First column vector of the rotation matrix.
 * @param C1 Second column vector of the rotation matrix.
 * @param C2 Third column vector of the rotation matrix.
 * @return true if success, false if the quaternion cannot be computed.
 */
bool Quaternion::ComputeFromRotationMatrix(Column3Vector C0, Column3Vector C1, 
                                           Column3Vector C2)
{
  float tx, ty, tz, tw;

  tw = 1 + C0.getX() + C1.getY() + C2.getZ();
  tx = 1 + C0.getX() - C1.getY() - C2.getZ();
  ty = 1 - C0.getX() + C1.getY() - C2.getZ();
  tz = 1 - C0.getX() - C1.getY() + C2.getZ();

  // tx, ty, tz and tw should be non-negative if the matrix is orthogonal.
  if(tx<0.0) tx=0.0;
  if(ty<0.0) ty=0.0;
  if(tz<0.0) tz=0.0;
  if(tw<0.0) tw=0.0;
 
  // Set quaternion elements (magnitude only)
  this->setX( sqrtf((float)(tx))/2.0f );
  this->setY( sqrtf((float)(ty))/2.0f );
  this->setZ( sqrtf((float)(tz))/2.0f );
  this->setW( sqrtf((float)(tw))/2.0f );

  // Set sign.
  this->setX( this->getX() * (((C1.getZ()-C2.getY())<0)?-1:1) );
  this->setY( this->getY() * (((C2.getX()-C0.getZ())<0)?-1:1) );
  this->setZ( this->getZ() * (((C0.getY()-C1.getX())<0)?-1:1) );
  
  return(true);
}

/*----------------------------------------------------------------------------*/

/**
 * Rotate a vector using a quaternion representation of that rotation.
 * @param P1 The vector to be rotated.
 * @return 
 */
Column3Vector Quaternion::RotateVector(Column3Vector P1)
{
  Quaternion      qP1, qP2, cQ1, qtmp;
  Column3Vector   presult;

  // Put the vector into the quaternion domain.
  qP1.setX( P1.getX() );
  qP1.setY( P1.getY() );
  qP1.setZ( P1.getZ() );
  qP1.setW( 0.0 );

  // Conjugate the quaternion.
  cQ1 = *this;
  cQ1.conjugate();

  // Compute the rotation.
  qP2 = *this * qP1 * cQ1;

  // Output result.
  presult.setX( qP2.getX() );
  presult.setY( qP2.getY() );
  presult.setZ( qP2.getZ() );

  return(presult);
}

/*----------------------------------------------------------------------------*/

/**
 * Multiply two quaternions.
 * @param Qright The righthand multiplier.
 * @return The result.
 */
Quaternion Quaternion::operator*(Quaternion Qright)
{
  Quaternion Qresult;

  // TO DO: Check orthogonality;

  // Multiply Qleft x Qright
  Qresult.setX(  this->getX()*Qright.getW() + this->getY()*Qright.getZ() - 
                  this->getZ()*Qright.getY() + this->getW()*Qright.getX() );
  Qresult.setY( -this->getX()*Qright.getZ() + this->getY()*Qright.getW() + 
                  this->getZ()*Qright.getX() + this->getW()*Qright.getY() );
  Qresult.setZ(  this->getX()*Qright.getY() - this->getY()*Qright.getX() + 
                  this->getZ()*Qright.getW() + this->getW()*Qright.getZ() );
  Qresult.setW( -this->getX()*Qright.getX() - this->getY()*Qright.getY() - 
                  this->getZ()*Qright.getZ() + this->getW()*Qright.getW() );

  return(Qresult);
}

/*----------------------------------------------------------------------------*/

/**
 * Find relative rotation between two frames represented by two quaternions.
 * Rotation multiplied by rotation inverse.
 * @param Qright 
 * @return Result.
 */
Quaternion Quaternion::operator/(Quaternion Qright)
{
  Quaternion cQright, Qresult;

  // TO DO: Check orthogonality
  // Invert Q2 (conjugate)
  cQright = Qright;
  cQright.conjugate();

  Qresult = (*this) * cQright;

  return(Qresult);
}

/*----------------------------------------------------------------------------*/

/**
 * Method for quaternion assignment operator.
 * @param Qnew The new quaternion to be assigned.
 * @return Result.
 */
Quaternion& Quaternion::operator=(Quaternion Qnew)
{
    this->setX( Qnew.getX() );
    this->setY( Qnew.getY() );
    this->setZ( Qnew.getZ() );
    this->setW( Qnew.getW() );
    return(*this);
}

/*----------------------------------------------------------------------------*/

/**
 * Print the contents of the quaternion vector to the console.
 */
void Quaternion::PrintSelf()
{
  std::cout << "Quaternion: [" << this->getX() << ", " << this->getY() << ", " 
            << this->getZ() << ", " << this->getW() << "]" <<std::endl;
}


/*= END ZLinAlg.cxx ==========================================================*/
