#include "vtkITKBSplineTransform.h"

#include <cassert>
#include "vtkObjectFactory.h"

// Helper classes to handle dynamic setting of spline orders
class vtkITKBSplineTransformHelper
{
public:
  typedef itk::Array< double > ParametersType;
  virtual unsigned GetOrder() = 0;
  virtual unsigned int GetNumberOfParameters() = 0;
  virtual void SetParameters( ParametersType const& ) = 0;
  virtual void SetParameters( vtkDoubleArray& param ) = 0;
  virtual void SetParameters( double* ) = 0;
  
  virtual ParametersType const& GetParameters() const = 0;
  virtual void SetGridOrigin( const double origin[3] ) = 0;
  virtual void SetGridSpacing( const double spacing[3] ) = 0;
  virtual void SetGridSize( const unsigned int size[3] ) = 0;
  virtual void ForwardTransformPoint( const double in[3], double out[3] ) = 0;
  virtual void ForwardTransformPoint( const float in[3], float out[3] ) = 0;
  virtual void ForwardTransformDerivative( const double in[3], double out[3],
                                           double derivative[3][3] ) = 0;
  virtual void ForwardTransformDerivative( const float in[3], float out[3],
                                           float derivative[3][3] ) = 0;
  virtual void InverseTransformDerivative( const double in[3], double out[3],
                                           double derivative[3][3] )=0;
  virtual void InverseTransformDerivative( const float in[3], float out[3],
                                           float derivative[3][3] )=0;
  virtual void InverseTransformPoint( const float in[3], float out[3] ) = 0;
  virtual void InverseTransformPoint( const double in[3], double out[3] ) = 0;
};


template<unsigned O>
class vtkITKBSplineTransformHelperImpl : public vtkITKBSplineTransformHelper
{
public:
  typedef itk::BSplineDeformableTransform< double, 3, O > BSplineType;

  vtkITKBSplineTransformHelperImpl():BSpline( BSplineType::New() ) {}
  virtual unsigned GetOrder() { return O; }
  virtual unsigned int GetNumberOfParameters() 
  { return BSpline->GetNumberOfParameters(); }
  virtual ParametersType const& GetParameters() const
  { return BSpline->GetParameters(); }
  virtual void SetParameters( ParametersType const& );
  virtual void SetParameters( vtkDoubleArray& param );
  virtual void SetParameters( double* );
  virtual void SetGridOrigin( const double origin[3] );
  virtual void SetGridSpacing( const double spacing[3] );
  virtual void SetGridSize( const unsigned int size[3] );
  virtual void ForwardTransformPoint( const double in[3], double out[3] );
  virtual void ForwardTransformPoint( const float in[3], float out[3] );
  virtual void ForwardTransformDerivative( const double in[3], double out[3],
                                           double derivative[3][3] );
  virtual void ForwardTransformDerivative( const float in[3], float out[3],
                                           float derivative[3][3] );
  virtual void InverseTransformDerivative( const double in[3], double out[3],
                                           double derivative[3][3] );
  virtual void InverseTransformDerivative( const float in[3], float out[3],
                                           float derivative[3][3] );
  virtual void InverseTransformPoint( const float in[3], float out[3] );
  virtual void InverseTransformPoint( const double in[3], double out[3] );
private:
  typename BSplineType::Pointer BSpline;
};


//---------------------------------------------------------------------------
// Implementation of main class.
// This mostly just forwards everything to the helper class.


vtkCxxRevisionMacro(vtkITKBSplineTransform, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkITKBSplineTransform);

void
vtkITKBSplineTransform
::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Spline order: " << Helper->GetOrder() << "\n";
  os << indent << "Num parameters: " << Helper->GetNumberOfParameters() << "\n";
}


vtkAbstractTransform*
vtkITKBSplineTransform
::MakeTransform()
{
  vtkITKBSplineTransform* N = new vtkITKBSplineTransform;
  if( Helper ) 
  {
    N->SetSplineOrder( Helper->GetOrder() );
    N->Helper->SetParameters( Helper->GetParameters() );
  }
  return N;
}

vtkITKBSplineTransform
::vtkITKBSplineTransform() :
  Helper( 0 )
{
}

vtkITKBSplineTransform
::~vtkITKBSplineTransform()
{
  delete Helper;
}


void
vtkITKBSplineTransform
::SetSplineOrder( unsigned int order )
{
  if( Helper && Helper->GetOrder() == order )
  {
    return;
  }

  delete Helper;
  switch( order ) 
  {
  case 2:
    Helper = new vtkITKBSplineTransformHelperImpl< 2 >;
    break;
  case 3:
    Helper = new vtkITKBSplineTransformHelperImpl< 3 >;
    break;
  default:
    abort();
    break;
  }
}

void 
vtkITKBSplineTransform
::SetGridOrigin( const double origin[3] ) 
{
  // Need to have called SetSplineOrder before calling this.
  assert( Helper != 0 );
  Helper->SetGridOrigin( origin );
}

void 
vtkITKBSplineTransform
::SetGridSpacing( const double spacing[3] ) 
{
  // Need to have called SetSplineOrder before calling this.
  assert( Helper != 0 );
  Helper->SetGridSpacing( spacing );
}

void
vtkITKBSplineTransform
::SetGridSize( const unsigned int size[3] ) 
{
  // Need to have called SetSplineOrder before calling this.
  assert( Helper != 0 );
  Helper->SetGridSize( size );
}

void
vtkITKBSplineTransform
::SetParameters( vtkDoubleArray& param ) 
{
  // Need to have called SetSplineOrder before calling this.
  assert( Helper != 0 );
  Helper->SetParameters( param );
}

void
vtkITKBSplineTransform
::SetParameters( double* param ) 
{
  // Need to have called SetSplineOrder before calling this.
  assert( Helper != 0 );
  Helper->SetParameters( param );
}

unsigned int 
vtkITKBSplineTransform
::GetNumberOfParameters() const 
{
  // Need to have called SetSplineOrder before calling this.
  assert( Helper != 0 );
  return Helper->GetNumberOfParameters();
}

void
vtkITKBSplineTransform
::ForwardTransformPoint( const float in[3], float out[3] )
{
  // Need to have called SetSplineOrder before calling this.
  assert( Helper != 0 );
  Helper->ForwardTransformPoint( in, out );
}

void 
vtkITKBSplineTransform
::ForwardTransformPoint( const double in[3], double out[3] )
{
  // Need to have called SetSplineOrder before calling this.
  assert( Helper != 0 );
  Helper->ForwardTransformPoint( in, out );
}

void
vtkITKBSplineTransform
::ForwardTransformDerivative( const float in[3], float out[3],
                            float derivative[3][3] )
{
  // Need to have called SetSplineOrder before calling this.
  assert( Helper != 0 );
  Helper->ForwardTransformDerivative( in, out, derivative );
}
void
vtkITKBSplineTransform
::ForwardTransformDerivative( const double in[3], double out[3],
                            double derivative[3][3] )
{
  // Need to have called SetSplineOrder before calling this.
  assert( Helper != 0 );
  Helper->ForwardTransformDerivative( in, out, derivative );
}

void
vtkITKBSplineTransform
::InverseTransformPoint( const float in[3], float out[3] )
{
  // Need to have called SetSplineOrder before calling this.
  assert( Helper != 0 );
  Helper->InverseTransformPoint( in, out );
}

void
vtkITKBSplineTransform
::InverseTransformPoint( const double in[3], double out[3] )
{
  // Need to have called SetSplineOrder before calling this.
  assert( Helper != 0 );
  Helper->InverseTransformPoint( in, out );
}

void
vtkITKBSplineTransform
::InverseTransformDerivative( const float in[3], float out[3],
                            float derivative[3][3] )
{
  // Need to have called SetSplineOrder before calling this.
  assert( Helper != 0 );
  Helper->InverseTransformDerivative( in, out, derivative );
}

void
vtkITKBSplineTransform
::InverseTransformDerivative( const double in[3], double out[3],
                            double derivative[3][3] )
{
  // Need to have called SetSplineOrder before calling this.
  assert( Helper != 0 );
  Helper->InverseTransformDerivative( in, out, derivative );
}


// ---------------------------------------------------------------------------
// implement the actual wrapper around the itkBSplineDeformableTransform


template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::SetGridOrigin( const double vOrigin[3] )
{
  typename BSplineType::OriginType origin;
  for( unsigned int i=0; i<3; ++i )
    origin[i] = vOrigin[i];
  BSpline->SetGridOrigin( origin );
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::SetGridSpacing( const double vSpacing[3] )
{
  typename BSplineType::SpacingType spacing;
  for( unsigned int i=0; i<3; ++i )
    spacing[i] = vSpacing[i];
  BSpline->SetGridSpacing( spacing );
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::SetGridSize( const unsigned int vSize[3] )
{
  typename BSplineType::RegionType region;
  typename BSplineType::RegionType::IndexType index;
  typename BSplineType::RegionType::SizeType size;
  for( unsigned int i=0; i<3; ++i ) {
    index[i] = 0;
    size[i] = vSize[i];
  }
  region.SetSize( size );
  region.SetIndex( index );
  BSpline->SetGridRegion( region );
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::SetParameters( ParametersType const& param )
{
  BSpline->SetParameters( param );
}

template< unsigned O >
void 
vtkITKBSplineTransformHelperImpl<O>
::SetParameters( vtkDoubleArray& param )
{
  unsigned numberOfParam = BSpline->GetNumberOfParameters();
  typename BSplineType::ParametersType parameters( numberOfParam );
                                         
  for( unsigned int i=0; i<numberOfParam; ++i )
    parameters.SetElement( i, param.GetTuple1(i) );

  BSpline->SetParameters( parameters );
}

template< unsigned O >
void 
vtkITKBSplineTransformHelperImpl<O>
::SetParameters( double* param )
{
  unsigned numberOfParam = BSpline->GetNumberOfParameters();
  typename BSplineType::ParametersType parameters( numberOfParam );
                                         
  for( unsigned int i=0; i<numberOfParam; ++i )
    parameters.SetElement( i, param[i] );

  BSpline->SetParameters( parameters );
}

template <class T, unsigned O>
void
ForwardTransformHelper( typename itk::BSplineDeformableTransform<double, 3, O>::Pointer BSpline, 
                        const T in[3], T out[3] )
{
  typedef itk::BSplineDeformableTransform<double, 3, O> BSplineType;
  typename BSplineType::InputPointType inputPoint;

  inputPoint[0] = in[0];
  inputPoint[1] = in[1]; 
  inputPoint[2] = in[2];

  typename BSplineType::OutputPointType outputPoint;
  outputPoint = BSpline->TransformPoint( inputPoint );

  out[0] = static_cast<T>(outputPoint[0]); 
  out[1] = static_cast<T>(outputPoint[1]); 
  out[2] = static_cast<T>(outputPoint[2]);
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::ForwardTransformPoint( const double in[3], double out[3] )
{
  ForwardTransformHelper<double, O>( BSpline, in, out );
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::ForwardTransformPoint( const float in[3], float out[3] )
{
  ForwardTransformHelper<float, O>( BSpline, in, out );
}

template <class T, unsigned O>
void
ForwardTransformDerivativeHelper( typename itk::BSplineDeformableTransform<double, 3, O>::Pointer BSpline, 
                                  const T in[3], T out[3], 
                                  T derivative[3][3] )
{
  typedef itk::BSplineDeformableTransform<double, 3, O> BSplineType;
  typename BSplineType::InputPointType inputPoint;

  inputPoint[0] = in[0];
  inputPoint[1] = in[1]; 
  inputPoint[2] = in[2];

  typename BSplineType::OutputPointType outputPoint;
  outputPoint = BSpline->TransformPoint( inputPoint );

  out[0] = static_cast<T>( outputPoint[0] ); 
  out[1] = static_cast<T>( outputPoint[1] ); 
  out[2] = static_cast<T>( outputPoint[2] );

  typename BSplineType::JacobianType jacobian = BSpline->GetJacobian( inputPoint );
  for( unsigned i=0; i<3; ++i )
  {
    derivative[i][0] = static_cast<T>( jacobian( i, 0 ) );
    derivative[i][1] = static_cast<T>( jacobian( i, 1 ) );
    derivative[i][2] = static_cast<T>( jacobian( i, 2 ) );
  }
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::ForwardTransformDerivative( const double in[3], double out[3],
                            double derivative[3][3] )
{
  ForwardTransformDerivativeHelper<double, O>( BSpline, in, out, derivative );
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::ForwardTransformDerivative( const float in[3], float out[3],
                            float derivative[3][3] )
{
  ForwardTransformDerivativeHelper<float, O>( BSpline, in, out, derivative );
}


template <class T, unsigned O>
void
InverseTransformPointHelper( typename itk::BSplineDeformableTransform<double, 3, O>::Pointer BSpline, 
                             const T in[3], T out[3] )
{
  typedef itk::BSplineDeformableTransform<double, 3, O> BSplineType;
  typedef BSplineType::OutputPointType OutputPointType;
  typedef BSplineType::InputPointType InputPointType;

  //iterative inverse bSpline transform
  int const MaxIterationNumber = 10;
  double const Tolerance = 1;
  OutputPointType opt;
  InputPointType ipt;
  opt[0] = in[0];
  opt[1] = in[1];
  opt[2] = in[2];
  ipt[0] = opt[0];
  ipt[1] = opt[1];
  ipt[2] = opt[2];
  for (int k = 0; k <= MaxIterationNumber; k++)
  {
    OutputPointType optTrail = BSpline->TransformPoint( ipt );
    ipt[0] = ipt[0] + (opt[0]-optTrail[0]);
    ipt[1] = ipt[1] + (opt[1]-optTrail[1]);
    ipt[2] = ipt[2] + (opt[2]-optTrail[2]);
    double dist = fabs(opt[0]-optTrail[0])+fabs(opt[1]-optTrail[1])+fabs(opt[2]-optTrail[2]);
    if (dist < Tolerance )
    {
      break;
    }
  }

  out[0] = static_cast<T>(ipt[0]); 
  out[1] = static_cast<T>(ipt[1]); 
  out[2] = static_cast<T>(ipt[2]);
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::InverseTransformPoint( const float in[3], float out[3] )
{
  InverseTransformPointHelper<float, O>( BSpline, in, out );
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::InverseTransformPoint( const double in[3], double out[3] )
{
  InverseTransformPointHelper<double, O>( BSpline, in, out );
}


template <class T, unsigned O>
void
InverseTransformDerivativeHelper( typename itk::BSplineDeformableTransform<double, 3, O>::Pointer BSpline, 
                                  const T in[3], T out[3], T derivative[3][3] )
{
  typedef itk::BSplineDeformableTransform<double, 3, O> BSplineType;
  typedef BSplineType::InputPointType InputPointType;
  typedef BSplineType::JacobianType JacobianType;

  InverseTransformPointHelper<T,O>( BSpline, in, out );

  InputPointType pt;

  pt[0] = out[0];
  pt[1] = out[1]; 
  pt[2] = out[2];

  JacobianType const& jacobian = BSpline->GetJacobian( pt );
  for( unsigned i=0; i<3; ++i )
  {
    derivative[i][0] = static_cast<T>( jacobian( i, 0 ) );
    derivative[i][1] = static_cast<T>( jacobian( i, 1 ) );
    derivative[i][2] = static_cast<T>( jacobian( i, 2 ) );
  }  
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::InverseTransformDerivative( const float in[3], float out[3],
                            float derivative[3][3] )
{
  InverseTransformDerivativeHelper<float, O>( BSpline, in, out, derivative );
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::InverseTransformDerivative( const double in[3], double out[3],
                            double derivative[3][3] )
{
  InverseTransformDerivativeHelper<double, O>( BSpline, in, out, derivative );
}
