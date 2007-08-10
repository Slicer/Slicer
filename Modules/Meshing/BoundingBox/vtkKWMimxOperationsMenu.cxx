/*=========================================================================

  Module:    $RCSfile: vtkKWMimxOperationsMenu.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWMimxOperationsMenu.h"

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
vtkStandardNewMacro(vtkKWMimxOperationsMenu);
vtkCxxRevisionMacro(vtkKWMimxOperationsMenu, "$Revision: 1.6 $");

//----------------------------------------------------------------------------
vtkKWMimxOperationsMenu::vtkKWMimxOperationsMenu()
{
  this->MimxViewWindow = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxOperationsMenu::~vtkKWMimxOperationsMenu()
{
}
//----------------------------------------------------------------------------
void vtkKWMimxOperationsMenu::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }
  this->Superclass::CreateWidget();
}
//----------------------------------------------------------------------------
void vtkKWMimxOperationsMenu::Update()
{
  this->UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxOperationsMenu::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxOperationsMenu::ShowHideActor(int Row_Num, int Col_Num, int Vis)
{
  int x, y,z;
}
//----------------------------------------------------------------------------
void vtkKWMimxOperationsMenu::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
