#include "vtkITKBSplineTransform.h"

#include "vtkObjectFactory.h"

// Helper classes to handle dynamic setting of spline orders
class vtkITKBSplineTransformHelper
{
public:
  typedef itk::Array< double > ParametersType;
  virtual unsigned GetOrder() const = 0;
  virtual unsigned int GetNumberOfParameters() const = 0;
  virtual void SetParameters( ParametersType const& ) = 0;
  virtual void SetParameters( vtkDoubleArray& param ) = 0;
  virtual void SetParameters( double const* ) = 0;
  
  virtual ParametersType const& GetParameters() const = 0;

  virtual void SetFixedParameters( double const*, unsigned N ) = 0;
  virtual const double* GetFixedParameters( unsigned& N ) const = 0;

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

  virtual void SetSwitchCoordinateSystem( bool v ) = 0;
  virtual bool GetSwitchCoordinateSystem() const = 0;
};


template<unsigned O>
class vtkITKBSplineTransformHelperImpl : public vtkITKBSplineTransformHelper
{
public:
  typedef itk::BSplineDeformableTransform< double, 3, O > BSplineType;

  vtkITKBSplineTransformHelperImpl();

  virtual unsigned GetOrder() const
    {
    return O;
    }

  virtual unsigned int GetNumberOfParameters() const
    {
    return BSpline->GetNumberOfParameters();
    }

  virtual ParametersType const& GetParameters() const
    {
    return BSpline->GetParameters();
    }

  virtual void SetParameters( ParametersType const& );
  virtual void SetParameters( vtkDoubleArray& param );
  virtual void SetParameters( double const* );
  virtual void SetFixedParameters( double const*, unsigned N );
  virtual const double* GetFixedParameters( unsigned& N ) const;
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

  virtual void SetSwitchCoordinateSystem( bool v )
    {
    switchCoordSystems = v;
    }

  virtual bool GetSwitchCoordinateSystem() const
    {
    return switchCoordSystems;
    }

  // the data is also public to allow the helper templates to access
  // them.
  typename BSplineType::Pointer BSpline;
  typename BSplineType::ParametersType parameters;
  bool switchCoordSystems;
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

  if( Helper != NULL )
    {
    os << indent << "Spline order: " << Helper->GetOrder() << "\n";
    os << indent << "Num parameters: " << Helper->GetNumberOfParameters() << "\n";
    }
  else
    {
    os << indent << "(no spline)\n";
    }
}


vtkAbstractTransform*
vtkITKBSplineTransform
::MakeTransform()
{
  vtkITKBSplineTransform* N = new vtkITKBSplineTransform;
  if( Helper ) 
  {
    N->SetSplineOrder( this->GetSplineOrder() );
    N->SetFixedParameters( this->GetFixedParameters(),
                           this->GetNumberOfFixedParameters() );
    N->SetParameters( this->GetParameters() );
    N->SetSwitchCoordinateSystem( this->GetSwitchCoordinateSystem() );
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
    vtkErrorMacro( "order " << order << " not yet implemented" );
    break;
  }
}


unsigned int
vtkITKBSplineTransform
::GetSplineOrder() const
{
  if( Helper )
  {
    return Helper->GetOrder();
  }
  else
  {
    return 0;
  }
}


void 
vtkITKBSplineTransform
::SetGridOrigin( const double origin[3] ) 
{
  // Need to have called SetSplineOrder before calling this.
  if( Helper != NULL )
    {
    Helper->SetGridOrigin( origin );
    }
  else
    {
    vtkErrorMacro( "need to call SetSplineOrder before SetGridOrigin" );
    }
}

void 
vtkITKBSplineTransform
::SetGridSpacing( const double spacing[3] ) 
{
  // Need to have called SetSplineOrder before calling this.
  if( Helper != NULL )
    {
    Helper->SetGridSpacing( spacing );
    }
  else
    {
    vtkErrorMacro( "need to call SetSplineOrder before SetGridSpacing" );
    }
}

void
vtkITKBSplineTransform
::SetGridSize( const unsigned int size[3] ) 
{
  // Need to have called SetSplineOrder before calling this.
  if( Helper != NULL )
    {
    Helper->SetGridSize( size );
    }
  else
    {
    vtkErrorMacro( "need to call SetSplineOrder before SetGridSize" );
    }
}

void
vtkITKBSplineTransform
::SetParameters( vtkDoubleArray& param ) 
{
  // Need to have called SetSplineOrder before calling this.
  if( Helper != NULL )
    {
    Helper->SetParameters( param );
    }
  else
    {
    vtkErrorMacro( "need to call SetSplineOrder before SetParameters" );
    }
}

void
vtkITKBSplineTransform
::SetParameters( double const* param ) 
{
  // Need to have called SetSplineOrder before calling this.
  if( Helper != NULL )
    {
    Helper->SetParameters( param );
    }
  else
    {
    vtkErrorMacro( "need to call SetSplineOrder before SetParameters" );
    }
}

unsigned int 
vtkITKBSplineTransform
::GetNumberOfParameters() const 
{
  // Need to have called SetSplineOrder before calling this.
  if( Helper != NULL )
    {
    return Helper->GetNumberOfParameters();
    }
  else
    {
    return 0;
    }
}

double const*
vtkITKBSplineTransform
::GetParameters() const 
{
  if( Helper != NULL )
    {
    return Helper->GetParameters().data_block();
    }
  else
    {
    return NULL;
    }
}

void
vtkITKBSplineTransform
::SetFixedParameters( double const* param, unsigned N ) 
{
  // Need to have called SetSplineOrder before calling this.
  if( Helper != NULL )
    {
    Helper->SetFixedParameters( param, N );
    }
  else
    {
    vtkErrorMacro( "need to call SetSplineOrder before SetFixedParameters" );
    }
}

unsigned int
vtkITKBSplineTransform
::GetNumberOfFixedParameters() const
{
  unsigned N = 0;
  if( Helper != NULL )
    {
    Helper->GetFixedParameters( N );
    }
  return N;
}

const double*
vtkITKBSplineTransform
::GetFixedParameters() const
{
  if( Helper != NULL )
    {
    unsigned N;
    return Helper->GetFixedParameters(N);
    }
  else
    {
    return NULL;
    }
}

void
vtkITKBSplineTransform
::ForwardTransformPoint( const float in[3], float out[3] )
{
  // Need to have called SetSplineOrder before calling this.
  if( Helper != NULL )
    {
    Helper->ForwardTransformPoint( in, out );
    }
  else
    {
    for( unsigned i = 0; i < 3; ++i )
      {
      out[i] = in[i];
      }
    }
}

void 
vtkITKBSplineTransform
::ForwardTransformPoint( const double in[3], double out[3] )
{
  // Need to have called SetSplineOrder before calling this.
  if( Helper != NULL )
    {
    Helper->ForwardTransformPoint( in, out );
    }
  else
    {
    for( unsigned i = 0; i < 3; ++i )
      {
      out[i] = in[i];
      }
    }
}

void
vtkITKBSplineTransform
::ForwardTransformDerivative( const float in[3], float out[3],
                              float derivative[3][3] )
{
  // Need to have called SetSplineOrder before calling this.
  if( Helper != NULL )
    {
    Helper->ForwardTransformDerivative( in, out, derivative );
    }
  else
    {
    for( unsigned i = 0; i < 3; ++i )
      {
      out[i] = in[i];
      }
    for( unsigned i = 0; i < 3; ++i )
      for( unsigned j = 0; j < 3; ++j )
        {
          derivative[i][j] = i==j ? 1.0f : 0.0f;
        }
    }
}

void
vtkITKBSplineTransform
::ForwardTransformDerivative( const double in[3], double out[3],
                              double derivative[3][3] )
{
  // Need to have called SetSplineOrder before calling this.
  if( Helper != NULL )
    {
    Helper->ForwardTransformDerivative( in, out, derivative );
    }
  else
    {
    for( unsigned i = 0; i < 3; ++i )
      {
      out[i] = in[i];
      }
    for( unsigned i = 0; i < 3; ++i )
      for( unsigned j = 0; j < 3; ++j )
        {
          derivative[i][j] = i==j ? 1.0 : 0.0;
        }
    }
}

void
vtkITKBSplineTransform
::InverseTransformPoint( const float in[3], float out[3] )
{
  // Need to have called SetSplineOrder before calling this.
  if( Helper != NULL )
    {
    Helper->InverseTransformPoint( in, out );
    }
  else
    {
    for( unsigned i = 0; i < 3; ++i )
      {
      out[i] = in[i];
      }
    }
}

void
vtkITKBSplineTransform
::InverseTransformPoint( const double in[3], double out[3] )
{
  // Need to have called SetSplineOrder before calling this.
  if( Helper != NULL )
    {
    Helper->InverseTransformPoint( in, out );
    }
  else
    {
    for( unsigned i = 0; i < 3; ++i )
      {
      out[i] = in[i];
      }
    }
}

void
vtkITKBSplineTransform
::InverseTransformDerivative( const float in[3], float out[3],
                              float derivative[3][3] )
{
  // Need to have called SetSplineOrder before calling this.
  if( Helper != NULL )
    {
    Helper->InverseTransformDerivative( in, out, derivative );
    }
  else
    {
    for( unsigned i = 0; i < 3; ++i )
      {
      out[i] = in[i];
      }
    for( unsigned i = 0; i < 3; ++i )
      for( unsigned j = 0; j < 3; ++j )
        {
          derivative[i][j] = i==j ? 1.0f : 0.0f;
        }
    }
}

void
vtkITKBSplineTransform
::InverseTransformDerivative( const double in[3], double out[3],
                              double derivative[3][3] )
{
  // Need to have called SetSplineOrder before calling this.
  if( Helper != NULL )
    {
    Helper->InverseTransformDerivative( in, out, derivative );
    }
  else
    {
    for( unsigned i = 0; i < 3; ++i )
      {
      out[i] = in[i];
      }
    for( unsigned i = 0; i < 3; ++i )
      for( unsigned j = 0; j < 3; ++j )
        {
          derivative[i][j] = i==j ? 1.0 : 0.0;
        }
    }
}


void
vtkITKBSplineTransform
::SetSwitchCoordinateSystem( bool v )
{
  Helper->SetSwitchCoordinateSystem( v );
}


bool
vtkITKBSplineTransform
::GetSwitchCoordinateSystem() const
{
  return Helper->GetSwitchCoordinateSystem();
}



// ---------------------------------------------------------------------------
// implement the actual wrapper around the itkBSplineDeformableTransform


template< unsigned O >
vtkITKBSplineTransformHelperImpl<O>
::vtkITKBSplineTransformHelperImpl()
  : BSpline( BSplineType::New() ),
    switchCoordSystems( false )
{
}


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
  this->parameters.SetSize( numberOfParam );
                                         
  for( unsigned int i=0; i<numberOfParam; ++i )
    this->parameters.SetElement( i, param.GetTuple1(i) );

  BSpline->SetParameters( parameters );
}

template< unsigned O >
void 
vtkITKBSplineTransformHelperImpl<O>
::SetParameters( double const* param )
{
  unsigned numberOfParam = BSpline->GetNumberOfParameters();
  this->parameters.SetSize( numberOfParam );
                                         
  for( unsigned int i=0; i<numberOfParam; ++i )
    this->parameters.SetElement( i, param[i] );

  BSpline->SetParameters( parameters );
}

template< unsigned O >
void 
vtkITKBSplineTransformHelperImpl<O>
::SetFixedParameters( double const* param, unsigned N )
{
  typename BSplineType::ParametersType parameters( N );
                                         
  for( unsigned int i=0; i<N; ++i )
    parameters.SetElement( i, param[i] );

  BSpline->SetFixedParameters( parameters );
}


template< unsigned O >
const double*
vtkITKBSplineTransformHelperImpl<O>
::GetFixedParameters( unsigned& N ) const
{
  N = BSpline->GetFixedParameters().GetSize();
  return BSpline->GetFixedParameters().data_block();
}


template <class T, unsigned O>
void
ForwardTransformHelper( vtkITKBSplineTransformHelperImpl<O>* helper,
                        const T in[3], T out[3] )
{
  typedef itk::BSplineDeformableTransform<double, 3, O> BSplineType;
  typename BSplineType::InputPointType inputPoint;

  inputPoint[0] = in[0];
  inputPoint[1] = in[1]; 
  inputPoint[2] = in[2];

  // See the comments in ForwardTransformDerivativeHelper about the
  // reasoning behind the switch coordinate system code.
  if (helper->switchCoordSystems)
    {
    inputPoint[0] = -inputPoint[0];
    inputPoint[1] = -inputPoint[1];
    }

  typename BSplineType::OutputPointType outputPoint;
  outputPoint = helper->BSpline->TransformPoint( inputPoint );

  out[0] = static_cast<T>(outputPoint[0]); 
  out[1] = static_cast<T>(outputPoint[1]); 
  out[2] = static_cast<T>(outputPoint[2]);

  if (helper->switchCoordSystems)
    {
    out[0] = -out[0];
    out[1] = -out[1];
    }
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::ForwardTransformPoint( const double in[3], double out[3] )
{
  ForwardTransformHelper<double, O>( this, in, out );
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::ForwardTransformPoint( const float in[3], float out[3] )
{
  ForwardTransformHelper<float, O>( this, in, out );
}

template <class T, unsigned O>
void
ForwardTransformDerivativeHelper( vtkITKBSplineTransformHelperImpl<O>* helper,
                                  const T in[3], T out[3], 
                                  T derivative[3][3] )
{
  // The logic for the LPS->RAS coordinate conversion is as follows.
  // Suppose x is a 3D point in a LPS coordinate system, and u is the
  // corresponding point in a RAS coordinate system.  We have an ITK
  // BSpline function f(x) that transforms points in an LPS coordinate
  // system.  We have this vtkITKBspline g that wraps f.  If there is
  // no coordinate system transformation required, we want g(x) =
  // f(x).  Life is easy.
  //
  // However, if we want g to operate in a RAS coordinate system, then
  // we want g(u).  We then need two more functions: t(u)=x that maps
  // from LPS to RAS, and r(x)=u that maps from RAS to LPS. Then, we
  // can define
  //    g(u) = r( f( t( u ) ) ).
  //
  // Now, going by the code in vtkMRMLTransformStorageNode.cxx:155,
  // the two conversion functions are simply defined by
  //    r(x1,x2,x3) = (-x1,-x2,x3)
  // and
  //    t(u1,u2,u2) = (-u1,-u2,u3)
  // (These functions simply flip the first two coordinate axes.)
  //
  // Then, transforming a point is straightforward: negate the first
  // two coordinates, apply f, and negate the first two coordinates of
  // the output.
  //
  // Transforming the Jacobian is a little more complex, but not too
  // bad. From the chain-rule,
  //   J[g](u) = J[r](f(t(u))) J[f](t(u)) J[t](u)
  //
  // Now, J[r] = J[t] = [ -1  0  0 ]
  //                    [  0 -1  0 ]
  //                    [  0  0  1 ]
  //
  // So, we negate the first two coordinates of the input point and
  // compute J[f].  Then, we pre- and post- multiply the resulting
  // matrix by the constant matrix above.  That produces the
  // transformed Jacobian.

  typedef itk::BSplineDeformableTransform<double, 3, O> BSplineType;
  typename BSplineType::InputPointType inputPoint;

  inputPoint[0] = in[0];
  inputPoint[1] = in[1]; 
  inputPoint[2] = in[2];

  if (helper->switchCoordSystems)
    {
    inputPoint[0] = -inputPoint[0];
    inputPoint[1] = -inputPoint[1];
    }

  typename BSplineType::OutputPointType outputPoint;
  outputPoint = helper->BSpline->TransformPoint( inputPoint );

  out[0] = static_cast<T>( outputPoint[0] ); 
  out[1] = static_cast<T>( outputPoint[1] ); 
  out[2] = static_cast<T>( outputPoint[2] );

  if (helper->switchCoordSystems)
    {
    out[0] = -out[0];
    out[1] = -out[1];
    }

  typename BSplineType::JacobianType jacobian = helper->BSpline->GetJacobian( inputPoint );
  for( unsigned i=0; i<3; ++i )
  {
    derivative[i][0] = static_cast<T>( jacobian( i, 0 ) );
    derivative[i][1] = static_cast<T>( jacobian( i, 1 ) );
    derivative[i][2] = static_cast<T>( jacobian( i, 2 ) );
  }

  if (helper->switchCoordSystems)
    {
    derivative[0][2] = -derivative[0][2];
    derivative[1][2] = -derivative[1][2];
    derivative[2][0] = -derivative[2][0];
    derivative[2][1] = -derivative[2][1];
    }
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::ForwardTransformDerivative( const double in[3], double out[3],
                              double derivative[3][3] )
{
  ForwardTransformDerivativeHelper<double, O>( this, in, out, derivative );
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::ForwardTransformDerivative( const float in[3], float out[3],
                            float derivative[3][3] )
{
  ForwardTransformDerivativeHelper<float, O>( this, in, out, derivative );
}


template <class T, unsigned O>
void
InverseTransformPointHelper( vtkITKBSplineTransformHelperImpl<O>* helper,
                             const T in[3], T out[3] )
{
  typedef itk::BSplineDeformableTransform<double, 3, O> BSplineType;
  typedef typename BSplineType::OutputPointType OutputPointType;
  typedef typename BSplineType::InputPointType InputPointType;

  //iterative inverse bSpline transform
  int const MaxIterationNumber = 10;
  double const Tolerance = 1;
  OutputPointType opt;
  InputPointType ipt;
  opt[0] = in[0];
  opt[1] = in[1];
  opt[2] = in[2];

  // See the comments in ForwardTransformDerivativeHelper about the
  // reasoning behind the switch coordinate system code.
  if (helper->switchCoordSystems)
    {
    opt[0] = -opt[0];
    opt[1] = -opt[1];
    }


  ipt[0] = opt[0];
  ipt[1] = opt[1];
  ipt[2] = opt[2];
  for (int k = 0; k <= MaxIterationNumber; k++)
  {
    OutputPointType optTrail = helper->BSpline->TransformPoint( ipt );
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

  if (helper->switchCoordSystems)
    {
    out[0] = -out[0];
    out[1] = -out[1];
    }
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::InverseTransformPoint( const float in[3], float out[3] )
{
  InverseTransformPointHelper<float, O>( this, in, out );
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::InverseTransformPoint( const double in[3], double out[3] )
{
  InverseTransformPointHelper<double, O>( this, in, out );
}


template <class T, unsigned O>
void
InverseTransformDerivativeHelper( vtkITKBSplineTransformHelperImpl<O>* helper,
                                  const T in[3], T out[3], T derivative[3][3] )
{
  typedef itk::BSplineDeformableTransform<double, 3, O> BSplineType;
  typedef typename BSplineType::InputPointType InputPointType;
  typedef typename BSplineType::JacobianType JacobianType;

  InverseTransformPointHelper<T,O>( helper, in, out );

  InputPointType pt;

  pt[0] = out[0];
  pt[1] = out[1]; 
  pt[2] = out[2];

  // See the comments in ForwardTransformDerivativeHelper about the
  // reasoning behind the switch coordinate system code.
  if (helper->switchCoordSystems)
    {
    pt[0] = -pt[0];
    pt[1] = -pt[1];
    }

  JacobianType const& jacobian = helper->BSpline->GetJacobian( pt );
  for( unsigned i=0; i<3; ++i )
  {
    derivative[i][0] = static_cast<T>( jacobian( i, 0 ) );
    derivative[i][1] = static_cast<T>( jacobian( i, 1 ) );
    derivative[i][2] = static_cast<T>( jacobian( i, 2 ) );
  }  

  if (helper->switchCoordSystems)
    {
    derivative[0][2] = -derivative[0][2];
    derivative[1][2] = -derivative[1][2];
    derivative[2][0] = -derivative[2][0];
    derivative[2][1] = -derivative[2][1];
    }
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::InverseTransformDerivative( const float in[3], float out[3],
                            float derivative[3][3] )
{
  InverseTransformDerivativeHelper<float, O>( this, in, out, derivative );
}

template< unsigned O >
void
vtkITKBSplineTransformHelperImpl<O>
::InverseTransformDerivative( const double in[3], double out[3],
                            double derivative[3][3] )
{
  InverseTransformDerivativeHelper<double, O>( this, in, out, derivative );
}
