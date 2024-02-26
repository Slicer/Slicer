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

// Qt includes
#include <qSlicerCoreApplication.h>

// CTK includes
#include <ctkTest.h>
#include <ctkUtils.h>

// qRestAPI includes
#include <qRestAPI.h>

// QtCore includes
#include "qSlicerApplicationUpdateManager.h"
#include "vtkSlicerConfigure.h"
#include "vtkSlicerVersionConfigureMinimal.h"

// STD includes
#include <iostream>

// ----------------------------------------------------------------------------
class qSlicerApplicationUpdateManagerTester : public QObject
{
  Q_OBJECT
  typedef qSlicerApplicationUpdateManagerTester Self;

private:
  bool prepareJson(const QString& jsonFile);

  bool resetTmp();
  QDir Tmp;
  QString TemporaryDirName;

private slots:
  void initTestCase();
  void init();
  void cleanup();
  void cleanupTestCase();

  void testServerUrl();
  void testServerUrl_data();

  void testUpdateCheck();
  void testUpdateCheck_data();
};

// ----------------------------------------------------------------------------
bool qSlicerApplicationUpdateManagerTester::prepareJson(const QString& jsonFile)
{
  bool success = true;
  QDir tmp = QDir::temp();
  if (!QFile::exists(tmp.filePath(this->TemporaryDirName)))
  {
    success = tmp.mkdir(this->TemporaryDirName);
  }
  success = tmp.cd(this->TemporaryDirName);
  QString filename = QLatin1String("find");
  success = success && QFile::copy(jsonFile, tmp.filePath(filename));
  success = success && QFile::setPermissions(tmp.filePath(filename), QFile::ReadOwner | QFile::WriteOwner);
  return success;
}

// ----------------------------------------------------------------------------
bool qSlicerApplicationUpdateManagerTester::resetTmp()
{
  if (this->TemporaryDirName.isEmpty())
  {
    return false;
  }
  QDir tmp = QDir::temp();
  QFile(tmp.filePath(this->TemporaryDirName)).setPermissions(QFile::ReadUser | QFile::WriteUser | QFile::ExeUser);
  ctk::removeDirRecursively(tmp.filePath(this->TemporaryDirName));
  tmp.mkdir(this->TemporaryDirName);
  tmp.cd(this->TemporaryDirName);
  this->Tmp = tmp;
  return this->Tmp.exists();
}

// ----------------------------------------------------------------------------
void qSlicerApplicationUpdateManagerTester::initTestCase()
{
  QSettings::setDefaultFormat(QSettings::IniFormat);

  QVERIFY(QDir::temp().exists());

  this->TemporaryDirName =
    QString("qSlicerApplicationUpdateManagerTester.%1").arg(QTime::currentTime().toString("hhmmsszzz"));

  QSettings().clear();
}

// ----------------------------------------------------------------------------
void qSlicerApplicationUpdateManagerTester::init() {}

// ----------------------------------------------------------------------------
void qSlicerApplicationUpdateManagerTester::cleanup()
{
  QSettings().clear();
}

// ----------------------------------------------------------------------------
void qSlicerApplicationUpdateManagerTester::cleanupTestCase()
{
  if (this->Tmp != QDir::current() && this->Tmp.exists())
  {
    ctk::removeDirRecursively(this->Tmp.absolutePath());
    this->Tmp = QDir();
  }
  QFile::remove(QSettings().fileName());
}

// ----------------------------------------------------------------------------
void qSlicerApplicationUpdateManagerTester::testServerUrl()
{
  QFETCH(QString, serverUrl);

  QSettings().setValue("ApplicationUpdate/ServerUrl", serverUrl);
  qSlicerApplicationUpdateManager model;

  QUrl currentServerUrl = model.serverUrl();
  QVERIFY(currentServerUrl.isValid());
  QCOMPARE(currentServerUrl.toString(), serverUrl);
}

// ----------------------------------------------------------------------------
void qSlicerApplicationUpdateManagerTester::testServerUrl_data()
{
  QTest::addColumn<QString>("serverUrl");

  QTest::newRow("localhost") << "http://localhost";

  QTest::newRow("windows-file") << QUrl::fromLocalFile("C:/path/to/foo").toString();
}

// ----------------------------------------------------------------------------
void qSlicerApplicationUpdateManagerTester::testUpdateCheck()
{
  QVERIFY(this->resetTmp());

  QFETCH(QString, jsonFile);
  QFETCH(QString, operatingSystem);
  QFETCH(QString, architecture);
  QFETCH(QString, slicerRevision);
  QFETCH(bool, expectedIsUpdateAvailable);
  QFETCH(QString, expectedLatestReleaseVersion);
  QFETCH(QString, expectedLatestReleaseRevision);

  QVERIFY2(this->prepareJson(jsonFile), QString("Failed to prepare json file: %1").arg(jsonFile).toUtf8());

  QSettings().setValue("ApplicationUpdate/ServerUrl", QUrl::fromLocalFile(this->Tmp.absolutePath()));

  qSlicerApplicationUpdateManager updateManager;
  updateManager.setSlicerRequirements(slicerRevision, operatingSystem, architecture);

  QSignalSpy spyUpdateCheckCompleted(&updateManager, SIGNAL(updateCheckCompleted(bool)));
  QSignalSpy spyUpdateAvailable(&updateManager, SIGNAL(updateAvailable(bool)));

  updateManager.checkForUpdate(/* force= */ true, /* waitForCompletion= */ true);

  QCOMPARE(spyUpdateCheckCompleted.count(), 1);
  QCOMPARE(spyUpdateAvailable.count(), 1);
  QCOMPARE(updateManager.isUpdateAvailable(), expectedIsUpdateAvailable);
  QCOMPARE(updateManager.isUpdateAvailable(), expectedIsUpdateAvailable);
  QCOMPARE(updateManager.latestReleaseVersion(), expectedLatestReleaseVersion);
  QCOMPARE(updateManager.latestReleaseRevision(), expectedLatestReleaseRevision);
}

// ----------------------------------------------------------------------------
void qSlicerApplicationUpdateManagerTester::testUpdateCheck_data()
{
  QTest::addColumn<QString>("jsonFile");
  QTest::addColumn<QString>("operatingSystem");
  QTest::addColumn<QString>("architecture");
  QTest::addColumn<QString>("slicerRevision");
  QTest::addColumn<bool>("expectedIsUpdateAvailable");
  QTest::addColumn<QString>("expectedLatestReleaseVersion");
  QTest::addColumn<QString>("expectedLatestReleaseRevision");

  QTest::newRow("1") << ":/application_30893_slicer_win_amd64.json" << Slicer_OS_WIN_NAME << "amd64"
                     << "29000" << true << "5.0.3"
                     << "30893";
  QTest::newRow("2") << ":/application_30893_slicer_linux_amd64.json" << Slicer_OS_LINUX_NAME << "amd64"
                     << "29000" << true << "5.0.3"
                     << "30893";
  QTest::newRow("3") << ":/application_30893_slicer_macosx_amd64.json" << Slicer_OS_MAC_NAME << "amd64"
                     << "29000" << true << "5.0.3"
                     << "30893";

  QTest::newRow("4") << ":/application_30893_slicer_win_amd64.json" << Slicer_OS_WIN_NAME << "amd64"
                     << "31000" << false << "5.0.3"
                     << "30893";
  QTest::newRow("5") << ":/application_30893_slicer_linux_amd64.json" << Slicer_OS_LINUX_NAME << "amd64"
                     << "31000" << false << "5.0.3"
                     << "30893";
  QTest::newRow("6") << ":/application_30893_slicer_macosx_amd64.json" << Slicer_OS_MAC_NAME << "amd64"
                     << "31000" << false << "5.0.3"
                     << "30893";
}

// ----------------------------------------------------------------------------
int qSlicerApplicationUpdateManagerTest(int argc, char* argv[])
{
  qSlicerCoreApplication app(argc, argv);
  QTEST_DISABLE_KEYPAD_NAVIGATION
  qSlicerApplicationUpdateManagerTester tc;
  return QTest::qExec(&tc, argc, argv);
}
#include "moc_qSlicerApplicationUpdateManagerTest.cxx"
