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
#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkTransformFilter.h"
#include "vtkPointSet.h"
#include "vtkImageReslice.h"
#include "vtkImageChangeInformation.h"

// filter to fix ITK / VTK image orientation.  This class uses an ITK actor to read in
// files but Slicer is a VTK application, so reorient images to VTK style using a flip
// around the Y axis.

#include "vtkImageFlip.h"

vtkCxxRevisionMacro(vtkMimxImageActor, "$Revision: 1.5 $");

vtkStandardNewMacro(vtkMimxImageActor);

vtkMimxImageActor::vtkMimxImageActor()
{
        this->PlaneX = vtkImagePlaneWidget::New();
        this->PlaneY = vtkImagePlaneWidget::New();
        this->PlaneZ = vtkImagePlaneWidget::New();
//        this->PlaneX = NULL;
//        this->PlaneY = NULL;
//        this->PlaneZ = NULL;

        this->Reader = ReaderType::New();
        this->Filter = FilterType::New();
        this->FlipFilter = vtkImageFlip::New();
        // set flip to be around Y axis
        this->FlipFilter->SetFilteredAxis(1);
        this->FlipFilter->SetInput(this->Filter->GetOutput());
        this->Interactor = NULL;
        this->SavedImage = NULL;

        this->RASToIJKMatrix = NULL;
}

vtkMimxImageActor::~vtkMimxImageActor()
{
        if (this->PlaneX) this->PlaneX->Delete();
        if (this->PlaneY) this->PlaneY->Delete();
        if (this->PlaneZ) this->PlaneZ->Delete();
        if (this->Reader) this->Reader->Delete();
        if (this->Filter) this->Filter->Delete();
        if (this->FlipFilter) this->FlipFilter->SetInput(NULL);
        if (this->FlipFilter) this->FlipFilter->Delete();
        //if (this->RASToIJKMatrix) this->RASToIJKMatrix->Delete();
}

vtkImageData* vtkMimxImageActor::GetDataSet()
{
        return this->SavedImage;
        //return this->Filter->GetOutput();
        //return this->FlipFilter->GetOutput();
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

        // point to the output of the filter as the current dataset
        this->SavedImage = Filter->GetOutput();
        this-> InitializePlaneWidgets();
}


void vtkMimxImageActor::SetImageDataSet(vtkImageData* image)
{
    this->SavedImage = image;
    this->InitializePlaneWidgets();
}

// slicer stores datasets in RAS format, so we have queried slicer to get the xform matrix.
// apply the matrix to what is fed to the rendering planes, so the rendering matches.  Don't
// change the actual dataset since it is just a reference to the dataset kept by Slicer.

void vtkMimxImageActor::SetImageDataSet(vtkImageData* image, vtkMatrix4x4* matrix, double origin[3], double spacing[3])
{
    this->SavedImage = image;
    this->Spacing[0]=spacing[0]; this->Spacing[1]=spacing[1]; this->Spacing[2]=spacing[2];
    this->Origin[0]=origin[0]; this->Origin[1]=origin[1]; this->Origin[2]=origin[2];
    this->RASToIJKMatrix = vtkMatrix4x4::New();
    this->RASToIJKMatrix->DeepCopy(matrix);
    //this->InitializePlaneWidgets(matrix, origin,spacing);
}

void vtkMimxImageActor::InitializePlaneWidgets()
{

     ////////////////////////////////////////////////
     // Resample the image to Standard Orientation
     // using Direction Cosines
     ////////////////////////////////////////////////
     double directionCosines[9];

     vtkMatrix4x4 *cosineMatrix = vtkMatrix4x4::New();
     cosineMatrix->Identity();
     vtkImageData *vtkImage = vtkImageData::SafeDownCast(this->SavedImage);

     //this->SavedImage->Print(std::cout);

     vtkMatrix4x4* LpsToRasMatrix = vtkMatrix4x4::New();
     LpsToRasMatrix->Identity();
     //LpsToRasMatrix->SetElement(0, 0, -1.0);
     //LpsToRasMatrix->SetElement(1, 1, -1.0);

     vtkMatrix4x4* resultMatrix = vtkMatrix4x4::New();
     vtkMatrix4x4::Multiply4x4(LpsToRasMatrix, cosineMatrix, resultMatrix);
     resultMatrix->Invert();
     for (int i=0,k=0; i<3; i++)
     {
       for ( int j=0; j < 3; j++,k++ )
       {
         directionCosines[k] =  resultMatrix->GetElement(j, i);
         //std::cout << "dirCosine[" << k << "] = " << directionCosines[k];
       }
     }
     //std::cout << std::endl;

     // Debug Printing
     std::cout << "*** Cosine Matrix ***" << std::endl;
     cosineMatrix->Print(std::cout);
     std::cout << "*** Result Matrix ***" << std::endl;
     resultMatrix->Print(std::cout);

     vtkImageReslice *reslice = vtkImageReslice::New();
     reslice->SetInput( vtkImage );
     reslice->SetResliceAxesDirectionCosines( directionCosines );
     reslice->Update();

     int *extent = reslice->GetOutput()->GetExtent();
     std::cout << "extent: ( " << extent[1] << "," << extent[3] << "," << extent[5] << ")" << std::endl;

        // setting up the imagePlaneWidgets.  They point to the current vtkImageData
        // record which is the volume being observed by this actor.  The position of
        // the planes is determined by halfway into the data set

    //
    cosineMatrix->Delete();
    reslice->Delete();
    LpsToRasMatrix->Delete();
    resultMatrix->Delete();
    
     if (this->PlaneX)
     {
         PlaneX->SetPlaneOrientationToXAxes();
         PlaneX->SetInput(reslice->GetOutput());
         PlaneX->DisplayTextOn();
         PlaneX->RestrictPlaneToVolumeOn();
         PlaneX->SetSliceIndex(static_cast<int>(extent[1]/2));
         PlaneX->SetKeyPressActivationValue('x');
         PlaneX->GetPlaneProperty()->SetColor(1,0,0);
         PlaneX->SetInteractor(this->Interactor);
         PlaneX->On();
     }

     if (this->PlaneY)
     {
         PlaneY->SetPlaneOrientationToYAxes();
         PlaneY->SetInput(reslice->GetOutput());
         PlaneY->DisplayTextOn();
         PlaneY->RestrictPlaneToVolumeOn();
         PlaneY->SetSliceIndex(static_cast<int>(extent[3]/2));
         //PlaneY->KeyPressActivationOn();
         PlaneY->SetKeyPressActivationValue('y');
         PlaneY->GetPlaneProperty()->SetColor(0,1,0);
        PlaneY->SetInteractor(this->Interactor);
        PlaneY->On();
        }
     if (this->PlaneZ)
     {
         PlaneZ->SetPlaneOrientationToZAxes();
         PlaneZ->SetInput(reslice->GetOutput());
         PlaneZ->DisplayTextOn();
         PlaneZ->RestrictPlaneToVolumeOn();
         PlaneZ->SetSliceIndex(static_cast<int>(extent[5]/2));
         PlaneZ->SetKeyPressActivationValue('z');
         PlaneZ->GetPlaneProperty()->SetColor(0,0,1);
         PlaneZ->SetInteractor(this->Interactor);
         PlaneZ->On();
     }
}


//void vtkMimxImageActor::InitializePlaneWidgets(vtkMatrix4x4* matrix, double origin[3], double spacing[3])
//{
//    // use Slicer's display structure to display with a MRML node corresponding to this image
//}



void vtkMimxImageActor::InitializePlaneWidgets(vtkMatrix4x4* matrix, double origin[3], double spacing[3])
{

    cout << "spacing from MRML node: (" << spacing[0] << "," << spacing[1] << "," << spacing[2] << ")" << endl;

    double beforespace[3],afterspace[3], afterorigin[3];
    this->SavedImage->GetSpacing(beforespace);
    cout << "spacing from vtk image: (" << beforespace[0] << "," << beforespace[1] << "," << beforespace[2] << ")" << endl;
    this->SavedImage->SetSpacing(spacing);
    this->SavedImage->SetOrigin(origin);
    this->SavedImage->GetSpacing(afterspace);
    this->SavedImage->GetOrigin(afterorigin);
    cout << "spacing from vtk image after set: (" << afterspace[0] << "," << afterspace[1] << "," << afterspace[2] << ")" << endl;
    cout << "origin from vtk image after set: (" << afterorigin[0] << "," << afterorigin[1] << "," << afterorigin[2] << ")" << endl;

    // a matrix is used since this is called from within Slicer.  The image datasets
    // in slicer are referenced in RAS coordinates, so transform the dataset before passing
    // it to the imagePlaneWidgets.

     double directionCosines[9];
     for (int i=0,k=0; i<3; i++)
     {
       for ( int j=0; j < 3; j++,k++ )
       {
         directionCosines[k] =  matrix->GetElement(j, i);
       }
     }

     double bbox[6];
     this->SavedImage->GetBounds(bbox);
     std::cout << "before bounds: (" << bbox[0] << "," << bbox[2] << "," << bbox[4] << ") to " ;
     std::cout <<        "(" << bbox[1] << "," << bbox[3] << "," << bbox[5] << ")" << std::endl;

     vtkImageData *vtkImage = vtkImageData::SafeDownCast(this->SavedImage);
     vtkImageReslice *reslice = vtkImageReslice::New();
     reslice->SetInput( vtkImage );
     reslice->SetResliceAxesDirectionCosines( directionCosines );
     reslice->Update();

     int *extent = reslice->GetOutput()->GetExtent();
     std::cout << "extent: ( " << extent[1] << "," << extent[3] << "," << extent[5] << ")" << std::endl;

    // setting up the imagePlaneWidgets.  They point to the current vtkImageData
    // record which is the volume being observed by this actor.  The position of
    // the planes is determined by halfway into the data set.

    PlaneX->SetPlaneOrientationToXAxes();
    PlaneX->SetInteractor(this->Interactor);
    PlaneX->SetInput(vtkImageData::SafeDownCast(reslice->GetOutput()));
    PlaneX->DisplayTextOn();
    PlaneX->RestrictPlaneToVolumeOn();
    PlaneX->SetSliceIndex(static_cast<int>(extent[1]/2));
    PlaneX->SetKeyPressActivationValue('x');
    PlaneX->GetPlaneProperty()->SetColor(1,0,0);
    PlaneX->On();

    PlaneY->SetPlaneOrientationToYAxes();
    PlaneY->SetInteractor(this->Interactor);
    PlaneY->SetInput(vtkImageData::SafeDownCast(reslice->GetOutput()));
    PlaneY->DisplayTextOn();
    PlaneY->RestrictPlaneToVolumeOn();
    PlaneY->SetSliceIndex(static_cast<int>(extent[3]/2));
    //PlaneY->KeyPressActivationOn();
    PlaneY->SetKeyPressActivationValue('y');
    PlaneY->GetPlaneProperty()->SetColor(0,1,0);
    PlaneY->On();

    PlaneZ->SetPlaneOrientationToZAxes();
    PlaneZ->SetInteractor(this->Interactor);
    PlaneZ->SetInput(vtkImageData::SafeDownCast(reslice->GetOutput()));
    PlaneZ->DisplayTextOn();
    PlaneZ->RestrictPlaneToVolumeOn();
    PlaneZ->SetSliceIndex(static_cast<int>(extent[5]/2));
    PlaneZ->SetKeyPressActivationValue('z');
    PlaneZ->GetPlaneProperty()->SetColor(0,0,1);
    PlaneZ->On();

    reslice->Delete();
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
        if (this->PlaneX) this->PlaneX->SetEnabled(Display);
        if (this->PlaneY) this->PlaneY->SetEnabled(Display);
        if (this->PlaneZ) this->PlaneZ->SetEnabled(Display);
}

void vtkMimxImageActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

