/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital) and was supported by The Research Council of Norway
  through the ALive project (grant nr. 311393).

==============================================================================*/

#include "vtkSlicerPluggableMarkupsTestLogic.h"

// Liver Markups MRML includes
#include "vtkMRMLMarkupsTestLineNode.h"

// QTGUI includes
#include <qSlicerApplication.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>

// Markups logic includes
#include <vtkSlicerMarkupsLogic.h>

// Markups MRML includes
#include <vtkMRMLMarkupsDisplayNode.h>

// VTK includes
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerPluggableMarkupsTestLogic);

//---------------------------------------------------------------------------
vtkSlicerPluggableMarkupsTestLogic::vtkSlicerPluggableMarkupsTestLogic()
{

}

//---------------------------------------------------------------------------
vtkSlicerPluggableMarkupsTestLogic::~vtkSlicerPluggableMarkupsTestLogic() = default;

//---------------------------------------------------------------------------
void vtkSlicerPluggableMarkupsTestLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkSlicerPluggableMarkupsTestLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != nullptr);

  vtkMRMLScene *scene = this->GetMRMLScene();

  // Nodes
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLMarkupsTestLineNode>::New());
}

//---------------------------------------------------------------------------
void vtkSlicerPluggableMarkupsTestLogic::ObserveMRMLScene()
{
  if (!this->GetMRMLScene())
    {
    return;
    }

  vtkMRMLApplicationLogic *mrmlAppLogic = this->GetMRMLApplicationLogic();
  if (!mrmlAppLogic)
    {
    vtkErrorMacro("ObserveMRMLScene: invalid MRML Application Logic.") ;
    return;
    }

  vtkMRMLNode* node =
    this->GetMRMLScene()->GetNodeByID(this->GetSelectionNodeID().c_str());
  if (!node)
    {
    vtkErrorMacro("Observe MRMLScene: invalid Selection Node");
    return;
    }

  // If testing is enabled then we register the new markup
  bool isTestingEnabled = qSlicerApplication::testAttribute(qSlicerCoreApplication::AA_EnableTesting);
  if (isTestingEnabled)
    {

    // add known markup types to the selection node
    vtkMRMLSelectionNode *selectionNode =
      vtkMRMLSelectionNode::SafeDownCast(node);
    if (selectionNode)
      {
      // got into batch process mode so that an update on the mouse mode tool
      // bar is triggered when leave it
      this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

      auto testLineNode = vtkSmartPointer<vtkMRMLMarkupsTestLineNode>::New();

      selectionNode->AddNewPlaceNodeClassNameToList(testLineNode->GetClassName(),
                                                    testLineNode->GetAddIcon(),
                                                    testLineNode->GetMarkupType());

      // trigger an update on the mouse mode toolbar
      this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
      }
    }

    this->Superclass::ObserveMRMLScene();
}

//---------------------------------------------------------------------------
void vtkSlicerPluggableMarkupsTestLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  Superclass::OnMRMLSceneNodeAdded(node);
}
