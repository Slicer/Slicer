/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxBWidgetPlusIPWidget.cxx,v $
Language:  C++
Date:      $Date: 2007/05/10 16:32:38 $
Version:   $Revision: 1.3 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "vtkBWidgetPlusIPWidget.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"
#include "vtkTransform.h"
#include "vtkActor.h"


vtkBWidgetPlusIPWidget::vtkBWidgetPlusIPWidget()
{
        this->Actor = vtkActor::New();
        this->FileName = NULL;
        this->UGrid = vtkUnstructuredGrid::New();
        this->UGridMapper = vtkDataSetMapper::New();
        this->UGridMapper->SetInput(this->UGrid);
        this->Actor->SetMapper(this->UGridMapper);
        this->Actor->GetProperty()->SetColor(1.0,1.0,1.0);
        for(int i=0; i<3; i++)  this->PrevColor[i] = 0.0;
}

vtkBWidgetPlusIPWidget::~vtkBWidgetPlusIPWidget()
{
        this->UGridMapper->Delete();
        this->Actor->Delete();
        if(this->FileName)      delete [] this->FileName;
}

vtkActor* vtkBWidgetPlusIPWidget::GetActor()
{
        return this->Actor;
}

void vtkBWidgetPlusIPWidget::SetPrevColor(double color[3])
{
        for(int i=0; i<3; i++)  this->PrevColor[i] = color[i];
}

void vtkBWidgetPlusIPWidget::GetPrevColor(double color[3])
{
        for(int i=0; i<3; i++)  color[i] = this->PrevColor[i];
}
