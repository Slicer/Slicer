/*=========================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Benjamin LONG, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

=========================================================================*/

// Qt includes
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>

// Slicer includes
#include "vtkSlicerConfigure.h"

// CTK includes
#include "ctkCallback.h"
#include "ctkEventTranslatorPlayerWidget.h"
#include "ctkQtTestingUtility.h"

// qMRML includes
#include "qMRMLSliceWidget.h"
#include "qMRMLNodeObject.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLColorLogic.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkMultiThreader.h>
#include <vtkNew.h>
#include "qMRMLWidget.h"

// STD includes
#include <cstdlib>
#include <iostream>

namespace
{
//-----------------------------------------------------------------------------
void checkFinalWidgetState(void* data)
  {
  qMRMLSliceWidget* widget = reinterpret_cast<qMRMLSliceWidget*>(data);

  Q_UNUSED(widget);
  }
}

//-----------------------------------------------------------------------------
int qMRMLSliceWidgetEventTranslatorPlayerTest1(int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  QString xmlDirectory = QString(argv[1]) + "/Libs/MRML/Widgets/Testing/";

  // ------------------------
  ctkEventTranslatorPlayerWidget etpWidget;
  ctkQtTestingUtility* testUtility = new ctkQtTestingUtility(&etpWidget);
  etpWidget.setTestUtility(testUtility);

  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->SetMRMLApplicationLogic(applicationLogic);

  vtkNew<vtkMRMLColorLogic> colorLogic;

  // Test case 1
  vtkNew<vtkMRMLScene> scene;
  applicationLogic->SetMRMLScene(scene.GetPointer());
  colorLogic->SetMRMLScene(scene.GetPointer());

  scene->SetURL(argv[2]);
  scene->Connect();

  vtkMRMLSliceNode* redSliceNode = nullptr;
  // search for a red slice node
  std::vector<vtkMRMLNode*> sliceNodes;
  scene->GetNodesByClass("vtkMRMLSliceNode", sliceNodes);

  for (unsigned int i = 0; i < sliceNodes.size(); ++i)
    {
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(sliceNodes[i]);
    if (!strcmp(sliceNode->GetLayoutName(), "Red") )
      {
      redSliceNode = sliceNode;
      break;
      }
    }
  if (!redSliceNode)
    {
    std::cerr << "Scene must contain a valid vtkMRMLSliceNode:" << redSliceNode << std::endl;
    return EXIT_FAILURE;
    }

  // "Red" slice by default
  qMRMLSliceWidget sliceWidget;
  sliceWidget.setMRMLScene(scene.GetPointer());

  sliceWidget.setMRMLSliceNode(redSliceNode);

  etpWidget.addTestCase(&sliceWidget,
                        xmlDirectory + "qMRMLSliceWidgetEventTranslatorPlayerTest1.xml",
                        &checkFinalWidgetState);

  // ------------------------
  if (!app.arguments().contains("-I"))
    {
    QTimer::singleShot(0, &etpWidget, SLOT(play()));
    }

  etpWidget.show();
  return app.exec();
}

