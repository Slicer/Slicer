#ifndef __itkLookAtTransformInitializer_txx
#define __itkLookAtTransformInitializer_txx

#include "itkLookAtTransformInitializer.h"

namespace itk
{

template < class TTransform, class TImage >
LookAtTransformInitializer< TTransform, TImage >
::LookAtTransformInitializer()
{
  m_Center.Fill( 0.0 );
  m_Direction.Fill( 0.0 );
  m_Direction[2] = 1.0;
  m_Up.Fill( 0.0 );
  m_Up[1] = 1.0;
}


template < class TTransform, class TImage >
void
LookAtTransformInitializer< TTransform, TImage >
::InitializeTransform()
{
  // Some useful vectors
  VectorType zero; zero.Fill( 0.0 );
  VectorType x; x.Fill( 0.0 ); x[0] = 1.0;
  VectorType y; y.Fill( 0.0 ); y[1] = 1.0;

  // Sanity check
  if( !m_Transform )
    {
    itkExceptionMacro( "Transform has not been set" );
    return;
    }
  if( !m_Image )
    {
    itkExceptionMacro( "Image has not been set" );
    return;
    }
  if ( m_Direction == zero )
    {
    itkExceptionMacro( "Direction can not be zero" );
    return;
    }
  if ( m_Up == zero )
    {
    itkExceptionMacro( "Up can not be zero" );
    return;
    }

  // Initialize the transform
  m_Transform->SetIdentity();

  //NOTE: this code is based on the gluLookAt(..) function
  //URL: http://pyopengl.sourceforge.net/documentation/manual/gluLookAt.3G.html
  //URL: http://developer.apple.com/documentation/Darwin/Reference/ManPages/man3/gluLookAt.3.html

  // The plane direction vector corresponds to the "f" vector
  VectorType f( m_Direction );


  // Ensure "up" is not the same direction as "f"
  if (f == m_Up || f == -m_Up)
    {
    //itkWarningMacro("The up vector can not be the same as the direction vector. ");
    // Redefine "up"
    if ( m_Up != y && m_Up != -y )
    {
      m_Up = y;
    }
  else
    {
      m_Up = x;
    }
    }

  // Do some cross products to get orthogonal axes
  VectorType s = CrossProduct(f, m_Up);
  s.Normalize();
  VectorType u = CrossProduct(s, f);
  u.Normalize();

  // Set the matrix
  // NOTE: Unlike gluLookAt(..) we DON'T use "-f", which looks down
  //       the negative z-axis. Instead we use "f" because we want
  //       to look down the POSITIVE z-axis.
  typename TransformType::MatrixType matrix = m_Transform->GetMatrix();
  matrix[0][0]=s[0]; matrix[0][1]=u[0]; matrix[0][2]=f[0];
  matrix[1][0]=s[1]; matrix[1][1]=u[1]; matrix[1][2]=f[1];
  matrix[2][0]=s[2]; matrix[2][1]=u[2]; matrix[2][2]=f[2];
  m_Transform->SetMatrix(matrix);

  // Set the transform center
  m_Transform->SetCenter( m_Center );

  // Set the transform translation
  VectorType offset;
  /*
  offset[0] = m_Center[0] - (m_Size[0] * 0.5 * m_Image->GetSpacing()[0]);
  offset[1] = m_Center[1] - (m_Size[1] * 0.5 * m_Image->GetSpacing()[1]);
  offset[2] = m_Center[2];
  */
  
  // Fedorov: no translation initialization
  offset[0] = 0;
  offset[1] = 0;
  offset[2] = 0;
  
  VectorType offsetT = m_Transform->TransformVector( offset );
  m_Transform->SetTranslation( offsetT );
}


template < class TTransform, class TImage >
void
LookAtTransformInitializer< TTransform, TImage >
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  os << indent << "Center = " << m_Center << std::endl;
  os << indent << "Direction = " << m_Direction << std::endl;
  os << indent << "Up = " << m_Up << std::endl;
  os << indent << "Transform = " << m_Transform << std::endl;
  os << indent << "Image = " << m_Image << std::endl;
}

}  // namespace itk

#endif
