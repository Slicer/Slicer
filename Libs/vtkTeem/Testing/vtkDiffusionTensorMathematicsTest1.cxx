/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRMLDisplayableManager includes
#include <vtkDiffusionTensorMathematics.h>

// VTK includes
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkMultiThreader.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>

// STD includes
#include <cstdlib>

//----------------------------------------------------------------------------
int vtkDiffusionTensorMathematicsTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  vtkMultiThreader::SetGlobalMaximumNumberOfThreads(2);
  // Generate a 2x2x2 tensor image with identity at each voxel
  vtkSmartPointer<vtkImageData> tensorImage = vtkSmartPointer<vtkImageData>::New();
  int dimensions[3] = {2, 2, 2};
  tensorImage->SetDimensions(dimensions);
  tensorImage->SetSpacing(1.5, 10., 100.);
  tensorImage->SetOrigin(-10., 40, 0.1);

  vtkDataArray *scalars = vtkDataArray::CreateDataArray(VTK_FLOAT);
  scalars->SetNumberOfComponents(9);
  scalars->SetName("tensors");
  scalars->SetNumberOfTuples(dimensions[0]*dimensions[1]*dimensions[2]);

  tensorImage->GetPointData()->SetTensors(scalars);
  scalars->Delete();

  float* ptr = reinterpret_cast<float*>(scalars->GetVoidPointer(0));
  for (int z=0; z < dimensions[2]; ++z )
    {
    for (int y=0; y < dimensions[1]; ++y )
      {
      for (int x=0; x < dimensions[0]; ++x )
        {
        ptr[0] = ptr[4] = ptr[8] = 1.f;
        ptr[1] = ptr[2] = ptr[3] = ptr[5] = ptr[6] = ptr[7] = 0.f;
        ptr+=9;
        }
      }
    }
  ptr = reinterpret_cast<float*>(scalars->GetVoidPointer(0));
  // Change the last element of the last tensor to 2.f (instead of 1.f)
  // so the trace would be 4.f not 3.f
  ptr[dimensions[0]*dimensions[1]*dimensions[2]*9-1] = 2.f;

  // Execute the filter
  vtkSmartPointer<vtkDiffusionTensorMathematics> filter =
    vtkSmartPointer<vtkDiffusionTensorMathematics>::New();
  filter->SetOperationToTrace();
  filter->SetInput(tensorImage);
  filter->Update();

  // Checkout the results
  vtkImageData* output = filter->GetOutput();
  ptr = reinterpret_cast<float*>(output->GetScalarPointer());
  for (int z=0; z < dimensions[2]; ++z )
    {
    for (int y=0; y < dimensions[1]; ++y )
      {
      for (int x=0; x < dimensions[0]; ++x )
        {
        std::cout << '(' << x << ',' << y << ',' << z << ")=" << *ptr << std::endl;
        ++ptr;
        }
      }
    }
  return EXIT_SUCCESS;
}
