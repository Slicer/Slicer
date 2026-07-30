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

==============================================================================*/

// This test guards the save result that the application-exit confirmation
// relies on. qSlicerMainWindowPrivate::confirmCloseApplication() quits only if
// openSaveDataDialog() returns true, and that in turn is the Save Data dialog's
// accepted/rejected result. So the exit is safe only if the dialog accepts
// exclusively on a clean, fully successful save:
//
//   - a successful save     -> dialog accepts        -> the application may exit;
//   - a failed save         -> dialog does not accept -> exit is cancelled;
//   - a save with a warning  -> dialog does not accept -> exit is cancelled, so
//                              the user can review the warning.
//
// The dialog is driven black-box: the real modal dialog is located through
// QApplication::activeModalWidget() and its Save button is clicked, exactly as
// a user would; no test-only hooks are added to the dialog.

// Qt includes
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTimer>

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerFileWriter.h"
#include "qSlicerSaveDataDialog.h"

// MRML includes
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLStorableNode.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLTransformStorageNode.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>

// STD includes
#include <functional>
#include <iostream>
#include <memory>

namespace
{

enum SaveOutcome
{
  Success,
  Failure,
  Warning
};

//-----------------------------------------------------------------------------
// A node writer whose outcome is configurable, so a single registered writer
// can stand in for a successful, failing, or warning save of a transform node.
class qSlicerConfigurableFileWriter : public qSlicerFileWriter
{
public:
  qSlicerConfigurableFileWriter(qSlicerIO::IOFileType fileType, QObject* parent = nullptr)
    : qSlicerFileWriter(parent)
    , FileType(fileType)
    , Outcome(Success)
  {
  }
  QString description() const override { return "Configurable outcome"; }
  qSlicerIO::IOFileType fileType() const override { return this->FileType; }
  // Must be overridden (the base returns false); otherwise the Save Data dialog
  // does not offer the node for saving and there is nothing to exercise.
  bool canWriteObject(vtkObject* object) const override { return vtkMRMLLinearTransformNode::SafeDownCast(object) != nullptr; }
  // Claim the transform storage node's default extension (.h5) so this writer is
  // the one the dialog selects for the node's file, and write() actually runs.
  QStringList extensions(vtkObject*) const override { return QStringList(QString("Transform (.h5)")); }

  bool write(const IOProperties& properties) override
  {
    if (this->Outcome == Failure)
    {
      // Report failure like a real writer that could not write the file. The
      // dialog adds the error message itself when a writer returns false.
      return false;
    }
    const QString nodeID = properties["nodeID"].toString();
    if (this->Outcome == Warning)
    {
      // The write succeeded but produced a non-fatal warning. Real storage
      // nodes report this on their own message collection; emulate that so the
      // dialog treats the save as not fully clean.
      vtkMRMLScene* scene = qSlicerCoreApplication::application()->mrmlScene();
      vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(scene->GetNodeByID(nodeID.toUtf8().constData()));
      if (storableNode && storableNode->GetStorageNode())
      {
        storableNode->GetStorageNode()->GetUserMessages()->AddMessage(vtkCommand::WarningEvent, "Simulated non-fatal save warning");
      }
    }
    this->setWrittenNodes(QStringList() << nodeID);
    return true;
  }

  qSlicerIO::IOFileType FileType;
  SaveOutcome Outcome;
};

//-----------------------------------------------------------------------------
// The Save Data dialog also offers to save the (modified) scene, but the
// minimal test application registers no scene writer. Provide one that succeeds
// without touching disk, so the scene save never masks the node save result
// under test. Only successful node saves plus a successful scene save let the
// dialog accept.
class qSlicerNoopSceneWriter : public qSlicerFileWriter
{
public:
  qSlicerNoopSceneWriter(QObject* parent = nullptr)
    : qSlicerFileWriter(parent)
  {
  }
  QString description() const override { return "No-op scene"; }
  qSlicerIO::IOFileType fileType() const override { return QString("SceneFile"); }
  bool canWriteObject(vtkObject* object) const override { return vtkMRMLScene::SafeDownCast(object) != nullptr; }
  QStringList extensions(vtkObject*) const override { return QStringList(QString("MRML Scene (.mrml)")); }
  bool write(const IOProperties& /*properties*/) override
  {
    this->setWrittenNodes(QStringList() << QString("Scene"));
    return true;
  }
};

//-----------------------------------------------------------------------------
// Click the Save button of whatever modal Save Data dialog is currently up. If
// the save does not accept the dialog (failure or warning), reject it so exec()
// returns and the caller sees a not-accepted result.
void driveSaveButton(std::shared_ptr<int> attempts, const std::function<void()>& self)
{
  QDialog* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
  QDialogButtonBox* buttonBox = dialog ? dialog->findChild<QDialogButtonBox*>("ButtonBox") : nullptr;
  QPushButton* saveButton = buttonBox ? buttonBox->button(QDialogButtonBox::Save) : nullptr;
  if (!saveButton)
  {
    // The dialog may not be shown yet; retry briefly, then give up.
    if (++(*attempts) < 100)
    {
      QTimer::singleShot(20, self);
    }
    else if (dialog)
    {
      dialog->reject();
    }
    return;
  }
  saveButton->click(); // synchronously triggers accept() -> save()
  if (dialog->isVisible())
  {
    // save() returned false, so accept() did not close the dialog.
    dialog->reject();
  }
}

//-----------------------------------------------------------------------------
bool runSaveCase(qSlicerConfigurableFileWriter* writer, SaveOutcome outcome)
{
  vtkMRMLScene* scene = qSlicerCoreApplication::application()->mrmlScene();
  writer->Outcome = outcome;

  vtkNew<vtkMRMLTransformStorageNode> storageNode;
  scene->AddNode(storageNode);
  vtkNew<vtkMRMLLinearTransformNode> transformNode;
  scene->AddNode(transformNode);
  transformNode->SetAndObserveStorageNodeID(storageNode->GetID());
  // Modify the node so the dialog lists it and checks it for saving
  // (populateNode() keys the check state off GetModifiedSinceRead()).
  vtkNew<vtkMatrix4x4> matrix;
  matrix->SetElement(0, 3, 5.0);
  transformNode->SetMatrixTransformToParent(matrix);

  qSlicerSaveDataDialog dialog;
  auto attempts = std::make_shared<int>(0);
  auto driver = std::make_shared<std::function<void()>>();
  *driver = [attempts, driver]() { driveSaveButton(attempts, *driver); };
  QTimer::singleShot(0, *driver);
  // Safety net so the test can never hang if the dialog cannot be driven.
  QTimer::singleShot(10000,
                     []()
                     {
                       if (QApplication::activeModalWidget())
                       {
                         QApplication::activeModalWidget()->close();
                       }
                     });
  const bool accepted = dialog.exec();

  scene->RemoveNode(transformNode);
  scene->RemoveNode(storageNode);
  return accepted;
}

} // namespace

//-----------------------------------------------------------------------------
int qSlicerSaveDataDialogSaveOutcomeTest(int argc, char* argv[])
{
  qSlicerApplication app(argc, argv);
  app.coreIOManager()->registerIO(new qSlicerNoopSceneWriter(nullptr));
  qSlicerConfigurableFileWriter* writer = new qSlicerConfigurableFileWriter(QString("TransformFile"), nullptr);
  app.coreIOManager()->registerIO(writer);

  struct TestCase
  {
    SaveOutcome outcome;
    bool expectedAccepted;
    const char* name;
  };
  const TestCase cases[] = {
    { Success, true, "successful save accepts (exit allowed)" },
    { Failure, false, "failed save does not accept (exit cancelled)" },
    { Warning, false, "save with warning does not accept (exit cancelled)" },
  };

  bool allPassed = true;
  for (const TestCase& testCase : cases)
  {
    const bool accepted = runSaveCase(writer, testCase.outcome);
    const bool passed = (accepted == testCase.expectedAccepted);
    std::cout << (passed ? "PASSED: " : "FAILED: ") << testCase.name << " (accepted=" << (accepted ? "true" : "false")
              << ", expected=" << (testCase.expectedAccepted ? "true" : "false") << ")" << std::endl;
    allPassed = allPassed && passed;
  }

  return allPassed ? EXIT_SUCCESS : EXIT_FAILURE;
}
