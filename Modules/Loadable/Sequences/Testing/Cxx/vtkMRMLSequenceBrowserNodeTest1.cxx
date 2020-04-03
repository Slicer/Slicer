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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLSequenceBrowserNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkNew.h>

double valueForIndex(int i)
{
  return i * 1234.567890;
}

int vtkMRMLSequenceBrowserNodeTest1(int argc, char* argv[])
{
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLSequenceNode> sequenceNode;
  const int numberOfDataNodes = 135;
  for (int i = 0; i < numberOfDataNodes; i++)
    {
    vtkNew<vtkMRMLTransformNode> transform;
    std::string indexValue = vtkVariant(valueForIndex(i)).ToString();
    sequenceNode->SetDataNodeAtValue(transform.GetPointer(), indexValue);
    }
  scene->AddNode(sequenceNode.GetPointer());

  vtkNew<vtkMRMLSequenceBrowserNode> browserNode;
  scene->AddNode(browserNode.GetPointer());
  browserNode->SetAndObserveMasterSequenceNodeID(sequenceNode->GetID());

  int numberOfDecimals = 5;
  std::string prefix = "%%x%t%y";
  std::stringstream formatSS;
  formatSS << "%." << numberOfDecimals << "f";
  std::string format = formatSS.str();
  std::string suffix = "%s";

  // Basic sprintf string "%.5f"
  browserNode->SetIndexDisplayFormat(format);
  for (int i = 0; i < sequenceNode->GetNumberOfDataNodes(); ++i)
    {
    std::string formattedIndexValue = browserNode->GetFormattedIndexValue(i);
    std::stringstream expectedFormatSS;
    expectedFormatSS.precision(numberOfDecimals);

    // Perform same conversion as sequence browser node (string to float with vtkVariant)
    expectedFormatSS << std::fixed << vtkVariant(vtkVariant(valueForIndex(i)).ToString()).ToFloat();
    std::string expectedFormat = expectedFormatSS.str();

    CHECK_STD_STRING(formattedIndexValue, expectedFormat);
    }

  // Basic sprintf string "%s"
  browserNode->SetIndexDisplayFormat("%s");
  for (int i = 0; i < sequenceNode->GetNumberOfDataNodes(); ++i)
    {
    std::string formattedIndexValue = browserNode->GetFormattedIndexValue(i);
    std::string expectedFormat = sequenceNode->GetNthIndexValue(i);
    CHECK_STD_STRING(formattedIndexValue, expectedFormat);
    }

  // Complex sprintf string ""%%x%t%y%.7f%.7f%s". Should only match with first "%.7f"
  numberOfDecimals = 7;
  std::stringstream formatSS2;
  formatSS2 << "%." << numberOfDecimals << "f";
  format = formatSS2.str();
  suffix = format + suffix;
  browserNode->SetIndexDisplayFormat(prefix + format + suffix);
  for (int i = 0; i < sequenceNode->GetNumberOfDataNodes(); ++i)
    {
    std::string formattedIndexValue = browserNode->GetFormattedIndexValue(i);
    std::stringstream expectedFormatSS;
    expectedFormatSS.precision(numberOfDecimals);

    // Perform same conversion as sequence browser node (string to float with vtkVariant)
    expectedFormatSS << std::fixed << prefix << vtkVariant(vtkVariant(valueForIndex(i)).ToString()).ToFloat() << suffix;
    std::string expectedFormat = expectedFormatSS.str();

    CHECK_STD_STRING(formattedIndexValue, expectedFormat);
    }

  return 0;
}
