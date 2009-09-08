#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include <iostream>
#include <algorithm>
#include <string>

#include "itkPluginFilterWatcher.h"
#include "itkPluginUtilities.h"
#include "vtkSmartPointer.h"
#include "vtkMatrix4x4.h"
#include "vtkNRRDReader.h"
#include "vtkNRRDWriter.h"
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMath.h"
#include "vtkDoubleArray.h"
#include "vtkTensorMask.h"
#include "vtkTeemEstimateDiffusionTensor.h"

#include "vtkImageData.h"
#include "vtkImageCast.h"
#include "vtkImageSeedConnectivity.h"
#include "vtkImageConnectivity.h"
#include "vtkITKNewOtsuThresholdImageFilter.h"

#include "DiffusionTensorEstimationCLP.h"

int main( int argc, char * argv[] )
  {

  PARSE_ARGS;
    {
    vtkSmartPointer<vtkNRRDReader> reader =
      vtkSmartPointer<vtkNRRDReader>::New();
    reader->SetFileName(inputVolume.c_str());
    reader->Update();
    if ( reader->GetReadStatus() )
      {
      std::cerr << argv[0] << ": Error reading Diffusion file" << std::endl;
      return EXIT_FAILURE;
      }

    vtkSmartPointer<vtkDoubleArray> bValues =
      vtkSmartPointer<vtkDoubleArray>::New();
    vtkSmartPointer<vtkDoubleArray> grads =
      vtkSmartPointer<vtkDoubleArray>::New();
    vtkSmartPointer<vtkMRMLNRRDStorageNode> helper =
      vtkSmartPointer<vtkMRMLNRRDStorageNode>::New();

    if ( !helper->ParseDiffusionInformation(reader,grads,bValues) )
      {
      std::cerr << argv[0] << ": Error parsing Diffusion information" << std::endl;
      return EXIT_FAILURE;
      }
    vtkSmartPointer<vtkTeemEstimateDiffusionTensor> estim =
      vtkSmartPointer<vtkTeemEstimateDiffusionTensor>::New();

    estim->SetInput(reader->GetOutput());
    estim->SetNumberOfGradients(grads->GetNumberOfTuples());
    estim->SetDiffusionGradients(grads);
    estim->SetBValues(bValues);
    estim->SetShiftNegativeEigenvalues(ShiftNegativeEigenvalues);

    // Compute Transformation that brings the gradients to ijk
    // double *sp = reader->GetOutput()->GetSpacing();
    vtkSmartPointer<vtkMatrix4x4> mf =
      vtkSmartPointer<vtkMatrix4x4>::New();
    mf->DeepCopy(reader->GetMeasurementFrameMatrix());
    vtkSmartPointer<vtkMatrix4x4> rasToIjkRotation =
      vtkSmartPointer<vtkMatrix4x4>::New();
    rasToIjkRotation->DeepCopy(reader->GetRasToIjkMatrix());

    //Set Translation to zero
    for (int i=0;i<3;i++)
      {
      rasToIjkRotation->SetElement(i,3,0);
      }
    //Remove scaling in rasToIjk to make a real roation matrix
    double col[3];
    for (int jjj = 0; jjj < 3; jjj++) 
      {
      for (int iii = 0; iii < 3; iii++)
        {
        col[iii]=rasToIjkRotation->GetElement(iii,jjj);
        }
      vtkMath::Normalize(col);
      for (int iii = 0; iii < 3; iii++)
        {
        rasToIjkRotation->SetElement(iii,jjj,col[iii]);
        }  
      }

    vtkSmartPointer<vtkTransform> trans =
      vtkSmartPointer<vtkTransform>::New();
    trans->PostMultiply();
    trans->SetMatrix(mf);
    trans->Concatenate(rasToIjkRotation);
    trans->Update();

    estim->SetTransform(trans);
    if (estimationMethod == std::string("LS"))
      {
      estim->SetEstimationMethodToLLS();
      }
    else if(estimationMethod == std::string("WLS"))
      {
      estim->SetEstimationMethodToWLS();
      }
    else if (estimationMethod == std::string("NL"))
      {
      estim->SetEstimationMethodToNLS();
      }
    estim->Update();
    vtkImageData *tensorImage = estim->GetOutput();
    tensorImage->GetPointData()->SetScalars(NULL);

    //compute tenor mask

    vtkSmartPointer<vtkITKNewOtsuThresholdImageFilter> otsu =
      vtkSmartPointer<vtkITKNewOtsuThresholdImageFilter>::New();
    otsu->SetInput(estim->GetBaseline());
    otsu->SetOmega (1 + otsuOmegaThreshold);
    otsu->SetOutsideValue(1);
    otsu->SetInsideValue(0);
    otsu->Update();

    vtkSmartPointer<vtkImageData> mask = vtkSmartPointer<vtkImageData>::New();
    mask->DeepCopy(otsu->GetOutput());

    int *dims = mask->GetDimensions();
    int px = dims[0]/2;
    int py = dims[1]/2;
    int pz = dims[2]/2;

    vtkSmartPointer<vtkImageCast> cast =
      vtkSmartPointer<vtkImageCast>::New();
    cast->SetInput(mask);
    cast->SetOutputScalarTypeToUnsignedChar();
    cast->Update();

    vtkSmartPointer<vtkImageSeedConnectivity> con =
      vtkSmartPointer<vtkImageSeedConnectivity>::New();
    con->SetInput(cast->GetOutput());
    con->SetInputConnectValue(1);
    con->SetOutputConnectedValue(1);
    con->SetOutputUnconnectedValue(0);
    con->AddSeed(px, py, pz);
    con->Update();

    vtkSmartPointer<vtkImageCast> cast1 =
      vtkSmartPointer<vtkImageCast>::New();
    cast1->SetInput(con->GetOutput());
    cast1->SetOutputScalarTypeToShort();
    cast1->Update();


    vtkSmartPointer<vtkImageConnectivity> conn =
      vtkSmartPointer<vtkImageConnectivity>::New();
    if (removeIslands)  
      {
      conn->SetBackground(1);
      conn->SetMinForeground( -32768);
      conn->SetMaxForeground( 32767);
      conn->SetFunctionToRemoveIslands();
      conn->SetMinSize(10000);
      conn->SliceBySliceOn();
      conn->SetInput(cast1->GetOutput());
      conn->Update();
      } 

    // Maks tensor
    //TODO: fix tenosr mask
    /**/
    vtkSmartPointer<vtkTensorMask> tensorMask =
      vtkSmartPointer<vtkTensorMask>::New();
    tensorMask->SetNumberOfThreads(1);
    vtkSmartPointer<vtkImageCast> cast2 =
      vtkSmartPointer<vtkImageCast>::New();
    cast2->SetOutputScalarTypeToUnsignedChar();
    if (applyMask)
      {  

      tensorMask->SetMaskAlpha(0.0);
      tensorMask->SetInput(tensorImage);
      if (removeIslands)  
        {
        cast2->SetInput(conn->GetOutput());
        }
      else
        {
        cast2->SetInput(cast1->GetOutput());
        }
      tensorMask->SetMaskInput(cast2->GetOutput());
      tensorMask->Update();
      tensorImage = tensorMask->GetOutput();
      }
    /**/

    //Save tensor
    vtkSmartPointer<vtkNRRDWriter> writer =
      vtkSmartPointer<vtkNRRDWriter>::New();
    tensorImage->GetPointData()->SetScalars(NULL);
    writer->SetInput(tensorImage);
    writer->SetFileName( outputTensor.c_str() );
    writer->UseCompressionOn();
    //Compute IjkToRas (used by Writer)
    reader->GetRasToIjkMatrix()->Invert();
    writer->SetIJKToRASMatrix( reader->GetRasToIjkMatrix() );
    //Compute measurement frame: Take into account that we have transformed
    //the gradients so tensor components are defined in ijk.
    rasToIjkRotation->Invert();
    writer->SetMeasurementFrameMatrix( rasToIjkRotation );
    writer->Write();

    //Save baseline
    vtkSmartPointer<vtkNRRDWriter> writer2 =
      vtkSmartPointer<vtkNRRDWriter>::New();
    writer2->SetInput(estim->GetBaseline());
    writer2->SetFileName( outputBaseline.c_str() );
    writer2->UseCompressionOn();
    writer2->SetIJKToRASMatrix( reader->GetRasToIjkMatrix() );
    writer2->Write();

    //Save mask
    vtkSmartPointer<vtkNRRDWriter> writer3 =
      vtkSmartPointer<vtkNRRDWriter>::New();
    if (removeIslands)  
      {
      writer3->SetInput(conn->GetOutput());
      }
    else
      {
      writer3->SetInput(cast1->GetOutput());
      }
    writer3->SetFileName( thresholdMask.c_str() );
    writer3->UseCompressionOn();
    writer3->SetIJKToRASMatrix( reader->GetRasToIjkMatrix() );
    writer3->Write();

    }

    return EXIT_SUCCESS;
  }
