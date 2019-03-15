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

// CTK includes
#include <ctkTest.h>
#include <ctkUtils.h>

// VTK includes
#include <vtkNew.h>

// Slicer includes
#include "qSlicerPythonManager.h"
#include "qSlicerScriptedLoadableModuleWidget.h"
#include "vtkMRMLModelNode.h"

#include <PythonQt.h>
// ----------------------------------------------------------------------------
class qSlicerScriptedLoadableModuleWidgetTester: public QObject
{
  Q_OBJECT

private:

  QString preparePythonSource(const QString& scriptName);

  qSlicerPythonManager PythonManager;

  bool resetTmp();
  QDir Tmp;
  QString TemporaryDirName;

private slots:
  void initTestCase();
  void cleanupTestCase();

  void testSetPythonSource();
  void testSetPythonSource_data();

  void testEnterExit();
  void testEnterExit_data();

  void testSetup();
  void testSetup_data();

  void testNodeEdit();
  void testNodeEdit_data();

};

// ----------------------------------------------------------------------------
QString qSlicerScriptedLoadableModuleWidgetTester::preparePythonSource(const QString& scriptName)
{
  QFile::copy(":" + scriptName, this->Tmp.filePath(scriptName));
  return this->Tmp.filePath(scriptName);
}

// ----------------------------------------------------------------------------
bool qSlicerScriptedLoadableModuleWidgetTester::resetTmp()
{
  if (this->TemporaryDirName.isEmpty())
    {
    return false;
    }
  QDir tmp = QDir::temp();
  ctk::removeDirRecursively(tmp.filePath(this->TemporaryDirName));
  tmp.mkdir(this->TemporaryDirName);
  tmp.cd(this->TemporaryDirName);
  this->Tmp = tmp;
  return this->Tmp.exists();
}

// ----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleWidgetTester::initTestCase()
{
  QVERIFY(this->PythonManager.initialize());

  QVERIFY(QDir::temp().exists());

  this->TemporaryDirName =
      QString("qSlicerScriptedLoadableModuleWidgetTester.%1").arg(QTime::currentTime().toString("hhmmsszzz"));
}

// ----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleWidgetTester::cleanupTestCase()
{
  if (this->Tmp != QDir::current() && this->Tmp.exists())
    {
    ctk::removeDirRecursively(this->Tmp.absolutePath());
    this->Tmp = QDir();
    }
}

// ----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleWidgetTester::testSetPythonSource()
{
  QVERIFY(this->resetTmp());

  QFETCH(QString, scriptName);
  QString scriptPath = this->preparePythonSource(scriptName);
  QVERIFY(QFile::exists(scriptPath));

  qSlicerScriptedLoadableModuleWidget w;
  QVERIFY(w.pythonSource().isEmpty());

  w.setPythonSource(scriptPath);
  QFETCH(bool, syntaxErrorExpected);
  QString expectedScriptPath = syntaxErrorExpected ? QString() : scriptPath;
  QCOMPARE(w.pythonSource(), expectedScriptPath);

  QVERIFY(!PyErr_Occurred());
}

// ----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleWidgetTester::testSetPythonSource_data()
{
  QTest::addColumn<QString>("scriptName");
  QTest::addColumn<bool>("syntaxErrorExpected");

  QTest::newRow("0") << "qSlicerScriptedLoadableModuleTestWidget.py" << false;
  QTest::newRow("1") << "qSlicerScriptedLoadableModuleTest.py" << false;
  QTest::newRow("2") << "qSlicerScriptedLoadableModuleSyntaxErrorTestWidget.py" << true;
  QTest::newRow("3") << "qSlicerScriptedLoadableModuleNewStyleTestWidget.py" << false;
}

// ----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleWidgetTester::testEnterExit()
{
  QVERIFY(this->resetTmp());

  QFETCH(QString, scriptName);
  QString scriptPath = this->preparePythonSource(scriptName);
  QVERIFY(QFile::exists(scriptPath));

  qSlicerScriptedLoadableModuleWidget w;
  w.setPythonSource(scriptPath);

  QVERIFY(!w.property("enter_called_within_Python").toBool());
  w.enter();
  QVERIFY(w.property("enter_called_within_Python").toBool());

  QVERIFY(!w.property("exit_called_within_Python").toBool());
  w.exit();
  QVERIFY(w.property("exit_called_within_Python").toBool());
}

// ----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleWidgetTester::testEnterExit_data()
{
  QTest::addColumn<QString>("scriptName");

  QTest::newRow("0") << "qSlicerScriptedLoadableModuleTestWidget.py";
  QTest::newRow("1") << "qSlicerScriptedLoadableModuleNewStyleTestWidget.py";
}

// ----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleWidgetTester::testNodeEdit()
{
  QVERIFY(this->resetTmp());

  QFETCH(QString, scriptName);
  QString scriptPath = this->preparePythonSource(scriptName);
  QVERIFY(QFile::exists(scriptPath));

  qSlicerScriptedLoadableModuleWidget w;
  w.setPythonSource(scriptPath);

  vtkNew<vtkMRMLModelNode> node;
  node->SetName("Some");

  QVERIFY(w.nodeEditable(nullptr) == 0.3);
  QVERIFY(w.property("editableNodeName").toString() == QString(""));
  QVERIFY(w.nodeEditable(node.GetPointer()) == 0.7);
  QVERIFY(w.property("editableNodeName").toString() == QString("Some"));

  QVERIFY(w.setEditedNode(nullptr) == false);
  QVERIFY(w.property("editedNodeName").toString() == QString(""));
  QVERIFY(w.setEditedNode(node.GetPointer(), "someRole", "someContext") == true);
  QVERIFY(w.property("editedNodeName").toString() == QString("Some"));
  QVERIFY(w.property("editedNodeRole").toString() == QString("someRole"));
  QVERIFY(w.property("editedNodeContext").toString() == QString("someContext"));
}

// ----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleWidgetTester::testNodeEdit_data()
{
  QTest::addColumn<QString>("scriptName");

  QTest::newRow("0") << "qSlicerScriptedLoadableModuleTestWidget.py";
  QTest::newRow("1") << "qSlicerScriptedLoadableModuleNewStyleTestWidget.py";
}

namespace
{
// ----------------------------------------------------------------------------
class qSlicerScriptedLoadableModuleWidgetTestSetup : public qSlicerScriptedLoadableModuleWidget
{
public:
  void callSetup() { this->setup(); }
};

} // end of anonymous namespace

// ----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleWidgetTester::testSetup()
{
  QVERIFY(this->resetTmp());
  QFETCH(QString, scriptName);
  QString scriptPath = this->preparePythonSource(scriptName);
  QVERIFY(QFile::exists(scriptPath));

  qSlicerScriptedLoadableModuleWidgetTestSetup w;
  w.setPythonSource(scriptPath);

  QVERIFY(!w.property("setup_called_within_Python").toBool());
  w.callSetup();
  QVERIFY(w.property("setup_called_within_Python").toBool());
}

// ----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleWidgetTester::testSetup_data()
{
  QTest::addColumn<QString>("scriptName");

  QTest::newRow("0") << "qSlicerScriptedLoadableModuleTestWidget.py";
  QTest::newRow("1") << "qSlicerScriptedLoadableModuleNewStyleTestWidget.py";
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qSlicerScriptedLoadableModuleWidgetTest)
#include "moc_qSlicerScriptedLoadableModuleWidgetTest.cxx"
