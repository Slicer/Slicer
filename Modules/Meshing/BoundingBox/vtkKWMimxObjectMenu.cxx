/*=========================================================================

  Module:    $RCSfile: vtkKWMimxObjectMenu.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWMimxObjectMenu.h"

#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkSTLReader.h"

#include "vtkKWApplication.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithScrollbar.h"
#include "vtkKWIcon.h"
#include "vtkKWInternationalization.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWOptions.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxObjectMenu);
vtkCxxRevisionMacro(vtkKWMimxObjectMenu, "$Revision: 1.6 $");

//----------------------------------------------------------------------------
vtkKWMimxObjectMenu::vtkKWMimxObjectMenu()
{
  this->MimxOperationsMenu = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxObjectMenu::~vtkKWMimxObjectMenu()
{
}
//----------------------------------------------------------------------------
void vtkKWMimxObjectMenu::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }
  this->Superclass::CreateWidget();
}
//----------------------------------------------------------------------------
void vtkKWMimxObjectMenu::Update()
{
  this->UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxObjectMenu::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxObjectMenu::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
