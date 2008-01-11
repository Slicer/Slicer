/*=========================================================================

Program:   Maverick
Module:    $RCSfile: config.h,v $

Copyright (c) Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

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

RegisterImagesModuleLogic* RegisterImagesModuleLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("RegisterImagesModuleLogic");
  if(ret)
    {
    return (RegisterImagesModuleLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new RegisterImagesModuleLogic;
}


//----------------------------------------------------------------------------
RegisterImagesModuleLogic::RegisterImagesModuleLogic()
{
}

//----------------------------------------------------------------------------
RegisterImagesModuleLogic::~RegisterImagesModuleLogic()
{
}

//----------------------------------------------------------------------------
void RegisterImagesModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

/** Derived to send an EndEvent */
void RegisterImagesModuleLogic::ApplyTask(void *clientdata)
{
  vtkCommandLineModuleLogic::ApplyTask(clientdata);

  std::string resampledVolumeId = this->GetCommandLineModuleNode()->GetParameterAsString("resampledImage");
  vtkMRMLNode* node = MRMLScene->GetNodeByID(resampledVolumeId);
  if(node)
    {
    node->InvokeEvent(vtkCommand::ModifiedEvent);
    }
}
