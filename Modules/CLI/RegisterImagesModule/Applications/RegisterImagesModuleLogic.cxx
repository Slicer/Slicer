/*=========================================================================

Program:   Maverick
Module:    $RCSfile: config.h,v $

Copyright (c) Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

All rights reserved.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
#include "RegisterImagesModuleLogic.h"

RegisterImagesModuleLogic * RegisterImagesModuleLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("RegisterImagesModuleLogic");

  if( ret )
    {
    return (RegisterImagesModuleLogic *)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new RegisterImagesModuleLogic;
}

// ----------------------------------------------------------------------------
RegisterImagesModuleLogic::RegisterImagesModuleLogic()
{
}

// ----------------------------------------------------------------------------
RegisterImagesModuleLogic::~RegisterImagesModuleLogic()
{
}

// ----------------------------------------------------------------------------
void RegisterImagesModuleLogic::PrintSelf( ostream & vtkNotUsed(os),
                                           vtkIndent vtkNotUsed(indent) )
{

}

/** Derived to send an EndEvent */
void RegisterImagesModuleLogic::ApplyTask(void *clientdata)
{
  vtkCommandLineModuleLogic::ApplyTask(clientdata);

  std::string resampledVolumeId =
    this->GetCommandLineModuleNode()->GetParameterAsString("resampledImage");
  vtkMRMLNode* node = MRMLScene->GetNodeByID(resampledVolumeId);
  if( node )
    {
    node->InvokeEvent(vtkCommand::ModifiedEvent);
    }
}

void RegisterImagesModuleLogic
::SetLandmarks( const std::vector<std::vector<float> > & fixed,
                const std::vector<std::vector<float> > & moving)
{
  if( fixed.size() != moving.size() )
    {
    std::cerr << "Fixed and moving landmarks don't have same size!" << std::endl;
    return;
    }
  if( fixed != m_FixedLandmarks )
    {
    m_FixedLandmarks = fixed;
    }
  if( moving != m_MovingLandmarks )
    {
    m_MovingLandmarks = moving;
    }

  std::stringstream                                strvalue;
  std::vector<std::vector<float> >::const_iterator i;
  std::vector<std::vector<float> >::const_iterator end;
  bool                                             first = true;
  for( i = fixed.begin(), end = fixed.end(); i != end; ++i )
    {
    if( !first )
      {
      strvalue << ",";
      }
    strvalue << (*i)[0];
    for( unsigned int j = 1; j < i->size(); ++j )
      {
      strvalue << "," << (*i)[j];
      }
    first = false;
    }
  strvalue << std::endl;
  this->SetFixedLandmarks( strvalue.str().c_str() );

  first = true;
  std::stringstream strMovingValue;
  for( i = moving.begin(), end = moving.end(); i != end; ++i )
    {
    if( !first )
      {
      strMovingValue << ",";
      }
    strMovingValue << (*i)[0];
    for( unsigned int j = 1; j < i->size(); ++j )
      {
      strMovingValue << "," << (*i)[j];
      }
    first = false;
    }
  strMovingValue << std::endl;
  this->SetMovingLandmarks( strMovingValue.str().c_str() );
}

int RegisterImagesModuleLogic
::AddLandmark( const std::vector<float> & fixed,
               const std::vector<float> & moving )
{
  m_FixedLandmarks.push_back(fixed);
  m_MovingLandmarks.push_back(moving);

  this->SetLandmarks( m_FixedLandmarks, m_MovingLandmarks );
  return m_FixedLandmarks.size() - 1;
}

void RegisterImagesModuleLogic
::SetLandmark( int landmark,
               const std::vector<float> & fixed,
               const std::vector<float> & moving )
{
  if( landmark > 0 &&
      (unsigned int)landmark > m_FixedLandmarks.size() )
    {
    std::cerr << "Landmark can't be set !" << std::endl;
    return;
    }
  if( landmark < 0 ||
      (unsigned int)landmark == m_FixedLandmarks.size() )
    {
    std::vector<float> a;
    landmark = m_FixedLandmarks.size();
    m_FixedLandmarks.push_back(a);
    m_MovingLandmarks.push_back(a);
    }
  m_FixedLandmarks[landmark] = fixed;
  m_MovingLandmarks[landmark] = moving;

  this->SetLandmarks( m_FixedLandmarks, m_MovingLandmarks );
}

void RegisterImagesModuleLogic
::RemoveLandmark( int landmark )
{
  if( landmark >= static_cast<int>( m_FixedLandmarks.size() ) )
    {
    return;
    }
  m_FixedLandmarks.erase( m_FixedLandmarks.begin() + landmark );
  m_MovingLandmarks.erase( m_MovingLandmarks.begin() + landmark );
  this->SetLandmarks( m_FixedLandmarks, m_MovingLandmarks );
}
