/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
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
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkVersion.h>

//----------------------------------------------------------------------------
int vtkDiffusionTensorMathematicsTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  vtkMultiThreader::SetGlobalMaximumNumberOfThreads(1);
  // Generate a 2x2x2 tensor image with identity at each voxel
  vtkNew<vtkImageData> tensorImage;
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

  // Generate mask
  // Generate a 2x2x2 tensor image with identity at each voxel
  vtkNew<vtkImageData> maskImage;
  maskImage->SetDimensions(dimensions);
  maskImage->SetSpacing(1.5, 10., 100.);
  maskImage->SetOrigin(-10., 40, 0.1);
  maskImage->AllocateScalars(VTK_SHORT, 9);

  short* maskPtr = reinterpret_cast<short*>(maskImage->GetScalarPointer());
  for (int z=0; z < dimensions[2]; ++z )
    {
    for (int y=0; y < dimensions[1]; ++y )
      {
      for (int x=0; x < dimensions[0]; ++x )
        {
        *maskPtr++ = 0;
        }
      }
    }
  // Mask the 2nd voxel
  maskPtr = reinterpret_cast<short*>(maskImage->GetScalarPointer());
  maskPtr[1] = 1;

  // Execute the filter
  vtkNew<vtkDiffusionTensorMathematics> filter;
  filter->SetInputData(tensorImage.GetPointer());
  filter->SetScalarMask(maskImage.GetPointer());
  filter->SetMaskLabelValue(0);  // mask all the labels different from 0
  filter->SetMaskWithScalars(1); // turn on masking
  for (int i = vtkDiffusionTensorMathematics::VTK_TENS_TRACE;
       i <=vtkDiffusionTensorMathematics::VTK_TENS_MEAN_DIFFUSIVITY;
       ++i)
    {
    filter->SetOperation(i);
    filter->Update();

    std::cout << "Operation " << i << ":" << std::endl;
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
    std::cout << std::endl << std::endl;
    }
  return EXIT_SUCCESS;
}
