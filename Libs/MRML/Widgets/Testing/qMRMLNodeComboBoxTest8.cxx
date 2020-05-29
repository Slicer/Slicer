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

// Qt includes
#include <QAction>
#include <QApplication>
#include <QTimer>

// Slicer includes
#include "vtkSlicerConfigure.h"

// qMRML includes
#include "qMRMLNodeComboBox.h"
#include "qMRMLSceneModel.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

namespace
{

// --------------------------------------------------------------------------
bool checkActionCount(int line, qMRMLSceneModel* sceneModel, int expected)
{
  int current = sceneModel->postItems(sceneModel->mrmlSceneItem()).size();
  if (current != expected)
    {
    std::cerr << "Line " << line
              << " - After adding a new user action with valid text,"
                 " new number of actions " << current << " != " << expected << std::endl;
    return false;
    }
  return true;
}

} // end of anonymous namespace

// --------------------------------------------------------------------------
// test the adding of user menu actions
int qMRMLNodeComboBoxTest8( int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkNew<vtkMRMLScene> scene;

  // a node selector with user actions
  qMRMLNodeComboBox nodeSelector;
  nodeSelector.setAddEnabled(true);
  nodeSelector.setRenameEnabled(true);
  nodeSelector.setBaseName("testing");
  nodeSelector.setNodeTypes(QStringList(QString("vtkMRMLScalarVolumeNode")));
  nodeSelector.setShowHidden(true);
  nodeSelector.setMRMLScene(scene.GetPointer());

  // add some test nodes
  for (int i = 0; i < 5; i++)
    {
    vtkNew<vtkMRMLScalarVolumeNode> cnode;
    scene->AddNode(cnode.GetPointer());
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
    std::cout << "After adding a fourth new user action with valid text, new number of actions = " << actionsPlusTwo << std::endl;
    }

  // Check if "Create new" and "Create new node as..." actions are added if multiple node types are enabled

  int actionsWithOneNodeType = sceneModel->postItems(sceneModel->mrmlSceneItem()).size();

  // don't use the view node as that has an attribute filter on it
  nodeSelector.setNodeTypes(QStringList()<<"vtkMRMLScalarVolumeNode"<<"vtkMRMLCameraNode");

  // Two new actions should have been added (Create new node; Create new node as...)
  int actionsWithTwoNodeTypes = sceneModel->postItems(sceneModel->mrmlSceneItem()).size();
  if (actionsWithTwoNodeTypes-actionsWithOneNodeType!=2)
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector: 2 new actions are expected for each new node type, but actually " << actionsWithTwoNodeTypes-actionsWithOneNodeType
      <<" new actions have been added." << std::endl;
    return EXIT_FAILURE;
    }

  nodeSelector.setNodeTypes(QStringList(QString("vtkMRMLScalarVolumeNode")));

  // disable rename action
  startingActions = sceneModel->postItems(sceneModel->mrmlSceneItem()).size();
  nodeSelector.setRenameEnabled(false);

  int expected = startingActions - 2;
  if (!checkActionCount(__LINE__, sceneModel, expected))
    {
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "After disabling rename action, new number of actions = " << expected << std::endl;
    }

  // disable add action
  startingActions = sceneModel->postItems(sceneModel->mrmlSceneItem()).size();
  nodeSelector.setAddEnabled(false);

  expected = startingActions - 1;
  if (!checkActionCount(__LINE__, sceneModel, expected))
    {
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "After disabling add action, new number of actions = " << expected << std::endl;
    }

  // disable delete action
  startingActions = sceneModel->postItems(sceneModel->mrmlSceneItem()).size();
  nodeSelector.setRemoveEnabled(false);

  expected = startingActions - 1;
  if (!checkActionCount(__LINE__, sceneModel, expected))
    {
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "After disabling remove action, new number of actions = " << expected << std::endl;
    }

  // add a custom action starting respectively with:
  //  * "Create new "
  //  * "Delete current "
  //  * "Edit current "
  //  * "Rename current "
  //  * "Create and rename "
  foreach(const QString& actionPrefix,
          QStringList()\
            << "Create new "
            << "Delete current "
            << "Edit current "
            << "Rename current ")
    {
    startingActions = sceneModel->postItems(sceneModel->mrmlSceneItem()).size();

    QString actionName = QString("%1node using custom action").arg(actionPrefix);
    QAction *action = new QAction(actionName, &nodeSelector);
    nodeSelector.addMenuAction(action);

    expected = startingActions + 1;
    if (!checkActionCount(__LINE__, sceneModel, expected))
      {
      return EXIT_FAILURE;
      }
    else
      {
      std::cout << "After adding a new user action with text ["
                << qPrintable(actionName) << "], "
                << "new number of actions = " << expected << std::endl;
      }
    }

  // enabling add action is expected to fail since a custom action
  // with same name already exists
  startingActions = sceneModel->postItems(sceneModel->mrmlSceneItem()).size();
  nodeSelector.setAddEnabled(true);

  if (nodeSelector.addEnabled() != false)
    {
    std::cerr << "Enabling add action after adding a custom action"
                 " with the same name is expected to fail." << std::endl;
    return EXIT_FAILURE;
    }

  expected = startingActions;
  if (!checkActionCount(__LINE__, sceneModel, expected))
    {
    return EXIT_FAILURE;
    }

  // enabling remove action is expected to fail since a custom action
  // with same name already exists
  startingActions = sceneModel->postItems(sceneModel->mrmlSceneItem()).size();
  nodeSelector.setRemoveEnabled(true);

  if (nodeSelector.removeEnabled() != false)
    {
    std::cerr << "Enabling remove action after adding a custom action"
                 " with the same name is expected to fail." << std::endl;
    return EXIT_FAILURE;
    }

  expected = startingActions;
  if (!checkActionCount(__LINE__, sceneModel, expected))
    {
    return EXIT_FAILURE;
    }

  // enabling edit action is expected to fail since a custom action
  // with same name already exists
  startingActions = sceneModel->postItems(sceneModel->mrmlSceneItem()).size();
  nodeSelector.setEditEnabled(true);

  if (nodeSelector.editEnabled() != false)
    {
    std::cerr << "Enabling edit action after adding a custom action"
                 " with the same name is expected to fail." << std::endl;
    return EXIT_FAILURE;
    }

  expected = startingActions;
  if (!checkActionCount(__LINE__, sceneModel, expected))
    {
    return EXIT_FAILURE;
    }

  // enabling rename action is expected to fail since a custom action
  // with same name already exists
  startingActions = sceneModel->postItems(sceneModel->mrmlSceneItem()).size();
  nodeSelector.setRenameEnabled(true);

  if (nodeSelector.renameEnabled() != false)
    {
    std::cerr << "Enabling rename action after adding a custom action"
                 " with the same name is expected to fail." << std::endl;
    return EXIT_FAILURE;
    }

  expected = startingActions;
  if (!checkActionCount(__LINE__, sceneModel, expected))
    {
    return EXIT_FAILURE;
    }

  // Enabling the default "Add" or "Rename" action is expected to fail
  // when a custom action starting with "Create and rename " exists.

  qMRMLNodeComboBox nodeSelector2;
  nodeSelector2.setAddEnabled(false);
  nodeSelector2.setRenameEnabled(false);
  nodeSelector2.setBaseName("testing");
  nodeSelector2.setNodeTypes(QStringList(QString("vtkMRMLScalarVolumeNode")));
  nodeSelector2.setShowHidden(true);
  nodeSelector2.setMRMLScene(scene.GetPointer());

  qMRMLSceneModel *sceneModel2 = nodeSelector2.sceneModel();

  QAction* action = new QAction("Rename current node using custom action", &nodeSelector2);

  startingActions = sceneModel2->postItems(sceneModel2->mrmlSceneItem()).size();
  nodeSelector2.addMenuAction(action);
  expected = startingActions + 1;
  if (!checkActionCount(__LINE__, sceneModel2, expected))
    {
    return EXIT_FAILURE;
    }

  // enabling rename action is expected to fail since a "Rename current "
  // custom action already exists
  startingActions = sceneModel2->postItems(sceneModel2->mrmlSceneItem()).size();
  nodeSelector2.setRenameEnabled(true);

  if (nodeSelector2.renameEnabled() != false)
    {
    std::cerr << "Enabling rename action after adding a custom action"
                 " starting with 'Create and rename ' is expected to fail."
              << std::endl;
    return EXIT_FAILURE;
    }
  expected = startingActions;
  if (!checkActionCount(__LINE__, sceneModel2, expected))
    {
    return EXIT_FAILURE;
    }

  nodeSelector.show();

  std::cout << "Test completed successfully" << std::endl;

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
