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

// Qt includes
#include <qSlicerCoreApplication.h>
#include <QJSEngine>

// CTK includes
#include <ctkTest.h>
#include <ctkUtils.h>

// qRestAPI includes
#include <qRestAPI.h>

// QtCore includes
#include "qSlicerExtensionsManagerModel.h"
#include "vtkSlicerConfigure.h"
#include "vtkSlicerVersionConfigureMinimal.h"

// STD includes
#include <iostream>

// ----------------------------------------------------------------------------
class qSlicerExtensionsManagerModelTester : public QObject
{
  Q_OBJECT
  typedef qSlicerExtensionsManagerModelTester Self;
  typedef qSlicerExtensionsManagerModel::ExtensionMetadataType ExtensionMetadataType;

public:
  typedef QPair<QString, int> ExtensionIdType;

private:
  void dumpExtensionMetatype(const char* varname, const ExtensionMetadataType& extensionMetadata);

  static ExtensionMetadataType extensionMetadata(const QString& os,
                                                 int extensionId,
                                                 bool filtered = false,
                                                 bool installed = false);

  QStringList expectedExtensionNames() const;

  QString extensionNameFromExtenionId(int extensionId);

  QString slicerVersion(const QString& operatingSystem, int extensionId);

  bool prepareJson(const QString& jsonFile, qSlicerExtensionsManagerModel::ServerAPI serverAPI);

  void installHelper(qSlicerExtensionsManagerModel* model, const QString& os, int extensionId, const QString& tmp);

  bool uninstallHelper(qSlicerExtensionsManagerModel* model, const QString& extensionName);

  bool resetTmp();
  QDir Tmp;
  QString TemporaryDirName;

  static const QString LIB_DIR;

  static const QString CLIMODULES_LIB_DIR;
  static const QString QTLOADABLEMODULES_LIB_DIR;
#ifdef Slicer_USE_PYTHONQT
  static const QString QTLOADABLEMODULES_PYTHON_LIB_DIR;
  static const QString QTSCRIPTEDMODULES_LIB_DIR;
#endif

private slots:
  void initTestCase();
  void init();
  void cleanup();
  void cleanupTestCase();

  void testDefaults();

  void testServerUrl();
  void testServerUrl_data();

  void testServerKeysToIgnore();
  void testServerKeysToIgnore_data();

  void testServerToExtensionDescriptionKey();
  void testServerToExtensionDescriptionKey_data();

  void testRetrieveExtensionMetadata();
  void testRetrieveExtensionMetadata_data();

  void testFilterExtensionMetadata();
  void testFilterExtensionMetadata_data();

  void testExtractExtensionArchive();
  void testExtractExtensionArchive_data();

  void testWriteAndParseExtensionDescriptionFile();
  void testWriteAndParseExtensionDescriptionFile_data();

  void testInstallExtension();

  void testUninstallExtension();

  void testScheduleExtensionForUninstall();
  void testScheduleExtensionForUninstall_data();

  void testCancelExtensionScheduledForUninstall();
  void testCancelExtensionScheduledForUninstall_data();

  void testUpdateModel();

  void testIsExtensionInstalled();
  void testIsExtensionInstalled_data();

  void testInstalledExtensionsCount();
  void testInstalledExtensionsCount_data();

  void testInstalledExtensions();
  void testInstalledExtensions_data();

  void testIsExtensionEnabled();
  void testIsExtensionEnabled_data();

  void testSetExtensionsSettingsFilePath();
  void testSetExtensionsSettingsFilePath_data();

  void testExtensionAdditionalPathsSettingsUpdated();
  void testExtensionAdditionalPathsSettingsUpdated_data();

  void testExtensionExtensionsSettingsUpdated();
  void testExtensionExtensionsSettingsUpdated_data();

  void testSetSlicerRevision();
  void testSetSlicerRevision_data();

  void testSetSlicerOs();
  void testSetSlicerOs_data();

  void testSetSlicerArch();
  void testSetSlicerArch_data();

  void testSetSlicerRequirements();
  void testSetSlicerRequirements_data();

  void testSetSlicerVersion();
  void testSetSlicerVersion_data();
};

Q_DECLARE_METATYPE(qSlicerExtensionsManagerModelTester::ExtensionIdType)
Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<QString>)
Q_DECLARE_METATYPE(QList<QStringList>)

const QString qSlicerExtensionsManagerModelTester::LIB_DIR = Slicer_LIB_DIR;

const QString qSlicerExtensionsManagerModelTester::CLIMODULES_LIB_DIR = Slicer_CLIMODULES_LIB_DIR;
const QString qSlicerExtensionsManagerModelTester::QTLOADABLEMODULES_LIB_DIR = Slicer_QTLOADABLEMODULES_LIB_DIR;
#ifdef Slicer_USE_PYTHONQT
const QString qSlicerExtensionsManagerModelTester::QTLOADABLEMODULES_PYTHON_LIB_DIR =
  Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR;
const QString qSlicerExtensionsManagerModelTester::QTSCRIPTEDMODULES_LIB_DIR = Slicer_QTSCRIPTEDMODULES_LIB_DIR;
#endif

// ----------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModelTester::expectedExtensionNames() const
{
  return QStringList() << "ImageMaker"
                       << "MarkupsToModel"
                       << "CurveMaker";
}

// ----------------------------------------------------------------------------
QString qSlicerExtensionsManagerModelTester::extensionNameFromExtenionId(int extensionId)
{
  if (extensionId < 0 || extensionId >= this->expectedExtensionNames().count())
  {
    return QString();
  }
  return this->expectedExtensionNames().at(extensionId);
}

// ----------------------------------------------------------------------------
QString qSlicerExtensionsManagerModelTester::slicerVersion(const QString& operatingSystem, int extensionId)
{
  QString osExtensionId = QString("%1-%2").arg(operatingSystem).arg(extensionId);

  if (osExtensionId == "linux-0" || osExtensionId == "linux-1" || osExtensionId == "linux-2")
  {
    return QLatin1String("5.1");
  }
  else if (osExtensionId == "macosx-0" || osExtensionId == "macosx-1" || osExtensionId == "macosx-2")
  {
    return QLatin1String("5.1");
  }
  return QLatin1String("Invalid");
}

// ----------------------------------------------------------------------------
bool qSlicerExtensionsManagerModelTester::prepareJson(const QString& jsonFile,
                                                      qSlicerExtensionsManagerModel::ServerAPI serverAPI)
{
  bool success = true;
  QDir tmp = QDir::temp();
  if (!QFile::exists(tmp.filePath(this->TemporaryDirName)))
  {
    success = tmp.mkdir(this->TemporaryDirName);
  }
  success = tmp.cd(this->TemporaryDirName);
  if (tmp.exists("api"))
  {
    success = success && ctk::removeDirRecursively(tmp.filePath("api"));
  }
  success = success && tmp.mkdir("api");
  success = success && tmp.cd("api");
  QString filename;
  if (serverAPI == qSlicerExtensionsManagerModel::Girder_v1)
  {
    QString appID = "5f4474d0e1d8c75dfc705482";
    success = success && tmp.mkdir("v1");
    success = success && tmp.cd("v1");
    success = success && tmp.mkdir("app");
    success = success && tmp.cd("app");
    success = success && tmp.mkdir(appID);
    success = success && tmp.cd(appID);
    filename = "extension";
  }
  success = success && QFile::copy(jsonFile, tmp.filePath(filename));
  success = success && QFile::setPermissions(tmp.filePath(filename), QFile::ReadOwner | QFile::WriteOwner);
  return success;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::installHelper(qSlicerExtensionsManagerModel* model,
                                                        const QString& os,
                                                        int extensionId,
                                                        const QString& tmp)
{
  QVERIFY(model != nullptr);
  QVERIFY(extensionId >= 0 && extensionId <= 3);

  QString inputArchiveFile = QString(":/extension-%1-%2.tar.gz").arg(os).arg(extensionId);
  QString copiedArchiveFile = tmp + "/" + QFileInfo(inputArchiveFile).fileName();
  if (!QFile::exists(copiedArchiveFile))
  {
    QVERIFY2(QFile::copy(inputArchiveFile, copiedArchiveFile),
             QString("Failed to copy %1 into %2").arg(inputArchiveFile).arg(copiedArchiveFile).toUtf8());
    QFile::setPermissions(copiedArchiveFile, QFile::ReadOwner | QFile::WriteOwner);
  }

  QVERIFY2(this->prepareJson(QString(":/extension-%1-%2.json").arg(os).arg(extensionId),
                             qSlicerExtensionsManagerModel::Girder_v1),
           QString("Failed to prepare json for extensionId: %1-%2").arg(os).arg(extensionId).toUtf8());

  qSlicerExtensionsManagerModel::ServerAPI serverAPI = qSlicerExtensionsManagerModel::Girder_v1;

  QString expectedExtensionName = this->expectedExtensionNames().at(extensionId);

  // The only API for changing server API is an environment variable.
  // We save the environment variable current value, modify it temporarily, then restore it.
  QString oldServerApiStr = qEnvironmentVariable("SLICER_EXTENSIONS_MANAGER_SERVER_API");
  qputenv("SLICER_EXTENSIONS_MANAGER_SERVER_API", qSlicerExtensionsManagerModel::serverAPIToString(serverAPI).toUtf8());
  model->updateExtensionsMetadataFromServer(/* force= */ true, /* waitForCompletion= */ true);
  ExtensionMetadataType metadata = model->extensionMetadata(expectedExtensionName);
  qputenv("SLICER_EXTENSIONS_MANAGER_SERVER_API", oldServerApiStr.toUtf8());

  QVERIFY(metadata.count() > 0);
  QVariantMap expectedMetadata = Self::extensionMetadata(os, extensionId);

  QString extensionName = metadata.value("extensionname").toString();
  QCOMPARE(extensionName, expectedExtensionName);

  QVERIFY(!model->isExtensionInstalled(extensionName));
  QVERIFY(!model->isExtensionEnabled(extensionName));

  QSignalSpy spyExtensionInstalled(model, SIGNAL(extensionInstalled(QString)));
  QSignalSpy spyExtensionEnabledChanged(model, SIGNAL(extensionEnabledChanged(QString, bool)));

  QVERIFY(model->installExtension(extensionName, metadata, copiedArchiveFile));
  QCOMPARE(model->extensionMetadata(extensionName, /* source= */ qSlicerExtensionsManagerModel::MetadataServer),
           Self::extensionMetadata(os, extensionId, /* filtered= */ true, /* installed= */ false));
  QCOMPARE(model->extensionDescriptionFile(extensionName),
           model->extensionsInstallPath() + "/" + extensionName + ".s4ext");

  QCOMPARE(spyExtensionInstalled.count(), 1);
  QCOMPARE(spyExtensionEnabledChanged.count(), 0);

  // Attempt to install the same extension twice and check that no additional signals are emitted.
  QVERIFY(!model->installExtension(extensionName, metadata, copiedArchiveFile));
  QCOMPARE(spyExtensionInstalled.count(), 1);
  QCOMPARE(spyExtensionEnabledChanged.count(), 0);

  QVERIFY(model->isExtensionInstalled(extensionName));
  QVERIFY(model->isExtensionEnabled(extensionName));
}

// ----------------------------------------------------------------------------
bool qSlicerExtensionsManagerModelTester::uninstallHelper(qSlicerExtensionsManagerModel* model,
                                                          const QString& extensionName)
{
  bool success = model->scheduleExtensionForUninstall(extensionName);
  success = success && model->uninstallScheduledExtensions();
  return success;
}

// ----------------------------------------------------------------------------
bool qSlicerExtensionsManagerModelTester::resetTmp()
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
void qSlicerExtensionsManagerModelTester::dumpExtensionMetatype(const char* varname,
                                                                const ExtensionMetadataType& extensionMetadata)
{
  std::cout << "[" << varname << "]" << std::endl;
  foreach (const QString& key, extensionMetadata.keys())
  {
    std::cout << varname << ".insert(\"" << qPrintable(key) << "\", "
              << "\"" << qPrintable(extensionMetadata.value(key).toString()) << "\");" << std::endl;
  }
}

// ----------------------------------------------------------------------------
qSlicerExtensionsManagerModelTester::ExtensionMetadataType qSlicerExtensionsManagerModelTester::extensionMetadata(
  const QString& os,
  int extensionId,
  bool filtered,
  bool installed)
{
  int serverAPI = qSlicerExtensionsManagerModel::Girder_v1;

  QFile metadataFile(QString(":/extension-%1-%2.json").arg(os).arg(extensionId));
  if (!QTest::qVerify(metadataFile.open(QFile::ReadOnly), "metadataFile.open(QFile::ReadOnly)", "", __FILE__, __LINE__))
  {
    return ExtensionMetadataType();
  }

  QJSEngine scriptEngine;
  QJSValue scriptValue = scriptEngine.evaluate("(" + QString(metadataFile.readAll()) + ")");

  ExtensionMetadataType allMetadata;

  {
    // Girder_v1
    allMetadata = scriptValue.property(0).toVariant().toMap();
  }

  allMetadata = qRestAPI::qVariantMapFlattened(allMetadata);

  if (!QTest::qVerify(allMetadata.count() > 0, "allMetadata.count() > 0", "", __FILE__, __LINE__))
  {
    return ExtensionMetadataType();
  }

  if (installed)
  {
    if (serverAPI == qSlicerExtensionsManagerModel::Girder_v1)
    {
      allMetadata["archivename"] = "";
      allMetadata["enabled"] = "true";
      allMetadata["md5"] = "";
      allMetadata["release"] = "";
      allMetadata["status"] = "";
    }
    filtered = true;
  }

  QStringList keysToIgnore(qSlicerExtensionsManagerModel::serverKeysToIgnore(serverAPI));
  QVariantMap metadata;
  foreach (const QString& key, allMetadata.keys())
  {
    if (filtered && keysToIgnore.contains(key))
    {
      continue;
    }
    metadata.insert(qSlicerExtensionsManagerModel::serverToExtensionDescriptionKey(serverAPI).value(key, key),
                    allMetadata.value(key));
  }
  return metadata;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::initTestCase()
{
  QSettings::setDefaultFormat(QSettings::IniFormat);

  QVERIFY(QDir::temp().exists());

  this->TemporaryDirName =
    QString("qSlicerExtensionsManagerModelTester.%1").arg(QTime::currentTime().toString("hhmmsszzz"));

  QSettings().clear();
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::init() {}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::cleanup()
{
  QSettings().clear();
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::cleanupTestCase()
{
  if (this->Tmp != QDir::current() && this->Tmp.exists())
  {
    ctk::removeDirRecursively(this->Tmp.absolutePath());
    this->Tmp = QDir();
  }
  QFile::remove(QSettings().fileName());
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testDefaults()
{
  qSlicerExtensionsManagerModel model;
  // No initialization required

  QCOMPARE(model.serverUrl().toString(), QLatin1String(""));
  QCOMPARE(model.extensionsInstallPath(), QString(""));
  QCOMPARE(model.extensionInstallPath(""), QString(""));
  QCOMPARE(model.extensionDescriptionFile(""), QString(""));
  QCOMPARE(model.newExtensionEnabledByDefault(), true);
  QCOMPARE(model.slicerVersion(), QString(Slicer_VERSION));

  ExtensionMetadataType metadata = model.extensionMetadata("");
  QCOMPARE(metadata.count(), 0);

  QCOMPARE(model.isExtensionInstalled(""), false);
  QCOMPARE(model.installedExtensionsCount(), 0);
  QCOMPARE(model.installedExtensions(), QStringList());
  QCOMPARE(model.isExtensionEnabled(""), false);
  QCOMPARE(model.slicerRevision(), QString());
  QCOMPARE(model.slicerOs(), QString());
  QCOMPARE(model.slicerArch(), QString());
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testServerUrl()
{
  QFETCH(QString, serverUrl);

  QSettings().setValue("Extensions/ServerUrl", serverUrl);
  qSlicerExtensionsManagerModel model;
  model.setExtensionsSettingsFilePath(QSettings().fileName());
  model.setSlicerVersion(Slicer_VERSION);

  QUrl currentServerUrl = model.serverUrl();
  QVERIFY(currentServerUrl.isValid());
  QCOMPARE(currentServerUrl.toString(), serverUrl);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testServerUrl_data()
{
  QTest::addColumn<QString>("serverUrl");

  QTest::newRow("localhost") << "http://localhost/midas";

  QTest::newRow("windows-file") << QUrl::fromLocalFile("C:/path/to/foo").toString();
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testServerKeysToIgnore()
{
  QFETCH(QStringList, expectedServerKeysToIgnore);
  QFETCH(qSlicerExtensionsManagerModel::ServerAPI, serverAPI);
  QCOMPARE(qSlicerExtensionsManagerModel::serverKeysToIgnore(serverAPI), expectedServerKeysToIgnore);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testServerKeysToIgnore_data()
{
  QTest::addColumn<QStringList>("expectedServerKeysToIgnore");
  QTest::addColumn<qSlicerExtensionsManagerModel::ServerAPI>("serverAPI");

  QTest::newRow("0") << (QStringList() << "baseParentId"
                                       << "baseParentType"
                                       << "created"
                                       << "creatorId"
                                       << "description"
                                       << "folderId"
                                       << "lowerName"
                                       << "meta.app_id"
                                       << "name"
                                       << "size")
                     << qSlicerExtensionsManagerModel::Girder_v1;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testServerToExtensionDescriptionKey()
{
  QFETCH(qSlicerExtensionsManagerModel::ServerAPI, serverAPI);

  QHash<QString, QString> serverToExtensionDescriptionKey =
    qSlicerExtensionsManagerModel::serverToExtensionDescriptionKey(serverAPI);

  QFETCH(QList<QString>, expectedServerKeys);
  QCOMPARE(serverToExtensionDescriptionKey.keys(), expectedServerKeys);

  QFETCH(QList<QString>, expectedExtensionDescriptionKeys);
  QCOMPARE(serverToExtensionDescriptionKey.values(), expectedExtensionDescriptionKeys);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testServerToExtensionDescriptionKey_data()
{
  QTest::addColumn<QList<QString>>("expectedServerKeys");
  QTest::addColumn<QList<QString>>("expectedExtensionDescriptionKeys");
  QTest::addColumn<qSlicerExtensionsManagerModel::ServerAPI>("serverAPI");

  {
    QHash<QString, QString> expected;
    expected.insert("_id", "extension_id");
    expected.insert("meta.baseName", "extensionname");
    expected.insert("meta.repository_type", "scm");
    expected.insert("meta.repository_url", "scmurl");
    expected.insert("meta.app_revision", "slicer_revision");
    expected.insert("meta.revision", "revision");
    // release
    expected.insert("meta.arch", "arch");
    expected.insert("meta.os", "os");
    expected.insert("meta.dependency", "depends");
    expected.insert("meta.homepage", "homepage");
    expected.insert("meta.icon_url", "iconurl");
    expected.insert("meta.category", "category");
    // status
    expected.insert("meta.contributors", "contributors");
    expected.insert("meta.description", "description");
    expected.insert("meta.screenshots", "screenshots");
    // enabled
    // archivename
    // md5
    expected.insert("updated", "updated");
    QTest::newRow("1") << expected.keys() << expected.values() << qSlicerExtensionsManagerModel::Girder_v1;
  }
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testRetrieveExtensionMetadata()
{
  QVERIFY(this->resetTmp());

  QFETCH(qSlicerExtensionsManagerModel::ServerAPI, serverAPI);
  QSettings().setValue("Extensions/ServerUrl", QUrl::fromLocalFile(this->Tmp.absolutePath()));
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QFETCH(QString, extensionName);
  QFETCH(QString, jsonFile);
  QVERIFY2(this->prepareJson(jsonFile, serverAPI),
           QString("Failed to prepare json for extensionId: %1").arg(extensionName).toUtf8());

  QFETCH(QString, slicerVersion);
  qSlicerExtensionsManagerModel model;
  model.setExtensionsSettingsFilePath(QSettings().fileName());
  model.setSlicerVersion(slicerVersion);

  QString oldServerApiStr = qEnvironmentVariable("SLICER_EXTENSIONS_MANAGER_SERVER_API");
  qputenv("SLICER_EXTENSIONS_MANAGER_SERVER_API", qSlicerExtensionsManagerModel::serverAPIToString(serverAPI).toUtf8());
  model.updateExtensionsMetadataFromServer(/* force= */ true, /* waitForCompletion= */ true);
  ExtensionMetadataType extensionMetadata = model.extensionMetadata(extensionName);
  qputenv("SLICER_EXTENSIONS_MANAGER_SERVER_API", oldServerApiStr.toUtf8());

  QFETCH(QVariantMap, expectedResult);
  QCOMPARE(extensionMetadata, expectedResult);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testRetrieveExtensionMetadata_data()
{
  QTest::addColumn<qSlicerExtensionsManagerModel::ServerAPI>("serverAPI");
  QTest::addColumn<QString>("extensionName");
  QTest::addColumn<QString>("jsonFile");
  QTest::addColumn<QString>("slicerVersion");
  QTest::addColumn<QVariantMap>("expectedResult");

  int extensionId = 0;
  QTest::newRow("1") << qSlicerExtensionsManagerModel::Girder_v1 << this->extensionNameFromExtenionId(extensionId)
                     << QString(":/extension-%1-%2.json").arg(Slicer_OS_LINUX_NAME).arg(extensionId)
                     << this->slicerVersion(Slicer_OS_LINUX_NAME, extensionId)
                     << Self::extensionMetadata(Slicer_OS_LINUX_NAME, extensionId, /* filtered= */ true);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testFilterExtensionMetadata()
{
  QFETCH(QVariantMap, inputExtensionMetadata);
  QFETCH(QVariantMap, expectedExtensionMetadata);
  QFETCH(qSlicerExtensionsManagerModel::ServerAPI, serverAPI);
  ExtensionMetadataType filteredExtensionMetadata =
    qSlicerExtensionsManagerModel::filterExtensionMetadata(inputExtensionMetadata, serverAPI);
  QCOMPARE(filteredExtensionMetadata, expectedExtensionMetadata);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testFilterExtensionMetadata_data()
{
  QTest::addColumn<QVariantMap>("inputExtensionMetadata");
  QTest::addColumn<QVariantMap>("expectedExtensionMetadata");
  QTest::addColumn<qSlicerExtensionsManagerModel::ServerAPI>("serverAPI");

  QTest::newRow("0") << Self::extensionMetadata(Slicer_OS_WIN_NAME, 0)
                     << Self::extensionMetadata(Slicer_OS_WIN_NAME, 0, /* filtered= */ true)
                     << qSlicerExtensionsManagerModel::Girder_v1;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testExtractExtensionArchive()
{
  QVERIFY(this->resetTmp());

  QFETCH(QString, inputExtensionName);
  QFETCH(QString, inputArchiveFile);
  QFETCH(QString, slicerVersion);
  QFETCH(QString, slicerRevision);
  QFETCH(QStringList, expectedFiles);
  QFETCH(QString, operatingSystem);
  QFETCH(bool, expectedExtractSuccess);
  QFETCH(bool, nonExistentDestinationPath);
  QFETCH(bool, readOnlyDestinationPath);

  QCOMPARE(static_cast<bool>(QFile::permissions(this->Tmp.absolutePath()) & QFile::WriteUser), true);

  qSlicerExtensionsManagerModel model;
  model.setSlicerRevision(slicerRevision);
  model.setSlicerVersion(slicerVersion);
  model.setSlicerOs(operatingSystem);

  QString copiedArchiveFile = this->Tmp.filePath(QFileInfo(inputArchiveFile).fileName());
  QVERIFY(QFile::copy(inputArchiveFile, copiedArchiveFile));
  QVERIFY(QFile::setPermissions(copiedArchiveFile, QFile::ReadUser | QFile::WriteUser));

  if (nonExistentDestinationPath)
  {
    QVERIFY(ctk::removeDirRecursively(this->Tmp.absolutePath()));
    QVERIFY(!QFile::exists(this->Tmp.absolutePath()));
  }

  if (readOnlyDestinationPath)
  {
    QVERIFY(QFile::setPermissions(this->Tmp.absolutePath(), QFile::ExeUser | QFile::ReadUser));
    QCOMPARE(static_cast<bool>(QFile::permissions(this->Tmp.absolutePath()) & QFile::WriteUser), false);
  }

  bool extractSuccess = model.extractExtensionArchive(inputExtensionName, copiedArchiveFile, this->Tmp.absolutePath());
  QCOMPARE(extractSuccess, expectedExtractSuccess);
  if (!expectedExtractSuccess)
  {
    return;
  }

  bool expectedFilesExist = true;
  foreach (const QString& expectedFile, expectedFiles)
  {
    expectedFilesExist = expectedFilesExist && QFile::exists(this->Tmp.filePath(expectedFile));
    if (!expectedFilesExist)
    {
      qCritical() << "Missing file" << expectedFile;
    }
  }
  QVERIFY(expectedFilesExist);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testExtractExtensionArchive_data()
{
  QTest::addColumn<QString>("inputExtensionName");
  QTest::addColumn<QString>("inputArchiveFile");
  QTest::addColumn<QString>("slicerVersion");
  QTest::addColumn<QString>("slicerRevision");
  QTest::addColumn<QStringList>("expectedFiles");
  QTest::addColumn<QString>("operatingSystem");
  QTest::addColumn<bool>("expectedExtractSuccess");
  QTest::addColumn<bool>("nonExistentDestinationPath");
  QTest::addColumn<bool>("readOnlyDestinationPath");
  {
    QStringList expectedFiles;
    expectedFiles << "ImageMaker";
    expectedFiles << "ImageMaker/lib/Slicer-5.1/cli-modules/ImageMaker";
    expectedFiles << "ImageMaker/lib/Slicer-5.1/cli-modules/libImageMakerLib.so";
    {
      QTest::newRow("linux-0-success") << "ImageMaker" << QString(":/extension-%1-0.tar.gz").arg(Slicer_OS_LINUX_NAME)
                                       << this->slicerVersion(Slicer_OS_LINUX_NAME, 0) << "30987" << expectedFiles
                                       << Slicer_OS_LINUX_NAME << true /* expectedExtractSuccess */
                                       << false                        /* nonExistentDestinationPath */
                                       << false /* readOnlyDestinationPath */;
    }
    {
      QTest::newRow("linux-0-nonexistent-destinationPath")
        << "ImageMaker" << QString(":/extension-%1-0.tar.gz").arg(Slicer_OS_LINUX_NAME)
        << this->slicerVersion(Slicer_OS_LINUX_NAME, 0) << "30987" << expectedFiles << Slicer_OS_LINUX_NAME
        << false /* expectedExtractSuccess */
        << true  /* nonExistentDestinationPath */
        << false /* readOnlyDestinationPath */;
    }
#if !(defined Q_OS_WIN || defined Q_OS_MAC)
    {
      QTest::newRow("linux-0-readonly-destinationPath")
        << "ImageMaker" << QString(":/extension-%1-0.tar.gz").arg(Slicer_OS_LINUX_NAME)
        << this->slicerVersion(Slicer_OS_LINUX_NAME, 0) << "30987" << expectedFiles << Slicer_OS_LINUX_NAME
        << false /* expectedExtractSuccess */
        << false /* nonExistentDestinationPath */
        << true /* readOnlyDestinationPath */;
    }
#endif
  }
  {
    QStringList expectedFiles;
    expectedFiles << "ImageMaker";
    expectedFiles << "ImageMaker/lib/Slicer-5.1/cli-modules/ImageMaker";
    expectedFiles << "ImageMaker/lib/Slicer-5.1/cli-modules/libImageMakerLib.dylib";
    {
      QTest::newRow("macosx-0-success") << "ImageMaker" << QString(":/extension-%1-0.tar.gz").arg(Slicer_OS_MAC_NAME)
                                        << this->slicerVersion(Slicer_OS_MAC_NAME, 0) << "30987" << expectedFiles
                                        << Slicer_OS_MAC_NAME << true /* expectedExtractSuccess */
                                        << false                      /* nonExistentDestinationPath */
                                        << false /* readOnlyDestinationPath */;
    }
  }
  {
    QStringList expectedFiles;
    expectedFiles << "MarkupsToModel";
    expectedFiles << "MarkupsToModel/lib/Slicer-5.1/qt-loadable-modules/libqSlicerMarkupsToModelModule.dylib";
    expectedFiles << "MarkupsToModel/lib/Slicer-5.1/qt-loadable-modules/libvtkSlicerMarkupsToModelModuleLogic.dylib";
    expectedFiles << "MarkupsToModel/lib/Slicer-5.1/qt-loadable-modules/libvtkSlicerMarkupsToModelModuleMRML.dylib";
    expectedFiles << "MarkupsToModel/lib/Slicer-5.1/qt-loadable-modules/vtkSlicerMarkupsToModelModuleLogicPython.so";
    expectedFiles << "MarkupsToModel/lib/Slicer-5.1/qt-loadable-modules/vtkSlicerMarkupsToModelModuleMRMLPython.so";
    {
      QTest::newRow("macosx-1-success") << "MarkupsToModel"
                                        << QString(":/extension-%1-1.tar.gz").arg(Slicer_OS_MAC_NAME)
                                        << this->slicerVersion(Slicer_OS_MAC_NAME, 1) << "30987" << expectedFiles
                                        << Slicer_OS_MAC_NAME << true /* expectedExtractSuccess */
                                        << false                      /* nonExistentDestinationPath */
                                        << false /* readOnlyDestinationPath */;
    }
  }
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testWriteAndParseExtensionDescriptionFile()
{
  QVERIFY(this->resetTmp());

  QFETCH(ExtensionMetadataType, metadata);

  QVERIFY(
    qSlicerExtensionsManagerModel::writeExtensionDescriptionFile(this->Tmp.filePath("ImageMaker.s4ext"), metadata));

  ExtensionMetadataType parsedMetadata =
    qSlicerExtensionsManagerModel::parseExtensionDescriptionFile(this->Tmp.filePath("ImageMaker.s4ext"));

  QCOMPARE(metadata.count(), parsedMetadata.count());
  QCOMPARE(metadata, parsedMetadata);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testWriteAndParseExtensionDescriptionFile_data()
{
  QTest::addColumn<ExtensionMetadataType>("metadata");

  ExtensionMetadataType metadata;
  metadata.insert("arch", "amd64");
  metadata.insert("archivename", "30987-linux-amd64-ImageMaker-git6293f99-2017-07-03.tar.gz");
  metadata.insert("category", "Developer Tools");
  metadata.insert("contributors", "Julien Finet (Kitware)");
  metadata.insert("description", "This is a CLI module to create an image from scratch.");
  metadata.insert("enabled", "true");
  metadata.insert("extensionname", "ImageMaker");
  metadata.insert("homepage", "http://www.slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/ImageMaker");
  metadata.insert("installed", "true");
  metadata.insert("md5", "d4726e1fd85b19930e0e8e8e5d6afa62");
  metadata.insert("os", "linux");
  metadata.insert("release", "");
  metadata.insert("revision", "6293f99");
  metadata.insert("scm", "git");
  metadata.insert("scmurl", "https://github.com/finetjul/ImageMaker");
  metadata.insert("screenshots", "");
  metadata.insert("slicer_revision", "30987");
  QTest::newRow("0-ImageMaker") << metadata;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testInstallExtension()
{
  QVERIFY(this->resetTmp());

  QSettings().setValue("Extensions/ServerUrl", QUrl::fromLocalFile(this->Tmp.absolutePath()));
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QString operatingSystem = Slicer_OS_LINUX_NAME;
  QString architecture("amd64");
  QString slicerRevision("30987");
  QString slicerVersion = "5.1";

  qSlicerExtensionsManagerModel model;
  model.setExtensionsSettingsFilePath(QSettings().fileName());
  model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
  model.setSlicerVersion(slicerVersion);
  QSignalSpy spyModelUpdated(&model, SIGNAL(modelUpdated()));
  QSignalSpy spyExtensionUpdated(&model, SIGNAL(extensionUpdated(QString)));
  QSignalSpy spyExtensionUninstalled(&model, SIGNAL(extensionUninstalled(QString)));
  QSignalSpy spySlicerRequirementsChanged(&model, SIGNAL(slicerRequirementsChanged(QString, QString, QString)));

  for (int extensionId = 0; extensionId < this->expectedExtensionNames().count(); ++extensionId)
  {
    this->installHelper(&model, operatingSystem, extensionId, this->Tmp.absolutePath());

    QCOMPARE(model.installedExtensionsCount(), extensionId + 1);

    QString extensionName = this->expectedExtensionNames().at(extensionId);

    QSignalSpy spyExtensionInstalled(&model, SIGNAL(extensionInstalled(QString)));
    QSignalSpy spyExtensionEnabledChanged(&model, SIGNAL(extensionEnabledChanged(QString, bool)));

    model.setExtensionEnabled(extensionName, true);
    QVERIFY(model.isExtensionInstalled(extensionName));
    QVERIFY(model.isExtensionEnabled(extensionName));
    QCOMPARE(spyExtensionEnabledChanged.count(), 0);

    model.setExtensionEnabled(extensionName, false);
    QVERIFY(model.isExtensionInstalled(extensionName));
    QVERIFY(!model.isExtensionEnabled(extensionName));
    QCOMPARE(spyExtensionEnabledChanged.count(), 1);

    model.setExtensionEnabled(extensionName, true);
    QVERIFY(model.isExtensionInstalled(extensionName));
    QVERIFY(model.isExtensionEnabled(extensionName));
    QCOMPARE(spyExtensionInstalled.count(), 0);
    QCOMPARE(spyExtensionEnabledChanged.count(), 2);
  }

  QStringList expectedInstalledExtensionNames = this->expectedExtensionNames();
  std::sort(expectedInstalledExtensionNames.begin(), expectedInstalledExtensionNames.end());

  QCOMPARE(model.installedExtensions(), expectedInstalledExtensionNames);
  QCOMPARE(spyModelUpdated.count(), expectedInstalledExtensionNames.count());
  QCOMPARE(spyExtensionUpdated.count(), 0);
  QCOMPARE(spyExtensionUninstalled.count(), 0);
  QCOMPARE(spySlicerRequirementsChanged.count(), 0);
  QCOMPARE(model.installedExtensionsCount(), expectedInstalledExtensionNames.count());
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testUninstallExtension()
{
  QVERIFY(this->resetTmp());

  QSettings().setValue("Extensions/ServerUrl", QUrl::fromLocalFile(this->Tmp.absolutePath()));
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QString operatingSystem = Slicer_OS_LINUX_NAME;
  QString architecture("amd64");
  QString slicerRevision("30987");
  QString slicerVersion = "5.1";

  {
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
    model.setSlicerVersion(slicerVersion);

    for (int extensionId = 0; extensionId < this->expectedExtensionNames().count(); ++extensionId)
    {
      this->installHelper(&model, operatingSystem, extensionId, this->Tmp.absolutePath());
    }

    foreach (const QString& extensionName, QStringList() << "MarkupsToModel")
    {
      QVERIFY(this->uninstallHelper(&model, extensionName));
      QVERIFY(!model.isExtensionInstalled(extensionName));
    }
  }
  {
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
    model.updateModel();

    foreach (const QString& extensionName,
             QStringList() << "ImageMaker"
                           << "CurveMaker")
    {
      QVERIFY(model.isExtensionInstalled(extensionName));
    }

    foreach (const QString& extensionName, QStringList() << "MarkupsToModel")
    {
      QVERIFY(!model.isExtensionInstalled(extensionName));
    }

    foreach (const QString& extensionName, QStringList() << "MarkupsToModel")
    {
      this->installHelper(
        &model, operatingSystem, this->expectedExtensionNames().indexOf(extensionName), this->Tmp.absolutePath());
    }

    foreach (const QString& extensionName, this->expectedExtensionNames())
    {
      QVERIFY(model.isExtensionInstalled(extensionName));
    }
  }
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testScheduleExtensionForUninstall()
{
  QVERIFY(this->resetTmp());

  QSettings().setValue("Extensions/ServerUrl", QUrl::fromLocalFile(this->Tmp.absolutePath()));
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QString slicerVersion = "5.1";

  QFETCH(QString, operatingSystem);
  QFETCH(QString, architecture);
  QFETCH(QString, slicerRevision);

  {
    QFETCH(QString, operatingSystem);
    QFETCH(QList<int>, extensionIdsToInstall);
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
    model.setSlicerVersion(slicerVersion);
    foreach (int extensionIdToInstall, extensionIdsToInstall)
    {
      this->installHelper(&model, operatingSystem, extensionIdToInstall, this->Tmp.absolutePath());
    }
  }

  QFETCH(QStringList, extensionNamesToScheduleForUninstall);
  QFETCH(QStringList, expectedExtensionNamesScheduledForUninstall);

  {
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    QSignalSpy spyExtensionScheduledForUninstall(&model, SIGNAL(extensionScheduledForUninstall(QString)));
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
    model.setSlicerVersion(slicerVersion);
    model.updateModel();
    foreach (const QString& extensionNameToScheduleForUninstall, extensionNamesToScheduleForUninstall)
    {
      model.scheduleExtensionForUninstall(extensionNameToScheduleForUninstall);
    }
    QCOMPARE(spyExtensionScheduledForUninstall.count(), expectedExtensionNamesScheduledForUninstall.count());
  }

  {
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
    model.setSlicerVersion(slicerVersion);
    model.updateModel();
    QCOMPARE(model.scheduledForUninstallExtensions(), expectedExtensionNamesScheduledForUninstall);
  }
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testScheduleExtensionForUninstall_data()
{
  QTest::addColumn<QString>("operatingSystem");
  QTest::addColumn<QString>("architecture");
  QTest::addColumn<QString>("slicerRevision");
  QTest::addColumn<QList<int>>("extensionIdsToInstall");
  QTest::addColumn<QStringList>("extensionNamesToScheduleForUninstall");
  QTest::addColumn<QStringList>("expectedExtensionNamesScheduledForUninstall");

  QString operatingSystem = Slicer_OS_LINUX_NAME;
  QString architecture("amd64");
  QString slicerRevision("30987");

  QTest::newRow("1") << operatingSystem << architecture << slicerRevision << (QList<int>() << 0 << 1 << 2)
                     << (QStringList()) << (QStringList());

  QTest::newRow("2") << operatingSystem << architecture << slicerRevision << (QList<int>() << 0 << 1 << 2)
                     << (QStringList() << this->expectedExtensionNames().at(0) << this->expectedExtensionNames().at(2))
                     << (QStringList() << this->expectedExtensionNames().at(0) << this->expectedExtensionNames().at(2));

  QTest::newRow("3") << operatingSystem << architecture << slicerRevision << (QList<int>() << 0 << 1 << 2)
                     << (QStringList() << this->expectedExtensionNames().at(0) << "Invalid"
                                       << this->expectedExtensionNames().at(2))
                     << (QStringList() << this->expectedExtensionNames().at(0) << this->expectedExtensionNames().at(2));
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testCancelExtensionScheduledForUninstall()
{
  QVERIFY(this->resetTmp());

  QSettings().setValue("Extensions/ServerUrl", QUrl::fromLocalFile(this->Tmp.absolutePath()));
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QString slicerVersion = "5.1";
  QFETCH(QString, operatingSystem);
  QFETCH(QString, slicerRevision);
  QFETCH(QString, architecture);

  {
    QFETCH(QList<int>, extensionIdsToInstall);
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
    model.setSlicerVersion(slicerVersion);
    foreach (int extensionIdToInstall, extensionIdsToInstall)
    {
      this->installHelper(&model, operatingSystem, extensionIdToInstall, this->Tmp.absolutePath());
    }
  }

  {
    QFETCH(QStringList, extensionNamesToScheduleForUninstall);
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
    model.setSlicerVersion(slicerVersion);
    model.updateModel();
    foreach (const QString& extensionNameToScheduleForUninstall, extensionNamesToScheduleForUninstall)
    {
      model.scheduleExtensionForUninstall(extensionNameToScheduleForUninstall);
    }
  }

  {
    QFETCH(QStringList, extensionNamesToCancelScheduledForUninstall);
    QFETCH(int, expectedSpyExtensionCancelledScheduleForUninstallCount);
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    QSignalSpy spyExtensionCancelledScheduleForUninstall(&model,
                                                         SIGNAL(extensionCancelledScheduleForUninstall(QString)));
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
    model.setSlicerVersion(slicerVersion);
    model.updateModel();
    foreach (const QString& extensionNameToCancelScheduledForUninstall, extensionNamesToCancelScheduledForUninstall)
    {
      model.cancelExtensionScheduledForUninstall(extensionNameToCancelScheduledForUninstall);
    }
    QCOMPARE(spyExtensionCancelledScheduleForUninstall.count(), expectedSpyExtensionCancelledScheduleForUninstallCount);
  }

  {
    QFETCH(QStringList, expectedExtensionNamesScheduledForUninstall);
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
    model.setSlicerVersion(slicerVersion);
    model.updateModel();
    QCOMPARE(model.scheduledForUninstallExtensions(), expectedExtensionNamesScheduledForUninstall);
  }
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testCancelExtensionScheduledForUninstall_data()
{
  QTest::addColumn<QString>("operatingSystem");
  QTest::addColumn<QString>("architecture");
  QTest::addColumn<QString>("slicerRevision");
  QTest::addColumn<QList<int>>("extensionIdsToInstall");
  QTest::addColumn<QStringList>("extensionNamesToScheduleForUninstall");
  QTest::addColumn<QStringList>("extensionNamesToCancelScheduledForUninstall");
  QTest::addColumn<int>("expectedSpyExtensionCancelledScheduleForUninstallCount");
  QTest::addColumn<QStringList>("expectedExtensionNamesScheduledForUninstall");

  QString slicerRevision("30987");
  QString architecture("amd64");

  QTest::newRow("1") << Slicer_OS_LINUX_NAME << architecture << slicerRevision << (QList<int>() << 0 << 1 << 2)
                     << (QStringList() << this->expectedExtensionNames().at(0) << this->expectedExtensionNames().at(2))
                     << (QStringList()) << 0
                     << (QStringList() << this->expectedExtensionNames().at(0) << this->expectedExtensionNames().at(2));

  QTest::newRow("2") << Slicer_OS_LINUX_NAME << architecture << slicerRevision << (QList<int>() << 0 << 1 << 2)
                     << (QStringList() << this->expectedExtensionNames().at(0) << this->expectedExtensionNames().at(2))
                     << (QStringList() << this->expectedExtensionNames().at(2) << "Invalid") << 1
                     << (QStringList() << this->expectedExtensionNames().at(0));

  QTest::newRow("3") << Slicer_OS_LINUX_NAME << architecture << slicerRevision << (QList<int>() << 0 << 1 << 2)
                     << (QStringList() << this->expectedExtensionNames().at(0) << this->expectedExtensionNames().at(2))
                     << (QStringList() << this->expectedExtensionNames().at(0) << this->expectedExtensionNames().at(2))
                     << 2 << (QStringList());
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testUpdateModel()
{
  QVERIFY(this->resetTmp());

  QSettings().setValue("Extensions/ServerUrl", QUrl::fromLocalFile(this->Tmp.absolutePath()));
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QString operatingSystem = Slicer_OS_LINUX_NAME;
  QString architecture("amd64");
  QString slicerRevision("30987");

  QStringList expectedInstalledExtensionNames = this->expectedExtensionNames();
  std::sort(expectedInstalledExtensionNames.begin(), expectedInstalledExtensionNames.end());

  {
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
    QSignalSpy spyModelUpdated(&model, SIGNAL(modelUpdated()));
    for (int extensionId = 0; extensionId < this->expectedExtensionNames().count(); ++extensionId)
    {
      this->installHelper(&model, operatingSystem, extensionId, this->Tmp.absolutePath());
    }
    QCOMPARE(spyModelUpdated.count(), this->expectedExtensionNames().count());
  }
  {
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);

    QSignalSpy spyModelUpdated(&model, SIGNAL(modelUpdated()));
    QSignalSpy spyExtensionInstalled(&model, SIGNAL(extensionInstalled(QString)));
    QSignalSpy spyExtensionUpdated(&model, SIGNAL(extensionUpdated(QString)));
    QSignalSpy spyExtensionUninstalled(&model, SIGNAL(extensionUninstalled(QString)));
    QSignalSpy spyExtensionEnabledChanged(&model, SIGNAL(extensionEnabledChanged(QString, bool)));
    QSignalSpy spySlicerRequirementsChanged(&model, SIGNAL(slicerRequirementsChanged(QString, QString, QString)));

    model.updateModel();

    QCOMPARE(spyModelUpdated.count(), 1);
    QCOMPARE(spyExtensionInstalled.count(), 0);
    QCOMPARE(spyExtensionUpdated.count(), 0);
    QCOMPARE(spyExtensionEnabledChanged.count(), 0);
    QCOMPARE(spyExtensionUninstalled.count(), 0);
    QCOMPARE(spySlicerRequirementsChanged.count(), 0);
    QCOMPARE(model.installedExtensions(), expectedInstalledExtensionNames);
    QCOMPARE(model.installedExtensionsCount(), expectedInstalledExtensionNames.count());

    model.updateModel();

    QCOMPARE(spyModelUpdated.count(), 2);
    QCOMPARE(spyExtensionInstalled.count(), 0);
    QCOMPARE(spyExtensionUpdated.count(), 0);
    QCOMPARE(spyExtensionEnabledChanged.count(), 0);
    QCOMPARE(spyExtensionUninstalled.count(), 0);
    QCOMPARE(spySlicerRequirementsChanged.count(), 0);

    QCOMPARE(model.installedExtensions(), expectedInstalledExtensionNames);
    QCOMPARE(model.installedExtensionsCount(), expectedInstalledExtensionNames.count());
  }
  {
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);

    QSignalSpy spyModelUpdated(&model, SIGNAL(modelUpdated()));
    QSignalSpy spyExtensionInstalled(&model, SIGNAL(extensionInstalled(QString)));
    QSignalSpy spyExtensionUpdated(&model, SIGNAL(extensionUpdated(QString)));
    QSignalSpy spyExtensionUninstalled(&model, SIGNAL(extensionUninstalled(QString)));
    QSignalSpy spyExtensionEnabledChanged(&model, SIGNAL(extensionEnabledChanged(QString, bool)));
    QSignalSpy spySlicerRequirementsChanged(&model, SIGNAL(slicerRequirementsChanged(QString, QString, QString)));

    model.updateModel();

    model.setExtensionEnabled("ImageMaker", false);
    QCOMPARE(spyExtensionEnabledChanged.count(), 1);

    QCOMPARE(model.isExtensionEnabled("ImageMaker"), false);
    foreach (const QString& extensionName,
             QStringList() << "MarkupsToModel"
                           << "CurveMaker")
    {
      QCOMPARE(model.isExtensionEnabled(extensionName), true);
    }

    model.updateModel();

    QCOMPARE(model.isExtensionEnabled("ImageMaker"), false);
    foreach (const QString& extensionName,
             QStringList() << "MarkupsToModel"
                           << "CurveMaker")
    {
      QCOMPARE(model.isExtensionEnabled(extensionName), true);
    }

    QCOMPARE(spyModelUpdated.count(), 2);
    QCOMPARE(spyExtensionInstalled.count(), 0);
    QCOMPARE(spyExtensionUpdated.count(), 0);
    QCOMPARE(spyExtensionEnabledChanged.count(), 1);
    QCOMPARE(spyExtensionUninstalled.count(), 0);
    QCOMPARE(spySlicerRequirementsChanged.count(), 0);

    QCOMPARE(model.installedExtensions(), expectedInstalledExtensionNames);
    QCOMPARE(model.installedExtensionsCount(), expectedInstalledExtensionNames.count());
  }
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testIsExtensionInstalled()
{
  QVERIFY(this->resetTmp());

  QSettings().setValue("Extensions/ServerUrl", QUrl::fromLocalFile(this->Tmp.absolutePath()));
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QFETCH(QString, operatingSystem);
  QFETCH(QString, architecture);
  QFETCH(QString, slicerRevision);
  QFETCH(int, extensionIdToInstall);
  QFETCH(bool, isExtensionZeroInstalled);
  QFETCH(bool, isExtensionOneInstalled);
  QFETCH(bool, isExtensionTwoInstalled);

  {
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
    if (extensionIdToInstall != -1)
    {
      this->installHelper(&model, operatingSystem, extensionIdToInstall, this->Tmp.absolutePath());
    }
    QCOMPARE(model.isExtensionInstalled(this->expectedExtensionNames().at(0)), isExtensionZeroInstalled);
    QCOMPARE(model.isExtensionInstalled(this->expectedExtensionNames().at(1)), isExtensionOneInstalled);
    QCOMPARE(model.isExtensionInstalled(this->expectedExtensionNames().at(2)), isExtensionTwoInstalled);
  }

  {
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
    model.updateModel();
    QCOMPARE(model.isExtensionInstalled(this->expectedExtensionNames().at(0)), isExtensionZeroInstalled);
    QCOMPARE(model.isExtensionInstalled(this->expectedExtensionNames().at(1)), isExtensionOneInstalled);
    QCOMPARE(model.isExtensionInstalled(this->expectedExtensionNames().at(2)), isExtensionTwoInstalled);
  }
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testIsExtensionInstalled_data()
{
  QTest::addColumn<QString>("operatingSystem");
  QTest::addColumn<QString>("architecture");
  QTest::addColumn<QString>("slicerRevision");
  QTest::addColumn<int>("extensionIdToInstall");
  QTest::addColumn<bool>("isExtensionZeroInstalled");
  QTest::addColumn<bool>("isExtensionOneInstalled");
  QTest::addColumn<bool>("isExtensionTwoInstalled");

  QString operatingSystem = Slicer_OS_LINUX_NAME;
  QString architecture("amd64");
  QString slicerRevision("30987");

  QTest::newRow("0 installed") << operatingSystem << architecture << slicerRevision << -1 << false << false << false;

  QTest::newRow("1 installed") << operatingSystem << architecture << slicerRevision << 0 << true << false << false;

  QTest::newRow("2 installed") << operatingSystem << architecture << slicerRevision << 1 << false << true << false;

  QTest::newRow("3 installed") << operatingSystem << architecture << slicerRevision << 2 << false << false << true;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testInstalledExtensionsCount()
{
  QVERIFY(this->resetTmp());

  QSettings().setValue("Extensions/ServerUrl", QUrl::fromLocalFile(this->Tmp.absolutePath()));
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QFETCH(QString, operatingSystem);
  QFETCH(QString, architecture);
  QFETCH(QString, slicerRevision);
  QFETCH(QList<int>, extensionIdsToInstall);
  QFETCH(int, expectedInstalledExtensionsCount);

  {
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
    foreach (int extensionIdToInstall, extensionIdsToInstall)
    {
      this->installHelper(&model, operatingSystem, extensionIdToInstall, this->Tmp.absolutePath());
    }
    QCOMPARE(model.installedExtensionsCount(), expectedInstalledExtensionsCount);
  }
  {
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
    model.updateModel();
    QCOMPARE(model.installedExtensionsCount(), expectedInstalledExtensionsCount);
  }
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testInstalledExtensionsCount_data()
{
  QTest::addColumn<QString>("operatingSystem");
  QTest::addColumn<QString>("architecture");
  QTest::addColumn<QString>("slicerRevision");
  QTest::addColumn<QList<int>>("extensionIdsToInstall");
  QTest::addColumn<int>("expectedInstalledExtensionsCount");

  QString operatingSystem = Slicer_OS_LINUX_NAME;
  QString architecture("amd64");
  QString slicerRevision("30987");

  QTest::newRow("0 installed") << operatingSystem << architecture << slicerRevision << (QList<int>()) << 0;

  QTest::newRow("1 installed") << operatingSystem << architecture << slicerRevision << (QList<int>() << 0) << 1;

  QTest::newRow("2 installed") << operatingSystem << architecture << slicerRevision << (QList<int>() << 0 << 1) << 2;

  QTest::newRow("3 installed") << operatingSystem << architecture << slicerRevision << (QList<int>() << 0 << 1 << 2)
                               << 3;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testInstalledExtensions()
{
  QVERIFY(this->resetTmp());

  QSettings().setValue("Extensions/ServerUrl", QUrl::fromLocalFile(this->Tmp.absolutePath()));
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QFETCH(QString, operatingSystem);
  QFETCH(QString, architecture);
  QFETCH(QString, slicerRevision);
  QFETCH(QList<int>, extensionIdsToInstall);
  QFETCH(QStringList, expectedInstalledExtensionNames);

  {
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
    foreach (int extensionIdToInstall, extensionIdsToInstall)
    {
      this->installHelper(&model, operatingSystem, extensionIdToInstall, this->Tmp.absolutePath());
    }
    QCOMPARE(model.installedExtensions(), expectedInstalledExtensionNames);
  }
  {
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
    model.updateModel();
    QCOMPARE(model.installedExtensions(), expectedInstalledExtensionNames);
  }
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testInstalledExtensions_data()
{
  QTest::addColumn<QString>("operatingSystem");
  QTest::addColumn<QString>("architecture");
  QTest::addColumn<QString>("slicerRevision");
  QTest::addColumn<QList<int>>("extensionIdsToInstall");
  QTest::addColumn<QStringList>("expectedInstalledExtensionNames");

  QString operatingSystem = Slicer_OS_LINUX_NAME;
  QString architecture("amd64");
  QString slicerRevision("30987");

  QStringList expectedInstalledExtensionNames;
  QTest::newRow("0 installed") << operatingSystem << architecture << slicerRevision << (QList<int>())
                               << expectedInstalledExtensionNames;

  expectedInstalledExtensionNames = this->expectedExtensionNames();
  std::sort(expectedInstalledExtensionNames.begin(), expectedInstalledExtensionNames.end());

  QTest::newRow("3 installed") << operatingSystem << architecture << slicerRevision << (QList<int>() << 0 << 1 << 2)
                               << expectedInstalledExtensionNames;

  expectedInstalledExtensionNames.removeOne(this->extensionNameFromExtenionId(2));
  QTest::newRow("2 installed") << operatingSystem << architecture << slicerRevision << (QList<int>() << 0 << 1)
                               << expectedInstalledExtensionNames;

  expectedInstalledExtensionNames.removeOne(this->extensionNameFromExtenionId(1));
  QTest::newRow("1 installed") << operatingSystem << architecture << slicerRevision << (QList<int>() << 0)
                               << expectedInstalledExtensionNames;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testIsExtensionEnabled()
{
  QVERIFY(this->resetTmp());

  QSettings().setValue("Extensions/ServerUrl", QUrl::fromLocalFile(this->Tmp.absolutePath()));
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QFETCH(QString, operatingSystem);
  QFETCH(QString, architecture);
  QFETCH(QString, slicerRevision);
  QFETCH(QList<int>, extensionIdsToInstall);
  QFETCH(QStringList, expectedEnabledExtensionNames);
  QFETCH(QStringList, extensionNamesToDisable);
  QFETCH(QStringList, expectedEnabledExtensionNamesAfterDisable);

  QHash<QString, QStringList> additionalModulePathsPerExtension;

  {
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
    foreach (int extensionIdToInstall, extensionIdsToInstall)
    {
      this->installHelper(&model, operatingSystem, extensionIdToInstall, this->Tmp.absolutePath());
    }
    QCOMPARE(model.enabledExtensions(), expectedEnabledExtensionNames);

    foreach (const QString& extensionName, expectedEnabledExtensionNames)
    {
      additionalModulePathsPerExtension.insert(extensionName, model.extensionModulePaths(extensionName));
    }

    foreach (const QString& extensionName, extensionNamesToDisable)
    {
      model.setExtensionEnabled(extensionName, false);
    }
    QCOMPARE(model.enabledExtensions(), expectedEnabledExtensionNamesAfterDisable);

    QStringList expectedAdditionalPaths;
    foreach (const QString& extensionName, expectedEnabledExtensionNamesAfterDisable)
    {
      expectedAdditionalPaths << additionalModulePathsPerExtension.value(extensionName);
    }

    QStringList currentAdditionalPaths = QSettings().value("Modules/AdditionalPaths").toStringList();
    QCOMPARE(currentAdditionalPaths, expectedAdditionalPaths);
  }
  {
    qSlicerExtensionsManagerModel model;
    model.setExtensionsSettingsFilePath(QSettings().fileName());
    model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
    model.updateModel();
    QCOMPARE(model.enabledExtensions(), expectedEnabledExtensionNamesAfterDisable);

    QStringList expectedAdditionalPaths;
    foreach (const QString& extensionName, expectedEnabledExtensionNamesAfterDisable)
    {
      expectedAdditionalPaths << additionalModulePathsPerExtension.value(extensionName);
    }

    QStringList currentAdditionalPaths = QSettings().value("Modules/AdditionalPaths").toStringList();
    QCOMPARE(currentAdditionalPaths, expectedAdditionalPaths);
  }
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testIsExtensionEnabled_data()
{
  QTest::addColumn<QString>("operatingSystem");
  QTest::addColumn<QString>("architecture");
  QTest::addColumn<QString>("slicerRevision");
  QTest::addColumn<QList<int>>("extensionIdsToInstall");
  QTest::addColumn<QStringList>("expectedEnabledExtensionNames");
  QTest::addColumn<QStringList>("extensionNamesToDisable");
  QTest::addColumn<QStringList>("expectedEnabledExtensionNamesAfterDisable");

  QString operatingSystem = Slicer_OS_LINUX_NAME;
  QString architecture("amd64");
  QString slicerRevision("30987");

  QStringList expectedEnabledExtensionNames;
  QStringList extensionNamesToDisable;
  QStringList expectedEnabledExtensionNamesAfterDisable;
  QTest::newRow("0 installed") << operatingSystem << architecture << slicerRevision << (QList<int>())
                               << expectedEnabledExtensionNames << extensionNamesToDisable
                               << expectedEnabledExtensionNamesAfterDisable;

  expectedEnabledExtensionNames = this->expectedExtensionNames();
  std::sort(expectedEnabledExtensionNames.begin(), expectedEnabledExtensionNames.end());
  extensionNamesToDisable = QStringList()
                            << this->expectedExtensionNames().at(1) << this->expectedExtensionNames().at(2);
  expectedEnabledExtensionNamesAfterDisable = QStringList() << this->expectedExtensionNames().at(0);
  std::sort(expectedEnabledExtensionNamesAfterDisable.begin(), expectedEnabledExtensionNamesAfterDisable.end());
  QTest::newRow("3 installed") << operatingSystem << architecture << slicerRevision << (QList<int>() << 0 << 1 << 2)
                               << expectedEnabledExtensionNames << extensionNamesToDisable
                               << expectedEnabledExtensionNamesAfterDisable;

  expectedEnabledExtensionNames.removeOne(this->extensionNameFromExtenionId(2));
  extensionNamesToDisable = QStringList() << this->expectedExtensionNames().at(0);
  expectedEnabledExtensionNamesAfterDisable = QStringList() << this->expectedExtensionNames().at(1);
  std::sort(expectedEnabledExtensionNamesAfterDisable.begin(), expectedEnabledExtensionNamesAfterDisable.end());
  QTest::newRow("2 installed") << operatingSystem << architecture << slicerRevision << (QList<int>() << 0 << 1)
                               << expectedEnabledExtensionNames << extensionNamesToDisable
                               << expectedEnabledExtensionNamesAfterDisable;

  expectedEnabledExtensionNames.removeOne(this->extensionNameFromExtenionId(1));
  extensionNamesToDisable = QStringList() << this->expectedExtensionNames().at(0);
  expectedEnabledExtensionNamesAfterDisable = QStringList();
  std::sort(expectedEnabledExtensionNamesAfterDisable.begin(), expectedEnabledExtensionNamesAfterDisable.end());
  QTest::newRow("1 installed") << operatingSystem << architecture << slicerRevision << (QList<int>() << 0)
                               << expectedEnabledExtensionNames << extensionNamesToDisable
                               << expectedEnabledExtensionNamesAfterDisable;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetExtensionsSettingsFilePath()
{
  QFETCH(QString, extensionsSettingsFilePath);
  QFETCH(QString, expectedExtensionsSettingsFilePath);

  qSlicerExtensionsManagerModel model;
  QCOMPARE(model.extensionsSettingsFilePath(), QString());

  QSignalSpy spyModelUpdated(&model, SIGNAL(modelUpdated()));
  QSignalSpy spyExtensionsSettingsFilePathChanged(&model, SIGNAL(extensionsSettingsFilePathChanged(QString)));

  model.setExtensionsSettingsFilePath(extensionsSettingsFilePath);
  QCOMPARE(model.extensionsSettingsFilePath(), expectedExtensionsSettingsFilePath);

  QCOMPARE(spyModelUpdated.count(), 0);
  QCOMPARE(spyExtensionsSettingsFilePathChanged.count(), 1);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetExtensionsSettingsFilePath_data()
{
  QVERIFY(this->resetTmp());

  QTest::addColumn<QString>("extensionsSettingsFilePath");
  QTest::addColumn<QString>("expectedExtensionsSettingsFilePath");

  QTest::newRow("existing absolute path - ini extension")
    << this->Tmp.filePath("settings.ini") << this->Tmp.filePath("settings.ini");

  QTest::newRow("existing absolute path - foo extension")
    << this->Tmp.filePath("settings.foo") << this->Tmp.filePath("settings.foo");

  QTest::newRow("existing relative path - ini extension") << "settings.ini"
                                                          << "settings.ini";

  QTest::newRow("nonexistent absolute path - ini extension") << "/path/to/nowhere/45659832254686/settings.ini"
                                                             << "/path/to/nowhere/45659832254686/settings.ini";
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testExtensionAdditionalPathsSettingsUpdated()
{
  QSettings().setValue("Extensions/ServerUrl", QUrl::fromLocalFile(this->Tmp.absolutePath()));
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QFETCH(QString, operatingSystem);
  QFETCH(QString, architecture);
  QFETCH(QString, slicerRevision);
  QFETCH(int, extensionIdToUninstall);
  QFETCH(int, extensionIdToInstall);
  QFETCH(QStringList, modulePaths);

  qSlicerExtensionsManagerModel model;
  model.setExtensionsSettingsFilePath(QSettings().fileName());
  model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
  model.updateModel();

  QCOMPARE(model.installedExtensionsCount(), extensionIdToUninstall == -1 ? 0 : 1);
  if (extensionIdToUninstall >= 0)
  {
    model.setSlicerVersion(this->slicerVersion(operatingSystem, extensionIdToUninstall));
    this->uninstallHelper(&model, this->expectedExtensionNames().at(extensionIdToUninstall));
  }
  QCOMPARE(model.installedExtensionsCount(), 0);

  if (extensionIdToInstall >= 0)
  {
    model.setSlicerVersion(this->slicerVersion(operatingSystem, extensionIdToInstall));
    this->installHelper(&model, operatingSystem, extensionIdToInstall, this->Tmp.absolutePath());
  }

  QStringList currentAdditionalPaths = QSettings().value("Modules/AdditionalPaths").toStringList();
  QCOMPARE(currentAdditionalPaths, modulePaths);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testExtensionAdditionalPathsSettingsUpdated_data()
{
  QVERIFY(this->resetTmp());

  QTest::addColumn<QString>("operatingSystem");
  QTest::addColumn<QString>("architecture");
  QTest::addColumn<QString>("slicerRevision");
  QTest::addColumn<int>("extensionIdToUninstall");
  QTest::addColumn<int>("extensionIdToInstall");
  QTest::addColumn<QStringList>("modulePaths");

  QString operatingSystem = Slicer_OS_LINUX_NAME;
  QString architecture("amd64");
  QString slicerRevision("30987");

  {
    int extensionId = 0;
    QString climodules_lib_dir =
      QString(Self::CLIMODULES_LIB_DIR).replace(Slicer_VERSION, this->slicerVersion(operatingSystem, extensionId));
    QTest::newRow("linux-0-ImageMaker") << operatingSystem << architecture << slicerRevision << -1 << extensionId
                                        << (QStringList() << this->Tmp.filePath("ImageMaker/" + climodules_lib_dir));
  }

  {
    int extensionId = 1;
    QString qtloadablemodules_lib_dir = QString(Self::QTLOADABLEMODULES_LIB_DIR)
                                          .replace(Slicer_VERSION, this->slicerVersion(operatingSystem, extensionId));
    QTest::newRow("linux-1-MarkupsToModel")
      << operatingSystem << architecture << slicerRevision << 0 << extensionId
      << (QStringList() << this->Tmp.filePath("MarkupsToModel/" + qtloadablemodules_lib_dir));
  }

#ifdef Slicer_USE_PYTHONQT
  {
    int extensionId = 2;
    QString qtscriptedmodules_lib_dir = QString(Self::QTSCRIPTEDMODULES_LIB_DIR)
                                          .replace(Slicer_VERSION, this->slicerVersion(operatingSystem, extensionId));
    QTest::newRow("linux-2-CurveMaker") << operatingSystem << architecture << slicerRevision << 1 << extensionId
                                        << (QStringList()
                                            << this->Tmp.filePath("CurveMaker/" + qtscriptedmodules_lib_dir));
  }
#endif

  {
    QTest::newRow("linux-Cleanup") << operatingSystem << architecture << slicerRevision << 2 << -1 << QStringList();
  }

  operatingSystem = Slicer_OS_MAC_NAME;
  architecture = "amd64";
  slicerRevision = "30987";

  {
    int extensionId = 0;
    QString climodules_lib_dir =
      QString(Self::CLIMODULES_LIB_DIR).replace(Slicer_VERSION, this->slicerVersion(operatingSystem, extensionId));
    QTest::newRow("macosx-0-ImageMaker") << operatingSystem << architecture << slicerRevision << -1 << extensionId
                                         << (QStringList() << this->Tmp.filePath("ImageMaker/" + climodules_lib_dir));
  }

  {
    int extensionId = 1;
    QString qtloadablemodules_lib_dir = QString(Self::QTLOADABLEMODULES_LIB_DIR)
                                          .replace(Slicer_VERSION, this->slicerVersion(operatingSystem, extensionId));
    QTest::newRow("macosx-1-MarkupsToModel")
      << operatingSystem << architecture << slicerRevision << 0 << extensionId
      << (QStringList() << this->Tmp.filePath("MarkupsToModel/" + qtloadablemodules_lib_dir));
  }

#ifdef Slicer_USE_PYTHONQT
  {
    int extensionId = 2;
    QString qtscriptedmodules_lib_dir = QString(Self::QTSCRIPTEDMODULES_LIB_DIR)
                                          .replace(Slicer_VERSION, this->slicerVersion(operatingSystem, extensionId));
    QTest::newRow("macosx-2-CurveMaker") << operatingSystem << architecture << slicerRevision << 1 << extensionId
                                         << (QStringList()
                                             << this->Tmp.filePath("CurveMaker/" + qtscriptedmodules_lib_dir));
  }
#endif

  {
    QTest::newRow("macosx-Cleanup") << operatingSystem << architecture << slicerRevision << 2 << -1 << QStringList();
  }
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testExtensionExtensionsSettingsUpdated()
{
  QSettings().setValue("Extensions/ServerUrl", QUrl::fromLocalFile(this->Tmp.absolutePath()));
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QFETCH(QString, operatingSystem);
  QFETCH(QString, architecture);
  QFETCH(QString, slicerRevision);
  QFETCH(ExtensionIdType, extensionIdToUninstall);
  QFETCH(ExtensionIdType, extensionIdToInstall);
  QFETCH(QStringList, libraryPaths);
  QFETCH(QStringList, paths);
  QFETCH(QStringList, pythonPaths);

  qSlicerExtensionsManagerModel model;
  model.setExtensionsSettingsFilePath(QSettings().fileName());
  model.setSlicerRequirements(slicerRevision, operatingSystem, architecture);
  model.updateModel();

  QCOMPARE(model.installedExtensionsCount(), extensionIdToUninstall.second == -1 ? 0 : 1);
  if (extensionIdToUninstall.second >= 0)
  {
    model.setSlicerVersion(this->slicerVersion(extensionIdToUninstall.first, extensionIdToUninstall.second));
    this->uninstallHelper(&model, this->expectedExtensionNames().at(extensionIdToUninstall.second));
  }
  QCOMPARE(model.installedExtensionsCount(), 0);

  if (extensionIdToInstall.second >= 0)
  {
    model.setSlicerVersion(this->slicerVersion(extensionIdToInstall.first, extensionIdToInstall.second));
    this->installHelper(&model, extensionIdToInstall.first, extensionIdToInstall.second, this->Tmp.absolutePath());
  }

  QSettings extensionsSettings(model.extensionsSettingsFilePath(), QSettings::IniFormat);

  QStringList currentLibraryPath =
    qSlicerExtensionsManagerModel::readArrayValues(extensionsSettings, "LibraryPaths", "path");
  QCOMPARE(currentLibraryPath, libraryPaths);

  QStringList currentPaths = qSlicerExtensionsManagerModel::readArrayValues(extensionsSettings, "Paths", "path");
  QCOMPARE(currentPaths, paths);

  QStringList currentPythonPaths =
    qSlicerExtensionsManagerModel::readArrayValues(extensionsSettings, "PYTHONPATH", "path");
  QCOMPARE(currentPythonPaths, pythonPaths);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testExtensionExtensionsSettingsUpdated_data()
{
  QVERIFY(this->resetTmp());

  QTest::addColumn<QString>("operatingSystem");
  QTest::addColumn<QString>("architecture");
  QTest::addColumn<QString>("slicerRevision");
  QTest::addColumn<ExtensionIdType>("extensionIdToUninstall");
  QTest::addColumn<ExtensionIdType>("extensionIdToInstall");
  QTest::addColumn<QStringList>("libraryPaths");
  QTest::addColumn<QStringList>("paths");
  QTest::addColumn<QStringList>("pythonPaths");

  QString operatingSystem = Slicer_OS_LINUX_NAME;
  QString architecture("amd64");
  QString slicerRevision("30987");

  {
    int extensionId = 0;
    QString lib_dir = QString(Self::LIB_DIR).replace(Slicer_VERSION, this->slicerVersion(operatingSystem, extensionId));
    QString climodules_lib_dir =
      QString(Self::CLIMODULES_LIB_DIR).replace(Slicer_VERSION, this->slicerVersion(operatingSystem, extensionId));
    QTest::newRow("linux-0-ImageMaker") << operatingSystem << architecture << slicerRevision
                                        << ExtensionIdType(operatingSystem, -1)
                                        << ExtensionIdType(operatingSystem, extensionId)
                                        << (QStringList() << this->Tmp.filePath("ImageMaker/" + lib_dir)
                                                          << this->Tmp.filePath("ImageMaker/" + climodules_lib_dir))
                                        << (QStringList() << this->Tmp.filePath("ImageMaker/" + climodules_lib_dir))
                                        << QStringList();
  }

  {
    int extensionId = 1;
    QString lib_dir = QString(Self::LIB_DIR).replace(Slicer_VERSION, this->slicerVersion(operatingSystem, extensionId));
    QString qtloadablemodules_lib_dir = QString(Self::QTLOADABLEMODULES_LIB_DIR)
                                          .replace(Slicer_VERSION, this->slicerVersion(operatingSystem, extensionId));
    QTest::newRow("linux-1-MarkupsToModel")
      << operatingSystem << architecture << slicerRevision << ExtensionIdType(operatingSystem, 0)
      << ExtensionIdType(operatingSystem, extensionId)
      << (QStringList() << this->Tmp.filePath("MarkupsToModel/" + lib_dir)
                        << this->Tmp.filePath("MarkupsToModel/" + qtloadablemodules_lib_dir))
      << QStringList() << (QStringList() << this->Tmp.filePath("MarkupsToModel/" + qtloadablemodules_lib_dir));
  }

#ifdef Slicer_USE_PYTHONQT
  {
    int extensionId = 2;
    QString lib_dir = QString(Self::LIB_DIR).replace(Slicer_VERSION, this->slicerVersion(operatingSystem, extensionId));
    QString qtscriptedmodules_lib_dir = QString(Self::QTSCRIPTEDMODULES_LIB_DIR)
                                          .replace(Slicer_VERSION, this->slicerVersion(operatingSystem, extensionId));
    QTest::newRow("linux-2-CurveMaker") << operatingSystem << architecture << slicerRevision
                                        << ExtensionIdType(operatingSystem, 1)
                                        << ExtensionIdType(operatingSystem, extensionId)
                                        << (QStringList() << this->Tmp.filePath("CurveMaker/" + lib_dir))
                                        << QStringList()
                                        << (QStringList()
                                            << this->Tmp.filePath("CurveMaker/" + qtscriptedmodules_lib_dir));
  }
#endif

  {
    QTest::newRow("linux-Cleanup") << operatingSystem << architecture << slicerRevision
                                   << ExtensionIdType(operatingSystem, 2) << ExtensionIdType("", -1) << QStringList()
                                   << QStringList() << QStringList();
  }
}

// ----------------------------------------------------------------------------
namespace
{
typedef void (qSlicerExtensionsManagerModel::*QStringSetter)(const QString&);
typedef QString (qSlicerExtensionsManagerModel::*QStringGetter)() const;

// ----------------------------------------------------------------------------
void testRequirementsHelper(qSlicerExtensionsManagerModel* model,
                            QStringSetter qStringSetterFuncPtr,
                            QStringGetter qStringGetterFuncPtr,
                            const char* changedPropertySignal)
{
  QFETCH(QStringList, valuesToSet);
  QFETCH(QString, expectedFinalValue);
  QFETCH(int, expectedSlicerRequirementsChangedCount);

  QSignalSpy spySlicerRequirementsChanged(model, SIGNAL(slicerRequirementsChanged(QString, QString, QString)));
  QSignalSpy spySlicerPropertyChanged(model, changedPropertySignal);
  foreach (const QString& valuetoSet, valuesToSet)
  {
    (model->*qStringSetterFuncPtr)(valuetoSet);
  }
  QCOMPARE((model->*qStringGetterFuncPtr)(), expectedFinalValue);
  QCOMPARE(spySlicerRequirementsChanged.count(), expectedSlicerRequirementsChangedCount);
  QCOMPARE(spySlicerPropertyChanged.count(), expectedSlicerRequirementsChangedCount);
}

} // end of anonymous namespace

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerRevision()
{
  qSlicerExtensionsManagerModel model;
  testRequirementsHelper(&model,
                         &qSlicerExtensionsManagerModel::setSlicerRevision,
                         &qSlicerExtensionsManagerModel::slicerRevision,
                         SIGNAL(slicerRevisionChanged(QString)));
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerRevision_data()
{
  QTest::addColumn<QStringList>("valuesToSet");
  QTest::addColumn<QString>("expectedFinalValue");
  QTest::addColumn<int>("expectedSlicerRequirementsChangedCount");

  QTest::newRow("0") << (QStringList() << ""
                                       << "1"
                                       << ""
                                       << "1")
                     << "1" << 3;
  QTest::newRow("1") << (QStringList() << ""
                                       << "1"
                                       << "1")
                     << "1" << 1;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerOs()
{
  qSlicerExtensionsManagerModel model;
  testRequirementsHelper(&model,
                         &qSlicerExtensionsManagerModel::setSlicerOs,
                         &qSlicerExtensionsManagerModel::slicerOs,
                         SIGNAL(slicerOsChanged(QString)));
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerOs_data()
{
  QTest::addColumn<QStringList>("valuesToSet");
  QTest::addColumn<QString>("expectedFinalValue");
  QTest::addColumn<int>("expectedSlicerRequirementsChangedCount");

  QTest::newRow("0") << (QStringList() << ""
                                       << "linux"
                                       << ""
                                       << "linux")
                     << "linux" << 3;
  QTest::newRow("1") << (QStringList() << ""
                                       << "linux"
                                       << "linux")
                     << "linux" << 1;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerArch()
{
  qSlicerExtensionsManagerModel model;
  testRequirementsHelper(&model,
                         &qSlicerExtensionsManagerModel::setSlicerArch,
                         &qSlicerExtensionsManagerModel::slicerArch,
                         SIGNAL(slicerArchChanged(QString)));
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerArch_data()
{
  QTest::addColumn<QStringList>("valuesToSet");
  QTest::addColumn<QString>("expectedFinalValue");
  QTest::addColumn<int>("expectedSlicerRequirementsChangedCount");

  QTest::newRow("0") << (QStringList() << ""
                                       << "amd64"
                                       << ""
                                       << "amd64")
                     << "amd64" << 3;
  QTest::newRow("1") << (QStringList() << ""
                                       << "amd64"
                                       << "amd64")
                     << "amd64" << 1;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerRequirements()
{
  qSlicerExtensionsManagerModel model;
  QSignalSpy spySlicerRequirementsChanged(&model, SIGNAL(slicerRequirementsChanged(QString, QString, QString)));

  QCOMPARE(model.slicerRevision(), QString(""));
  QCOMPARE(model.slicerOs(), QString(""));
  QCOMPARE(model.slicerArch(), QString(""));

  QFETCH(QList<QStringList>, requirementsToSetList);
  QFETCH(QList<int>, expectedSlicerRequirementsChangedCounts);
  QFETCH(QStringList, finalExpectedSlicerRequirement);

  QCOMPARE(requirementsToSetList.count(), expectedSlicerRequirementsChangedCounts.count());

  for (int setIdx = 0; setIdx < requirementsToSetList.count(); ++setIdx)
  {
    QStringList set(requirementsToSetList.at(setIdx));
    QCOMPARE(set.count(), 3);
    model.setSlicerRequirements(set.at(0), set.at(1), set.at(2));
    QCOMPARE(spySlicerRequirementsChanged.count(), expectedSlicerRequirementsChangedCounts.at(setIdx));
  }

  QCOMPARE(model.slicerRevision(), finalExpectedSlicerRequirement.at(0));
  QCOMPARE(model.slicerOs(), finalExpectedSlicerRequirement.at(1));
  QCOMPARE(model.slicerArch(), finalExpectedSlicerRequirement.at(2));
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerRequirements_data()
{
  QTest::addColumn<QList<QStringList>>("requirementsToSetList");
  QTest::addColumn<QList<int>>("expectedSlicerRequirementsChangedCounts");
  QTest::addColumn<QStringList>("finalExpectedSlicerRequirement");

  QList<QStringList> requirementsToSetList = QList<QStringList>() << (QStringList() << ""
                                                                                    << ""
                                                                                    << "")
                                                                  << (QStringList() << "5"
                                                                                    << ""
                                                                                    << "")
                                                                  << (QStringList() << "5"
                                                                                    << "linux"
                                                                                    << "")
                                                                  << (QStringList() << "5"
                                                                                    << "linux"
                                                                                    << "")
                                                                  << (QStringList() << "5"
                                                                                    << "linux"
                                                                                    << "amd64")
                                                                  << (QStringList() << "5"
                                                                                    << "linux"
                                                                                    << "amd64")
                                                                  << (QStringList() << "74"
                                                                                    << "linux"
                                                                                    << "amd64");
  QTest::newRow("0") << requirementsToSetList << (QList<int>() << 0 << 1 << 2 << 2 << 3 << 3 << 4)
                     << (QStringList() << "74"
                                       << "linux"
                                       << "amd64");
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerVersion()
{
  qSlicerExtensionsManagerModel model;

  QFETCH(QString, valueToSet);
  QFETCH(QString, expectedFinalValue);

  QSignalSpy spySlicerRequirementsChanged(&model, SIGNAL(slicerRequirementsChanged(QString, QString, QString)));
  model.setSlicerVersion(valueToSet);
  QCOMPARE(model.slicerVersion(), expectedFinalValue);
  QCOMPARE(spySlicerRequirementsChanged.count(), 0);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerVersion_data()
{
  QTest::addColumn<QString>("valueToSet");
  QTest::addColumn<QString>("expectedFinalValue");

  QTest::newRow("0") << "3.9"
                     << "3.9";
  QTest::newRow("1") << "4.0"
                     << "4.0";
  QTest::newRow("2") << "4.1"
                     << "4.1";
}

// ----------------------------------------------------------------------------
int qSlicerExtensionsManagerModelTest(int argc, char* argv[])
{
  qSlicerCoreApplication app(argc, argv);
  QTEST_DISABLE_KEYPAD_NAVIGATION
  qSlicerExtensionsManagerModelTester tc;
  return QTest::qExec(&tc, argc, argv);
}
#include "moc_qSlicerExtensionsManagerModelTest.cxx"
