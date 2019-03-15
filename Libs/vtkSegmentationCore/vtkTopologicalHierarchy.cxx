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
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkNew.h>
#include <vtkPolyDataCollection.h>
#include <vtkIntArray.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkTopologicalHierarchy);

//----------------------------------------------------------------------------
vtkTopologicalHierarchy::vtkTopologicalHierarchy()
{
  this->InputPolyDataCollection = nullptr;
  vtkSmartPointer<vtkPolyDataCollection> inputPolyData = vtkSmartPointer<vtkPolyDataCollection>::New();
  this->SetInputPolyDataCollection(inputPolyData);

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
  this->SetOutputLevels(nullptr);
}

//----------------------------------------------------------------------------
void vtkTopologicalHierarchy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkIntArray* vtkTopologicalHierarchy::GetOutputLevels()
{
  return this->OutputLevels;
}

//----------------------------------------------------------------------------
bool vtkTopologicalHierarchy::Contains(vtkPolyData* polyOut, vtkPolyData* polyIn)
{
  if (!polyIn || !polyOut)
    {
    vtkErrorMacro("Contains: Empty input parameters!");
    return false;
    }

  double extentOut[6] = {0.0,0.0,0.0,0.0,0.0,0.0};
  polyOut->GetBounds(extentOut);

  double extentIn[6] = {0.0,0.0,0.0,0.0,0.0,0.0};
  polyIn->GetBounds(extentIn);

  if ( extentOut[0] < extentIn[0] - this->ContainConstraintFactor * (extentOut[1]-extentOut[0])
    && extentOut[1] > extentIn[1] + this->ContainConstraintFactor * (extentOut[1]-extentOut[0])
    && extentOut[2] < extentIn[2] - this->ContainConstraintFactor * (extentOut[3]-extentOut[2])
    && extentOut[3] > extentIn[3] + this->ContainConstraintFactor * (extentOut[3]-extentOut[2])
    && extentOut[4] < extentIn[4] - this->ContainConstraintFactor * (extentOut[5]-extentOut[4])
    && extentOut[5] > extentIn[5] + this->ContainConstraintFactor * (extentOut[5]-extentOut[4]) )
    {
    return true;
    }

  return false;
}

//----------------------------------------------------------------------------
void vtkTopologicalHierarchy::Update()
{
  if (!this->InputPolyDataCollection || !this->OutputLevels)
    {
    vtkErrorMacro("Update: Input poly data collection and output int array have to be initialized!");
    return;
    }

  this->OutputLevels->Initialize();
  unsigned int numberOfPolyData = this->InputPolyDataCollection->GetNumberOfItems();

  // Check input polydata collection
  for (unsigned int polyOutIndex=0; polyOutIndex<numberOfPolyData; ++polyOutIndex)
    {
    vtkPolyData* polyOut = vtkPolyData::SafeDownCast(this->InputPolyDataCollection->GetItemAsObject(polyOutIndex));
    if (!polyOut)
      {
      vtkErrorMacro("Update: Input collection contains invalid object at item " << polyOutIndex);
      return;
      }
    }

  std::vector<std::vector<unsigned int> > containedPolyData(numberOfPolyData);
  this->OutputLevels->SetNumberOfComponents(1);
  this->OutputLevels->SetNumberOfTuples(numberOfPolyData);
  this->OutputLevels->FillComponent(0, -1);

  // Step 1: Set level of polydata containing no other polydata to 0
  this->InputPolyDataCollection->InitTraversal();
  for (unsigned int polyOutIndex=0; polyOutIndex<numberOfPolyData; ++polyOutIndex)
    {
    vtkPolyData* polyOut = vtkPolyData::SafeDownCast(this->InputPolyDataCollection->GetItemAsObject(polyOutIndex));

    for (unsigned int polyInIndex=0; polyInIndex<numberOfPolyData; ++polyInIndex)
      {
      if (polyOutIndex==polyInIndex)
        {
        continue;
        }

      vtkPolyData* polyIn = vtkPolyData::SafeDownCast(this->InputPolyDataCollection->GetItemAsObject(polyInIndex));

      if (this->Contains(polyOut, polyIn))
        {
        containedPolyData[polyOutIndex].push_back(polyInIndex);
        }
      }

    if (containedPolyData[polyOutIndex].size() == 0)
      {
      this->OutputLevels->SetValue(polyOutIndex, 0);
      }
    }

  // Step 2: Set level of the polydata containing other polydata to one bigger than the highest contained level
  vtkSmartPointer<vtkIntArray> outputLevelsSnapshot = vtkSmartPointer<vtkIntArray>::New();
  unsigned int currentLevel = 1;
  while (this->OutputContainsEmptyLevels() && currentLevel < this->MaximumLevel)
    {
    // Creating snapshot of the level array state so that the newly set values don't interfere with the check
    // Without this, the check "does all contained polydata have level values assigned" is corrupted
    outputLevelsSnapshot->DeepCopy(this->OutputLevels);

    // Step 3: For all polydata without level value assigned
    for (unsigned int polyOutIndex=0; polyOutIndex<numberOfPolyData; ++polyOutIndex)
      {
      if (this->OutputLevels->GetValue(polyOutIndex) > -1)
        {
        continue;
        }

      // Step 4: If all contained polydata have level values assigned, then set it to the current level value
      //   The level that is to be set cannot be lower than the current level value, because then we would
      //   already have assigned it in the previous iterations.
      bool allContainedPolydataHasLevelValueAssigned = true;
      for (unsigned int polyInIndex = 0;
           polyInIndex < numberOfPolyData;
           ++polyInIndex)
        {
        if (polyOutIndex==polyInIndex)
          {
          continue;
          }
        bool isContained = false;
        for (std::vector<unsigned int>::iterator it = containedPolyData[polyOutIndex].begin();
             it != containedPolyData[polyOutIndex].end();
             ++it)
          {
          if ((*it) == polyInIndex)
            {
            isContained = true;
            break;
            }
          }
        if (!isContained)
          {
          continue;
          }

        if (outputLevelsSnapshot->GetValue(polyInIndex) == -1)
          {
          allContainedPolydataHasLevelValueAssigned = false;
          break;
          }
        }
      if (allContainedPolydataHasLevelValueAssigned)
        {
        this->OutputLevels->SetValue(polyOutIndex, currentLevel);
        }
      }

    // Increase current level for the next iteration
    currentLevel++;
    }

  // Step 5: Set maximum level to all polydata that has no level value assigned
  for (unsigned int polyOutIndex=0; polyOutIndex<numberOfPolyData; ++polyOutIndex)
    {
    if (this->OutputLevels->GetValue(polyOutIndex) == -1)
      {
      this->OutputLevels->SetValue(polyOutIndex, this->MaximumLevel);
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

  for (int i=0; i<this->OutputLevels->GetNumberOfTuples(); ++i)
    {
    if (this->OutputLevels->GetValue(i) == -1)
      {
      return true;
      }
    }

  return false;
}
