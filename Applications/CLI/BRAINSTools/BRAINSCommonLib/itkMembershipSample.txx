/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkMembershipSample.txx,v $
  Language:  C++
  Date:      $Date: 2009-05-02 05:43:58 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkMembershipSample_txx
#define __itkMembershipSample_txx

namespace itk {
namespace Statistics {

template< class TSample >
MembershipSample< TSample >
::MembershipSample()
{
  this->m_NumberOfClasses = 0;
}

template< class TSample >
void
MembershipSample< TSample >
::SetNumberOfClasses(unsigned int numberOfClasses)
{
  m_NumberOfClasses = numberOfClasses;
  m_ClassSamples.resize(m_NumberOfClasses);
  for ( unsigned int i = 0; i < m_NumberOfClasses; i++ )
    {
    m_ClassSamples[i] = ClassSampleType::New();
    (m_ClassSamples[i])->SetSample(this->GetSample());
    }
}

template< class TSample >
inline void 
MembershipSample< TSample >
::AddInstance(const ClassLabelType &classLabel, const InstanceIdentifier &id) 
{ 
  m_ClassLabelHolder[id] = classLabel; 
  int classIndex = this->GetInternalClassLabel(classLabel);
  if ( classIndex == -1 )
    {
    m_UniqueClassLabels.push_back(classLabel);
    classIndex = m_UniqueClassLabels.size() - 1;
    }

  (m_ClassSamples[classIndex])->AddInstance(id);
}

template< class TSample >
inline unsigned int 
MembershipSample< TSample >
::GetClassLabel(const InstanceIdentifier &id) const
{
  return (*(m_ClassLabelHolder.find(id))).second;
}

template< class TSample >
inline int 
MembershipSample< TSample >
::GetInternalClassLabel(const ClassLabelType classLabel) const
{
  for ( unsigned int i = 0; i < m_UniqueClassLabels.size(); i++ )
    {
    if ( m_UniqueClassLabels[i] == classLabel )
      {
      return i;
      }
    }

  return -1;
}

template< class TSample >
const typename MembershipSample< TSample>::ClassLabelHolderType
MembershipSample< TSample >
::GetClassLabelHolder() const
{
  return m_ClassLabelHolder;
}

template< class TSample >
const typename MembershipSample< TSample >::ClassSampleType*
MembershipSample< TSample >
::GetClassSample(const ClassLabelType &classLabel) const
{
  int classIndex = this->GetInternalClassLabel(classLabel);
  if (classIndex == -1)
    {
    return NULL;
    }
  return m_ClassSamples[classIndex]; 
}

template< class TSample >
inline const typename MembershipSample< TSample >::MeasurementVectorType &
MembershipSample< TSample >
::GetMeasurementVector(const InstanceIdentifier &id) const
{
  return m_Sample->GetMeasurementVector(id); 
}

template< class TSample >
inline typename MembershipSample< TSample >::MeasurementType
MembershipSample< TSample >
::GetMeasurement(const InstanceIdentifier &id,
                 const unsigned int &dimension)
{ 
  return m_Sample->GetMeasurement(id, dimension);
}

template< class TSample >
inline typename MembershipSample< TSample >::AbsoluteFrequencyType
MembershipSample< TSample >
::GetFrequency(const InstanceIdentifier &id) const
{
  return m_Sample->GetFrequency(id); 
}
  
template< class TSample >
inline typename MembershipSample< TSample >::TotalAbsoluteFrequencyType
MembershipSample< TSample >
::GetTotalFrequency() const
{
  return m_Sample->GetTotalFrequency();
}

template< class TSample >
void
MembershipSample< TSample >
::Graft( const DataObject *thatObject )
{
  this->Superclass::Graft(thatObject);

  // Most of what follows is really a deep copy, rather than grafting of 
  // output. Wish it were managed by pointers to bulk data. Sigh !
  
  const Self *thatConst = dynamic_cast< const Self * >(thatObject);
  if (thatConst)
    {
    Self *that = const_cast< Self * >(thatConst); 
    this->m_UniqueClassLabels = that->m_UniqueClassLabels;
    this->m_ClassLabelHolder  = that->m_ClassLabelHolder;
    this->m_ClassSamples      = that->m_ClassSamples;
    this->m_Sample            = that->m_Sample;
    this->m_NumberOfClasses   = that->m_NumberOfClasses;
    }
}

template< class TSample >
void
MembershipSample< TSample >
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  os << indent << "Sample: " << m_Sample.GetPointer() << std::endl;
  os << indent << "NumberOfClasses: " << this->GetNumberOfClasses() << std::endl;
}


} // end of namespace Statistics 
} // end of namespace itk

#endif
