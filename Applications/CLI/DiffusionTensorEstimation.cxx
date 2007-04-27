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

#include "vtkTeemEstimateDiffusionTensor.h"
#include "vtkMatrix4x4.h"
#include "vtkNRRDReader.h"
#include "vtkNRRDWriter.h"
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkImageData.h"
#include "vtkDoubleArray.h"

#include "DiffusionTensorEstimationCLP.h"


int main( int argc, const char * argv[] )
{

  PARSE_ARGS;

  vtkNRRDReader *reader = vtkNRRDReader::New();
  reader->SetFileName(inputVolume.c_str());
  reader->Update();

  vtkDoubleArray *bValues = vtkDoubleArray::New();
  vtkDoubleArray *grads = vtkDoubleArray::New();
  vtkMRMLNRRDStorageNode *helper = vtkMRMLNRRDStorageNode::New();

  if ( !helper->ParseDiffusionInformation(reader,grads,bValues) )
    {
    std::cerr << argv[0] << ": Error parsing Diffusion information" << std::endl;
    bValues->Delete();
    grads->Delete();
    reader->Delete();
    helper->Delete();
    return EXIT_FAILURE;
    }

  vtkTeemEstimateDiffusionTensor *estim = vtkTeemEstimateDiffusionTensor::New();

  estim->SetInput(reader->GetOutput());
  estim->SetNumberOfGradients(grads->GetNumberOfTuples());
  estim->SetDiffusionGradients(grads);
  estim->SetBValues(bValues);

  // Compute Transformation that brings the gradients to ijk
  double *sp = reader->GetOutput()->GetSpacing();
  vtkMatrix4x4 *mf = vtkMatrix4x4::New();
  mf->DeepCopy(reader->GetMeasurementFrameMatrix());
  vtkMatrix4x4 *rasToIjk = vtkMatrix4x4::New();
  rasToIjk->DeepCopy(reader->GetRasToIjkMatrix());

  //Set Translation to zero
  for (int i=0;i<3;i++)
    {
    rasToIjk->SetElement(i,3,0);
    }
  vtkMatrix4x4 *scale = vtkMatrix4x4::New();
  scale->Identity();
  scale->SetElement(0,0,sp[0]);
  scale->SetElement(1,1,sp[1]);
  scale->SetElement(2,2,sp[2]);

  vtkTransform *trans = vtkTransform::New();
  trans->PostMultiply();
  trans->SetMatrix(mf);
  trans->Concatenate(rasToIjk);
  trans->Concatenate(scale);
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

  //Compute IjkToRas (used by Writer)
  reader->GetRasToIjkMatrix()->Invert();
  
  //Save tensor
  vtkNRRDWriter *writer = vtkNRRDWriter::New();
  writer->SetInput(estim->GetOutput());
  writer->SetFileName( outputTensor.c_str() );
  writer->UseCompressionOn();
  writer->SetIJKToRASMatrix( reader->GetRasToIjkMatrix() );
  rasToIjk->Invert();
  writer->SetMeasurementFrameMatrix( rasToIjk );
  writer->Write();

  writer->Delete();

  //Save baseline
  vtkNRRDWriter *writer2 = vtkNRRDWriter::New();
  writer2->SetInput(estim->GetBaseline());
  writer2->SetFileName( outputBaseline.c_str() );
  writer2->UseCompressionOn();
  writer2->SetIJKToRASMatrix( reader->GetRasToIjkMatrix() );
  writer2->Write();
  
  // Deleting objects
  //bValues->Delete();
  //grads->Delete();
  //helper->Delete();
  //mf->Delete();
  //rasToIjk->Delete();
  //scale->Delete();
  //trans->Delete();
  //estim->Delete();
  //writer2->Delete();
  //reader->Delete();


  return EXIT_SUCCESS;
}
