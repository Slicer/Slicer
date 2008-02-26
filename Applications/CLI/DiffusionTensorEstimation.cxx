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

int main( int argc, const char * argv[] )
  {

  PARSE_ARGS;
    {
    //vtkNRRDReader *reader = vtkNRRDReader::New();
    vtkSmartPointer<vtkNRRDReader> reader = vtkNRRDReader::New();
    reader->SetFileName(inputVolume.c_str());
    reader->Update();

    vtkSmartPointer<vtkDoubleArray> bValues = vtkDoubleArray::New();
    vtkSmartPointer<vtkDoubleArray> grads = vtkDoubleArray::New();
    vtkSmartPointer<vtkMRMLNRRDStorageNode> helper = vtkMRMLNRRDStorageNode::New();

    if ( !helper->ParseDiffusionInformation(reader,grads,bValues) )
      {
      std::cerr << argv[0] << ": Error parsing Diffusion information" << std::endl;
      return EXIT_FAILURE;
      }
    vtkSmartPointer<vtkTeemEstimateDiffusionTensor> estim = vtkTeemEstimateDiffusionTensor::New();

    estim->SetInput(reader->GetOutput());
    estim->SetNumberOfGradients(grads->GetNumberOfTuples());
    estim->SetDiffusionGradients(grads);
    estim->SetBValues(bValues);

    // Compute Transformation that brings the gradients to ijk
    double *sp = reader->GetOutput()->GetSpacing();
    vtkSmartPointer<vtkMatrix4x4> mf = vtkMatrix4x4::New();
    mf->DeepCopy(reader->GetMeasurementFrameMatrix());
    vtkSmartPointer<vtkMatrix4x4> rasToIjkRotation = vtkMatrix4x4::New();
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

    vtkSmartPointer<vtkTransform> trans = vtkTransform::New();
    trans->PostMultiply();
    trans->SetMatrix(mf);
    trans->Concatenate(rasToIjkRotation);
    trans->Update();

    estim->SetTransform(trans);
    if (estimationMethod == std::string("Least Squares"))
      {
      estim->SetEstimationMethodToLLS();
      }
    else if(estimationMethod == std::string("Weighted Least Squares"))
      {
      estim->SetEstimationMethodToWLS();
      }
    else if (estimationMethod == std::string("Non Linear"))
      {
      estim->SetEstimationMethodToNLS();
      }

    estim->Update();
    vtkImageData *tensorImage = estim->GetOutput();
    tensorImage->GetPointData()->SetScalars(NULL);

    //compute tenor mask

    vtkSmartPointer<vtkITKNewOtsuThresholdImageFilter> otsu = vtkITKNewOtsuThresholdImageFilter::New();
    otsu->SetInput(estim->GetBaseline());
    otsu->SetOmega (1 + otsuOmegaThreshold);
    otsu->SetOutsideValue(1);
    otsu->SetInsideValue(0);
    otsu->Update();

    vtkImageData *mask = vtkImageData::New();
    mask->DeepCopy(otsu->GetOutput());

    int *dims = mask->GetDimensions();
    int px = dims[0]/2;
    int py = dims[1]/2;
    int pz = dims[2]/2;

    vtkSmartPointer<vtkImageCast> cast = vtkImageCast::New();
    cast->SetInput(mask);
    cast->SetOutputScalarTypeToUnsignedChar();
    cast->Update();

    vtkSmartPointer<vtkImageSeedConnectivity> con = vtkImageSeedConnectivity::New();
    con->SetInput(cast->GetOutput());
    con->SetInputConnectValue(1);
    con->SetOutputConnectedValue(1);
    con->SetOutputUnconnectedValue(0);
    con->AddSeed(px, py, pz);
    con->Update();

    vtkSmartPointer<vtkImageCast> cast1 = vtkImageCast::New();
    cast1->SetInput(con->GetOutput());
    cast1->SetOutputScalarTypeToShort();
    cast1->Update();


    vtkSmartPointer<vtkImageConnectivity> conn = vtkImageConnectivity::New();
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
    //vtkSmartPointer<vtkTensorMask> tensorMask = vtkTensorMask::New();
    vtkTensorMask *tensorMask = vtkTensorMask::New();
    vtkSmartPointer<vtkImageCast> cast2 = vtkImageCast::New();
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
    vtkSmartPointer<vtkNRRDWriter> writer = vtkNRRDWriter::New();
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
    vtkSmartPointer<vtkNRRDWriter> writer2 = vtkNRRDWriter::New();
    writer2->SetInput(estim->GetBaseline());
    writer2->SetFileName( outputBaseline.c_str() );
    writer2->UseCompressionOn();
    writer2->SetIJKToRASMatrix( reader->GetRasToIjkMatrix() );
    writer2->Write();

    //Save mask
    vtkSmartPointer<vtkNRRDWriter> writer3 = vtkNRRDWriter::New();
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

    //clean up
    if (reader)
      {  
      reader->SetInput(NULL);
      reader->Delete();
      reader = NULL;
      }
    if (estim)
      {
      estim->SetInput(NULL);
      estim->Delete();
      estim = NULL;
      }
    if (otsu)
      {
      otsu->SetInput(NULL);
      otsu->Delete();
      otsu = NULL;
      }
    if (cast)
      {
      cast->SetInput(NULL);
      cast->Delete();
      cast = NULL;
      }
    if (cast1)
      {
      cast1->SetInput(NULL);
      cast1->Delete();
      cast1 = NULL;
      }
    if (cast2)
      {
      cast2->SetInput(NULL);
      cast2->Delete();
      cast2 = NULL;
      }
    if (con)
      {
      con->SetInput(NULL);
      con->Delete();
      con = NULL;
      }
    if (conn)
      {
      conn->SetInput(NULL);
      conn->Delete();
      conn = NULL;
      }
    if (writer)
      {
      writer->SetInput(NULL);
      writer->Delete();
      writer = NULL;
      }
    if (writer2)
      {
      writer2->SetInput(NULL);
      writer2->Delete();
      writer2 = NULL;
      }
    if (writer3)
      {
      writer3->SetInput(NULL);
      writer3->Delete();
      writer3 = NULL;
      }
    if (trans)
      {
      trans->SetInput(NULL);
      trans->Delete();
      trans = NULL;
      }
    if (tensorMask)
      {
      tensorMask->SetInput(NULL);
      tensorMask->Delete();
      tensorMask = NULL;
      }
    if (mask)
      {
      mask->Delete();
      mask = NULL;
      }
    if (bValues)
      {
      bValues->Delete();
      bValues = NULL;
      }
    if (grads)
      {
      grads->Delete();
      grads = NULL;
      } 
    if (helper)
      {
      helper->Delete();
      helper = NULL;
      } 
    if (rasToIjkRotation)
      {
      rasToIjkRotation->Delete();
      rasToIjkRotation = NULL;
      }
    if (mf)
      {
      mf->Delete();
      mf = NULL;
      }
    }

    return EXIT_SUCCESS;
  }
