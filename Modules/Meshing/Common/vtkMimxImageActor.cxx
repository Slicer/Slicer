/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxImageActor.cxx,v $
Language:  C++
Date:      $Date: 2008/08/06 03:04:38 $
Version:   $Revision: 1.5 $

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

#include "vtkMimxImageActor.h"

#include "vtkActor.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkImagePlaneWidget.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkProperty.h"

vtkCxxRevisionMacro(vtkMimxImageActor, "$Revision: 1.5 $");

vtkStandardNewMacro(vtkMimxImageActor);

vtkMimxImageActor::vtkMimxImageActor()
{
        PlaneX = vtkImagePlaneWidget::New();
        PlaneY = vtkImagePlaneWidget::New();
        PlaneZ = vtkImagePlaneWidget::New();
        Reader = ReaderType::New();
        Filter = FilterType::New();
        this->Interactor = NULL;
}

vtkMimxImageActor::~vtkMimxImageActor()
{
        this->PlaneX->Delete();
        this->PlaneY->Delete();
        this->PlaneZ->Delete();
        this->Reader->Delete();
        this->Filter->Delete();
}

vtkImageData* vtkMimxImageActor::GetDataSet()
{
        return this->Filter->GetOutput();
}

ImageType* vtkMimxImageActor::GetITKImage()
{
        return this->Reader->GetOutput();
}

void vtkMimxImageActor::SetITKImageFilePath(const char *FPath)
{
        Reader->SetFileName(FPath);
        Reader->Update();
        //IpImage = Reader->GetOutput();

        ImageType::SizeType size = 
                Reader->GetOutput()->GetLargestPossibleRegion().GetSize();
        Filter->SetInput(Reader->GetOutput());
        Filter->Update();
        // setting up the imagepanewidget
        PlaneX->SetInput(Filter->GetOutput());
        PlaneX->DisplayTextOn();
        PlaneX->SetPlaneOrientationToXAxes();
        PlaneX->RestrictPlaneToVolumeOn();
        PlaneX->SetSliceIndex(static_cast<int>(size[0]/2));
//      PlaneX->SetPicker(CellPicker);
        PlaneX->SetKeyPressActivationValue('x');
        PlaneX->GetPlaneProperty()->SetColor(1,0,0);
        PlaneX->SetInteractor(Interactor);
        PlaneX->On();

        
        PlaneY->SetInput(Filter->GetOutput());
        PlaneY->DisplayTextOn();
        PlaneY->SetPlaneOrientationToYAxes();
        PlaneY->RestrictPlaneToVolumeOn();
        PlaneY->SetSliceIndex(static_cast<int>(size[1]/2));
//      PlaneY->SetPicker(CellPicker);
  PlaneY->KeyPressActivationOn();
        PlaneY->SetKeyPressActivationValue('y');
        PlaneY->GetPlaneProperty()->SetColor(0,1,0);
        PlaneY->SetInteractor(Interactor);
        PlaneY->On();

        PlaneZ->SetInput(Filter->GetOutput());
        PlaneZ->DisplayTextOn();
        PlaneZ->SetPlaneOrientationToZAxes();
        PlaneZ->RestrictPlaneToVolumeOn();
        PlaneZ->SetSliceIndex(static_cast<int>(size[2]/2));
//      PlaneZ->SetPicker(CellPicker);
        PlaneZ->SetKeyPressActivationValue('z');
        PlaneZ->GetPlaneProperty()->SetColor(0,0,1);
        PlaneZ->SetInteractor(Interactor);
        PlaneZ->On();
}

int vtkMimxImageActor::GetActorVisibility()
{
        if(this->PlaneX->GetEnabled())
                return 1;
        else
                return 0;
}

void vtkMimxImageActor::DisplayActor(int Display)
{
        this->PlaneX->SetEnabled(Display);
        this->PlaneY->SetEnabled(Display);
        this->PlaneZ->SetEnabled(Display);
}

void vtkMimxImageActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

