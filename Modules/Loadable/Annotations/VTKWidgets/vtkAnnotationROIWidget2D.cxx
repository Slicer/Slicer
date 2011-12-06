/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkAnnotationROIWidget.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkAnnotationROIWidget2D.h"
#include "vtkAnnotationROIRepresentation2D.h"
#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkObjectFactory.h"
#include "vtkWidgetEventTranslator.h"
#include "vtkWidgetCallbackMapper.h" 
#include "vtkEvent.h"
#include "vtkWidgetEvent.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"


vtkCxxRevisionMacro(vtkAnnotationROIWidget2D, "$Revision: 12141 $");
vtkStandardNewMacro(vtkAnnotationROIWidget2D);

//----------------------------------------------------------------------------
vtkAnnotationROIWidget2D::vtkAnnotationROIWidget2D()
{
}

//----------------------------------------------------------------------------
vtkAnnotationROIWidget2D::~vtkAnnotationROIWidget2D()
{  
}

//----------------------------------------------------------------------
void vtkAnnotationROIWidget2D::CreateDefaultRepresentation()
{
  if ( ! this->WidgetRep )
    {
    this->WidgetRep = vtkAnnotationROIRepresentation2D::New();
    }
}

//----------------------------------------------------------------------------
void vtkAnnotationROIWidget2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

}




