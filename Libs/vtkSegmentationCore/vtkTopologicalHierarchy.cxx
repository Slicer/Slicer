/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "vtkTopologicalHierarchy.h"

// VTK includes
#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataCollection.h>
#include <vtkSmartPointer.h>

// STD includes
#include <array>
#include <vector>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkTopologicalHierarchy);

//----------------------------------------------------------------------------
vtkTopologicalHierarchy::vtkTopologicalHierarchy()
{
  this->InputPolyDataCollection = nullptr;
  vtkSmartPointer<vtkPolyDataCollection> inputPolyData = vtkSmartPointer<vtkPolyDataCollection>::New();
  this->SetInputPolyDataCollection(inputPolyData);

  this->InputBounds = nullptr;

  this->OutputLevels = nullptr;
  vtkSmartPointer<vtkIntArray> outputLevels = vtkSmartPointer<vtkIntArray>::New();
  this->SetOutputLevels(outputLevels);

  this->ContainConstraintFactor = 0.0;

  this->MaximumLevel = 7;
}

//----------------------------------------------------------------------------
vtkTopologicalHierarchy::~vtkTopologicalHierarchy()
{
  this->SetInputPolyDataCollection(nullptr);
  this->SetInputBounds(nullptr);
  this->SetOutputLevels(nullptr);
}

//----------------------------------------------------------------------------
void vtkTopologicalHierarchy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ContainConstraintFactor: " << this->ContainConstraintFactor << "\n";
  os << indent << "MaximumLevel: " << this->MaximumLevel << "\n";
  os << indent << "InputPolyDataCollection: " << (this->InputPolyDataCollection ? this->InputPolyDataCollection->GetNumberOfItems() : 0) << " items\n";
  os << indent << "InputBounds: " << (this->InputBounds ? this->InputBounds->GetNumberOfTuples() : 0) << " items\n";
}

//----------------------------------------------------------------------------
vtkIntArray* vtkTopologicalHierarchy::GetOutputLevels()
{
  return this->OutputLevels;
}

//----------------------------------------------------------------------------
bool vtkTopologicalHierarchy::Contains(const double boundsOut[6], const double boundsIn[6])
{
  if (!boundsIn || !boundsOut)
  {
    vtkErrorMacro("Contains: Empty input parameters!");
    return false;
  }
  // Empty bounds (min > max) neither contain nor are contained
  for (int axis = 0; axis < 3; ++axis)
  {
    if (boundsOut[axis * 2] > boundsOut[axis * 2 + 1] || boundsIn[axis * 2] > boundsIn[axis * 2 + 1])
    {
      return false;
    }
  }

  if (boundsOut[0] < boundsIn[0] - this->ContainConstraintFactor * (boundsOut[1] - boundsOut[0])    //
      && boundsOut[1] > boundsIn[1] + this->ContainConstraintFactor * (boundsOut[1] - boundsOut[0]) //
      && boundsOut[2] < boundsIn[2] - this->ContainConstraintFactor * (boundsOut[3] - boundsOut[2]) //
      && boundsOut[3] > boundsIn[3] + this->ContainConstraintFactor * (boundsOut[3] - boundsOut[2]) //
      && boundsOut[4] < boundsIn[4] - this->ContainConstraintFactor * (boundsOut[5] - boundsOut[4]) //
      && boundsOut[5] > boundsIn[5] + this->ContainConstraintFactor * (boundsOut[5] - boundsOut[4]))
  {
    return true;
  }

  return false;
}

//----------------------------------------------------------------------------
bool vtkTopologicalHierarchy::Contains(vtkPolyData* polyOut, vtkPolyData* polyIn)
{
  if (!polyIn || !polyOut)
  {
    vtkErrorMacro("Contains: Empty input parameters!");
    return false;
  }

  double boundsOut[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  polyOut->GetBounds(boundsOut);

  double boundsIn[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  polyIn->GetBounds(boundsIn);

  return this->Contains(boundsOut, boundsIn);
}

//----------------------------------------------------------------------------
void vtkTopologicalHierarchy::Update()
{
  if (!this->OutputLevels)
  {
    vtkErrorMacro("Update: Output int array has to be initialized!");
    return;
  }

  this->OutputLevels->Initialize();

  // Collect the bounding boxes of the input items, either given directly or computed from the poly data
  std::vector<std::array<double, 6>> bounds;
  if (this->InputBounds)
  {
    if (this->InputBounds->GetNumberOfComponents() != 6)
    {
      vtkErrorMacro("Update: Input bounds array must have 6 components (xmin, xmax, ymin, ymax, zmin, zmax)");
      return;
    }
    bounds.resize(this->InputBounds->GetNumberOfTuples());
    for (vtkIdType index = 0; index < this->InputBounds->GetNumberOfTuples(); ++index)
    {
      this->InputBounds->GetTypedTuple(index, bounds[index].data());
    }
  }
  else if (this->InputPolyDataCollection)
  {
    unsigned int numberOfPolyData = this->InputPolyDataCollection->GetNumberOfItems();
    bounds.resize(numberOfPolyData);
    for (unsigned int index = 0; index < numberOfPolyData; ++index)
    {
      vtkPolyData* polyData = vtkPolyData::SafeDownCast(this->InputPolyDataCollection->GetItemAsObject(index));
      if (!polyData)
      {
        vtkErrorMacro("Update: Input collection contains invalid object at item " << index);
        return;
      }
      polyData->GetBounds(bounds[index].data());
    }
  }
  else
  {
    vtkErrorMacro("Update: Input bounds or input poly data collection has to be set!");
    return;
  }

  unsigned int numberOfItems = static_cast<unsigned int>(bounds.size());
  std::vector<std::vector<unsigned int>> containedItems(numberOfItems);
  this->OutputLevels->SetNumberOfComponents(1);
  this->OutputLevels->SetNumberOfTuples(numberOfItems);
  this->OutputLevels->FillComponent(0, -1);

  // Step 1: Set level of items containing no other item to 0
  for (unsigned int outIndex = 0; outIndex < numberOfItems; ++outIndex)
  {
    for (unsigned int inIndex = 0; inIndex < numberOfItems; ++inIndex)
    {
      if (outIndex == inIndex)
      {
        continue;
      }
      if (this->Contains(bounds[outIndex].data(), bounds[inIndex].data()))
      {
        containedItems[outIndex].push_back(inIndex);
      }
    }

    if (containedItems[outIndex].empty())
    {
      this->OutputLevels->SetValue(outIndex, 0);
    }
  }

  // Step 2: Set level of the items containing other items to one bigger than the highest contained level
  vtkSmartPointer<vtkIntArray> outputLevelsSnapshot = vtkSmartPointer<vtkIntArray>::New();
  unsigned int currentLevel = 1;
  while (this->OutputContainsEmptyLevels() && currentLevel < this->MaximumLevel)
  {
    // Creating snapshot of the level array state so that the newly set values don't interfere with the check
    // Without this, the check "does all contained items have level values assigned" is corrupted
    outputLevelsSnapshot->DeepCopy(this->OutputLevels);

    // Step 3: For all items without level value assigned
    for (unsigned int outIndex = 0; outIndex < numberOfItems; ++outIndex)
    {
      if (this->OutputLevels->GetValue(outIndex) > -1)
      {
        continue;
      }

      // Step 4: If all contained items have level values assigned, then set it to the current level value
      //   The level that is to be set cannot be lower than the current level value, because then we would
      //   already have assigned it in the previous iterations.
      bool allContainedItemsHaveLevelValueAssigned = true;
      for (unsigned int inIndex : containedItems[outIndex])
      {
        if (outputLevelsSnapshot->GetValue(inIndex) == -1)
        {
          allContainedItemsHaveLevelValueAssigned = false;
          break;
        }
      }
      if (allContainedItemsHaveLevelValueAssigned)
      {
        this->OutputLevels->SetValue(outIndex, currentLevel);
      }
    }

    // Increase current level for the next iteration
    currentLevel++;
  }

  // Step 5: Set maximum level to all items that have no level value assigned
  for (unsigned int outIndex = 0; outIndex < numberOfItems; ++outIndex)
  {
    if (this->OutputLevels->GetValue(outIndex) == -1)
    {
      this->OutputLevels->SetValue(outIndex, this->MaximumLevel);
    }
  }
}

//----------------------------------------------------------------------------
bool vtkTopologicalHierarchy::OutputContainsEmptyLevels()
{
  if (!this->OutputLevels)
  {
    return false;
  }

  for (int i = 0; i < this->OutputLevels->GetNumberOfTuples(); ++i)
  {
    if (this->OutputLevels->GetValue(i) == -1)
    {
      return true;
    }
  }

  return false;
}
