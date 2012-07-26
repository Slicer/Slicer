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

// Slicer includes
#include "qSlicerLoadableModule.h"
#include "qSlicerPythonManager.h"

// ----------------------------------------------------------------------------
class qSlicerLoadableHelloWorldModule : public qSlicerLoadableModule
{
  Q_OBJECT
public:

  typedef qSlicerLoadableModule Superclass;
  qSlicerLoadableHelloWorldModule(QObject *parent=0):Superclass(parent){}
  virtual ~qSlicerLoadableHelloWorldModule(){}

  virtual QString helpText()const { return QString("helpText"); }
  virtual QString acknowledgementText()const { return QString("acknowledgementText"); }

  qSlicerGetTitleMacro("Loadable Hello world");

protected:
  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation()
  {
    return 0;
  }

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic()
  {
    return 0;
  }
};

// ----------------------------------------------------------------------------
class qSlicerLoadableModuleWithPythonTester: public QObject
{
  Q_OBJECT

private:
  qSlicerPythonManager PythonManager;

  QHash<QString, qSlicerAbstractModule*> Modules;

private slots:
  void testInitialize();

  void testAddModuleToSlicerModules();
  void testAddModuleToSlicerModules_data();

  void testAddModuleNameToSlicerModuleNames();
  void testAddModuleNameToSlicerModuleNames_data();
};

// ----------------------------------------------------------------------------
void qSlicerLoadableModuleWithPythonTester::testInitialize()
{
  this->PythonManager.initialize();
  this->PythonManager.executeString("import slicer");

  this->Modules.insert("LoadableHelloWorld", new qSlicerLoadableHelloWorldModule(this));
}

// ----------------------------------------------------------------------------
void qSlicerLoadableModuleWithPythonTester::testAddModuleToSlicerModules()
{
  QFETCH(bool, validPythonManager);
  QFETCH(QString, moduleName);
  QFETCH(bool, expectedResult);

  qSlicerAbstractModule * module = this->Modules.value(moduleName);
  QVERIFY(moduleName.isEmpty() ? true : module != 0);

  bool currentResult = qSlicerLoadableModule::addModuleToSlicerModules(
        validPythonManager ? &this->PythonManager : 0,
        module,
        moduleName);
  QCOMPARE(currentResult, expectedResult);

  if (expectedResult)
    {
    this->PythonManager.executeString(QString("dir(slicer.modules.%1)").arg(moduleName.toLower()));
    QCOMPARE(!this->PythonManager.pythonErrorOccured(), expectedResult);
    }
}

// ----------------------------------------------------------------------------
void qSlicerLoadableModuleWithPythonTester::testAddModuleToSlicerModules_data()
{
  QTest::addColumn<bool>("validPythonManager");
  QTest::addColumn<QString>("moduleName");
  QTest::addColumn<bool>("expectedResult");

  QTest::newRow("1") << true << "LoadableHelloWorld" << true;
  QTest::newRow("2") << true << "" << false;
  QTest::newRow("3") << false << "" << false;
}

// ----------------------------------------------------------------------------
void qSlicerLoadableModuleWithPythonTester::testAddModuleNameToSlicerModuleNames()
{
  QFETCH(bool, validPythonManager);
  QFETCH(QString, moduleName);
  QFETCH(bool, expectedResult);

  bool currentResult = qSlicerLoadableModule::addModuleNameToSlicerModuleNames(
        validPythonManager ? &this->PythonManager : 0, moduleName);
  QCOMPARE(currentResult, expectedResult);

  if (expectedResult)
    {
    this->PythonManager.executeString(QString("dir(slicer.moduleNames.%1)").arg(moduleName.toLower()));
    QCOMPARE(!this->PythonManager.pythonErrorOccured(), expectedResult);
    }
}

// ----------------------------------------------------------------------------
void qSlicerLoadableModuleWithPythonTester::testAddModuleNameToSlicerModuleNames_data()
{
  QTest::addColumn<bool>("validPythonManager");
  QTest::addColumn<QString>("moduleName");
  QTest::addColumn<bool>("expectedResult");

  QTest::newRow("0") << true << "CoreHelloWorld" << true;
  QTest::newRow("1") << true << "LoadableHelloWorld" << true;
  QTest::newRow("2") << true << "" << false;
  QTest::newRow("3") << false << "" << false;
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qSlicerLoadableModuleWithPythonTest)
#include "moc_qSlicerLoadableModuleWithPythonTest.cxx"
