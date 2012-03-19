
// Qt includes
#include <QApplication>

// CTK includes
#include <ctkTest.h>
#include <ctkUtils.h>

// QtCore includes
#include "qSlicerExtensionsManagerModel.h"
#include "vtkSlicerConfigure.h"
#include "vtkSlicerVersionConfigure.h"

// STD includes
#include <iostream>

// ----------------------------------------------------------------------------
class qSlicerExtensionsManagerModelTester: public QObject
{
  Q_OBJECT
  typedef qSlicerExtensionsManagerModel::ExtensionMetadataType ExtensionMetadataType;

  void dumpExtensionMetatype(const char * varname, const ExtensionMetadataType& extensionMetadata);

  ExtensionMetadataType extensionMetadata(int extensionId, bool filtered = false)const;
  ExtensionMetadataType extensionMetadata0(bool filtered = false)const;
  ExtensionMetadataType extensionMetadata1(bool filtered = false)const;
  ExtensionMetadataType extensionMetadata2(bool filtered = false)const;
  ExtensionMetadataType extensionMetadata3(bool filtered = false)const;

  QStringList expectedExtensionNames()const;

  void initializeModel(qSlicerExtensionsManagerModel *model);

  bool prepareJson(const QString& jsonFile);

  void installHelper(qSlicerExtensionsManagerModel *model, int extensionId, const QString &tmp);

  bool resetTmp();
  QDir Tmp;
  QString TemporaryDirName;

  QString SlicerVersion;
  QString CLIMODULES_LIB_DIR;
  QString QTLOADABLEMODULES_LIB_DIR;
  QString QTLOADABLEMODULES_PYTHON_LIB_DIR;
  QString QTSCRIPTEDMODULES_LIB_DIR;

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

  void testUpdateModel();

  void testIsExtensionInstalled();
  void testIsExtensionInstalled_data();

  void testNumberOfInstalledExtensions();
  void testNumberOfInstalledExtensions_data();

  void testInstalledExtensions();
  void testInstalledExtensions_data();

  void testIsExtensionEnabled();
  void testIsExtensionEnabled_data();

  void testSetLauncherSettingsFilePath();
  void testSetLauncherSettingsFilePath_data();

  void testExtensionAdditionalPathsSettingsUpdated();
  void testExtensionAdditionalPathsSettingsUpdated_data();

  void testExtensionLauncherSettingsUpdated();
  void testExtensionLauncherSettingsUpdated_data();

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

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<QString>)
Q_DECLARE_METATYPE(QList<QStringList>)

// ----------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModelTester::expectedExtensionNames()const
{
  return QStringList()
      << "CLIExtensionTemplate" << "LoadableExtensionTemplate"
      << "ScriptedLoadableExtensionTemplate" << "SuperBuildLoadableExtensionTemplate";
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::initializeModel(qSlicerExtensionsManagerModel * model)
{
  QVERIFY(model != 0);
  model->setSlicerVersion(this->SlicerVersion);
}

// ----------------------------------------------------------------------------
bool qSlicerExtensionsManagerModelTester::prepareJson(const QString& jsonFile)
{
  bool success = true;
  QDir tmp = QDir::temp();
  success = tmp.mkdir(this->TemporaryDirName);
  success = tmp.cd(this->TemporaryDirName);
  if (tmp.exists("api"))
    {
    success = success && ctk::removeDirRecursively(tmp.filePath("api"));
    }
  success = success && tmp.mkdir("api");
  success = success && tmp.cd("api");
  success = success && QFile::copy(jsonFile,  tmp.filePath("json"));
  return success;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::installHelper(qSlicerExtensionsManagerModel * model,
                                                        int extensionId,
                                                        const QString& tmp)
{
  QVERIFY(model != 0);
  QVERIFY(extensionId >= 0 && extensionId <= 3);

  QString inputArchiveFile = QString(":/extension-linux-%1.tar.gz").arg(extensionId);
  QString copiedArchiveFile = tmp + "/" + QFileInfo(inputArchiveFile).fileName();
  if (!QFile::exists(copiedArchiveFile))
    {
    QVERIFY2(QFile::copy(inputArchiveFile, copiedArchiveFile),
             QString("Failed to copy %1 into %2").arg(inputArchiveFile).arg(copiedArchiveFile).toLatin1());
    }

  QVERIFY2(this->prepareJson(QString(":/extension-linux-%1.json").arg(extensionId)),
           QString("Failed to prepare json for extensionId: %1").arg(extensionId).toLatin1());
  ExtensionMetadataType metadata = model->retrieveExtensionMetadata(QString("%1").arg(extensionId));
  QVERIFY(metadata.count() > 0);
  QCOMPARE(metadata, this->extensionMetadata(extensionId));

  QString extensionName = metadata.value("extensionname").toString();
  QCOMPARE(extensionName, this->expectedExtensionNames().at(extensionId));

  QVERIFY(!model->isExtensionInstalled(extensionName));
  QVERIFY(!model->isExtensionEnabled(extensionName));

  QSignalSpy spyExtensionInstalled(model, SIGNAL(extensionInstalled(QString)));
  QSignalSpy spyExtensionEnabledChanged(model, SIGNAL(extensionEnabledChanged(QString,bool)));

  QVERIFY(model->installExtension(extensionName, metadata, copiedArchiveFile));
  QCOMPARE(model->extensionMetadata(extensionName), this->extensionMetadata(extensionId, /* filtered= */ true));
  QCOMPARE(model->extensionDescriptionFile(extensionName),
           model->extensionsInstallPath() + "/" + extensionName + ".s4ext");

  QCOMPARE(spyExtensionInstalled.count(), 1);
  QCOMPARE(spyExtensionEnabledChanged.count(), 0);

  QVERIFY(!model->installExtension(extensionName, metadata, copiedArchiveFile));
  QCOMPARE(spyExtensionInstalled.count(), 1);
  QCOMPARE(spyExtensionEnabledChanged.count(), 0);

  QVERIFY(model->isExtensionInstalled(extensionName));
  QVERIFY(model->isExtensionEnabled(extensionName));
}

//// ----------------------------------------------------------------------------
//QDir qSlicerExtensionsManagerModelTester::tmp()
//{
//  QVERIFY(!this->TemporaryDirName.isEmpty());
//  QDir tmp(QDir::temp());
//  if (!tmp.exists(this->TemporaryDirName))
//    {
//    QVERIFY(tmp.mkdir(this->TemporaryDirName));
//    }
//  QVERIFY(tmp.cd(this->TemporaryDirName));
//  QVERIFY(tmp.exists());
//  return tmp;
//}

// ----------------------------------------------------------------------------
bool qSlicerExtensionsManagerModelTester::resetTmp()
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
void qSlicerExtensionsManagerModelTester::dumpExtensionMetatype(
    const char * varname, const ExtensionMetadataType& extensionMetadata)
{
  std::cout << "[" << varname << "]" << std::endl;
  foreach(const QString& key, extensionMetadata.keys())
    {
    std::cout << varname << ".insert(\"" << qPrintable(key) << "\", "
              << "\"" << qPrintable(extensionMetadata.value(key).toString()) << "\");" << std::endl;
    }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
qSlicerExtensionsManagerModelTester::ExtensionMetadataType
qSlicerExtensionsManagerModelTester::extensionMetadata(int extensionId, bool filtered)const
{
  if (extensionId == 0) { return this->extensionMetadata0(filtered); }
  else if (extensionId == 1) { return this->extensionMetadata1(filtered); }
  else if (extensionId == 2) { return this->extensionMetadata2(filtered); }
  else if (extensionId == 3) { return this->extensionMetadata3(filtered); }
  else { return ExtensionMetadataType(); }
}

// ----------------------------------------------------------------------------
qSlicerExtensionsManagerModelTester::ExtensionMetadataType
qSlicerExtensionsManagerModelTester::extensionMetadata0(bool filtered)const
{
  QVariantMap allMetadata;
  allMetadata.insert("arch", "amd64");
  allMetadata.insert("archivename", "19354-linux-amd64-CLIExtensionTemplate-svn19354-2012-02-23.tar.gz");
  allMetadata.insert("bitstream_id", "476");
  allMetadata.insert("category", "Examples");
  allMetadata.insert("codebase", "Slicer4");
  allMetadata.insert("contributors", "");
  allMetadata.insert("date_creation", "2012-02-23 15:01:16");
  allMetadata.insert("description", "This is an example of CLI module built as an extension");
  allMetadata.insert("status", "");
  allMetadata.insert("enabled", "1");
  allMetadata.insert("extension_id", "12");
  allMetadata.insert("extensionname", "CLIExtensionTemplate");
  allMetadata.insert("homepage", "http://www.slicer.org/slicerWiki/index.php/Slicer4:Developers:Projects:QtSlicer/Tutorials/ExtensionWriting");
  allMetadata.insert("item_id", "12");
  allMetadata.insert("md5", "d4726e1fd85b19930e0e8e8e5d6afa62");
  allMetadata.insert("os", "linux");
  allMetadata.insert("package", "archive");
  allMetadata.insert("release", "");
  allMetadata.insert("revision", "19354");
  allMetadata.insert("scm", "svn");
  allMetadata.insert("scmurl", "http://svn.slicer.org/Slicer4/trunk/Extensions/Testing/CLIExtensionTemplate");
  allMetadata.insert("screenshots", "");
  allMetadata.insert("size", "1747463");
  allMetadata.insert("slicer_revision", "19354");
  allMetadata.insert("submissiontype", "experimental");

  QStringList keysToIgnore(qSlicerExtensionsManagerModel::serverKeysToIgnore());
  QVariantMap metadata;
  foreach(const QString& key, allMetadata.keys())
    {
    if (filtered && keysToIgnore.contains(key))
      {
      continue;
      }
    metadata.insert(key, allMetadata.value(key));
    }

  return metadata;
}

// ----------------------------------------------------------------------------
qSlicerExtensionsManagerModelTester::ExtensionMetadataType
qSlicerExtensionsManagerModelTester::extensionMetadata1(bool filtered)const
{
  QVariantMap allMetadata;
  allMetadata.insert("arch", "amd64");
  allMetadata.insert("archivename", "19354-linux-amd64-LoadableExtensionTemplate-svn19354-2012-02-23.tar.gz");
  allMetadata.insert("bitstream_id", "475");
  allMetadata.insert("category", "Examples");
  allMetadata.insert("codebase", "Slicer4");
  allMetadata.insert("contributors", "");
  allMetadata.insert("date_creation", "2012-02-23 15:01:03");
  allMetadata.insert("description", "This is an example of Qt loadable module built as an extension");
  allMetadata.insert("status", "");
  allMetadata.insert("enabled", "1");
  allMetadata.insert("extension_id", "11");
  allMetadata.insert("extensionname", "LoadableExtensionTemplate");
  allMetadata.insert("homepage", "http://www.slicer.org/slicerWiki/index.php/Slicer4:Developers:Projects:QtSlicer/Tutorials/ExtensionWriting");
  allMetadata.insert("item_id", "11");
  allMetadata.insert("md5", "bfd588ed592a66cb62f3977980118726");
  allMetadata.insert("os", "linux");
  allMetadata.insert("package", "archive");
  allMetadata.insert("release", "");
  allMetadata.insert("revision", "19354");
  allMetadata.insert("scm", "svn");
  allMetadata.insert("scmurl", "http://svn.slicer.org/Slicer4/trunk/Extensions/Testing/LoadableExtensionTemplate");
  allMetadata.insert("screenshots", "");
  allMetadata.insert("size", "475700");
  allMetadata.insert("slicer_revision", "19354");
  allMetadata.insert("submissiontype", "experimental");

  QStringList keysToIgnore(qSlicerExtensionsManagerModel::serverKeysToIgnore());
  QVariantMap metadata;
  foreach(const QString& key, allMetadata.keys())
    {
    if (filtered && keysToIgnore.contains(key))
      {
      continue;
      }
    metadata.insert(key, allMetadata.value(key));
    }

  return metadata;
}

// ----------------------------------------------------------------------------
qSlicerExtensionsManagerModelTester::ExtensionMetadataType
qSlicerExtensionsManagerModelTester::extensionMetadata2(bool filtered)const
{
  QVariantMap allMetadata;
  allMetadata.insert("arch", "amd64");
  allMetadata.insert("archivename", "19354-linux-amd64-ScriptedLoadableExtensionTemplate-svn19354-2012-02-23.tar.gz");
  allMetadata.insert("bitstream_id", "473");
  allMetadata.insert("category", "Examples");
  allMetadata.insert("codebase", "Slicer4");
  allMetadata.insert("contributors", "");
  allMetadata.insert("date_creation", "2012-02-23 15:01:01");
  allMetadata.insert("description", "This is an example of scripted Qt loadable module built as an extension");
  allMetadata.insert("status", "");
  allMetadata.insert("enabled", "1");
  allMetadata.insert("extension_id", "10");
  allMetadata.insert("extensionname", "ScriptedLoadableExtensionTemplate");
  allMetadata.insert("homepage", "http://www.slicer.org/slicerWiki/index.php/Slicer4:Developers:Projects:QtSlicer/Tutorials/ExtensionWriting");
  allMetadata.insert("item_id", "9");
  allMetadata.insert("md5", "ff6a75a8ee2d926192342389d8068719");
  allMetadata.insert("os", "linux");
  allMetadata.insert("package", "archive");
  allMetadata.insert("release", "");
  allMetadata.insert("revision", "19354");
  allMetadata.insert("scm", "svn");
  allMetadata.insert("scmurl", "http://svn.slicer.org/Slicer4/trunk/Extensions/Testing/ScriptedLoadableExtensionTemplate");
  allMetadata.insert("screenshots", "");
  allMetadata.insert("size", "1894");
  allMetadata.insert("slicer_revision", "19354");
  allMetadata.insert("submissiontype", "experimental");

  QStringList keysToIgnore(qSlicerExtensionsManagerModel::serverKeysToIgnore());
  QVariantMap metadata;
  foreach(const QString& key, allMetadata.keys())
    {
    if (filtered && keysToIgnore.contains(key))
      {
      continue;
      }
    metadata.insert(key, allMetadata.value(key));
    }

  return metadata;
}

// ----------------------------------------------------------------------------
qSlicerExtensionsManagerModelTester::ExtensionMetadataType
qSlicerExtensionsManagerModelTester::extensionMetadata3(bool filtered)const
{
  QVariantMap allMetadata;
  allMetadata.insert("arch", "amd64");
  allMetadata.insert("archivename", "19354-linux-amd64-SuperBuildLoadableExtensionTemplate-svn19354-2012-02-23.tar.gz");
  allMetadata.insert("bitstream_id", "474");
  allMetadata.insert("category", "Examples");
  allMetadata.insert("codebase", "Slicer4");
  allMetadata.insert("contributors", "");
  allMetadata.insert("date_creation", "2012-02-23 15:01:02");
  allMetadata.insert("description", "This is an example of Qt loadable module using SuperBuild built as an extension");
  allMetadata.insert("status", "");
  allMetadata.insert("enabled", "1");
  allMetadata.insert("extension_id", "9");
  allMetadata.insert("extensionname", "SuperBuildLoadableExtensionTemplate");
  allMetadata.insert("homepage", "http://www.slicer.org/slicerWiki/index.php/Slicer4:Developers:Projects:QtSlicer/Tutorials/ExtensionWriting");
  allMetadata.insert("item_id", "10");
  allMetadata.insert("md5", "7ff17937aa83a8b05543d0dd158fa595");
  allMetadata.insert("os", "linux");
  allMetadata.insert("package", "archive");
  allMetadata.insert("release", "");
  allMetadata.insert("revision", "19354");
  allMetadata.insert("scm", "svn");
  allMetadata.insert("scmurl", "http://svn.slicer.org/Slicer4/trunk/Extensions/Testing/SuperBuildLoadableExtensionTemplate");
  allMetadata.insert("screenshots", "");
  allMetadata.insert("size", "476446");
  allMetadata.insert("slicer_revision", "19354");
  allMetadata.insert("submissiontype", "experimental");

  QStringList keysToIgnore(qSlicerExtensionsManagerModel::serverKeysToIgnore());
  QVariantMap metadata;
  foreach(const QString& key, allMetadata.keys())
    {
    if (filtered && keysToIgnore.contains(key))
      {
      continue;
      }
    metadata.insert(key, allMetadata.value(key));
    }

  return metadata;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::initTestCase()
{
  QVERIFY(QDir::temp().exists());

  this->TemporaryDirName =
      QString("qSlicerExtensionsManagerModelTester.%1").arg(QTime::currentTime().toString("hhmmsszzz"));

  // Setup QApplication for settings
  qApp->setOrganizationName("NA-MIC");
  qApp->setOrganizationDomain("www.slicer.org/tests");
  qApp->setApplicationName("SlicerTests");

  this->SlicerVersion = "4.0"; // Version of Slicer used to generate baseline data

  this->CLIMODULES_LIB_DIR = QString(Slicer_CLIMODULES_LIB_DIR).replace(Slicer_VERSION, this->SlicerVersion);
  this->QTLOADABLEMODULES_LIB_DIR = QString(Slicer_QTLOADABLEMODULES_LIB_DIR).replace(Slicer_VERSION, this->SlicerVersion);
  this->QTLOADABLEMODULES_PYTHON_LIB_DIR = QString(Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR).replace(Slicer_VERSION, this->SlicerVersion);
  this->QTSCRIPTEDMODULES_LIB_DIR = QString(Slicer_QTSCRIPTEDMODULES_LIB_DIR).replace(Slicer_VERSION, this->SlicerVersion);

  QSettings().clear();
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::init()
{
}

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
    //ctk::removeDirRecursively(this->Tmp.absolutePath());
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
  QCOMPARE(model.serverUrlWithPackagePath().toString(), QLatin1String("/slicerpackages"));
  QCOMPARE(model.serverUrlWithExtensionsStorePath().toString(), QLatin1String("/slicerappstore"));
  QCOMPARE(model.extensionsInstallPath(), QString(""));
  QCOMPARE(model.extensionInstallPath(""), QString(""));
  QCOMPARE(model.extensionDescriptionFile(""), QString(""));
  QCOMPARE(model.newExtensionEnabledByDefault(), true);
  QCOMPARE(model.slicerVersion(), QString(Slicer_VERSION));

  ExtensionMetadataType metadata = model.extensionMetadata("");
  QCOMPARE(metadata.count(), 0);

  QCOMPARE(model.isExtensionInstalled(""), false);
  QCOMPARE(model.numberOfInstalledExtensions(), 0);
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
  QFETCH(QString, serverUrlWithPackagePath);
  QFETCH(QString, serverUrlWithExtensionsStorePath);

  QSettings().setValue("Extensions/ServerUrl", serverUrl);
  qSlicerExtensionsManagerModel model;
  this->initializeModel(&model);

  QCOMPARE(model.serverUrl().toString(), serverUrl);
  QCOMPARE(model.serverUrlWithPackagePath().toString(), serverUrlWithPackagePath);
  QCOMPARE(model.serverUrlWithExtensionsStorePath().toString(), serverUrlWithExtensionsStorePath);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testServerUrl_data()
{
  QTest::addColumn<QString>("serverUrl");
  QTest::addColumn<QString>("serverUrlWithPackagePath");
  QTest::addColumn<QString>("serverUrlWithExtensionsStorePath");

  QTest::newRow("localhost") << "http://localhost/midas"
                             << "http://localhost/midas/slicerpackages"
                             << "http://localhost/midas/slicerappstore";
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testServerKeysToIgnore()
{
  QFETCH(QStringList, expectedServerKeysToIgnore);
  QCOMPARE(qSlicerExtensionsManagerModel::serverKeysToIgnore(), expectedServerKeysToIgnore);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testServerKeysToIgnore_data()
{
  QTest::addColumn<QStringList>("expectedServerKeysToIgnore");

  QTest::newRow("0") << (QStringList()
                         << "item_id" << "extension_id" << "bitstream_id"
                         << "submissiontype" << "codebase" << "package"
                         << "size" << "date_creation");
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testServerToExtensionDescriptionKey()
{
  QHash<QString, QString> serverToExtensionDescriptionKey =
      qSlicerExtensionsManagerModel::serverToExtensionDescriptionKey();

  QFETCH(QList<QString>, expectedServerKeys);
  QCOMPARE(serverToExtensionDescriptionKey.keys(), expectedServerKeys);

  QFETCH(QList<QString>, expectedExtensionDescriptionKeys);
  QCOMPARE(serverToExtensionDescriptionKey.values(), expectedExtensionDescriptionKeys);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testServerToExtensionDescriptionKey_data()
{
  QTest::addColumn<QList<QString> >("expectedServerKeys");
  QTest::addColumn<QList<QString> >("expectedExtensionDescriptionKeys");

  QHash<QString, QString> expected;
  expected.insert("productname", "extensionname");
  expected.insert("name", "archivename");
  expected.insert("repository_type", "scm");
  expected.insert("repository_url", "scmurl");
  expected.insert("development_status", "status");

  QTest::newRow("0") << expected.keys() << expected.values();
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testRetrieveExtensionMetadata()
{
  QVERIFY(this->resetTmp());

  QSettings().setValue("Extensions/ServerUrl", "file://" + this->Tmp.absolutePath());

  QFETCH(QString, extensionId);
  QFETCH(QString, jsonFile);
  QVERIFY2(this->prepareJson(jsonFile),
           QString("Failed to prepare json for extensionId: %1").arg(extensionId).toLatin1());

  qSlicerExtensionsManagerModel model;
  this->initializeModel(&model);
  ExtensionMetadataType extensionMetadata = model.retrieveExtensionMetadata(extensionId);

  QFETCH(QVariantMap, expectedResult);
  QCOMPARE(extensionMetadata, expectedResult);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testRetrieveExtensionMetadata_data()
{
  QTest::addColumn<QString>("extensionId");
  QTest::addColumn<QString>("jsonFile");
  QTest::addColumn<QVariantMap>("expectedResult");

  QTest::newRow("0") << "0" << QString(":/extension-linux-0.json") << this->extensionMetadata0();
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testFilterExtensionMetadata()
{
  QFETCH(QVariantMap, inputExtensionMetadata);
  QFETCH(QVariantMap, expectedExtensionMetadata);
  ExtensionMetadataType filteredExtensionMetadata =
      qSlicerExtensionsManagerModel::filterExtensionMetadata(inputExtensionMetadata);
  QCOMPARE(filteredExtensionMetadata, expectedExtensionMetadata);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testFilterExtensionMetadata_data()
{
  QTest::addColumn<QVariantMap>("inputExtensionMetadata");
  QTest::addColumn<QVariantMap>("expectedExtensionMetadata");

  QTest::newRow("0") << this->extensionMetadata0() << this->extensionMetadata0(/* filtered= */ true);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testExtractExtensionArchive()
{
  QVERIFY(this->resetTmp());

  QFETCH(QString, inputExtensionName);
  QFETCH(QString, inputArchiveFile);
  QFETCH(QStringList, expectedFiles);
  QFETCH(QString, operatingSystem);

  qSlicerExtensionsManagerModel model;
  this->initializeModel(&model);
  model.setSlicerOs(operatingSystem);

  QString copiedArchiveFile = this->Tmp.filePath(QFileInfo(inputArchiveFile).fileName());
  QVERIFY(QFile::copy(inputArchiveFile, copiedArchiveFile));

  QVERIFY(model.extractExtensionArchive(
        inputExtensionName, copiedArchiveFile, this->Tmp.absolutePath()));

  bool expectedFilesExist = true;
  foreach(const QString& expectedFile, expectedFiles)
    {
    expectedFilesExist = expectedFilesExist && QFile::exists(this->Tmp.filePath(expectedFile));
    if(!expectedFilesExist)
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
  QTest::addColumn<QStringList>("expectedFiles");
  QTest::addColumn<QString>("operatingSystem");
  {
    QStringList expectedFiles;
    expectedFiles << "CLIExtensionTemplate";
    expectedFiles << "CLIExtensionTemplate/lib/Slicer-4.0/cli-modules/CLIExtensionTemplate";
    expectedFiles << "CLIExtensionTemplate/lib/Slicer-4.0/cli-modules/libCLIExtensionTemplateLib.so";
    QTest::newRow("0") << "CLIExtensionTemplate"
                       << ":/extension-linux-0.tar.gz"
                       << expectedFiles
                       << "linux";
  }
  {
    QStringList expectedFiles;
    expectedFiles << "CLIExtensionTemplate";
    expectedFiles << "CLIExtensionTemplate/cli-modules/CLIExtensionTemplate";
    expectedFiles << "CLIExtensionTemplate/lib/Slicer-4.1/cli-modules/libCLIExtensionTemplateLib.dylib";
    QTest::newRow("1") << "CLIExtensionTemplate"
                       << ":/extension-macosx-0.tar.gz"
                       << expectedFiles
                       << Slicer_OS_MAC_NAME;
  }
  {
    QStringList expectedFiles;
    expectedFiles << "LoadableExtensionTemplate";
    expectedFiles << "LoadableExtensionTemplate/lib/Slicer-4.1/qt-loadable-modules/libqSlicerLoadableExtensionTemplateModule.dylib";
    expectedFiles << "LoadableExtensionTemplate/lib/Slicer-4.1/qt-loadable-modules/libvtkSlicerLoadableExtensionTemplateModuleLogic.dylib";
    expectedFiles << "LoadableExtensionTemplate/lib/Slicer-4.1/qt-loadable-modules/libvtkSlicerLoadableExtensionTemplateModuleLogicPythonD.dylib";
    expectedFiles << "LoadableExtensionTemplate/lib/Slicer-4.1/qt-loadable-modules/Python/vtkSlicerLoadableExtensionTemplateModuleLogic.py";
    expectedFiles << "LoadableExtensionTemplate/lib/Slicer-4.1/qt-loadable-modules/Python/vtkSlicerLoadableExtensionTemplateModuleLogic.pyc";
    expectedFiles << "LoadableExtensionTemplate/lib/Slicer-4.1/qt-loadable-modules/vtkSlicerLoadableExtensionTemplateModuleLogicPython.so";
    QTest::newRow("2") << "LoadableExtensionTemplate"
                       << ":/extension-macosx-1.tar.gz"
                       << expectedFiles
                       << Slicer_OS_MAC_NAME;
  }
  // TODO Add windows case
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testWriteAndParseExtensionDescriptionFile()
{
  QVERIFY(this->resetTmp());

  QFETCH(ExtensionMetadataType, metadata);

  QVERIFY(qSlicerExtensionsManagerModel::writeExtensionDescriptionFile(this->Tmp.filePath("CLIExtensionTemplate.s4ext"), metadata));

  ExtensionMetadataType parsedMetadata =
      qSlicerExtensionsManagerModel::parseExtensionDescriptionFile(this->Tmp.filePath("CLIExtensionTemplate.s4ext"));

  QCOMPARE(metadata.count(), parsedMetadata.count());
  QCOMPARE(metadata, parsedMetadata);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testWriteAndParseExtensionDescriptionFile_data()
{
  QTest::addColumn<ExtensionMetadataType>("metadata");

  ExtensionMetadataType metadata;
  metadata.insert("arch", "amd64");
  metadata.insert("archivename", "19354-linux-amd64-CLIExtensionTemplate-svn19354-2012-02-23.tar.gz");
  metadata.insert("category", "Examples");
  metadata.insert("contributors", "");
  metadata.insert("description", "This is an example of CLI module built as an extension");
  metadata.insert("enabled", "true");
  metadata.insert("extensionname", "CLIExtensionTemplate");
  metadata.insert("homepage", "http://www.slicer.org/slicerWiki/index.php/Slicer4:Developers:Projects:QtSlicer/Tutorials/ExtensionWriting");
  metadata.insert("md5", "d4726e1fd85b19930e0e8e8e5d6afa62");
  metadata.insert("os", "linux");
  metadata.insert("release", "");
  metadata.insert("revision", "19354");
  metadata.insert("scm", "svn");
  metadata.insert("scmurl", "http://svn.slicer.org/Slicer4/trunk/Extensions/Testing/CLIExtensionTemplate");
  metadata.insert("screenshots", "");
  metadata.insert("slicer_revision", "19354");
  QTest::newRow("0-CLIExtensionTemplate") << metadata;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testInstallExtension()
{
  QVERIFY(this->resetTmp());

  QSettings().setValue("Extensions/ServerUrl", "file://" + this->Tmp.absolutePath());
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QStringList expectedExtensionNames;
  expectedExtensionNames << "CLIExtensionTemplate" << "LoadableExtensionTemplate"
                         << "ScriptedLoadableExtensionTemplate" << "SuperBuildLoadableExtensionTemplate";

  qSlicerExtensionsManagerModel model;
  this->initializeModel(&model);
  QSignalSpy spyModelUpdated(&model, SIGNAL(modelUpdated()));
  QSignalSpy spyExtensionUninstalled(&model, SIGNAL(extensionUninstalled(QString)));
  QSignalSpy spySlicerRequirementsChanged(&model, SIGNAL(slicerRequirementsChanged(QString,QString,QString)));

  for (int extensionId = 0; extensionId < 4; ++extensionId)
    {
    this->installHelper(&model, extensionId, this->Tmp.absolutePath());

    QCOMPARE(model.numberOfInstalledExtensions(), extensionId + 1);

    QString extensionName = this->expectedExtensionNames().at(extensionId);

    QSignalSpy spyExtensionInstalled(&model, SIGNAL(extensionInstalled(QString)));
    QSignalSpy spyExtensionEnabledChanged(&model, SIGNAL(extensionEnabledChanged(QString,bool)));

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

  QCOMPARE(model.installedExtensions(), expectedExtensionNames);
  QCOMPARE(spyModelUpdated.count(), 0);
  QCOMPARE(spyExtensionUninstalled.count(), 0);
  QCOMPARE(spySlicerRequirementsChanged.count(), 0);
  QCOMPARE(model.numberOfInstalledExtensions(), 4);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testUninstallExtension()
{
  QVERIFY(this->resetTmp());

  QSettings().setValue("Extensions/ServerUrl", "file://" + this->Tmp.absolutePath());
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  {
    qSlicerExtensionsManagerModel model;
    this->initializeModel(&model);
    QSignalSpy spyModelUpdated(&model, SIGNAL(modelUpdated()));

    for (int extensionId = 0; extensionId < 4; ++extensionId)
      {
      this->installHelper(&model, extensionId, this->Tmp.absolutePath());
      }

    foreach(const QString& extensionName, QStringList()
            << "LoadableExtensionTemplate"
            << "SuperBuildLoadableExtensionTemplate")
      {
      QVERIFY(model.uninstallExtension(extensionName));
      QVERIFY(!model.isExtensionInstalled(extensionName));
      }
  }
  {
    qSlicerExtensionsManagerModel model;
    this->initializeModel(&model);
    model.updateModel();

    foreach(const QString& extensionName, QStringList()
            << "CLIExtensionTemplate"
            << "ScriptedLoadableExtensionTemplate")
      {
      QVERIFY(model.isExtensionInstalled(extensionName));
      }

    foreach(const QString& extensionName, QStringList()
            << "LoadableExtensionTemplate"
            << "SuperBuildLoadableExtensionTemplate")
      {
      QVERIFY(!model.isExtensionInstalled(extensionName));
      }

    foreach(const QString& extensionName, QStringList()
            << "LoadableExtensionTemplate"
            << "SuperBuildLoadableExtensionTemplate")
      {
      this->installHelper(&model, this->expectedExtensionNames().indexOf(extensionName), this->Tmp.absolutePath());
      }

    foreach(const QString& extensionName, this->expectedExtensionNames())
      {
      QVERIFY(model.isExtensionInstalled(extensionName));
      }
  }
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testUpdateModel()
{
  QVERIFY(this->resetTmp());

  QSettings().setValue("Extensions/ServerUrl", "file://" + this->Tmp.absolutePath());
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  {
    qSlicerExtensionsManagerModel model;
    this->initializeModel(&model);
    QSignalSpy spyModelUpdated(&model, SIGNAL(modelUpdated()));
    for (int extensionId = 0; extensionId < 4; ++extensionId)
      {
      this->installHelper(&model, extensionId, this->Tmp.absolutePath());
      }
    QCOMPARE(spyModelUpdated.count(), 0);
  }
  {
    qSlicerExtensionsManagerModel model;
    this->initializeModel(&model);
    QSignalSpy spyModelUpdated(&model, SIGNAL(modelUpdated()));
    QSignalSpy spyExtensionInstalled(&model, SIGNAL(extensionInstalled(QString)));
    QSignalSpy spyExtensionUninstalled(&model, SIGNAL(extensionUninstalled(QString)));
    QSignalSpy spyExtensionEnabledChanged(&model, SIGNAL(extensionEnabledChanged(QString,bool)));
    QSignalSpy spySlicerRequirementsChanged(&model, SIGNAL(slicerRequirementsChanged(QString,QString,QString)));

    model.updateModel();

    QCOMPARE(spyModelUpdated.count(), 1);
    QCOMPARE(spyExtensionInstalled.count(), 0);
    QCOMPARE(spyExtensionEnabledChanged.count(), 0);
    QCOMPARE(spyExtensionUninstalled.count(), 0);
    QCOMPARE(spySlicerRequirementsChanged.count(), 0);

    QCOMPARE(model.installedExtensions(), this->expectedExtensionNames());
    QCOMPARE(model.numberOfInstalledExtensions(), 4);

    model.updateModel();

    QCOMPARE(spyModelUpdated.count(), 2);
    QCOMPARE(spyExtensionInstalled.count(), 0);
    QCOMPARE(spyExtensionEnabledChanged.count(), 0);
    QCOMPARE(spyExtensionUninstalled.count(), 0);
    QCOMPARE(spySlicerRequirementsChanged.count(), 0);

    QCOMPARE(model.installedExtensions(), this->expectedExtensionNames());
    QCOMPARE(model.numberOfInstalledExtensions(), 4);
  }
  {
    qSlicerExtensionsManagerModel model;
    this->initializeModel(&model);
    QSignalSpy spyModelUpdated(&model, SIGNAL(modelUpdated()));
    QSignalSpy spyExtensionInstalled(&model, SIGNAL(extensionInstalled(QString)));
    QSignalSpy spyExtensionUninstalled(&model, SIGNAL(extensionUninstalled(QString)));
    QSignalSpy spyExtensionEnabledChanged(&model, SIGNAL(extensionEnabledChanged(QString,bool)));
    QSignalSpy spySlicerRequirementsChanged(&model, SIGNAL(slicerRequirementsChanged(QString,QString,QString)));

    model.updateModel();

    model.setExtensionEnabled("CLIExtensionTemplate", false);
    QCOMPARE(spyExtensionEnabledChanged.count(), 1);

    QCOMPARE(model.isExtensionEnabled("CLIExtensionTemplate"), false);
    foreach(const QString& extensionName, QStringList()
            << "LoadableExtensionTemplate"
            << "ScriptedLoadableExtensionTemplate"
            << "SuperBuildLoadableExtensionTemplate")
      {
      QCOMPARE(model.isExtensionEnabled(extensionName), true);
      }

    model.updateModel();

    QCOMPARE(model.isExtensionEnabled("CLIExtensionTemplate"), false);
    foreach(const QString& extensionName, QStringList()
            << "LoadableExtensionTemplate"
            << "ScriptedLoadableExtensionTemplate"
            << "SuperBuildLoadableExtensionTemplate")
      {
      QCOMPARE(model.isExtensionEnabled(extensionName), true);
      }

    QCOMPARE(spyModelUpdated.count(), 2);
    QCOMPARE(spyExtensionInstalled.count(), 0);
    QCOMPARE(spyExtensionEnabledChanged.count(), 1);
    QCOMPARE(spyExtensionUninstalled.count(), 0);
    QCOMPARE(spySlicerRequirementsChanged.count(), 0);

    QCOMPARE(model.installedExtensions(), this->expectedExtensionNames());
    QCOMPARE(model.numberOfInstalledExtensions(), 4);
  }
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testIsExtensionInstalled()
{
  QVERIFY(this->resetTmp());

  QSettings().setValue("Extensions/ServerUrl", "file://" + this->Tmp.absolutePath());
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QFETCH(int, extensionIdToInstall);
  QFETCH(bool, isExtensionZeroInstalled);
  QFETCH(bool, isExtensionOneInstalled);
  QFETCH(bool, isExtensionTwoInstalled);
  QFETCH(bool, isExtensionThreeInstalled);

  {
    qSlicerExtensionsManagerModel model;
    this->initializeModel(&model);
    if (extensionIdToInstall != -1)
      {
      this->installHelper(&model, extensionIdToInstall, this->Tmp.absolutePath());
      }
    QCOMPARE(model.isExtensionInstalled(this->expectedExtensionNames().at(0)), isExtensionZeroInstalled);
    QCOMPARE(model.isExtensionInstalled(this->expectedExtensionNames().at(1)), isExtensionOneInstalled);
    QCOMPARE(model.isExtensionInstalled(this->expectedExtensionNames().at(2)), isExtensionTwoInstalled);
    QCOMPARE(model.isExtensionInstalled(this->expectedExtensionNames().at(3)), isExtensionThreeInstalled);
  }

  {
    qSlicerExtensionsManagerModel model;
    this->initializeModel(&model);
    model.updateModel();
    QCOMPARE(model.isExtensionInstalled(this->expectedExtensionNames().at(0)), isExtensionZeroInstalled);
    QCOMPARE(model.isExtensionInstalled(this->expectedExtensionNames().at(1)), isExtensionOneInstalled);
    QCOMPARE(model.isExtensionInstalled(this->expectedExtensionNames().at(2)), isExtensionTwoInstalled);
    QCOMPARE(model.isExtensionInstalled(this->expectedExtensionNames().at(3)), isExtensionThreeInstalled);
  }

}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testIsExtensionInstalled_data()
{
  QTest::addColumn<int>("extensionIdToInstall");
  QTest::addColumn<bool>("isExtensionZeroInstalled");
  QTest::addColumn<bool>("isExtensionOneInstalled");
  QTest::addColumn<bool>("isExtensionTwoInstalled");
  QTest::addColumn<bool>("isExtensionThreeInstalled");

  QTest::newRow("1 installed") << -1 << false << false << false << false;
  QTest::newRow("1 installed") << 0 << true << false << false << false;
  QTest::newRow("2 installed") << 1 << false << true << false << false;
  QTest::newRow("3 installed") << 2 << false << false << true << false;
  QTest::newRow("4 installed") << 3 << false << false << false << true;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testNumberOfInstalledExtensions()
{
  QVERIFY(this->resetTmp());

  QSettings().setValue("Extensions/ServerUrl", "file://" + this->Tmp.absolutePath());
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QFETCH(QList<int>, extensionIdsToInstall);
  QFETCH(int, expectedNumberOfInstalledExtensions);

  {
    qSlicerExtensionsManagerModel model;
    this->initializeModel(&model);
    foreach(int extensionIdToInstall, extensionIdsToInstall)
      {
      this->installHelper(&model, extensionIdToInstall, this->Tmp.absolutePath());
      }
    QCOMPARE(model.numberOfInstalledExtensions(), expectedNumberOfInstalledExtensions);
  }
  {
    qSlicerExtensionsManagerModel model;
    this->initializeModel(&model);
    model.updateModel();
    QCOMPARE(model.numberOfInstalledExtensions(), expectedNumberOfInstalledExtensions);
  }
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testNumberOfInstalledExtensions_data()
{
  QTest::addColumn<QList<int> >("extensionIdsToInstall");
  QTest::addColumn<int>("expectedNumberOfInstalledExtensions");

  QTest::newRow("0 installed") << (QList<int>()) << 0;
  QTest::newRow("1 installed") << (QList<int>() << 0) << 1;
  QTest::newRow("2 installed") << (QList<int>() << 0 << 1) << 2;
  QTest::newRow("3 installed") << (QList<int>() << 0 << 1 << 2) << 3;
  QTest::newRow("4 installed") << (QList<int>() << 0 << 1 << 2 << 3) << 4;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testInstalledExtensions()
{
  QVERIFY(this->resetTmp());

  QSettings().setValue("Extensions/ServerUrl", "file://" + this->Tmp.absolutePath());
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QFETCH(QList<int>, extensionIdsToInstall);
  QFETCH(QStringList, expectedInstalledExtensionNames);

  {
    qSlicerExtensionsManagerModel model;
    this->initializeModel(&model);
    foreach(int extensionIdToInstall, extensionIdsToInstall)
      {
      this->installHelper(&model, extensionIdToInstall, this->Tmp.absolutePath());
      }
    QCOMPARE(model.installedExtensions(), expectedInstalledExtensionNames);
  }
  {
    qSlicerExtensionsManagerModel model;
    this->initializeModel(&model);
    model.updateModel();
    QCOMPARE(model.installedExtensions(), expectedInstalledExtensionNames);
  }
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testInstalledExtensions_data()
{
  QTest::addColumn<QList<int> >("extensionIdsToInstall");
  QTest::addColumn<QStringList>("expectedInstalledExtensionNames");

  QStringList expectedInstalledExtensionNames;
  QTest::newRow("0 installed") << (QList<int>()) << expectedInstalledExtensionNames;

  expectedInstalledExtensionNames = this->expectedExtensionNames();
  QTest::newRow("4 installed") << (QList<int>() << 0 << 1 << 2 << 3) << expectedInstalledExtensionNames;

  expectedInstalledExtensionNames.removeLast();
  QTest::newRow("3 installed") << (QList<int>() << 0 << 1 << 2) << expectedInstalledExtensionNames;

  expectedInstalledExtensionNames.removeLast();
  QTest::newRow("2 installed") << (QList<int>() << 0 << 1) << expectedInstalledExtensionNames;

  expectedInstalledExtensionNames.removeLast();
  QTest::newRow("1 installed") << (QList<int>() << 0) << expectedInstalledExtensionNames;

}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testIsExtensionEnabled()
{
  QVERIFY(this->resetTmp());

  QSettings().setValue("Extensions/ServerUrl", "file://" + this->Tmp.absolutePath());
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QFETCH(QList<int>, extensionIdsToInstall);
  QFETCH(QStringList, expectedEnabledExtensionNames);
  QFETCH(QStringList, extensionNamesToDisable);
  QFETCH(QStringList, expectedEnabledExtensionNamesAfterDisable);

  {
    qSlicerExtensionsManagerModel model;
    this->initializeModel(&model);
    foreach(int extensionIdToInstall, extensionIdsToInstall)
      {
      this->installHelper(&model, extensionIdToInstall, this->Tmp.absolutePath());
      }
    QCOMPARE(model.enabledExtensions(), expectedEnabledExtensionNames);

    foreach(const QString& extensionName, extensionNamesToDisable)
      {
      model.setExtensionEnabled(extensionName, false);
      }
    QCOMPARE(model.enabledExtensions(), expectedEnabledExtensionNamesAfterDisable);
  }
  {
    qSlicerExtensionsManagerModel model;
    this->initializeModel(&model);
    model.updateModel();
    QCOMPARE(model.enabledExtensions(), expectedEnabledExtensionNamesAfterDisable);
  }
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testIsExtensionEnabled_data()
{
  QTest::addColumn<QList<int> >("extensionIdsToInstall");
  QTest::addColumn<QStringList>("expectedEnabledExtensionNames");
  QTest::addColumn<QStringList>("extensionNamesToDisable");
  QTest::addColumn<QStringList>("expectedEnabledExtensionNamesAfterDisable");

  QStringList expectedEnabledExtensionNames;
  QStringList extensionNamesToDisable;
  QStringList expectedEnabledExtensionNamesAfterDisable;
  QTest::newRow("0 installed") << (QList<int>()) << expectedEnabledExtensionNames
                               << extensionNamesToDisable << expectedEnabledExtensionNamesAfterDisable;

  expectedEnabledExtensionNames = this->expectedExtensionNames();
  extensionNamesToDisable = QStringList()
      << this->expectedExtensionNames().at(0)
      << this->expectedExtensionNames().at(2);
  expectedEnabledExtensionNamesAfterDisable = QStringList()
      << this->expectedExtensionNames().at(1)
      << this->expectedExtensionNames().at(3);
  QTest::newRow("4 installed") << (QList<int>() << 0 << 1 << 2 << 3) << expectedEnabledExtensionNames
                                  << extensionNamesToDisable << expectedEnabledExtensionNamesAfterDisable;

  expectedEnabledExtensionNames.removeLast();
  extensionNamesToDisable = QStringList()
      << this->expectedExtensionNames().at(1)
      << this->expectedExtensionNames().at(2);
  expectedEnabledExtensionNamesAfterDisable = QStringList()
      << this->expectedExtensionNames().at(0);
  QTest::newRow("3 installed") << (QList<int>() << 0 << 1 << 2) << expectedEnabledExtensionNames
                               << extensionNamesToDisable << expectedEnabledExtensionNamesAfterDisable;

  expectedEnabledExtensionNames.removeLast();
  extensionNamesToDisable = QStringList()
      << this->expectedExtensionNames().at(0);
  expectedEnabledExtensionNamesAfterDisable = QStringList()
      << this->expectedExtensionNames().at(1);
  QTest::newRow("2 installed") << (QList<int>() << 0 << 1) << expectedEnabledExtensionNames
                               << extensionNamesToDisable << expectedEnabledExtensionNamesAfterDisable;

  expectedEnabledExtensionNames.removeLast();
  extensionNamesToDisable = QStringList() << this->expectedExtensionNames().at(0);
  expectedEnabledExtensionNamesAfterDisable = QStringList();
  QTest::newRow("1 installed") << (QList<int>() << 0) << expectedEnabledExtensionNames
                               << extensionNamesToDisable << expectedEnabledExtensionNamesAfterDisable;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetLauncherSettingsFilePath()
{
  QFETCH(QString, launcherSettingsFilePath);
  QFETCH(QString, expectedLauncherSettingsFilePath);

  qSlicerExtensionsManagerModel model;
  this->initializeModel(&model);
  QCOMPARE(model.launcherSettingsFilePath(), QString());

  QSignalSpy spyModelUpdated(&model, SIGNAL(modelUpdated()));

  model.setLauncherSettingsFilePath(launcherSettingsFilePath);
  QCOMPARE(model.launcherSettingsFilePath(), expectedLauncherSettingsFilePath);

  QCOMPARE(spyModelUpdated.count(), 0);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetLauncherSettingsFilePath_data()
{
  QVERIFY(this->resetTmp());

  QTest::addColumn<QString>("launcherSettingsFilePath");
  QTest::addColumn<QString>("expectedLauncherSettingsFilePath");

  QTest::newRow("existing absolute path - ini extension")
      << this->Tmp.filePath("settings.ini") << this->Tmp.filePath("settings.ini");

  QTest::newRow("existing absolute path - foo extension")
      << this->Tmp.filePath("settings.foo") << this->Tmp.filePath("settings.foo");

  QTest::newRow("existing relative path - ini extension")
      << "settings.ini" << "settings.ini";

  QTest::newRow("nonexistent absolute path - ini extension")
      << "/path/to/nowhere/45659832254686/settings.ini" << "/path/to/nowhere/45659832254686/settings.ini";
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testExtensionAdditionalPathsSettingsUpdated()
{
  QSettings().setValue("Extensions/ServerUrl", "file://" + this->Tmp.absolutePath());
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QFETCH(int, extensionIdToUninstall);
  QFETCH(int, extensionIdToInstall);
  QFETCH(QStringList, modulePaths);

  qSlicerExtensionsManagerModel model;
  this->initializeModel(&model);
  model.updateModel();

  QCOMPARE(model.numberOfInstalledExtensions(), extensionIdToUninstall == -1 ? 0 : 1);
  if (extensionIdToUninstall >= 0)
    {
    model.uninstallExtension(this->expectedExtensionNames().at(extensionIdToUninstall));
    }
  QCOMPARE(model.numberOfInstalledExtensions(), 0);

  if (extensionIdToInstall >= 0)
    {
    this->installHelper(&model, extensionIdToInstall, this->Tmp.absolutePath());
    }

  QStringList currentAdditionalPaths = QSettings().value("Modules/AdditionalPaths").toStringList();
  QCOMPARE(currentAdditionalPaths, modulePaths);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testExtensionAdditionalPathsSettingsUpdated_data()
{
  QVERIFY(this->resetTmp());

  QTest::addColumn<int>("extensionIdToUninstall");
  QTest::addColumn<int>("extensionIdToInstall");
  QTest::addColumn<QStringList>("modulePaths");

  QTest::newRow("0-CLIExtensionTemplate")
      << -1
      << 0
      << (QStringList() << this->Tmp.filePath("CLIExtensionTemplate/" + this->CLIMODULES_LIB_DIR));

  QTest::newRow("1-LoadableExtensionTemplate")
      << 0
      << 1
      << (QStringList() << this->Tmp.filePath("LoadableExtensionTemplate/" + this->QTLOADABLEMODULES_LIB_DIR));
#ifdef Slicer_USE_PYTHONQT
  QTest::newRow("2-ScriptedLoadableExtensionTemplate")
      << 1
      << 2
      << (QStringList() << this->Tmp.filePath("ScriptedLoadableExtensionTemplate/" + this->QTSCRIPTEDMODULES_LIB_DIR));
#endif
  QTest::newRow("3-SuperBuildLoadableExtensionTemplate")
      << 2
      << 3
      << (QStringList() << this->Tmp.filePath("SuperBuildLoadableExtensionTemplate/" + this->QTLOADABLEMODULES_LIB_DIR));

  QTest::newRow("Cleanup")
      << 3
      << -1
      << QStringList();
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testExtensionLauncherSettingsUpdated()
{
  QSettings().setValue("Extensions/ServerUrl", "file://" + this->Tmp.absolutePath());
  QSettings().setValue("Extensions/InstallPath", this->Tmp.absolutePath());

  QFETCH(int, extensionIdToUninstall);
  QFETCH(int, extensionIdToInstall);
  QFETCH(QStringList, libraryPaths);
  QFETCH(QStringList, paths);
  QFETCH(QString, pythonPath);

  qSlicerExtensionsManagerModel model;
  this->initializeModel(&model);
  model.updateModel();
  model.setLauncherSettingsFilePath(this->Tmp.filePath("launcherSettings.ini"));

  QCOMPARE(model.numberOfInstalledExtensions(), extensionIdToUninstall == -1 ? 0 : 1);
  if (extensionIdToUninstall >= 0)
    {
    model.uninstallExtension(this->expectedExtensionNames().at(extensionIdToUninstall));
    }
  QCOMPARE(model.numberOfInstalledExtensions(), 0);

  if (extensionIdToInstall >= 0)
    {
    this->installHelper(&model, extensionIdToInstall, this->Tmp.absolutePath());
    }

  QSettings launcherSettings(model.launcherSettingsFilePath(), QSettings::IniFormat);

  QStringList currentLibraryPath =
      qSlicerExtensionsManagerModel::readArrayValues(launcherSettings, "LibraryPaths", "path");
  QCOMPARE(currentLibraryPath, libraryPaths);

  QStringList currentPaths =
      qSlicerExtensionsManagerModel::readArrayValues(launcherSettings, "Paths", "path");
  QCOMPARE(currentPaths, paths);

  QString currentPythonPath = launcherSettings.value("EnvironmentVariables/PYTHONPATH").toString();
  QCOMPARE(currentPythonPath, pythonPath);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testExtensionLauncherSettingsUpdated_data()
{
  QVERIFY(this->resetTmp());

  QTest::addColumn<int>("extensionIdToUninstall");
  QTest::addColumn<int>("extensionIdToInstall");
  QTest::addColumn<QStringList>("libraryPaths");
  QTest::addColumn<QStringList>("paths");
  QTest::addColumn<QString>("pythonPath");

  QTest::newRow("0-CLIExtensionTemplate")
      << -1
      << 0
      << (QStringList() << this->Tmp.filePath("CLIExtensionTemplate/" + this->CLIMODULES_LIB_DIR))
      << (QStringList() << this->Tmp.filePath("CLIExtensionTemplate/" + this->CLIMODULES_LIB_DIR))
      << QString();

#ifdef Slicer_USE_PYTHONQT
  QTest::newRow("1-LoadableExtensionTemplate")
      << 0
      << 1
      << (QStringList() << this->Tmp.filePath("LoadableExtensionTemplate/" + this->QTLOADABLEMODULES_LIB_DIR))
      << QStringList()
      << QString("<PATHSEP>" + this->Tmp.filePath("LoadableExtensionTemplate/" + this->QTLOADABLEMODULES_PYTHON_LIB_DIR));
#endif

#ifdef Slicer_USE_PYTHONQT
  QTest::newRow("2-ScriptedLoadableExtensionTemplate")
      << 1
      << 2
      << QStringList()
      << QStringList()
      << QString("<PATHSEP>" + this->Tmp.filePath("ScriptedLoadableExtensionTemplate/" + this->QTSCRIPTEDMODULES_LIB_DIR));
#endif
#ifdef Slicer_USE_PYTHONQT
  QTest::newRow("3-SuperBuildLoadableExtensionTemplate")
      << 2
      << 3
      << (QStringList() << this->Tmp.filePath("SuperBuildLoadableExtensionTemplate/" + this->QTLOADABLEMODULES_LIB_DIR))
      << QStringList()
      << QString("<PATHSEP>" + this->Tmp.filePath("SuperBuildLoadableExtensionTemplate/" + this->QTLOADABLEMODULES_PYTHON_LIB_DIR));
#endif
  QTest::newRow("Cleanup")
      << 3
      << -1
      << QStringList()
      << QStringList()
      << QString();
}

// ----------------------------------------------------------------------------
namespace
{
typedef void (qSlicerExtensionsManagerModel::* QStringSetter)(const QString&);
typedef QString (qSlicerExtensionsManagerModel::* QStringGetter)()const;

// ----------------------------------------------------------------------------
void testRequirementsHelper(qSlicerExtensionsManagerModel * model,
                            QStringSetter qStringSetterFuncPtr, QStringGetter qStringGetterFuncPtr)
{
  QFETCH(QStringList, valuesToSet);
  QFETCH(QString, expectedFinalValue);
  QFETCH(int, expectedSlicerRequirementsChangedCount);

  QSignalSpy spySlicerRequirementsChanged(model, SIGNAL(slicerRequirementsChanged(QString,QString,QString)));
  foreach(const QString& valuetoSet, valuesToSet)
    {
    (model->*qStringSetterFuncPtr)(valuetoSet);
    }
  QCOMPARE((model->*qStringGetterFuncPtr)(), expectedFinalValue);
  QCOMPARE(spySlicerRequirementsChanged.count(), expectedSlicerRequirementsChangedCount);
}

} // end of anonymous namespace

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerRevision()
{
  qSlicerExtensionsManagerModel model;
  this->initializeModel(&model);
  testRequirementsHelper(&model,
                         &qSlicerExtensionsManagerModel::setSlicerRevision,
                         &qSlicerExtensionsManagerModel::slicerRevision);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerRevision_data()
{
  QTest::addColumn<QStringList>("valuesToSet");
  QTest::addColumn<QString>("expectedFinalValue");
  QTest::addColumn<int>("expectedSlicerRequirementsChangedCount");

  QTest::newRow("0") << (QStringList() << "" << "1" << "" << "1") << "1" << 3;
  QTest::newRow("1") << (QStringList() << "" << "1" << "1") << "1" << 1;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerOs()
{
  qSlicerExtensionsManagerModel model;
  this->initializeModel(&model);
  testRequirementsHelper(&model,
                         &qSlicerExtensionsManagerModel::setSlicerOs,
                         &qSlicerExtensionsManagerModel::slicerOs);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerOs_data()
{
  QTest::addColumn<QStringList>("valuesToSet");
  QTest::addColumn<QString>("expectedFinalValue");
  QTest::addColumn<int>("expectedSlicerRequirementsChangedCount");

  QTest::newRow("0") << (QStringList() << "" << "linux" << "" << "linux") << "linux" << 3;
  QTest::newRow("1") << (QStringList() << "" << "linux" << "linux") << "linux" << 1;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerArch()
{
  qSlicerExtensionsManagerModel model;
  this->initializeModel(&model);
  testRequirementsHelper(&model,
                         &qSlicerExtensionsManagerModel::setSlicerArch,
                         &qSlicerExtensionsManagerModel::slicerArch);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerArch_data()
{
  QTest::addColumn<QStringList>("valuesToSet");
  QTest::addColumn<QString>("expectedFinalValue");
  QTest::addColumn<int>("expectedSlicerRequirementsChangedCount");

  QTest::newRow("0") << (QStringList() << "" << "amd64" << "" << "amd64") << "amd64" << 3;
  QTest::newRow("1") << (QStringList() << "" << "amd64" << "amd64") << "amd64" << 1;
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerRequirements()
{
  qSlicerExtensionsManagerModel model;
  this->initializeModel(&model);
  QSignalSpy spySlicerRequirementsChanged(&model, SIGNAL(slicerRequirementsChanged(QString,QString,QString)));

  QCOMPARE(model.slicerRevision(), QString(""));
  QCOMPARE(model.slicerOs(), QString(""));
  QCOMPARE(model.slicerArch(), QString(""));

  QFETCH(QList<QStringList>, requirementsToSetList);
  QFETCH(QList<int>, expectedSlicerRequirementsChangedCounts);
  QFETCH(QStringList, finalExpectedSlicerRequirement);

  QCOMPARE(requirementsToSetList.count(), expectedSlicerRequirementsChangedCounts.count());

  for(int setIdx = 0; setIdx < requirementsToSetList.count(); ++setIdx)
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
  QTest::addColumn<QList<QStringList> >("requirementsToSetList");
  QTest::addColumn<QList<int> >("expectedSlicerRequirementsChangedCounts");
  QTest::addColumn<QStringList>("finalExpectedSlicerRequirement");

  QList<QStringList> requirementsToSetList = QList<QStringList>()
      << (QStringList() << "" << "" << "")
      << (QStringList() << "5" << "" << "")
      << (QStringList() << "5" << "linux" << "")
      << (QStringList() << "5" << "linux" << "")
      << (QStringList() << "5" << "linux" << "amd64")
      << (QStringList() << "5" << "linux" << "amd64")
      << (QStringList() << "74" << "linux" << "amd64");
  QTest::newRow("0") << requirementsToSetList
                     << (QList<int>() << 0 << 1 << 2 << 2 << 3 << 3 << 4)
                     << (QStringList() << "74" << "linux" << "amd64");
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerVersion()
{
  qSlicerExtensionsManagerModel model;
  this->initializeModel(&model);

  QFETCH(QString, valueToSet);
  QFETCH(QString, expectedFinalValue);

  QSignalSpy spySlicerRequirementsChanged(&model, SIGNAL(slicerRequirementsChanged(QString,QString,QString)));
  model.setSlicerVersion(valueToSet);
  QCOMPARE(model.slicerVersion(), expectedFinalValue);
  QCOMPARE(spySlicerRequirementsChanged.count(), 0);
}

// ----------------------------------------------------------------------------
void qSlicerExtensionsManagerModelTester::testSetSlicerVersion_data()
{
  QTest::addColumn<QString>("valueToSet");
  QTest::addColumn<QString>("expectedFinalValue");

  QTest::newRow("0") << "3.9" << "3.9";
  QTest::newRow("1") << "4.0" << "4.0";
  QTest::newRow("2") << "4.1" << "4.1";
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qSlicerExtensionsManagerModelTest)
#include "moc_qSlicerExtensionsManagerModelTest.cxx"

