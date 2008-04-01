#ifndef __itkDiffusionTensor3DTransform_txx
#define __itkDiffusionTensor3DTransform_txx

#include "itkDiffusionTensor3DTransform.h"

namespace itk
{

template< class TData >
DiffusionTensor3DTransform< TData >
::DiffusionTensor3DTransform()
{
  //Initialize the Measurement Frame to Identity
  m_MeasurementFrame.SetIdentity() ;
}


}//end namespace itk
#endif
