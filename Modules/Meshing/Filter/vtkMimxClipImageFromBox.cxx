/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxClipImageFromBox.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.7 $

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


#include "vtkMimxClipImageFromBox.h"
#include "vtkProperty.h"
#include "vtkTransform.h"
#include "vtkActor.h"
#include "vtkCellPicker.h"
#include "vtkPolyData.h"

#include "itkResampleImageFilter.h"
#include "itkAffineTransform.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkImageFileWriter.h"

vtkMimxClipImageFromBox::vtkMimxClipImageFromBox()
{
        BoxWidget = vtkBoxWidget::New();
        PlaneX = vtkImagePlaneWidget::New();
        PlaneY = vtkImagePlaneWidget::New();
        PlaneZ = vtkImagePlaneWidget::New();
        CellPicker = vtkCellPicker::New();
        CellPicker->SetTolerance(0.005);
        Reader = ReaderType::New();
        Filter = FilterType::New();
        ResampleFilter = ResampleFilterType::New();
        PWmode = 1;
        BWMode = 0;
        IpMode = 1;
        FName = NULL;
        IpImage = NULL;
}

vtkMimxClipImageFromBox::~vtkMimxClipImageFromBox()
{
        BoxWidget->Delete();
        PlaneX->Delete();
        PlaneY->Delete();
        PlaneZ->Delete();
        CellPicker->Delete();
}

void vtkMimxClipImageFromBox::DisplayWidgets()
{
        if(IpMode)
          {
                if (FName)
                  {
                        Reader->SetFileName(FName);
                        Reader->Update();
                        IpImage = Reader->GetOutput();
                  }
          }
        else
          {
//              if(IpImage);
          }

        ImageType::SizeType size = 
                IpImage->GetLargestPossibleRegion().GetSize();
        Filter->SetInput(IpImage);
        Filter->Update();
        // setting up the bounding box
        BoxWidget->PlaceWidget(Filter->GetOutput()->GetBounds());
        BoxWidget->SetInput(Filter->GetOutput());
        BoxWidget->SetInteractor(Interactor);
        // setting up the imagepanewidget
        PlaneX->SetInput(Filter->GetOutput());
        PlaneX->DisplayTextOn();
        PlaneX->SetPlaneOrientationToXAxes();
        PlaneX->RestrictPlaneToVolumeOn();
        PlaneX->SetSliceIndex(static_cast<int>(size[0]/2));
        PlaneX->SetPicker(CellPicker);
        PlaneX->SetKeyPressActivationValue('x');
        PlaneX->GetPlaneProperty()->SetColor(1,0,0);
        PlaneX->SetInteractor(Interactor);
        PlaneX->On();


        PlaneY->SetInput(Filter->GetOutput());
        PlaneY->DisplayTextOn();
        PlaneY->SetPlaneOrientationToYAxes();
        PlaneY->RestrictPlaneToVolumeOn();
        PlaneY->SetSliceIndex(static_cast<int>(size[1]/2));
        PlaneY->SetPicker(CellPicker);
        PlaneY->SetKeyPressActivationValue('y');
        PlaneY->GetPlaneProperty()->SetColor(0,1,0);
        PlaneY->SetInteractor(Interactor);
        PlaneY->On();


        PlaneZ->SetInput(Filter->GetOutput());
        PlaneZ->DisplayTextOn();
        PlaneZ->SetPlaneOrientationToZAxes();
        PlaneZ->RestrictPlaneToVolumeOn();
        PlaneZ->SetSliceIndex(static_cast<int>(size[2]/2));
        PlaneZ->SetPicker(CellPicker);
        PlaneZ->SetKeyPressActivationValue('z');
        PlaneZ->GetPlaneProperty()->SetColor(0,0,1);
        PlaneZ->SetInteractor(Interactor);
        PlaneZ->On();

        EnableImagePlaneWidget(0);
}

void vtkMimxClipImageFromBox::SetInput(ImageType* InputImage)
{
        IpMode = 0;
        this->IpImage = InputImage;
}

void vtkMimxClipImageFromBox::SetInteractor(vtkRenderWindowInteractor *Iren)
{
        this->Interactor = Iren;
}

void vtkMimxClipImageFromBox::SetFileName(const char *FileName)
{
        this->FName = FileName;
}

void vtkMimxClipImageFromBox::EnableBoxWidget(int BoxWidgetMode)
{
        this->BWMode = BoxWidgetMode;
        if(BWMode)      BoxWidget->SetEnabled(1);
        else    BoxWidget->SetEnabled(0);
}
void vtkMimxClipImageFromBox::EnableImagePlaneWidget(int PlaneWidgetMode)
{
        this->PWmode = PlaneWidgetMode;
        if (PWmode)
          {
                PlaneX->InteractionOn();
                PlaneY->InteractionOn();
                PlaneZ->InteractionOn();
          }
        else
    {
                PlaneX->InteractionOff();
                PlaneY->InteractionOff();
                PlaneZ->InteractionOff();
          }
}
void vtkMimxClipImageFromBox::ClipImage()
{

  typedef itk::AffineTransform< double, 3 >  TransformType;
  TransformType::Pointer transform = TransformType::New();

  typedef itk::LinearInterpolateImageFunction< 
  ImageType, double >  InterpolatorType;
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  ResampleFilter->SetInterpolator( interpolator );
  ResampleFilter->SetInput(Reader->GetOutput());
  ImageType::SizeType size = Reader->GetOutput()->GetLargestPossibleRegion().GetSize();
  ImageType::SpacingType spacing = Reader->GetOutput()->GetSpacing();
  ImageType::PointType origin = Reader->GetOutput()->GetOrigin();

  vtkPolyData *polydata = vtkPolyData::New();
  vtkPoints *points = vtkPoints::New();
  BoxWidget->GetPolyData(polydata);
  points->DeepCopy(polydata->GetPoints());

  TransformType::OutputVectorType translation1;
  double x[3];
  points->GetPoint(14,x);
  const double imageCenterX = x[0];
  const double imageCenterY = x[1];
  const double imageCenterZ = 0;

  translation1[0] = -imageCenterX; translation1[1] = -imageCenterY;
  translation1[2] = -imageCenterZ;
  transform->Translate( translation1 );
  // const double degreesToRadians = atan(1.0) / 45.0;
  TransformType::OutputVectorType rotation;
  rotation[0]=0;
  rotation[1]=0;
  rotation[2]=1;

  double pt1[3], pt2[3];
  points->GetPoint(10,pt1);
  points->GetPoint(11,pt2);

  double angle;
  if (pt2[0] >= pt1[0])
    {
    angle = acos((pt2[0]-pt1[0])
    /sqrt(pow(pt2[0]-pt1[0],2.0)+pow(pt2[1]-pt1[1],2.0)));
    angle = 3.1416/2.0 - angle;
    }
  else
    {
    angle = acos((pt1[0]-pt2[0])
    /sqrt(pow(pt2[0]-pt1[0],2.0)+pow(pt2[1]-pt1[1],2.0)));
    angle = angle - 3.1416/2.0;
    }

  transform->Rotate3D(rotation,-angle,false);
  TransformType::OutputVectorType translation2;
  translation2[0] =   imageCenterX;
  translation2[1] =   imageCenterY;
  translation2[2] =   imageCenterZ;
  transform->Translate( translation2, false );
  ResampleFilter->SetTransform( transform );
  vtkTransform* trans1 = vtkTransform::New();
  vtkPoints* mPointsStore1 = vtkPoints::New();
  trans1->PostMultiply();
  trans1->Translate(-imageCenterX, -imageCenterY, 0.0);
  trans1->RotateZ(angle*180.0/3.1416);
  trans1->Translate(imageCenterX, imageCenterY, 0.0);
  trans1->TransformPoints(points,mPointsStore1);

  double bounds[6];
  mPointsStore1->GetBounds(bounds);
  //ITK Allows negative spacing - Handle in size calculations
  size[0] = static_cast<int>((bounds[1]-bounds[0])/fabs(spacing[0]) + 0.5);
  size[1] = static_cast<int>((bounds[3]-bounds[2])/fabs(spacing[1]) + 0.5);
  size[2] = static_cast<int>((bounds[5]-bounds[4])/fabs(spacing[2]) + 0.5);
  ResampleFilter->SetSize(size);
  origin[0] = bounds[0];
  origin[1] = bounds[2];
  origin[2] = bounds[4];
  ResampleFilter->SetOutputOrigin(origin);
  ResampleFilter->SetOutputSpacing(spacing);
  ResampleFilter->Update();
}

ImageType* vtkMimxClipImageFromBox::GetitkImageOutput()
{
        return ResampleFilter->GetOutput();
}

void vtkMimxClipImageFromBox::ShowWidget(int VisibleMode)
{
        this->VisMode = VisibleMode;
        if(VisMode)
          {
                PlaneX->On();
                PlaneY->On();
                PlaneZ->On();
                BoxWidget->SetEnabled(1);
          } 
        else
    {
                PlaneX->Off();
                PlaneY->Off();
                PlaneZ->Off();
                BoxWidget->SetEnabled(0);
          }
}

ImageType* vtkMimxClipImageFromBox::GetitkImageInput()
{
        return this->IpImage;
}
