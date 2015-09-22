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

// Slicer includes
#include "qSlicerPythonManager.h"
#include "qSlicerScriptedLoadableModule.h"

#include <PythonQt.h>
// ----------------------------------------------------------------------------
class qSlicerScriptedLoadableModuleTester: public QObject
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

  void testSetup();
  void testSetup_data();

};

// ----------------------------------------------------------------------------
QString qSlicerScriptedLoadableModuleTester::preparePythonSource(const QString& scriptName)
{
  QFile::copy(":" + scriptName, this->Tmp.filePath(scriptName));
  return this->Tmp.filePath(scriptName);
}

// ----------------------------------------------------------------------------
bool qSlicerScriptedLoadableModuleTester::resetTmp()
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
void qSlicerScriptedLoadableModuleTester::initTestCase()
{
  QVERIFY(this->PythonManager.initialize());

  QVERIFY(QDir::temp().exists());

  this->TemporaryDirName =
      QString("qSlicerScriptedLoadableModuleTester.%1").arg(QTime::currentTime().toString("hhmmsszzz"));
}

// ----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleTester::cleanupTestCase()
{
  if (this->Tmp != QDir::current() && this->Tmp.exists())
    {
    ctk::removeDirRecursively(this->Tmp.absolutePath());
    this->Tmp = QDir();
    }
}

// ----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleTester::testSetPythonSource()
{
  QVERIFY(this->resetTmp());

  QFETCH(QString, scriptName);
  QString scriptPath = this->preparePythonSource(scriptName);
  QVERIFY(QFile::exists(scriptPath));

  qSlicerScriptedLoadableModule m;
  QVERIFY(m.pythonSource().isEmpty());

  m.setPythonSource(scriptPath);
  QFETCH(bool, syntaxErrorExpected);
  QString expectedScriptPath = syntaxErrorExpected ? QString() : scriptPath;
  QCOMPARE(m.pythonSource(), expectedScriptPath);

  QVERIFY(!PyErr_Occurred());
}

// ----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleTester::testSetPythonSource_data()
{
  QTest::addColumn<QString>("scriptName");
  QTest::addColumn<bool>("syntaxErrorExpected");

  QTest::newRow("0") << "qSlicerScriptedLoadableModuleTest.py" << false;
  QTest::newRow("1") << "qSlicerScriptedLoadableModuleSyntaxErrorTest.py" << true;
  QTest::newRow("2") << "qSlicerScriptedLoadableModuleNewStyleTest.py" << false;
}

namespace
{
// ----------------------------------------------------------------------------
class qSlicerScriptedLoadableModuleSetup : public qSlicerScriptedLoadableModule
{
public:
  void callSetup() { this->setup(); }
};

} // end of anonymous namespace

// ----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleTester::testSetup()
{
  QVERIFY(this->resetTmp());
  QFETCH(QString, scriptName);
  QString scriptPath = this->preparePythonSource(scriptName);
  QVERIFY(QFile::exists(scriptPath));

  qSlicerScriptedLoadableModuleSetup m;
  m.setPythonSource(scriptPath);

  QVERIFY(!m.property("setup_called_within_Python").toBool());
  m.callSetup();
  QVERIFY(m.property("setup_called_within_Python").toBool());
}

// ----------------------------------------------------------------------------
void qSlicerScriptedLoadableModuleTester::testSetup_data()
{
  QTest::addColumn<QString>("scriptName");

  QTest::newRow("0") << "qSlicerScriptedLoadableModuleTest.py";
  QTest::newRow("1") << "qSlicerScriptedLoadableModuleNewStyleTest.py";
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qSlicerScriptedLoadableModuleTest)
#include "moc_qSlicerScriptedLoadableModuleTest.cxx"
