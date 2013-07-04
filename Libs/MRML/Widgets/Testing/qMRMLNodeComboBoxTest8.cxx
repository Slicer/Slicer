/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Nicole Aucoin, BWH

==============================================================================*/

// QT includes
#include <QAction>
#include <QApplication>
#include <QTimer>

// qMRML includes
#include "qMRMLNodeComboBox.h"
#include "qMRMLSceneModel.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>

// VTK includes
#include "vtkSmartPointer.h"

// STD includes

// test the adding of user menu actions
int qMRMLNodeComboBoxTest8( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  vtkSmartPointer<vtkMRMLScene> scene =  vtkSmartPointer<vtkMRMLScene>::New();

  // a node selector with user actions
  qMRMLNodeComboBox nodeSelector;
  nodeSelector.setAddEnabled(true);
  nodeSelector.setRenameEnabled(true);
  nodeSelector.setBaseName("testing");
  nodeSelector.setNodeTypes(QStringList(QString("vtkMRMLScalarVolumeNode")));
  nodeSelector.setShowHidden(true);
  nodeSelector.setMRMLScene(scene);

  // add some test nodes
  for (int i = 0; i < 5; i++)
    {
    vtkSmartPointer<vtkMRMLScalarVolumeNode> cnode = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
    scene->AddNode(cnode);
    }


  std::cout << "Before adding new actions size policy = " << nodeSelector.sizeAdjustPolicy() << std::endl;

  qMRMLSceneModel *sceneModel = nodeSelector.sceneModel();

  int startingActions = sceneModel->postItems(sceneModel->mrmlSceneItem()).size();

  QAction* action1 = new QAction("Action one",
                                 &nodeSelector);
  nodeSelector.addMenuAction(action1);

  int actionsPlusOne = sceneModel->postItems(sceneModel->mrmlSceneItem()).size();
  if (startingActions + 1 != actionsPlusOne)
    {
    std::cout << "After adding a new user action, new number of actions " << actionsPlusOne << " != " << startingActions << " + 1" << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "After adding a new user action, new number of actions = " << actionsPlusOne << std::endl;
    }

  // test a conflict with one of the default actions
  QAction* action2 = new QAction("Create new type of action that conflicts with create new node", &nodeSelector);
  nodeSelector.addMenuAction(action2);

  actionsPlusOne = sceneModel->postItems(sceneModel->mrmlSceneItem()).size();
  if (startingActions + 1 != actionsPlusOne)
    {
    std::cout << "After adding a second new user action that conflicts with a "
              << "default one, new number of actions "
              << actionsPlusOne << " != " << startingActions << " + 1" << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "After adding a second new user action that conflicts with a "
              << "default one, new number of actions = "
              << actionsPlusOne << std::endl;
    }

  // try adding the same action again
  nodeSelector.addMenuAction(action1);

  actionsPlusOne = sceneModel->postItems(sceneModel->mrmlSceneItem()).size();
  if (startingActions + 1 != actionsPlusOne)
    {
    std::cout << "After adding a duplicate user action, new number of actions "
              << actionsPlusOne << " != " << startingActions << " + 1" << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "After adding a duplicate user action, new number of actions = "
              << actionsPlusOne << std::endl;
    }

  // add a new action with duplicate text
  QAction *action3 = new QAction("Action one", &nodeSelector);
  nodeSelector.addMenuAction(action3);

  actionsPlusOne  = sceneModel->postItems(sceneModel->mrmlSceneItem()).size();
  if (startingActions + 1 != actionsPlusOne)
    {
    std::cout << "After adding a third new user action with duplicate text, "
              << "new number of actions " << actionsPlusOne << " != "
              << startingActions << " + 1" << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "After adding a third new user action with duplicate text, new number of actions = " << actionsPlusOne << std::endl;
    }

  // add a valid action
  QAction *action4 = new QAction("Another action text addition", &nodeSelector);
  nodeSelector.addMenuAction(action4);

  int actionsPlusTwo  = sceneModel->postItems(sceneModel->mrmlSceneItem()).size();
  if (startingActions + 2 != actionsPlusTwo)
    {
    std::cout << "After adding a new user action with valid text, new number of actions " << actionsPlusTwo << " != " << startingActions << " + 2" << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "After adding a third new user action with valid text, new number of actions = " << actionsPlusTwo << std::endl;
    }

  nodeSelector.show();





  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
