/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkSimilarity3DTransformTest.cxx,v $
  Language:  C++
  Date:      $Date: 2008/01/17 15:26:39 $
  Version:   $Revision: 1.6 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

/**
 *
 *  This program illustrates the use of Similarity3DTransform
 *
 *  This transform performs: translation, rotation and uniform scaling.
 *
 */

#include "itkAnisotropicSimilarity3DTransform.h"
#include <iostream>

// -------------------------
//
//   Main code
//
// -------------------------
int itkAnisotropicSimilarity3DTransformTest(int, char * [] )
{

  typedef   double ValueType;

  const ValueType epsilon = 1e-12;

  //  Versor Transform type
  typedef    itk::AnisotropicSimilarity3DTransform<ValueType> TransformType;

  //  Versor type
  typedef    TransformType::VersorType VersorType;

  //  Vector type
  typedef    TransformType::InputVectorType VectorType;

  //  Point type
  typedef    TransformType::InputPointType PointType;

  //  Covariant Vector type
  typedef    TransformType::InputCovariantVectorType CovariantVectorType;

  //  VnlVector type
  typedef    TransformType::InputVnlVectorType VnlVectorType;

  //  Parameters type
  typedef    TransformType::ParametersType ParametersType;

  //  Jacobian type
  typedef    TransformType::JacobianType JacobianType;

  //  Rotation Matrix type
  typedef    TransformType::MatrixType MatrixType;

    {
    std::cout << "Test default constructor... ";

    TransformType::Pointer transform = TransformType::New();

    VectorType axis(1.5);

    ValueType angle = 120.0 * atan(1.0) / 45.0;

    VersorType versor;
    versor.Set( axis, angle );

    ParametersType parameters( transform->GetNumberOfParameters() ); // Number of parameters

    parameters[0] = versor.GetX();
    parameters[1] = versor.GetY();
    parameters[2] = versor.GetZ();
    parameters[3] = 0.0;             // Translation
    parameters[4] = 0.0;
    parameters[5] = 0.0;
    parameters[6] = 1.0;
    parameters[7] = 1.0;
    parameters[8] = 1.0;

    transform->SetParameters( parameters );

    if( 0.0 > epsilon )
      {
      std::cout << "Error ! " << std::endl;
      return EXIT_FAILURE;
      }
    std::cout << " PASSED !" << std::endl;

    }

    {
    std::cout << "Test initial rotation matrix " << std::endl;
    TransformType::Pointer transform = TransformType::New();
    MatrixType             matrix = transform->GetRotationMatrix();
    std::cout << "Matrix = " << std::endl;
    std::cout <<    matrix   << std::endl;
    }

  /* Create a Rigid 3D transform with rotation */

    {
    bool Ok = true;

    TransformType::Pointer rotation = TransformType::New();

    itk::Vector<double, 3> axis(1);

    const double angle = (atan(1.0) / 45.0) * 120.0; // turn 120 degrees

    // this rotation will permute the axis x->y, y->z, z->x
    rotation->SetRotation( axis, angle );

    TransformType::OffsetType offset = rotation->GetOffset();
    std::cout << "pure Rotation test:  ";
    std::cout << offset << std::endl;
    for( unsigned int i = 0; i < 3; i++ )
      {
      if( fabs( offset[i] - 0.0 ) > epsilon )
        {
        Ok = false;
        break;
        }
      }

    if( !Ok )
      {
      std::cerr << "Get Offset  differs from null in rotation " << std::endl;
      return EXIT_FAILURE;
      }

    VersorType versor;
    versor.Set( axis, angle );

      {
      // Rotate an itk::Point
      TransformType::InputPointType::ValueType pInit[3] = {1, 4, 9};
      TransformType::InputPointType            p = pInit;
      TransformType::OutputPointType           q;
      q = versor.Transform( p );

      TransformType::OutputPointType r;
      r = rotation->TransformPoint( p );
      for( unsigned int i = 0; i < 3; i++ )
        {
        if( fabs( q[i] - r[i] ) > epsilon )
          {
          Ok = false;
          break;
          }
        }
      if( !Ok )
        {
        std::cout << "Error rotating point : " << p << std::endl;
        std::cout << "Result should be     : " << q << std::endl;
        std::cout << "Reported Result is   : " << r << std::endl;
        return EXIT_FAILURE;
        }
      else
        {
        std::cout << "Ok rotating an itk::Point " << std::endl;
        }
      }

      {
      // Translate an itk::Vector
      TransformType::InputVectorType::ValueType pInit[3] = {1, 4, 9};
      TransformType::InputVectorType            p = pInit;
      TransformType::OutputVectorType           q;
      q = versor.Transform( p );

      TransformType::OutputVectorType r;
      r = rotation->TransformVector( p );
      for( unsigned int i = 0; i < 3; i++ )
        {
        if( fabs( q[i] - r[i] ) > epsilon )
          {
          Ok = false;
          break;
          }
        }
      if( !Ok )
        {
        std::cout << "Error rotating vector : " << p << std::endl;
        std::cout << "Result should be      : " << q << std::endl;
        std::cout << "Reported Result is    : " << r << std::endl;
        return EXIT_FAILURE;
        }
      else
        {
        std::cout << "Ok rotating an itk::Vector " << std::endl;
        }
      }

      {
      // Translate an itk::CovariantVector
      TransformType::InputCovariantVectorType::ValueType pInit[3] = {1, 4, 9};
      TransformType::InputCovariantVectorType            p = pInit;
      TransformType::OutputCovariantVectorType           q;
      q = versor.Transform( p );

      TransformType::OutputCovariantVectorType r;
      r = rotation->TransformCovariantVector( p );
      for( unsigned int i = 0; i < 3; i++ )
        {
        if( fabs( q[i] - r[i] ) > epsilon )
          {
          Ok = false;
          break;
          }
        }
      if( !Ok )
        {
        std::cout << "Error rotating covariant vector : " << p << std::endl;
        std::cout << "Result should be                : " << q << std::endl;
        std::cout << "Reported Result is              : " << r << std::endl;
        return EXIT_FAILURE;
        }
      else
        {
        std::cout << "Ok rotating an itk::CovariantVector " << std::endl;
        }
      }

      {
      // Translate a vnl_vector
      TransformType::InputVnlVectorType p;
      p[0] = 1;
      p[1] = 4;
      p[2] = 9;

      TransformType::OutputVnlVectorType q;
      q = versor.Transform( p );

      TransformType::OutputVnlVectorType r;
      r = rotation->TransformVector( p );
      for( unsigned int i = 0; i < 3; i++ )
        {
        if( fabs( q[i] - r[i] ) > epsilon )
          {
          Ok = false;
          break;
          }
        }
      if( !Ok )
        {
        std::cout << "Error rotating vnl_vector : " << p << std::endl;
        std::cout << "Result should be          : " << q << std::endl;
        std::cout << "Reported Result is        : " << r << std::endl;
        return EXIT_FAILURE;
        }
      else
        {
        std::cout << "Ok rotating an vnl_Vector " << std::endl;
        }
      }

    }

  /**  Exercise the SetCenter method  */
    {
    bool Ok = true;

    TransformType::Pointer transform = TransformType::New();

    itk::Vector<double, 3> axis(1);

    const double angle = (atan(1.0) / 45.0) * 30.0; // turn 30 degrees

    transform->SetRotation( axis, angle );

    TransformType::InputPointType center;
    center[0] = 31;
    center[1] = 62;
    center[2] = 93;

    transform->SetCenter( center );

    TransformType::OutputPointType transformedPoint;
    transformedPoint = transform->TransformPoint( center );
    for( unsigned int i = 0; i < 3; i++ )
      {
      if( fabs( center[i] - transformedPoint[i] ) > epsilon )
        {
        Ok = false;
        break;
        }
      }

    if( !Ok )
      {
      std::cout << "The center point was not invariant to rotation " << std::endl;
      return EXIT_FAILURE;
      }
    else
      {
      std::cout << "Ok center is invariant to rotation." << std::endl;
      }

    const unsigned int np = transform->GetNumberOfParameters();

    ParametersType parameters( np ); // Number of parameters

    VersorType versor;

    parameters[0] = versor.GetX();   // Rotation axis * sin(t/2)
    parameters[1] = versor.GetY();
    parameters[2] = versor.GetZ();
    parameters[3] = 8.0;             // Translation
    parameters[4] = 7.0;
    parameters[5] = 6.0;
    parameters[6] = 1.0;             // Scale
    parameters[7] = 1.0;             // Scale
    parameters[8] = 1.0;             // Scale

    transform->SetParameters( parameters );

    ParametersType parameters2 = transform->GetParameters();

    const double tolerance = 1e-8;
    for( unsigned int p = 0; p < np; p++ )
      {
      if( fabs( parameters[p] - parameters2[p] ) > tolerance )
        {
        std::cout << "Output parameter does not match input " << std::endl;
        return EXIT_FAILURE;
        }
      }
    std::cout << "Input/Output parameter check Passed !"  << std::endl;

    // Try the GetJacobian method
    TransformType::InputPointType aPoint;
    aPoint[0] = 10.0;
    aPoint[1] = 20.0;
    aPoint[2] = -10.0;
    // XXX This should be fixed to use ComputeJacobianWithRespectToParameters
    JacobianType jacobian = transform->GetJacobian( aPoint );
    std::cout << "Jacobian: "  << std::endl;
    std::cout << jacobian << std::endl;

    // copy the read one just for getting the right matrix size
    JacobianType TheoreticalJacobian = jacobian;

    TheoreticalJacobian[0][0] =    0.0;
    TheoreticalJacobian[1][0] =  206.0;
    TheoreticalJacobian[2][0] =  -84.0;

    TheoreticalJacobian[0][1] = -206.0;
    TheoreticalJacobian[1][1] =    0.0;
    TheoreticalJacobian[2][1] =   42.0;

    TheoreticalJacobian[0][2] =   84.0;
    TheoreticalJacobian[1][2] =  -42.0;
    TheoreticalJacobian[2][2] =    0.0;

    TheoreticalJacobian[0][3] = 1.0;
    TheoreticalJacobian[1][3] = 0.0;
    TheoreticalJacobian[2][3] = 0.0;

    TheoreticalJacobian[0][4] = 0.0;
    TheoreticalJacobian[1][4] = 1.0;
    TheoreticalJacobian[2][4] = 0.0;

    TheoreticalJacobian[0][5] = 0.0;
    TheoreticalJacobian[1][5] = 0.0;
    TheoreticalJacobian[2][5] = 1.0;

    TheoreticalJacobian[0][6] =  -21.0;
    TheoreticalJacobian[1][6] =  -42.0;
    TheoreticalJacobian[2][6] = -103.0;
    for( unsigned int ii = 0; ii < 3; ii++ )
      {
      for( unsigned int jj = 0; jj < 7; jj++ )
        {
        if( vnl_math::abs( TheoreticalJacobian[ii][jj] - jacobian[ii][jj] ) > 1e-5 )
          {
          std::cout << "Jacobian components differ from expected values ";
          std::cout << std::endl << std::endl;
          std::cout << "Expected Jacobian = " << std::endl;
          std::cout << TheoreticalJacobian << std::endl << std::endl;
          std::cout << "Computed Jacobian = " << std::endl;
          std::cout << jacobian << std::endl << std::endl;
          std::cout << std::endl << "Test FAILED ! " << std::endl;
          return EXIT_FAILURE;
          }
        }
      }
    }

    {
    std::cout << " Exercise the SetIdentity() method " << std::endl;
    TransformType::Pointer transform = TransformType::New();

    itk::Vector<double, 3> axis(1);

    const double angle = (atan(1.0) / 45.0) * 30.0; // turn 30 degrees

    transform->SetRotation( axis, angle );

    TransformType::InputPointType center;
    center[0] = 31;
    center[1] = 62;
    center[2] = 93;

    transform->SetCenter( center );

    transform->SetIdentity();

    const unsigned int np = transform->GetNumberOfParameters();

    ParametersType parameters( np ); // Number of parameters

    VersorType versor;

    parameters[0] = versor.GetX();   // Rotation axis * sin(t/2)
    parameters[1] = versor.GetY();
    parameters[2] = versor.GetZ();
    parameters[3] = 0.0;             // Translation
    parameters[4] = 0.0;
    parameters[5] = 0.0;
    parameters[6] = 1.0;
    parameters[7] = 1.0;
    parameters[8] = 1.0;

    ParametersType parameters2 = transform->GetParameters();

    const double tolerance = 1e-8;
    for( unsigned int p = 0; p < np; p++ )
      {
      if( fabs( parameters[p] - parameters2[p] ) > tolerance )
        {
        std::cout << "Output parameter does not match input " << std::endl;
        return EXIT_FAILURE;
        }
      }
    std::cout << "Input/Output parameter check Passed !"  << std::endl;
    }

    {
    std::cout << " Exercise the Scaling methods " << std::endl;
    TransformType::Pointer transform = TransformType::New();

    itk::Vector<double, 3> axis(1);

    const double angle = (atan(1.0) / 45.0) * 30.0; // turn 30 degrees

    transform->SetRotation( axis, angle );

    TransformType::InputPointType center;
    center[0] = 31;
    center[1] = 62;
    center[2] = 93;

    transform->SetCenter( center );

    TransformType::OutputVectorType translation;
    translation[0] = 17;
    translation[1] = 19;
    translation[2] = 23;

    transform->SetTranslation( translation );

    const double scale = 2.5;

    transform->SetScale( scale );

    const double rscale = transform->GetScale()[0];

    const double tolerance = 1e-8;

    if( fabs( rscale - scale ) > tolerance )
      {
      std::cout << "Error in Set/Get Scale() " << std::endl;
      return EXIT_FAILURE;
      }

    const unsigned int np = transform->GetNumberOfParameters();

    ParametersType parameters( np ); // Number of parameters

    VersorType versor;
    versor.Set( axis, angle );

    parameters[0] = versor.GetX();   // Rotation axis * sin(t/2)
    parameters[1] = versor.GetY();
    parameters[2] = versor.GetZ();
    parameters[3] = translation[0];
    parameters[4] = translation[1];
    parameters[5] = translation[2];
    parameters[6] = scale;
    parameters[7] = scale;
    parameters[8] = scale;

    ParametersType parameters2 = transform->GetParameters();
    for( unsigned int p = 0; p < np; p++ )
      {
      if( fabs( parameters[p] - parameters2[p] ) > tolerance )
        {
        std::cout << "Output parameter does not match input " << std::endl;
        return EXIT_FAILURE;
        }
      }
    std::cout << "Input/Output parameter check Passed !"  << std::endl;

    }

    {
    // Testing SetMatrix()
    std::cout << "Testing SetMatrix() ... ";
    bool         Ok;
    unsigned int par;

    typedef TransformType::MatrixType MatrixType;
    MatrixType matrix;

    TransformType::Pointer t = TransformType::New();

    // attempt to set an non-orthogonal matrix
    par = 0;
    for( unsigned int row = 0; row < 3; row++ )
      {
      for( unsigned int col = 0; col < 3; col++ )
        {
        matrix[row][col] = static_cast<double>( par + 1 );
        ++par;
        }
      }

    Ok = false;

    std::cout << "Setting non-orthogonal matrix = " << std::endl;
    std::cout << matrix << std::endl;

    try
      {
      t->SetMatrix( matrix );
      }
    catch( itk::ExceptionObject & err )
      {
      std::cout << "Caught expected exception" << err << std::endl;
      Ok = true;
      }
    catch( ... )
      {
      std::cout << "Caught unknown exception" << std::endl;
      }

    if( !Ok )
      {
      std::cout << "Error: expected to catch an exception when attempting";
      std::cout << " to set an non-orthogonal matrix." << std::endl;
      return EXIT_FAILURE;
      }

    t = TransformType::New();

    // attempt to set an (orthogonal + scale) matrix
    matrix.GetVnlMatrix().set_identity();

    double a = 1.0 / 180.0 * vnl_math::pi;
    double s = 0.5;
    matrix[0][0] =        cos( a ) * s;
    matrix[0][1] = -1.0 * sin( a ) * s;
    matrix[1][0] =        sin( a ) * s;
    matrix[1][1] =        cos( a ) * s;
    matrix[2][2] =                   s;

    Ok = true;
    try
      {
      t->SetMatrix( matrix );
      }
    catch( itk::ExceptionObject & err )
      {
      std::cout << err << std::endl;
      Ok = false;
      }
    catch( ... )
      {
      std::cout << "Caught unknown exception" << std::endl;
      Ok = false;
      }

    if( !Ok )
      {
      std::cout << "Error: caught unexpected exception" << std::endl;
      return EXIT_FAILURE;
      }

    // Check the computed parameters
    TransformType::InputPointType center;
    center[0] = 15.0;
    center[1] = 16.0;
    center[2] = 17.0;

    typedef TransformType::ParametersType ParametersType;
    ParametersType e( t->GetNumberOfParameters() );
    e.Fill( 0.0 );
    e[2] = sin(0.5 * a);
    e[6] = 0.5;
    e[7] = 0.5;
    e[8] = 0.5;

    t = TransformType::New();
    t->SetCenter( center );
    t->SetParameters( e );

    TransformType::Pointer t2 = TransformType::New();
    t2->SetCenter( center );
    t2->SetMatrix( t->GetMatrix() );

    ParametersType p = t2->GetParameters();
    for( unsigned int k = 0; k < e.GetSize(); k++ )
      {
      if( fabs( e[k] - p[k] ) > epsilon )
        {
        std::cout << " [ FAILED ] " << std::endl;
        std::cout << "Expected parameters: " << e << std::endl;
        std::cout << "but got: " << p << std::endl;
        return EXIT_FAILURE;
        }
      }

    std::cout << "[ PASSED ]" << std::endl;

    }

  std::cout << std::endl << "Test PASSED ! " << std::endl;

  return EXIT_SUCCESS;

}
