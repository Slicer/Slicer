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
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QScopedPointer>
#include <QSettings>
#include <QStandardItemModel>
#include <QTemporaryFile>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

// CTK includes
#include <ctkScopedCurrentDir.h>
#include <ctkPimpl.h>
#include <ctkUtils.h>

// qRestAPI includes
#include <qGirderAPI.h>
#include <qRestAPI.h>
#include <qRestResult.h>

// QtCore includes
#include "qSlicerCoreApplication.h"
#include "qSlicerExtensionsManagerModel.h"
#include "vtkSlicerConfigure.h"
#include "vtkSlicerVersionConfigureMinimal.h"

// MRML includes
#include "vtkArchive.h"

// --------------------------------------------------------------------------
namespace
{

// --------------------------------------------------------------------------
struct UpdateDownloadInformation
{
  UpdateDownloadInformation(const QString& extensionId = QString())
    : ExtensionId(extensionId)
  {
  }

  QString ExtensionId;
  QString ArchiveName;
  qint64 DownloadSize{ 0 };
  qint64 DownloadProgress{ 0 };
};

// --------------------------------------------------------------------------
class QStandardItemModelWithRole : public QStandardItemModel
{
public:
  QHash<int, QByteArray> roleNames() const override { return this->CustomRoleNames; }
  QHash<int, QByteArray> CustomRoleNames;
};

} // end of anonymous namespace

// --------------------------------------------------------------------------
// qSlicerExtensionsManagerModelPrivate methods

//-----------------------------------------------------------------------------
class qSlicerExtensionsManagerModelPrivate
{
  Q_DECLARE_PUBLIC(qSlicerExtensionsManagerModel);

protected:
  qSlicerExtensionsManagerModel* const q_ptr;

public:
  enum ColumnsIds
  {
    IdColumn = 0,
    NameColumn,
    ScmColumn,
    ScmUrlColumn,
    DependsColumn,
    SlicerRevisionColumn,
    RevisionColumn,
    ReleaseColumn,
    ArchColumn,
    OsColumn,
    IconUrlColumn,
    HomepageColumn,
    CategoryColumn,
    StatusColumn,
    ContributorsColumn,
    DescriptionColumn,
    ScreenshotsColumn,
    EnabledColumn,
    ArchiveNameColumn,
    MD5Column,
    UpdatedColumn,    // date and time when the extension on the server was updated
    InstalledColumn,  // extension files are in place
    LoadedColumn,     // extension actually loaded into the application
    BookmarkedColumn, // extension is bookmarked
  };

  enum ItemDataRole
  {
    IdRole = Qt::UserRole + 1,
    NameRole,
    ScmRole,
    ScmUrlRole,
    DependsRole,
    SlicerRevisionRole,
    RevisionRole,
    ReleaseRole,
    ArchRole,
    OsRole,
    IconUrlRole,
    HomepageRole,
    CategoryRole,
    StatusRole,
    ContributorsRole,
    DescriptionRole,
    ScreenshotsRole,
    EnabledRole,
    ArchiveRole,
    MD5Role,
    UpdatedRole,
    InstalledRole,
    LoadedRole,
    BookmarkedRole,
  };

  typedef qSlicerExtensionsManagerModelPrivate Self;
  typedef qSlicerExtensionsManagerModel::ExtensionMetadataType ExtensionMetadataType;
  qSlicerExtensionsManagerModelPrivate(qSlicerExtensionsManagerModel& object);
  void init();

  void debug(const QString& text) const;
  void info(const QString& text) const;
  void warning(const QString& text) const;
  void critical(const QString& text) const;
  void log(const QString& text, ctkErrorLogLevel::LogLevels level) const;

  int role(const QByteArray& roleName);

  /// Save/load extensions metadata that was retrieved from the server to a local cache
  /// (in application settings) to avoid too frequent polling of the server.
  void saveExtensionsMetadataFromServerToCache();
  void loadCachedExtensionsMetadataFromServer();

  QFileInfoList extensionDescriptionFileInfos(const QString& extensionDescriptionPath) const;

  void addExtensionModelRow(const ExtensionMetadataType& metadata);
  QStandardItem* extensionItem(const QString& extensionName, int column = Self::NameColumn) const;

  QStringList columnNames() const;

  void addExtensionPathToApplicationSettings(const QString& extensionName);
  void removeExtensionPathFromApplicationSettings(const QString& extensionName);

  void addExtensionPathToLauncherSettings(const QString& extensionName);
  void removeExtensionPathFromLauncherSettings(const QString& extensionName);

  bool checkExtensionsInstallDestinationPath(const QString& destinationPath, QString& error) const;

  bool checkExtensionSettingsPermissions(QString& error) const;
  void addExtensionSettings(const QString& extensionName);
  void removeExtensionSettings(const QString& extensionName);

  void removeExtensionFromScheduledForUpdateList(const QString& extensionName);
  void removeExtensionFromScheduledForUninstallList(const QString& extensionName);

  QString extractArchive(const QDir& extensionsDir, const QString& archiveFile);

  qSlicerExtensionDownloadTask* downloadExtensionByName(const QString& extensionName);

  QStringList dependenciesToInstall(const QStringList& directDependencies, QStringList& unresolvedDependencies);

  /// Update (reinstall) specified extension.
  ///
  /// This updates the specified extension
  ///
  /// \param extensionName Name of the extension.
  /// \param
  /// \sa downloadExtensionByName, installExtension
  bool updateExtension(const QString& extensionName, const QString& archiveFile);

  /// Returns true it is due to check if there are any updates for managed extensions.
  bool isExtensionsMetadataUpdateDue() const;

  /// Removes s4ext file and icon file from the extension install folder.
  /// The s4ext file is kept until the extension is managed (installed or bookmarked)
  /// for metadata access quickly and even without network connection.
  bool removeExtensionDescriptionFile(const QString& extensionName);

  QStringList extensionLibraryPaths(const QString& extensionName) const;
  QStringList extensionQtPluginPaths(const QString& extensionName) const;
  QStringList extensionPaths(const QString& extensionName) const;

#ifdef Slicer_USE_PYTHONQT
  QStringList extensionPythonPaths(const QString& extensionName) const;
#endif
  static bool validateExtensionMetadata(const ExtensionMetadataType& extensionMetadata, int serverAPI);

  static QStringList isExtensionCompatible(const ExtensionMetadataType& metadata,
                                           const QString& slicerRevision,
                                           const QString& slicerOs,
                                           const QString& slicerArch);

  void initializeColumnIdToNameMap(int columnIdx, const char* columnName);
  QHash<int, QString> ColumnIdToName;
  QStringList ColumnNames;

  bool NewExtensionEnabledByDefault{ true };
  bool Interactive{ true };
  // By default enable checking of the extension server for updates.
  // This is according to common practices in all operating systems
  // most software (including venerable open source apps like vim).
  // Anyone who is particularly concerned is expected to look
  // for ways to disable automatic updates.
  bool AutoUpdateCheck{ true };
  bool AutoUpdateInstall{ false };
  bool AutoInstallDependencies{ true };

  QNetworkAccessManager NetworkManager;

  qRestAPI ExtensionsMetadataFromServerAPI;
  QMap<QString, QVariantMap> ExtensionsMetadataFromServer;
  QUuid ExtensionsMetadataFromServerQueryUID; // if not null then it means that query is in progress

  QHash<QString, UpdateDownloadInformation> AvailableUpdates;

  QString ExtensionsSettingsFilePath;

  QString SlicerRevision;
  QString SlicerOs;
  QString SlicerArch;

  QString SlicerVersion;

  QStringList LoadedExtensions;

  QStandardItemModelWithRole Model;

  // Restore previous extension tab may want to run lots of queries.
  // Results are cached in this variable to improve performance.
  QMap<QString, ExtensionMetadataType> ServerResponseCache;

  QMap<qSlicerExtensionDownloadTask*, QString> ActiveTasks;
};

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModelPrivate::qSlicerExtensionsManagerModelPrivate(qSlicerExtensionsManagerModel& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::init()
{
  Q_Q(qSlicerExtensionsManagerModel);

  this->SlicerVersion = Slicer_VERSION;

  qRegisterMetaType<ExtensionMetadataType>("ExtensionMetadataType");

  this->initializeColumnIdToNameMap(Self::IdColumn, "extension_id");
  this->initializeColumnIdToNameMap(Self::NameColumn, "extensionname");
  this->initializeColumnIdToNameMap(Self::ScmColumn, "scm");
  this->initializeColumnIdToNameMap(Self::ScmUrlColumn, "scmurl");
  this->initializeColumnIdToNameMap(Self::SlicerRevisionColumn, "slicer_revision");
  this->initializeColumnIdToNameMap(Self::RevisionColumn, "revision");
  this->initializeColumnIdToNameMap(Self::ReleaseColumn, "release");
  this->initializeColumnIdToNameMap(Self::ArchColumn, "arch");
  this->initializeColumnIdToNameMap(Self::OsColumn, "os");
  this->initializeColumnIdToNameMap(Self::DependsColumn, "depends");
  this->initializeColumnIdToNameMap(Self::HomepageColumn, "homepage");
  this->initializeColumnIdToNameMap(Self::IconUrlColumn, "iconurl");
  this->initializeColumnIdToNameMap(Self::CategoryColumn, "category");
  this->initializeColumnIdToNameMap(Self::StatusColumn, "status");
  this->initializeColumnIdToNameMap(Self::ContributorsColumn, "contributors");
  this->initializeColumnIdToNameMap(Self::DescriptionColumn, "description");
  this->initializeColumnIdToNameMap(Self::ScreenshotsColumn, "screenshots");
  this->initializeColumnIdToNameMap(Self::EnabledColumn, "enabled");
  this->initializeColumnIdToNameMap(Self::ArchiveNameColumn, "archivename");
  this->initializeColumnIdToNameMap(Self::MD5Column, "md5");
  this->initializeColumnIdToNameMap(Self::UpdatedColumn, "updated");
  this->initializeColumnIdToNameMap(Self::InstalledColumn, "installed");
  this->initializeColumnIdToNameMap(Self::LoadedColumn, "loaded");
  this->initializeColumnIdToNameMap(Self::BookmarkedColumn, "bookmarked");

  // See https://www.developer.nokia.com/Community/Wiki/Using_QStandardItemModel_in_QML
  QHash<int, QByteArray> roleNames;
  int columnIdx = 0;
  foreach (const QString& columnName, this->columnNames())
  {
    roleNames[Qt::UserRole + 1 + columnIdx] = columnName.toUtf8();
    ++columnIdx;
  }

  this->Model.CustomRoleNames = roleNames;

  QSettings settings;
  this->AutoUpdateCheck = settings.value("Extensions/AutoUpdateCheck", this->AutoUpdateCheck).toBool();
  this->AutoUpdateInstall = settings.value("Extensions/AutoUpdateInstall", this->AutoUpdateInstall).toBool();
  this->AutoInstallDependencies =
    settings.value("Extensions/AutoInstallDependencies", this->AutoInstallDependencies).toBool();

  QObject::connect(
    q, SIGNAL(slicerRequirementsChanged(QString, QString, QString)), q, SLOT(identifyIncompatibleExtensions()));

  QObject::connect(q, SIGNAL(modelUpdated()), q, SLOT(identifyIncompatibleExtensions()));

  QObject::connect(&this->ExtensionsMetadataFromServerAPI,
                   SIGNAL(finished(QUuid)),
                   q,
                   SLOT(onExtensionsMetadataFromServerQueryFinished(QUuid)));
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::debug(const QString& text) const
{
  this->log(text, ctkErrorLogLevel::Debug);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::info(const QString& text) const
{
  this->log(text, ctkErrorLogLevel::Info);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::warning(const QString& text) const
{
  this->log(text, ctkErrorLogLevel::Warning);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::critical(const QString& text) const
{
  this->log(text, ctkErrorLogLevel::Critical);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::log(const QString& text, ctkErrorLogLevel::LogLevels level) const
{
  Q_Q(const qSlicerExtensionsManagerModel);
  if (level == ctkErrorLogLevel::Fatal)
  {
    qFatal("%s", qPrintable(text));
  }
  else if (level == ctkErrorLogLevel::Critical)
  {
    qCritical().noquote() << text;
  }
  else if (level == ctkErrorLogLevel::Warning)
  {
    qWarning().noquote() << text;
  }
  else
  {
    qDebug().noquote() << text;
  }
  emit q->messageLogged(text, level);
}

// --------------------------------------------------------------------------
int qSlicerExtensionsManagerModelPrivate::role(const QByteArray& roleName)
{
  QHashIterator<int, QByteArray> roleIterator(this->Model.roleNames());
  while (roleIterator.hasNext())
  {
    roleIterator.next();
    if (roleIterator.value() == roleName)
    {
      return roleIterator.key();
    }
  }
  return -1;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::saveExtensionsMetadataFromServerToCache()
{
  Q_Q(const qSlicerExtensionsManagerModel);

  // stores time of last update check for this Slicer installation
  QSettings extensionSettings(q->extensionsSettingsFilePath(), QSettings::IniFormat);
  QDateTime currentTime = QDateTime::currentDateTimeUtc();
  extensionSettings.setValue("Extensions/MetadataFromServerUpdateTime", currentTime.toString(Qt::ISODate));
  extensionSettings.setValue("Extensions/MetadataFromServerUrl", q->serverUrl().toString());

  // Convert to json to allow writing to settings TODO: use filename instead?
  QJsonObject jsonExtensionsMetadata;
  foreach (const QString& extensionName, this->ExtensionsMetadataFromServer.keys())
  {
    QJsonObject metadata = QJsonObject::fromVariantMap(this->ExtensionsMetadataFromServer[extensionName]);
    jsonExtensionsMetadata.insert(extensionName, metadata);
  }
  // Write to json file
  QByteArray renderedJson = QJsonDocument(jsonExtensionsMetadata).toJson();
  QString metadataFilePath = q->extensionInstallPath("ExtensionsMetadataFromServer.json");
  QFile jsonFile(metadataFilePath);
  jsonFile.open(QIODevice::WriteOnly);
  jsonFile.write(renderedJson);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::loadCachedExtensionsMetadataFromServer()
{
  Q_Q(const qSlicerExtensionsManagerModel);

  this->ExtensionsMetadataFromServer.clear();

  QSettings extensionSettings(q->extensionsSettingsFilePath(), QSettings::IniFormat);
  QString cachedServerUrl = extensionSettings.value("Extensions/MetadataFromServerUrl").toString();
  if (cachedServerUrl.isEmpty() || cachedServerUrl != q->serverUrl().toString())
  {
    // no metadata cached for this server URL
    return;
  }

  // Read last server response from json file
  QString metadataFilePath = q->extensionInstallPath("ExtensionsMetadataFromServer.json");
  QFile jsonFile(metadataFilePath);
  if (!jsonFile.exists() || !jsonFile.open(QIODevice::ReadOnly))
  {
    // no metadata has been cached yet
    return;
  }
  QByteArray renderedJson = jsonFile.readAll();
  jsonFile.close();
  // Convert to json
  QJsonParseError parseError;
  QJsonDocument jsonDoc = QJsonDocument::fromJson(renderedJson, &parseError);
  if (parseError.error != QJsonParseError::NoError)
  {
    this->warning(qSlicerExtensionsManagerModel::tr("Failed to parse %1: error at %2: %3")
                    .arg(metadataFilePath)
                    .arg(parseError.offset)
                    .arg(parseError.errorString()));
    return;
  }
  QJsonObject jsonExtensionsMetadata = jsonDoc.object();

  // Convert from json to string->QVariantMap map
  foreach (const QString& extensionName, jsonExtensionsMetadata.keys())
  {
    QJsonObject jsonMetadata = jsonExtensionsMetadata[extensionName].toObject();
    QVariantMap metadata = jsonMetadata.toVariantMap();
    this->ExtensionsMetadataFromServer[extensionName] = metadata;
  }
}

// --------------------------------------------------------------------------
QFileInfoList qSlicerExtensionsManagerModelPrivate::extensionDescriptionFileInfos(
  const QString& extensionDescriptionPath) const
{
  QDir extensionDescriptionDir(extensionDescriptionPath);
  QFileInfoList fileInfos = extensionDescriptionDir.entryInfoList(
    QStringList() << "*.s4ext", QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase);
  return fileInfos;
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModelPrivate::dependenciesToInstall(const QStringList& directDependencies,
                                                                        QStringList& unresolvedDependencies)
{
  Q_Q(qSlicerExtensionsManagerModel);

  // Gather information on dependency extensions
  QStringList dependencies = directDependencies;
  QStringList toInstall; // extension names that needs to be installed (directDependencies and their dependencies)

  while (!dependencies.isEmpty())
  {
    QString dependencyName = dependencies.takeFirst();
    if (dependencyName.isEmpty() || dependencyName == "NA" // "NA" is used for indicating no dependency
        || unresolvedDependencies.contains(dependencyName) || toInstall.contains(dependencyName))
    {
      continue;
    }

    if (!q->isExtensionInstalled(dependencyName))
    {
      // Dependency not yet installed
      const qSlicerExtensionsManagerModel::ExtensionMetadataType& dependencyMetadataOnServer =
        this->ExtensionsMetadataFromServer.value(dependencyName);
      if (dependencyMetadataOnServer.contains("extension_id"))
      {
        // found on server, add to the list of dependencies to install
        toInstall << dependencyName;
      }
      else
      {
        // not found on server, this is an unresolved issue
        unresolvedDependencies.append(dependencyName);
      }
    }

    // Add dependencies of dependencies.
    // We do this even if the extension was installed because it is possible that some dependencies did not get
    // installed.
    qSlicerExtensionsManagerModel::ExtensionMetadataType dependencyMetadata = q->extensionMetadata(dependencyName);
    dependencies << dependencyMetadata.value("depends").toString().split(" ");
    dependencies.removeDuplicates();
  }
  return toInstall;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::addExtensionModelRow(const ExtensionMetadataType& metadata)
{
  QString extensionName = metadata["extensionname"].toString();
  QList<QStandardItem*> foundItems = this->Model.findItems(extensionName, Qt::MatchExactly, Self::NameColumn);
  if (foundItems.count() == 1)
  {
    // Extension is in the model already, update items
    int row = foundItems.at(0)->row();
    int column = 0;
    foreach (const QString& key, this->columnNames())
    {
      QString value = metadata.value(key).toString();
      QStandardItem* item = this->Model.item(row, column);
      item->setData(value, this->role(key.toUtf8()));
      item->setData(value, Qt::DisplayRole);
      column++;
    }
  }
  else
  {
    // Extension is not in the model yet, create new row
    QList<QStandardItem*> itemList;
    foreach (const QString& key, this->columnNames())
    {
      QString value = metadata.value(key).toString();
      QStandardItem* item = new QStandardItem(value);
      item->setEditable(false);
      item->setData(value, this->role(key.toUtf8()));
      item->setData(value, Qt::DisplayRole);
      itemList << item;
    }
    this->Model.invisibleRootItem()->appendRow(itemList);
  }
}

// --------------------------------------------------------------------------
QStandardItem* qSlicerExtensionsManagerModelPrivate::extensionItem(const QString& extensionName, int column) const
{
  QList<QStandardItem*> foundItems = this->Model.findItems(extensionName, Qt::MatchExactly, Self::NameColumn);
  if (foundItems.count() != 1)
  {
    return nullptr;
  }
  return this->Model.item(foundItems.at(0)->row(), column);
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModelPrivate::columnNames() const
{
  return this->ColumnNames;
}

// --------------------------------------------------------------------------
namespace
{

// --------------------------------------------------------------------------
bool hasPath(const QStringList& paths, const QString& pathToCheck)
{
  QString dirToCheck = QDir::cleanPath(pathToCheck);
  foreach (const QString& path, paths)
  {
    if (dirToCheck == QDir::cleanPath(path))
    {
      return true;
    }
  }
  return false;
}

// --------------------------------------------------------------------------
QStringList appendToPathList(const QStringList& paths, const QStringList& pathsToAppend, bool shouldExist = true)
{
  QStringList updatedPaths(paths);
  foreach (const QString& pathToAppend, pathsToAppend)
  {
    if (hasPath(updatedPaths, pathToAppend))
    {
      // already inserted, skip it
      continue;
    }
    if (shouldExist && !QDir(pathToAppend).exists())
    {
      // only existing paths are asked to be added and this one does not exist, skip it
      continue;
    }
    updatedPaths << pathToAppend;
  }
  return updatedPaths;
}

// --------------------------------------------------------------------------
QStringList removeFromPathList(QStringList& paths, const QString& pathToRemove)
{
  QDir extensionDir(pathToRemove);
  foreach (const QString& path, paths)
  {
    if (extensionDir == QDir(path))
    {
      paths.removeOne(path);
      break;
    }
  }
  return paths;
}

// --------------------------------------------------------------------------
QStringList removeFromPathList(const QStringList& paths, const QStringList& pathsToRemove)
{
  QStringList updatedPaths(paths);
  foreach (const QString& pathToRemove, pathsToRemove)
  {
    removeFromPathList(updatedPaths, pathToRemove);
  }
  return updatedPaths;
}

} // end of anonymous namespace

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModelPrivate::isExtensionsMetadataUpdateDue() const
{
  Q_Q(const qSlicerExtensionsManagerModel);

  // stores common settings
  if (!this->AutoUpdateCheck)
  {
    return false;
  }

  // stores time of last update check for this Slicer installation
  QSettings extensionSettings(q->extensionsSettingsFilePath(), QSettings::IniFormat);
  if (!extensionSettings.contains("Extensions/MetadataFromServerUpdateTime"))
  {
    // there has never been an update check
    return true;
  }

  QString lastUpdateTimeStr = extensionSettings.value("Extensions/MetadataFromServerUpdateTime").toString();
  QDateTime lastUpdateTime = QDateTime::fromString(lastUpdateTimeStr, Qt::ISODate);
  QDateTime currentTime = QDateTime::currentDateTimeUtc();
  // By default, we check for updates once a day
  int updateFrequencyMinutes = QSettings().value("Extensions/AutoUpdateFrequencyMinutes", 24 * 60).toInt();
  qint64 updateFrequencyMsec = qint64(updateFrequencyMinutes) * qint64(60000);
  if (lastUpdateTime.msecsTo(currentTime) < updateFrequencyMsec)
  {
    // not enough time has passed since the last update check
    return false;
  }

  return true;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::addExtensionPathToApplicationSettings(const QString& extensionName)
{
  Q_Q(qSlicerExtensionsManagerModel);
  QSettings settings(q->extensionsSettingsFilePath(), QSettings::IniFormat);
  QStringList additionalPaths = qSlicerCoreApplication::application()->toSlicerHomeAbsolutePaths(
    settings.value("Modules/AdditionalPaths").toStringList());
  QStringList newModulePaths = q->extensionModulePaths(extensionName);
  settings.setValue("Modules/AdditionalPaths",
                    qSlicerCoreApplication::application()->toSlicerHomeRelativePaths(
                      appendToPathList(additionalPaths, newModulePaths)));
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::removeExtensionPathFromApplicationSettings(const QString& extensionName)
{
  Q_Q(qSlicerExtensionsManagerModel);
  QSettings settings(q->extensionsSettingsFilePath(), QSettings::IniFormat);
  QStringList additionalPaths = qSlicerCoreApplication::application()->toSlicerHomeAbsolutePaths(
    settings.value("Modules/AdditionalPaths").toStringList());
  settings.setValue("Modules/AdditionalPaths",
                    qSlicerCoreApplication::application()->toSlicerHomeRelativePaths(
                      removeFromPathList(additionalPaths, q->extensionModulePaths(extensionName))));
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::addExtensionPathToLauncherSettings(const QString& extensionName)
{
  if (this->ExtensionsSettingsFilePath.isEmpty())
  {
    return;
  }
  QSettings settings(this->ExtensionsSettingsFilePath, QSettings::IniFormat);
  if (settings.status() != QSettings::NoError)
  {
    this->warning(qSlicerExtensionsManagerModel::tr("Failed to open extensions settings file %1")
                    .arg(this->ExtensionsSettingsFilePath));
    return;
  }
  qSlicerCoreApplication* app = qSlicerCoreApplication::application();

  QStringList libraryPath =
    app->toSlicerHomeAbsolutePaths(qSlicerExtensionsManagerModel::readArrayValues(settings, "LibraryPaths", "path"));
  qSlicerExtensionsManagerModel::writeArrayValues(
    settings,
    app->toSlicerHomeRelativePaths(appendToPathList(libraryPath, this->extensionLibraryPaths(extensionName))),
    "LibraryPaths",
    "path");

  QStringList paths =
    app->toSlicerHomeAbsolutePaths(qSlicerExtensionsManagerModel::readArrayValues(settings, "Paths", "path"));
  qSlicerExtensionsManagerModel::writeArrayValues(
    settings,
    app->toSlicerHomeRelativePaths(appendToPathList(paths, this->extensionPaths(extensionName))),
    "Paths",
    "path");

#ifdef Slicer_USE_PYTHONQT
  QStringList pythonPaths =
    app->toSlicerHomeAbsolutePaths(qSlicerExtensionsManagerModel::readArrayValues(settings, "PYTHONPATH", "path"));
  qSlicerExtensionsManagerModel::writeArrayValues(
    settings,
    app->toSlicerHomeRelativePaths(appendToPathList(pythonPaths, this->extensionPythonPaths(extensionName))),
    "PYTHONPATH",
    "path");
#endif

  QStringList qtPluginPaths =
    app->toSlicerHomeAbsolutePaths(qSlicerExtensionsManagerModel::readArrayValues(settings, "QT_PLUGIN_PATH", "path"));
  qSlicerExtensionsManagerModel::writeArrayValues(
    settings,
    app->toSlicerHomeRelativePaths(appendToPathList(qtPluginPaths, this->extensionQtPluginPaths(extensionName))),
    "QT_PLUGIN_PATH",
    "path");
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::removeExtensionPathFromLauncherSettings(const QString& extensionName)
{
  if (this->ExtensionsSettingsFilePath.isEmpty())
  {
    return;
  }
  QSettings settings(this->ExtensionsSettingsFilePath, QSettings::IniFormat);
  if (settings.status() != QSettings::NoError)
  {
    this->warning(qSlicerExtensionsManagerModel::tr("Failed to open extensions settings file: %1")
                    .arg(this->ExtensionsSettingsFilePath));
    return;
  }
  qSlicerCoreApplication* app = qSlicerCoreApplication::application();

  QStringList libraryPath =
    app->toSlicerHomeAbsolutePaths(qSlicerExtensionsManagerModel::readArrayValues(settings, "LibraryPaths", "path"));
  qSlicerExtensionsManagerModel::writeArrayValues(
    settings,
    app->toSlicerHomeRelativePaths(removeFromPathList(libraryPath, this->extensionLibraryPaths(extensionName))),
    "LibraryPaths",
    "path");

  QStringList paths =
    app->toSlicerHomeAbsolutePaths(qSlicerExtensionsManagerModel::readArrayValues(settings, "Paths", "path"));
  qSlicerExtensionsManagerModel::writeArrayValues(
    settings,
    app->toSlicerHomeRelativePaths(removeFromPathList(paths, this->extensionPaths(extensionName))),
    "Paths",
    "path");

#ifdef Slicer_USE_PYTHONQT
  QStringList pythonPaths =
    app->toSlicerHomeAbsolutePaths(qSlicerExtensionsManagerModel::readArrayValues(settings, "PYTHONPATH", "path"));
  qSlicerExtensionsManagerModel::writeArrayValues(
    settings,
    app->toSlicerHomeRelativePaths(removeFromPathList(pythonPaths, this->extensionPythonPaths(extensionName))),
    "PYTHONPATH",
    "path");
#endif

  QStringList qtPluginPaths =
    app->toSlicerHomeAbsolutePaths(qSlicerExtensionsManagerModel::readArrayValues(settings, "QT_PLUGIN_PATH", "path"));
  qSlicerExtensionsManagerModel::writeArrayValues(
    settings,
    app->toSlicerHomeRelativePaths(removeFromPathList(qtPluginPaths, this->extensionQtPluginPaths(extensionName))),
    "QT_PLUGIN_PATH",
    "path");
}

#ifdef Q_OS_WIN
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModelPrivate::checkExtensionsInstallDestinationPath(const QString& destinationPath,
                                                                                 QString& error) const
{
  if (!QDir(destinationPath).exists())
  {
    error = qSlicerExtensionsManagerModel::tr("Extensions install directory does NOT exist: <strong>%1</strong>")
              .arg(destinationPath);
    return false;
  }

  QFileInfo destinationPathInfo(destinationPath);
#ifdef Q_OS_WIN
  struct QtNtfsPermissionLookupHelper
  {
    QtNtfsPermissionLookupHelper() { ++qt_ntfs_permission_lookup; }
    ~QtNtfsPermissionLookupHelper() { --qt_ntfs_permission_lookup; }
  };
  QtNtfsPermissionLookupHelper qt_ntfs_permission_lookup_helper;
#endif
  if (!destinationPathInfo.isReadable() || !destinationPathInfo.isWritable() || !destinationPathInfo.isExecutable())
  {
    error = qSlicerExtensionsManagerModel::tr("Extensions install directory is expected to be "
                                              "readable/writable/executable: <strong>%1</strong>")
              .arg(destinationPath);
    return false;
  }

  return true;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModelPrivate::checkExtensionSettingsPermissions(QString& error) const
{
  Q_Q(const qSlicerExtensionsManagerModel);

  QFileInfo settingsFileInfo = QFileInfo(q->extensionsSettingsFilePath());
  if (settingsFileInfo.exists())
  {
    if (!settingsFileInfo.isReadable() || !settingsFileInfo.isWritable())
    {
      error = QString("Extensions settings file is expected to be readable/writable: <strong>%1</strong>")
                .arg(q->extensionsSettingsFilePath());
      return false;
    }
  }

  return true;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::addExtensionSettings(const QString& extensionName)
{
  this->addExtensionPathToApplicationSettings(extensionName);
  this->addExtensionPathToLauncherSettings(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::removeExtensionSettings(const QString& extensionName)
{
  this->removeExtensionPathFromApplicationSettings(extensionName);
  this->removeExtensionPathFromLauncherSettings(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate ::removeExtensionFromScheduledForUpdateList(const QString& extensionName)
{
  Q_Q(qSlicerExtensionsManagerModel);

  QSettings settings(q->extensionsSettingsFilePath(), QSettings::IniFormat);
  QVariantMap scheduled = settings.value("Extensions/ScheduledForUpdate").toMap();

  scheduled.remove(extensionName);
  settings.setValue("Extensions/ScheduledForUpdate", scheduled);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::removeExtensionFromScheduledForUninstallList(const QString& extensionName)
{
  Q_Q(qSlicerExtensionsManagerModel);
  QSettings settings(q->extensionsSettingsFilePath(), QSettings::IniFormat);
  QStringList extensionsScheduledForUninstall = settings.value("Extensions/ScheduledForUninstall").toStringList();
  extensionsScheduledForUninstall.removeAll(extensionName);
  settings.setValue("Extensions/ScheduledForUninstall", extensionsScheduledForUninstall);
}

// --------------------------------------------------------------------------
QString qSlicerExtensionsManagerModelPrivate::extractArchive(const QDir& extensionsDir, const QString& archiveFile)
{
  // Set extension directory as current directory
  ctkScopedCurrentDir scopedCurrentDir(extensionsDir.absolutePath());

  std::vector<std::string> extracted_files;
  bool success =
    vtkArchive::ExtractTar(qPrintable(archiveFile), /* verbose */ false, /* extract */ true, &extracted_files);
  if (!success)
  {
    this->critical(qSlicerExtensionsManagerModel::tr("Failed to extract %1 into %2")
                     .arg(archiveFile)
                     .arg(extensionsDir.absolutePath()));
    return QString();
  }
  if (extracted_files.size() == 0)
  {
    this->warning(qSlicerExtensionsManagerModel::tr("Archive %1 doesn't contain any files").arg(archiveFile));
    return QString();
  }

  // Compute <topLevelArchiveDir>. We assume all files are extracted in top-level folder.
  QDir extractDirOfFirstFile = QFileInfo(extensionsDir, QString::fromStdString(extracted_files.at(0))).dir();
  QDir topLevelArchiveDir;
  while (extractDirOfFirstFile != extensionsDir)
  {
    topLevelArchiveDir = extractDirOfFirstFile;
    extractDirOfFirstFile.cdUp();
  }

  return topLevelArchiveDir.dirName();
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModelPrivate::extensionLibraryPaths(const QString& extensionName) const
{
  Q_Q(const qSlicerExtensionsManagerModel);
  if (this->SlicerVersion.isEmpty())
  {
    return QStringList();
  }
  QString path = q->extensionInstallPath(extensionName);
  return appendToPathList(
    QStringList(),
    QStringList() << path + "/" + QString(Slicer_BIN_DIR).replace(Slicer_VERSION, this->SlicerVersion)
                  << path + "/" + QString(Slicer_LIB_DIR).replace(Slicer_VERSION, this->SlicerVersion)
                  << path + "/" + QString(Slicer_CLIMODULES_LIB_DIR).replace(Slicer_VERSION, this->SlicerVersion)
                  << path + "/" + QString(Slicer_QTLOADABLEMODULES_LIB_DIR).replace(Slicer_VERSION, this->SlicerVersion)
                  << path + "/" + QString(Slicer_THIRDPARTY_LIB_DIR));
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModelPrivate::extensionQtPluginPaths(const QString& extensionName) const
{
  Q_Q(const qSlicerExtensionsManagerModel);
  if (this->SlicerVersion.isEmpty())
  {
    return QStringList();
  }
  QString path = q->extensionInstallPath(extensionName);
  return appendToPathList(
    QStringList(),
    QStringList() << path + "/" + QString(Slicer_QtPlugins_DIR).replace(Slicer_VERSION, this->SlicerVersion));
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModelPrivate::extensionPaths(const QString& extensionName) const
{
  Q_Q(const qSlicerExtensionsManagerModel);
  if (this->SlicerVersion.isEmpty())
  {
    return QStringList();
  }
  QString path = q->extensionInstallPath(extensionName);
  return appendToPathList(
    QStringList(),
    QStringList() << path + "/" + QString(Slicer_CLIMODULES_BIN_DIR).replace(Slicer_VERSION, this->SlicerVersion)
                  << path + "/" + QString(Slicer_THIRDPARTY_BIN_DIR));
}

#ifdef Slicer_USE_PYTHONQT
// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModelPrivate::extensionPythonPaths(const QString& extensionName) const
{
  Q_Q(const qSlicerExtensionsManagerModel);
  if (this->SlicerVersion.isEmpty())
  {
    return QStringList();
  }

  // When an extension depends on another extension then a .pyd file may depend on .pyd file in the other extension.
  // The .pyd file can only be loaded if it is in the PYTHONPATH folder list.
  // Since .pyd files are in the Slicer_QTLOADABLEMODULES_LIB_DIR, this folder needs to be added to
  // Python paths.

  QString path = q->extensionInstallPath(extensionName);
  return appendToPathList(
    QStringList(),
    QStringList() << path + "/" + QString(Slicer_QTSCRIPTEDMODULES_LIB_DIR).replace(Slicer_VERSION, this->SlicerVersion)
                  << path + "/" + QString(Slicer_QTLOADABLEMODULES_LIB_DIR).replace(Slicer_VERSION, this->SlicerVersion)
                  << path + "/"
                       + QString(Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR).replace(Slicer_VERSION, this->SlicerVersion)
                  << path + "/" + QString(PYTHON_SITE_PACKAGES_SUBDIR));
}
#endif

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModelPrivate::validateExtensionMetadata(const ExtensionMetadataType& extensionMetadata,
                                                                     int serverAPI)
{
  bool valid = true;
  QStringList requiredNonEmptyKeys; // essential keys, return with failure if not found
  QStringList expectedNonEmptyKeys; // log warning if not found (but return with success)
  if (serverAPI == qSlicerExtensionsManagerModel::Girder_v1)
  {
    requiredNonEmptyKeys << "_id"
                         << "meta.app_id"
                         << "meta.app_revision"
                         << "meta.arch"
                         << "meta.baseName"
                         << "meta.os"
                         << "name";
    expectedNonEmptyKeys << "meta.category"
                         << "meta.description"
                         << "meta.homepage"
                         << "meta.icon_url"
                         << "meta.repository_type"
                         << "meta.repository_url"
                         << "meta.revision"
                         << "meta.screenshots";
  }
  else
  {
    qWarning() << Q_FUNC_INFO << " failed: missing implementation for serverAPI" << serverAPI;
    return false;
  }
  // Check required keys (error if missing)
  foreach (const QString& key, requiredNonEmptyKeys)
  {
    if (extensionMetadata.value(key).toString().isEmpty())
    {
      qWarning() << Q_FUNC_INFO << " failed: required key '" << key << "' is missing from extension metadata.";
      valid = false;
    }
  }
  // Check expected keys (warning if missing)
  foreach (const QString& key, expectedNonEmptyKeys)
  {
    if (extensionMetadata.value(key).toString().isEmpty())
    {
      qWarning() << Q_FUNC_INFO << " failed: expected key '" << key << "' is missing from extension metadata.";
    }
  }
  return valid;
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModelPrivate::isExtensionCompatible(const ExtensionMetadataType& metadata,
                                                                        const QString& slicerRevision,
                                                                        const QString& slicerOs,
                                                                        const QString& slicerArch)
{
  if (slicerRevision.isEmpty())
  {
    return QStringList() << qSlicerExtensionsManagerModel::tr("'slicerRevision' parameter is not specified");
  }
  if (slicerOs.isEmpty())
  {
    return QStringList() << qSlicerExtensionsManagerModel::tr("'slicerOs' parameter is not specified");
  }
  if (slicerArch.isEmpty())
  {
    return QStringList() << qSlicerExtensionsManagerModel::tr("'slicerArch' parameter is not specified");
  }
  QStringList reasons;
  QString extensionSlicerRevision = metadata.value("slicer_revision").toString();
  if (!extensionSlicerRevision.isEmpty() && slicerRevision != extensionSlicerRevision)
  {
    reasons << qSlicerExtensionsManagerModel::tr("extensionSlicerRevision [%1] is different from slicerRevision [%2]")
                 .arg(extensionSlicerRevision)
                 .arg(slicerRevision);
  }
  QString extensionArch = metadata.value("arch").toString();
  if (!extensionArch.isEmpty() && slicerArch != extensionArch)
  {
    reasons << qSlicerExtensionsManagerModel::tr("extensionArch [%1] is different from slicerArch [%2]")
                 .arg(extensionArch)
                 .arg(slicerArch);
  }
  QString extensionOs = metadata.value("os").toString();
  if (!extensionOs.isEmpty() && slicerOs != extensionOs)
  {
    reasons << qSlicerExtensionsManagerModel::tr("extensionOs [%1] is different from slicerOs [%2]")
                 .arg(extensionOs)
                 .arg(slicerOs);
  }
  return reasons;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::initializeColumnIdToNameMap(int columnIdx, const char* columnName)
{
  this->ColumnIdToName[columnIdx] = columnName;
  this->ColumnNames << columnName;
}

// --------------------------------------------------------------------------
// qSlicerExtensionsManagerModel methods

CTK_GET_CPP(qSlicerExtensionsManagerModel, bool, interactive, Interactive);
CTK_SET_CPP(qSlicerExtensionsManagerModel, bool, setInteractive, Interactive);

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel::qSlicerExtensionsManagerModel(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerExtensionsManagerModelPrivate(*this))
{
  Q_D(qSlicerExtensionsManagerModel);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel::~qSlicerExtensionsManagerModel() = default;

// --------------------------------------------------------------------------
int qSlicerExtensionsManagerModel::serverAPI() const
{
  int defaultServerAPI = Self::Girder_v1;
  QString serverApiStr =
    qEnvironmentVariable("SLICER_EXTENSIONS_MANAGER_SERVER_API", Self::serverAPIToString(defaultServerAPI));
  int serverAPI = Self::serverAPIFromString(serverApiStr);
  if (serverAPI == -1)
  {
    qWarning() << "Unknown value" << serverApiStr
               << "associated with SLICER_EXTENSIONS_MANAGER_SERVER_API env. variable. "
               << "Defaulting to" << Self::serverAPIToString(defaultServerAPI);
    return defaultServerAPI;
  }
  return serverAPI;
}

// --------------------------------------------------------------------------
QString qSlicerExtensionsManagerModel::serverAPIToString(int serverAPI)
{
  switch (serverAPI)
  {
    case Girder_v1:
      return "Girder_v1";
    default:
      return "";
  }
}

// --------------------------------------------------------------------------
int qSlicerExtensionsManagerModel::serverAPIFromString(const QString& str)
{
  for (int idx = 0; idx < ServerAPI_Last; ++idx)
  {
    if (str == Self::serverAPIToString(idx))
    {
      return idx;
    }
  }
  return -1;
}

// --------------------------------------------------------------------------
QUrl qSlicerExtensionsManagerModel::serverUrl() const
{
  QSettings settings(this->extensionsSettingsFilePath(), QSettings::IniFormat);
  return QUrl(
    qEnvironmentVariable("SLICER_EXTENSIONS_MANAGER_SERVER_URL", settings.value("Extensions/ServerUrl").toString()));
}

// --------------------------------------------------------------------------
QUrl qSlicerExtensionsManagerModel::frontendServerUrl() const
{
  QSettings settings(this->extensionsSettingsFilePath(), QSettings::IniFormat);
  QString url = qEnvironmentVariable("SLICER_EXTENSIONS_MANAGER_FRONTEND_SERVER_URL",
                                     settings.value("Extensions/FrontendServerUrl").toString());
  if (url.endsWith("/"))
  {
    url.chop(1);
  }
  return QUrl(url);
}

// --------------------------------------------------------------------------
QString qSlicerExtensionsManagerModel::extensionsInstallPath() const
{
  QSettings settings(this->extensionsSettingsFilePath(), QSettings::IniFormat);
  return qSlicerCoreApplication::application()->toSlicerHomeAbsolutePath(
    settings.value("Extensions/InstallPath").toString());
}

// --------------------------------------------------------------------------
QString qSlicerExtensionsManagerModel::extensionInstallPath(const QString& extensionName) const
{
  if (extensionName.isEmpty() || this->extensionsInstallPath().isEmpty())
  {
    return QString();
  }
  return this->extensionsInstallPath() + "/" + extensionName;
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::extensionModulePaths(const QString& extensionName) const
{
  Q_D(const qSlicerExtensionsManagerModel);
  QString path = this->extensionInstallPath(extensionName);
  return appendToPathList(
    QStringList(),
    QStringList()
      << path + "/" Slicer_CLIMODULES_SUBDIR // Search for 'Slicer_INSTALL_CLIMODULES_BIN_DIR' in Slicer/CMakeLists.txt
      << path + "/" + QString(Slicer_CLIMODULES_LIB_DIR).replace(Slicer_VERSION, d->SlicerVersion)
      << path + "/" + QString(Slicer_QTLOADABLEMODULES_LIB_DIR).replace(Slicer_VERSION, d->SlicerVersion)
#ifdef Slicer_USE_PYTHONQT
      << path + "/" + QString(Slicer_QTSCRIPTEDMODULES_LIB_DIR).replace(Slicer_VERSION, d->SlicerVersion)
#endif
  );
}

// --------------------------------------------------------------------------
QString qSlicerExtensionsManagerModel::extensionDescriptionFile(const QString& extensionName) const
{
  if (extensionName.isEmpty() || this->extensionInstallPath(extensionName).isEmpty())
  {
    return QString();
  }
  return this->extensionInstallPath(extensionName) + ".s4ext";
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::setNewExtensionEnabledByDefault(bool value)
{
  Q_D(qSlicerExtensionsManagerModel);
  if (d->NewExtensionEnabledByDefault == value)
  {
    return;
  }
  d->NewExtensionEnabledByDefault = value;
  emit this->newExtensionEnabledByDefaultChanged(value);
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::newExtensionEnabledByDefault() const
{
  Q_D(const qSlicerExtensionsManagerModel);
  return d->NewExtensionEnabledByDefault;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::setAutoUpdateCheck(bool enable)
{
  Q_D(qSlicerExtensionsManagerModel);
  if (d->AutoUpdateCheck == enable)
  {
    return;
  }
  d->AutoUpdateCheck = enable;
  QSettings settings;
  settings.setValue("Extensions/AutoUpdateCheck", enable);
  emit this->autoUpdateSettingsChanged();
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::autoUpdateCheck() const
{
  Q_D(const qSlicerExtensionsManagerModel);
  return d->AutoUpdateCheck;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::setAutoUpdateInstall(bool enable)
{
  Q_D(qSlicerExtensionsManagerModel);
  if (d->AutoUpdateInstall == enable)
  {
    return;
  }
  d->AutoUpdateInstall = enable;
  QSettings settings;
  settings.setValue("Extensions/AutoUpdateInstall", enable);
  emit this->autoUpdateSettingsChanged();
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::autoUpdateInstall() const
{
  Q_D(const qSlicerExtensionsManagerModel);
  return d->AutoUpdateInstall;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::setAutoInstallDependencies(bool enable)
{
  Q_D(qSlicerExtensionsManagerModel);
  if (d->AutoInstallDependencies == enable)
  {
    return;
  }
  d->AutoInstallDependencies = enable;
  QSettings settings;
  settings.setValue("Extensions/AutoInstallDependencies", enable);
  emit this->autoUpdateSettingsChanged();
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::autoInstallDependencies() const
{
  Q_D(const qSlicerExtensionsManagerModel);
  return d->AutoInstallDependencies;
}

// --------------------------------------------------------------------------
QString qSlicerExtensionsManagerModel::extensionDescription(const QString& extensionName) const
{
  Q_D(const qSlicerExtensionsManagerModel);
  QStandardItem* item = d->extensionItem(extensionName);
  if (!item)
  {
    return QString();
  }
  int row = item->row();
  QString description = d->Model
                          .data(d->Model.index(row, qSlicerExtensionsManagerModelPrivate::DescriptionColumn),
                                qSlicerExtensionsManagerModelPrivate::DescriptionRole)
                          .toString();
  return description;
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel::ExtensionMetadataType qSlicerExtensionsManagerModel::extensionMetadata(
  const QString& extensionName,
  int source /*=MetadataAll*/) const
{
  Q_D(const qSlicerExtensionsManagerModel);

  ExtensionMetadataType metadata;
  if ((source == MetadataAll || source == MetadataServer) && d->ExtensionsMetadataFromServer.contains(extensionName))
  {
    metadata = d->ExtensionsMetadataFromServer[extensionName];
  }
  if (source == MetadataAll || source == MetadataLocal)
  {
    QStandardItem* item = d->extensionItem(extensionName);
    if (item)
    {
      int row = item->row();
      foreach (const QString& columnName, d->columnNames())
      {
        metadata.insert(
          columnName,
          d->Model.data(d->Model.index(row, d->columnNames().indexOf(columnName)), Qt::DisplayRole).toString());
      }
    }
  }
  return metadata;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::isExtensionLoaded(const QString& extensionName) const
{
  Q_D(const qSlicerExtensionsManagerModel);
  QStandardItem* item = d->extensionItem(extensionName, qSlicerExtensionsManagerModelPrivate::LoadedColumn);
  if (!item)
  {
    return false;
  }
  return item->data(qSlicerExtensionsManagerModelPrivate::LoadedRole).toBool();
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::isExtensionInstalled(const QString& extensionName) const
{
  Q_D(const qSlicerExtensionsManagerModel);
  QStandardItem* item = d->extensionItem(extensionName, qSlicerExtensionsManagerModelPrivate::InstalledColumn);
  if (!item)
  {
    return false;
  }
  return item->data(qSlicerExtensionsManagerModelPrivate::InstalledRole).toBool();
}

// --------------------------------------------------------------------------
int qSlicerExtensionsManagerModel::installedExtensionsCount() const
{
  return this->installedExtensions().size();
}

// --------------------------------------------------------------------------
int qSlicerExtensionsManagerModel::numberOfInstalledExtensions() const
{
  return this->installedExtensionsCount();
}

// --------------------------------------------------------------------------
int qSlicerExtensionsManagerModel::managedExtensionsCount() const
{
  return this->managedExtensions().size();
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::managedExtensions() const
{
  Q_D(const qSlicerExtensionsManagerModel);
  QStringList names;
  for (int rowIdx = 0; rowIdx < d->Model.rowCount(); ++rowIdx)
  {
    names << d->Model.item(rowIdx, qSlicerExtensionsManagerModelPrivate::NameColumn)->text();
  }
  std::sort(names.begin(), names.end());
  return names;
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::installedExtensions() const
{
  Q_D(const qSlicerExtensionsManagerModel);
  QStringList names;
  for (int rowIdx = 0; rowIdx < d->Model.rowCount(); ++rowIdx)
  {
    QStandardItem* installedItem = d->Model.item(rowIdx, qSlicerExtensionsManagerModelPrivate::InstalledColumn);
    if (!installedItem)
    {
      // invalid item
      continue;
    }
    if (!installedItem->data(qSlicerExtensionsManagerModelPrivate::InstalledRole).toBool())
    {
      // not installed
      continue;
    }
    names << d->Model.item(rowIdx, qSlicerExtensionsManagerModelPrivate::NameColumn)->text();
  }
  std::sort(names.begin(), names.end());
  return names;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::setExtensionBookmarked(const QString& extensionName, bool bookmark)
{
  Q_D(qSlicerExtensionsManagerModel);
  if (bookmark == this->isExtensionBookmarked(extensionName))
  {
    // no change
    return;
  }

  // Update model
  QStandardItem* item =
    const_cast<QStandardItem*>(d->extensionItem(extensionName, qSlicerExtensionsManagerModelPrivate::BookmarkedColumn));
  if (!item)
  {
    d->warning(tr("Failed to add bookmark extension: %1").arg(extensionName));
    return;
  }

  item->setData(bookmark, qSlicerExtensionsManagerModelPrivate::BookmarkedRole);
  item->setData(bookmark, Qt::DisplayRole);

  // Bookmarked status is shared between all Slicer installations, therefore this flag is not saved into the
  // extension description file but to application settings.

  QString error;
  if (!d->checkExtensionSettingsPermissions(error))
  {
    d->critical(error);
    return;
  }

  QSettings settings;
  QStringList bookmarkedExtensions = settings.value("Extensions/Bookmarked").toStringList();
  if (bookmarkedExtensions.contains(extensionName) != bookmark)
  {
    if (bookmark)
    {
      bookmarkedExtensions << extensionName;
    }
    else
    {
      bookmarkedExtensions.removeAll(extensionName);
    }
    settings.setValue("Extensions/Bookmarked", bookmarkedExtensions);
  }

  bool installed = this->isExtensionInstalled(extensionName);
  if (!installed)
  {
    d->removeExtensionDescriptionFile(extensionName);
    d->Model.removeRow(item->row());
  }

  emit this->extensionBookmarkedChanged(extensionName, bookmark);
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::isExtensionBookmarked(const QString& extensionName) const
{
  Q_D(const qSlicerExtensionsManagerModel);
  QStandardItem* item = d->extensionItem(extensionName, qSlicerExtensionsManagerModelPrivate::BookmarkedColumn);
  if (!item)
  {
    return false;
  }
  return item->data(qSlicerExtensionsManagerModelPrivate::BookmarkedRole).toBool() == true;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::setExtensionEnabled(const QString& extensionName, bool value)
{
  Q_D(qSlicerExtensionsManagerModel);

  QString error;
  if (!d->checkExtensionSettingsPermissions(error))
  {
    d->critical(error);
    return;
  }

  if (value && !this->isExtensionCompatible(extensionName).isEmpty())
  {
    return;
  }

  if (value)
  {
    d->addExtensionSettings(extensionName);
  }
  else
  {
    d->removeExtensionSettings(extensionName);
  }

  if (value == this->isExtensionEnabled(extensionName))
  {
    return;
  }

  QStandardItem* item =
    const_cast<QStandardItem*>(d->extensionItem(extensionName, qSlicerExtensionsManagerModelPrivate::EnabledColumn));
  Q_ASSERT(item);
  item->setData(value, qSlicerExtensionsManagerModelPrivate::EnabledRole);
  item->setData(value, Qt::DisplayRole);

  this->writeExtensionDescriptionFile(this->extensionDescriptionFile(extensionName),
                                      this->extensionMetadata(extensionName));

  emit this->extensionEnabledChanged(extensionName, value);
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::isExtensionEnabled(const QString& extensionName) const
{
  Q_D(const qSlicerExtensionsManagerModel);
  QStandardItem* item = d->extensionItem(extensionName, qSlicerExtensionsManagerModelPrivate::EnabledColumn);
  if (!item)
  {
    return false;
  }
  return item->data(qSlicerExtensionsManagerModelPrivate::EnabledRole).toBool() == true;
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::scheduledForUpdateExtensions() const
{
  QSettings settings(this->extensionsSettingsFilePath(), QSettings::IniFormat);
  return settings.value("Extensions/ScheduledForUpdate").toMap().keys();
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::isExtensionScheduledForUpdate(const QString& extensionName) const
{
  return this->scheduledForUpdateExtensions().contains(extensionName);
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::scheduledForUninstallExtensions() const
{
  QSettings settings(this->extensionsSettingsFilePath(), QSettings::IniFormat);
  return settings.value("Extensions/ScheduledForUninstall").toStringList();
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::isExtensionScheduledForUninstall(const QString& extensionName) const
{
  return this->scheduledForUninstallExtensions().contains(extensionName);
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::bookmarkedExtensions() const
{
  QSettings settings;
  QStringList bookmarkedExtensions = settings.value("Extensions/Bookmarked").toStringList();
  return bookmarkedExtensions;
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::enabledExtensions() const
{
  Q_D(const qSlicerExtensionsManagerModel);
  QStringList names;
  for (int rowIdx = 0; rowIdx < d->Model.rowCount(); ++rowIdx)
  {
    QString extensionName = d->Model.item(rowIdx, qSlicerExtensionsManagerModelPrivate::NameColumn)->text();
    if (this->isExtensionEnabled(extensionName))
    {
      names << d->Model.item(rowIdx, qSlicerExtensionsManagerModelPrivate::NameColumn)->text();
    }
  }
  std::sort(names.begin(), names.end());
  return names;
}

// --------------------------------------------------------------------------
qSlicerExtensionDownloadTask* qSlicerExtensionsManagerModelPrivate::downloadExtensionByName(
  const QString& extensionName)
{
  Q_Q(qSlicerExtensionsManagerModel);

  this->debug(qSlicerExtensionsManagerModel::tr("Retrieving extension metadata for %1 extension").arg(extensionName));
  ExtensionMetadataType extensionMetadata = this->ExtensionsMetadataFromServer.value(extensionName);
  if (extensionMetadata.count() == 0 || !extensionMetadata.contains("extension_id"))
  {
    // Ensure extension metadata has been downloaded from the server and try again.
    // We need to force the update because the extension might have been added a few hours ago
    // (so the cached version of the extension metadata may not contain it).
    q->updateExtensionsMetadataFromServer(true, true);
    extensionMetadata = this->ExtensionsMetadataFromServer.value(extensionName);
  }
  if (extensionMetadata.count() == 0)
  {
    this->critical(
      qSlicerExtensionsManagerModel::tr("Failed to get metadata from server for extension: %1").arg(extensionName));
  }

  QUrl downloadUrl(q->serverUrl());

  if (q->serverAPI() == qSlicerExtensionsManagerModel::Girder_v1)
  {
    QString item_id = extensionMetadata["extension_id"].toString();
    if (item_id.isEmpty())
    {
      return nullptr;
    }

    // Retrieve file_id and archive name (extension package filename) associated with the item
    QString file_id;
    QString archivename;

    this->debug(qSlicerExtensionsManagerModel::tr("Retrieving %1 extension files (extensionId: %2)")
                  .arg(extensionName)
                  .arg(item_id));
    qRestAPI getItemFilesApi;
    getItemFilesApi.setServerUrl(q->serverUrl().toString() + QString("/api/v1/item/%1/files").arg(item_id));
    const QUuid& queryUuid = getItemFilesApi.get("");
    QScopedPointer<qRestResult> restResult(getItemFilesApi.takeResult(queryUuid));
    if (restResult)
    {
      qGirderAPI::parseGirderAPIv1Response(restResult.data(), restResult->response());
      QList<QVariantMap> results = restResult->results();
      if (results.isEmpty())
      {
        // extension manager returned 0 file, this is not expected
        return nullptr;
      }
      else if (results.count() == 1)
      {
        file_id = results.at(0).value("_id").toString();
        archivename = results.at(0).value("name").toString();
      }
      else
      {
        // extension manager returned multiple files, this is not expected, do not use the results
        return nullptr;
      }
    }

    if (file_id.isEmpty() || archivename.isEmpty())
    {
      return nullptr;
    }

    this->debug(qSlicerExtensionsManagerModel::tr("Downloading %1 extension (item_id: %2, file_id: %3)")
                  .arg(extensionName)
                  .arg(item_id)
                  .arg(file_id));
    downloadUrl.setPath(downloadUrl.path() + QString("/api/v1/file/%1/download").arg(file_id));
    extensionMetadata.insert("archivename", archivename);
  }
  else
  {
    qWarning() << Q_FUNC_INFO << " failed: missing implementation for serverAPI" << q->serverAPI();
    return nullptr;
  }

  QNetworkReply* const reply = this->NetworkManager.get(QNetworkRequest(downloadUrl));
  qSlicerExtensionDownloadTask* const task = new qSlicerExtensionDownloadTask(reply);
  this->ActiveTasks[task] = QString("install %1 extension").arg(extensionMetadata["extensionname"].toString());

  task->setMetadata(extensionMetadata);
  emit q->downloadStarted(reply);
  emit q->activeTasksChanged();

  return task;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::downloadAndInstallExtension(const QString& extensionId,
                                                                bool installDependencies /*=true*/,
                                                                bool waitForCompletion /*=false*/)
{
  Q_D(qSlicerExtensionsManagerModel);
  // Installing an extension requires metadata of all other extensions to be able to track down dependencies,
  // therefore specifying the extension by a low-level id does not make sense. However, this is the method
  // used by the web interface, so for now we keep supporting it.

  if (extensionId.isEmpty())
  {
    d->critical(tr("Download of extension failed, id is invalid."));
    return false;
  }

  // Ensure extension metadata has been downloaded from the server
  if (d->ExtensionsMetadataFromServer.empty())
  {
    this->updateExtensionsMetadataFromServer(/* force= */ true, /* waitForCompletion= */ true);
  }

  // Find extension name by ID
  foreach (const QString& extensionName, d->ExtensionsMetadataFromServer.keys())
  {
    if (d->ExtensionsMetadataFromServer[extensionName].value("extension_id") == extensionId)
    {
      // found it
      return this->downloadAndInstallExtensionByName(extensionName, installDependencies, waitForCompletion);
    }
  }

  // not found
  d->critical(tr("Download of extension failed, could not find an extension with id = %1").arg(extensionId));
  return false;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::downloadAndInstallExtensionByName(const QString& extensionName,
                                                                      bool installDependencies,
                                                                      bool waitForCompletion /*=false*/)
{
  Q_D(qSlicerExtensionsManagerModel);
  QString error;
  if (!d->checkExtensionSettingsPermissions(error))
  {
    d->critical(error);
    return false;
  }
  qSlicerExtensionDownloadTask* const task = d->downloadExtensionByName(extensionName);
  if (!task)
  {
    d->critical(tr("Failed to retrieve metadata for %1 extension").arg(extensionName));
    return false;
  }
  task->setInstallDependencies(installDependencies);
  connect(task,
          SIGNAL(finished(qSlicerExtensionDownloadTask*)),
          this,
          SLOT(onInstallDownloadFinished(qSlicerExtensionDownloadTask*)));
  connect(task,
          SIGNAL(progress(qSlicerExtensionDownloadTask*, qint64, qint64)),
          this,
          SLOT(onInstallDownloadProgress(qSlicerExtensionDownloadTask*, qint64, qint64)));
  if (waitForCompletion)
  {
    this->waitForAllTasksCompletion();
  }
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::installExtensionFromServer(const QString& extensionName, bool restart, bool update)
{
  Q_D(qSlicerExtensionsManagerModel);

  if (this->isExtensionInstalled(extensionName))
  {
    if (update)
    {
      // Ensure extension metadata is retrieved from the server or cache.
      if (!this->updateExtensionsMetadataFromServer(/* force= */ true, /* waitForCompletion= */ true))
      {
        return false;
      }
      this->checkForExtensionsUpdates();
      this->scheduleExtensionForUpdate(extensionName);

      // wait for pending downloadAndInstallExtensionByName() completions
      this->waitForAllTasksCompletion();

      QStringList updatedExtensions;
      if (!this->updateScheduledExtensions(updatedExtensions))
      {
        return false;
      }
      foreach (const QString& extensionName, updatedExtensions)
      {
        qDebug() << "Successfully updated extension" << extensionName;
      }
    }
    return true;
  }

  bool isTestingEnabled = qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_EnableTesting);

  // Handle installation confirmation
  bool installationConfirmed = false;
  if (isTestingEnabled)
  {
    installationConfirmed = true;
    qDebug() << "Installing the extension(s) without asking for confirmation (testing mode is enabled)";
  }
  else if (!this->interactive())
  {
    installationConfirmed = true;
    qDebug() << "Installing the extension(s) without asking for confirmation (interactive mode is disabled)";
  }
  else
  {
    QString message = tr("Do you want to install '%1' now?").arg(extensionName);
    QMessageBox::StandardButton answer = QMessageBox::question(nullptr, tr("Install extension ?"), message);
    installationConfirmed = (answer == QMessageBox::StandardButton::Yes);
  }
  if (!installationConfirmed)
  {
    return false;
  }

  // Ensure extension metadata is retrieved from the server or cache.
  if (!this->updateExtensionsMetadataFromServer(/* force= */ true, /* waitForCompletion= */ true))
  {
    return false;
  }

  // Install extension and its dependencies
  if (!this->downloadAndInstallExtensionByName(
        extensionName, /* installDependencies= */ true, /* waitForCompletion= */ true))
  {
    d->critical(tr("Failed to install %1 extension").arg(extensionName));
    return false;
  }

  if (!restart)
  {
    return true;
  }

  // Handle restart confirmation
  bool restartConfirmed = false;
  if (isTestingEnabled)
  {
    restartConfirmed = false;
    qDebug() << "Skipping application restart (testing mode is enabled)";
  }
  else if (!this->interactive())
  {
    restartConfirmed = true;
    qDebug() << "Restarting the application without asking for confirmation (interactive mode is disabled)";
  }
  else
  {
    QString message = tr("Extension %1 has been installed from server.").arg(extensionName);
    message + "\n\n";
    message += tr("Slicer must be restarted. Do you want to restart now ?");
    QMessageBox::StandardButton answer = QMessageBox::question(nullptr, tr("Restart slicer ?"), message);
    restartConfirmed = (answer == QMessageBox::StandardButton::Yes);
  }
  if (restartConfirmed)
  {
    qSlicerCoreApplication::application()->restart();
  }
  return true;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::onInstallDownloadProgress(qSlicerExtensionDownloadTask* task,
                                                              qint64 received,
                                                              qint64 total)
{
  // Look up the update information
  const QString& extensionName = task->extensionName();

  // Notify observers of download progress
  emit this->installDownloadProgress(extensionName, received, total);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::onInstallDownloadFinished(qSlicerExtensionDownloadTask* task)
{
  Q_D(qSlicerExtensionsManagerModel);

  task->deleteLater();

  QNetworkReply* const reply = task->reply();
  QUrl downloadUrl = reply->url();
  Q_ASSERT(downloadUrl.path().contains("/download"));

  emit this->downloadFinished(reply);

  if (reply->error())
  {
    d->critical(tr("Failed downloading: %1").arg(downloadUrl.toString()));
    d->ActiveTasks.remove(task);
    emit activeTasksChanged();
    return;
  }

  const QString& extensionName = task->extensionName();
  const QString& archiveName = task->archiveName();
  QTemporaryFile file(QString("%1/%2.XXXXXX").arg(QDir::tempPath(), archiveName));
  if (!file.open())
  {
    d->critical(tr("Could not create temporary file for writing: %1").arg(file.errorString()));
    d->ActiveTasks.remove(task);
    emit activeTasksChanged();
    return;
  }
  file.write(reply->readAll());
  file.close();
  this->installExtension(extensionName, task->metadata(), file.fileName(), task->installDependencies());
  d->ActiveTasks.remove(task);
  emit activeTasksChanged();
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::installExtension(const QString& archiveFile,
                                                     bool installDependencies /*=true*/,
                                                     bool waitForCompletion /*=false*/)
{
  Q_D(qSlicerExtensionsManagerModel);

  std::vector<std::string> archiveContents;
  if (!vtkArchive::ListArchive(qPrintable(archiveFile), archiveContents))
  {
    d->critical(tr("Failed to list extension archive '%1'").arg(archiveFile));
    return false;
  }
  for (size_t n = 0; n < archiveContents.size(); ++n)
  {
    const std::string& s = archiveContents[n];
    const QString& fileName = QString::fromLocal8Bit(s.data(), static_cast<int>(s.size()));

    if (fileName.endsWith(".s4ext"))
    {
      const QFileInfo fi(fileName);
      return this->installExtension(
        fi.completeBaseName(), ExtensionMetadataType(), archiveFile, installDependencies, waitForCompletion);
    }
  }

  d->critical(tr("No extension description found in archive '%1'").arg(archiveFile));
  return false;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::installExtension(const QString& extensionName,
                                                     ExtensionMetadataType extensionMetadata,
                                                     const QString& archiveFile,
                                                     bool withDependencies /*=true*/,
                                                     bool waitForCompletion /*=false*/)
{
  Q_D(qSlicerExtensionsManagerModel);

  if (extensionName.isEmpty())
  {
    d->critical(tr("InstallExtension failed: extensionName is not set"));
    return false;
  }
  if (this->isExtensionInstalled(extensionName))
  {
    d->warning(tr("Skip installation of %1 extension. It is already installed.").arg(extensionName));
    return false;
  }

  if (this->extensionsInstallPath().isEmpty())
  {
    d->critical(tr("Extensions/InstallPath setting is not set"));
    return false;
  }

  if (!QDir().mkpath(this->extensionsInstallPath()))
  {
    d->critical(tr("Failed to create extension installation directory %1").arg(this->extensionsInstallPath()));
    return false;
  }

  QString error;
  if (!d->checkExtensionSettingsPermissions(error))
  {
    d->critical(error);
    return false;
  }

  if (!this->extractExtensionArchive(extensionName, archiveFile, this->extensionsInstallPath()))
  {
    // extractExtensionArchive has logged the error
    return false;
  }

  // Read description file provided by the extension itself, in order to obtain
  // reported dependency information (which is not provided by the extension
  // server)
  const QString& extensionIndexDescriptionFile =
    this->extensionsInstallPath() + "/" + extensionName + "/" + Slicer_SHARE_DIR + "/" + extensionName + ".s4ext";
  const ExtensionMetadataType& extensionIndexMetadata =
    Self::parseExtensionDescriptionFile(extensionIndexDescriptionFile);

  // Copy metadata if not provided from server (e.g. installing from file)
  if (extensionMetadata.isEmpty())
  {
    extensionMetadata.insert("archivename", QFileInfo(archiveFile).fileName());
    extensionMetadata.insert("extensionname", extensionName);

    // Copy expected keys from archive description
    QStringList expectedKeys;
    expectedKeys << /*no tr*/ "category" << /*no tr*/ "contributors" << /*no tr*/ "description" << /*no tr*/ "homepage"
                 << /*no tr*/ "iconurl" << /*no tr*/ "screenshots" << /*no tr*/ "status" << /*no tr*/ "updated";

    const ExtensionMetadataType::const_iterator notFound = extensionIndexMetadata.constEnd();
    foreach (const QString& key, expectedKeys)
    {
      const ExtensionMetadataType::const_iterator iter = extensionIndexMetadata.constFind(key);
      if (iter != notFound)
      {
        extensionMetadata.insert(key, iter.value());
      }
    }

    extensionMetadata.insert("scm", extensionIndexMetadata.value("scm", "NA"));
    extensionMetadata.insert("scmurl", extensionIndexMetadata.value("scmurl", "NA"));
    extensionMetadata.insert("revision", extensionIndexMetadata.value("scmrevision", "NA"));

    // Fill in keys related to the target Slicer platform
    extensionMetadata.insert("os", this->slicerOs());
    extensionMetadata.insert("arch", this->slicerArch());
    extensionMetadata.insert("slicer_revision", this->slicerRevision());
  }

  // Enable or disable the extension if state was not already set
  if (!extensionMetadata.contains("enabled"))
  {
    extensionMetadata.insert("enabled", d->NewExtensionEnabledByDefault);
  }

  bool success = true;
  if (withDependencies)
  {
    QStringList unresolvedDependencies;
    QStringList directDependencies = extensionMetadata.value("depends").toString().split(" ");
    QStringList dependenciesToInstall = d->dependenciesToInstall(directDependencies, unresolvedDependencies);

    // Prompt to install dependencies (if any)
    if (!dependenciesToInstall.isEmpty())
    {
      QMessageBox::StandardButton result = QMessageBox::Yes;
      if (d->Interactive && !d->AutoInstallDependencies)
      {
        QString msg = QString("<p>%1 depends on the following extensions:</p><ul>").arg(extensionName);
        foreach (const QString& dependencyName, dependenciesToInstall)
        {
          msg += QString("<li>%1</li>").arg(dependencyName);
        }
        msg += "</ul><p>Would you like to install them now?</p>";
        result = QMessageBox::question(nullptr, "Install dependencies", msg, QMessageBox::Yes | QMessageBox::No);
      }
      else
      {
        QString msg =
          QString("The following extensions are required by %1 extension therefore they will be installed now: %2")
            .arg(extensionName)
            .arg(dependenciesToInstall.join(", "));
        qDebug() << msg;
      }

      if (result == QMessageBox::Yes)
      {
        // Install dependencies
        QString msg;
        foreach (const QString& dependency, dependenciesToInstall)
        {
          bool res = this->downloadAndInstallExtensionByName(dependency,
                                                             false /*installation of dependencies already confirmed*/);
          if (!res)
          {
            msg += QString("<li>%1</li>").arg(dependency);
            success = false;
          }
        }
        if (!msg.isEmpty())
        {
          d->critical(tr("Error while installing dependent extensions:<ul>%1<ul>").arg(msg));
        }
      }
      else
      {
        // Skip installing dependencies
        qWarning() << QString("%1 extension requires extensions %2 but the user chose not to install them.")
                        .arg(extensionName)
                        .arg(dependenciesToInstall.join(", "));
        success = false;
      }
    }

    // Warn about unresolved dependencies
    if (!unresolvedDependencies.isEmpty())
    {
      success = false;
      qWarning() << QString(/*no tr*/ "%1 extension depends on the following extensions, which could not be found: %2")
                      .arg(extensionName)
                      .arg(unresolvedDependencies.join(", "));
      if (d->Interactive)
      {
        //: %1 is the extension name
        QString msg =
          QString("<p>%1</p><ul>")
            .arg(tr("%1 depends on the following extensions, which could not be found:").arg(extensionName));
        foreach (const QString& dependencyName, unresolvedDependencies)
        {
          msg += QString("<li>%1</li>").arg(dependencyName);
        }
        msg += QString("</ul><p>%1</p>").arg(tr("The extension may not function properly."));
        QMessageBox::warning(nullptr, tr("Unresolved dependencies"), msg);
      }
    }
  }

  // Finish installing the extension
  extensionMetadata["installed"] = true;
  d->addExtensionSettings(extensionName);
  // bookmarking is shared between all application installations, it is not in the
  // metadata fields
  bool bookmarked = QSettings().value("Extensions/Bookmarked").toStringList().contains(extensionName);
  extensionMetadata["bookmarked"] = bookmarked;
  d->addExtensionModelRow(extensionMetadata);

  this->writeExtensionDescriptionFile(this->extensionDescriptionFile(extensionName),
                                      this->extensionMetadata(extensionName));

  emit this->extensionInstalled(extensionName);

  // Log notice that extension was installed
  const QString& extensionId = extensionMetadata.value("extension_id").toString();
  const QString& extensionRevision = extensionMetadata.value("revision").toString();

  QString msg = tr("Installed extension %1").arg(extensionName);
  if (!extensionId.isEmpty())
  {
    msg += QString(" (%1)").arg(extensionId);
  }
  if (!extensionRevision.isEmpty())
  {
    msg += QString(" revision %1").arg(extensionRevision);
  }
  d->info(msg);

  if (waitForCompletion)
  {
    // wait for pending downloadAndInstallExtensionByName() completions
    this->waitForAllTasksCompletion();
  }

  return success;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::updateExtensionsMetadataFromServer(bool force /*=false*/,
                                                                       bool waitForCompletion /*=false*/)
{
  Q_D(qSlicerExtensionsManagerModel);

  // until the new data is read from the server
  d->loadCachedExtensionsMetadataFromServer();

  if (!force && !d->isExtensionsMetadataUpdateDue())
  {
    return true;
  }

  if (d->ExtensionsMetadataFromServerQueryUID.isNull())
  {
    // query is not in progress yet, start it
    qRestAPI::Parameters parameters;
    if (this->serverAPI() == qSlicerExtensionsManagerModel::Girder_v1)
    {
      QString appID = "5f4474d0e1d8c75dfc705482";
      if (this->serverUrl().toString().isEmpty())
      {
        // server address has not been specified, normal at very first startup
        // (default server address is set up by an application settings page)
        return false;
      }
      d->ExtensionsMetadataFromServerAPI.setServerUrl(this->serverUrl().toString()
                                                      + QString("/api/v1/app/%1/extension").arg(appID));
      parameters["app_revision"] = this->slicerRevision();
      parameters["os"] = this->slicerOs();
      parameters["arch"] = this->slicerArch();
      // request all metadata in a single response (it makes synchronous query simpler)
      parameters["limit"] = QString::number(0);
    }
    else
    {
      qWarning() << "Update extension information from server failed: missing implementation for serverAPI"
                 << this->serverAPI();
      return false;
    }

    // Issue the query
    d->ExtensionsMetadataFromServerQueryUID = d->ExtensionsMetadataFromServerAPI.get("", parameters);
  }

  if (!waitForCompletion)
  {
    return true;
  }

  // Temporarily disable onExtensionsMetadataFromServerQueryFinished call via signal/slot
  // because we'll call it directly to get returned result.
  QObject::disconnect(&d->ExtensionsMetadataFromServerAPI,
                      SIGNAL(finished(QUuid)),
                      this,
                      SLOT(onExtensionsMetadataFromServerQueryFinished(QUuid)));

  bool success = this->onExtensionsMetadataFromServerQueryFinished(d->ExtensionsMetadataFromServerQueryUID);

  QObject::connect(&d->ExtensionsMetadataFromServerAPI,
                   SIGNAL(finished(QUuid)),
                   this,
                   SLOT(onExtensionsMetadataFromServerQueryFinished(QUuid)));

  if (!success)
  {
    d->warning(
      tr("Update extension information from server failed: timed out while waiting for server response from %1")
        .arg(d->ExtensionsMetadataFromServerAPI.serverUrl()));
  }

  return success;
}

// --------------------------------------------------------------------------
QDateTime qSlicerExtensionsManagerModel::lastUpdateTimeExtensionsMetadataFromServer()
{
  QSettings extensionSettings(this->extensionsSettingsFilePath(), QSettings::IniFormat);
  if (!extensionSettings.contains("Extensions/MetadataFromServerUpdateTime"))
  {
    // there has never been an update check
    return QDateTime();
  }
  QString lastUpdateTimeStr = extensionSettings.value("Extensions/MetadataFromServerUpdateTime").toString();
  QDateTime lastUpdateTime = QDateTime::fromString(lastUpdateTimeStr, Qt::ISODate);
  return lastUpdateTime;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::isExtensionUpdateAvailable(const QString& extensionName) const
{
  Q_D(const qSlicerExtensionsManagerModel);
  return d->AvailableUpdates.contains(extensionName);
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::availableUpdateExtensions() const
{
  Q_D(const qSlicerExtensionsManagerModel);
  return d->AvailableUpdates.keys();
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::onExtensionsMetadataFromServerQueryFinished(const QUuid& requestId)
{
  Q_UNUSED(requestId);
  Q_D(qSlicerExtensionsManagerModel);

  QScopedPointer<qRestResult> restResult(
    d->ExtensionsMetadataFromServerAPI.takeResult(d->ExtensionsMetadataFromServerQueryUID));

  bool success = false;
  if (!restResult.isNull())
  {
    if (this->serverAPI() == qSlicerExtensionsManagerModel::Girder_v1)
    {
      success = qGirderAPI::parseGirderAPIv1Response(restResult.data(), restResult->response());
    }
  }
  if (!success)
  {
    // Query failed
    d->warning(tr("Failed to download extension metadata from server"));
    d->ExtensionsMetadataFromServerQueryUID = QUuid();
    emit updateExtensionsMetadataFromServerCompleted(false);
    return false;
  }

  d->ExtensionsMetadataFromServer.clear();

  // Process response
  foreach (const QVariantMap& result, restResult->results())
  {
    // Get extension information from server response
    ExtensionMetadataType serverExtensionMetadata = qRestAPI::qVariantMapFlattened(result);
    ExtensionMetadataType extensionMetadata = Self::filterExtensionMetadata(serverExtensionMetadata, this->serverAPI());
    extensionMetadata = Self::convertExtensionMetadata(extensionMetadata, this->serverAPI());
    QString extensionName = extensionMetadata["extensionname"].toString();
    if (extensionName.isEmpty())
    {
      d->warning(tr("Extension metadata response missed required 'extensionname' field"));
      continue;
    }
    d->ExtensionsMetadataFromServer[extensionName] = extensionMetadata;
  }
  d->ExtensionsMetadataFromServerQueryUID = QUuid();
  d->saveExtensionsMetadataFromServerToCache();

  this->updateModel();

  emit updateExtensionsMetadataFromServerCompleted(true);
  return true;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::checkForExtensionsUpdates()
{
  Q_D(qSlicerExtensionsManagerModel);
  bool updatedExtensionsFound = false;

  QSettings settings;
  foreach (const QString& extensionName, d->ExtensionsMetadataFromServer.keys())
  {
    ExtensionMetadataType metadata = d->ExtensionsMetadataFromServer[extensionName];
    // Check for updates
    if (this->isExtensionInstalled(extensionName))
    {
      // Extension is installed, check if an update is available
      const QString& extensionId = metadata.value("extension_id").toString();
      const QString& extensionRevision = metadata.value("revision").toString();
      if (extensionId.isEmpty() || extensionRevision.isEmpty())
      {
        d->warning(tr("Update check response for extension %1 missed required 'extension_id' or 'revision' field")
                     .arg(extensionName));
        continue;
      }
      ExtensionMetadataType installedExtensionMetadata = this->extensionMetadata(extensionName);
      QString installedRevision = installedExtensionMetadata.value("revision").toString();
      if (extensionRevision == installedRevision)
      {
        // up-to-date
        continue;
      }
      // Update is available
      d->debug(tr("Update found for %1 extension: '%2' installed, '%3' available, ")
                 .arg(extensionName, installedRevision, extensionRevision));
      d->AvailableUpdates.insert(extensionName, UpdateDownloadInformation(extensionId));
      // Immediately start update process if requested
      if (d->AutoUpdateInstall)
      {
        this->scheduleExtensionForUpdate(extensionName);
      }
      emit this->extensionUpdateAvailable(extensionName);
      updatedExtensionsFound = true;
    }
    else if (this->isExtensionBookmarked(extensionName))
    {
      // Extension is not installed just bookmarked.
      // It is included in the model, so we need to update the model.
      metadata["installed"] = false;
      metadata["loaded"] = false;
      metadata["bookmarked"] = true;
      d->addExtensionModelRow(metadata);
      emit this->extensionMetadataUpdated(extensionName);
    }
  }

  if (updatedExtensionsFound)
  {
    QSettings extensionSettings(this->extensionsSettingsFilePath(), QSettings::IniFormat);
    extensionSettings.setValue("Extensions/UpdatesAvailable", true);
  }
  emit extensionUpdatesAvailable(updatedExtensionsFound);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::onUpdateDownloadProgress(qSlicerExtensionDownloadTask* task,
                                                             qint64 received,
                                                             qint64 total)
{
  Q_D(qSlicerExtensionsManagerModel);

  // Look up the update information
  const QString& extensionName = task->extensionName();
  const QHash<QString, UpdateDownloadInformation>::iterator iter = d->AvailableUpdates.find(extensionName);

  // Update internal progress tracking
  if (iter != d->AvailableUpdates.end())
  {
    iter->DownloadProgress = received;
    iter->DownloadSize = total;
  }

  // Notify observers of download progress
  emit this->updateDownloadProgress(extensionName, received, total);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::onUpdateDownloadFinished(qSlicerExtensionDownloadTask* task)
{
  Q_D(qSlicerExtensionsManagerModel);

  // Mark task for clean-up
  task->deleteLater();

  // Get network reply
  QNetworkReply* const reply = task->reply();
  QUrl downloadUrl = reply->url();
  Q_ASSERT(downloadUrl.path().contains("/download"));

  // Notify observers of event
  emit this->downloadFinished(reply);

  const QString& extensionName = task->extensionName();

  // Did the download succeed?
  if (reply->error())
  {
    d->critical(tr("Failed downloading %1 extension from %2").arg(extensionName).arg(downloadUrl.toString()));
    d->ActiveTasks.remove(task);
    emit activeTasksChanged();
    return;
  }

  // Look up the update information
  const QHash<QString, UpdateDownloadInformation>::iterator iter = d->AvailableUpdates.find(extensionName);

  if (iter != d->AvailableUpdates.end())
  {
    // Create directory for update archives (a persistent location is desired,
    // since we won't be installing the update Immediately)
    if (!QDir(this->extensionsInstallPath()).mkpath(".updates"))
    {
      d->critical(
        tr("Could not create .updates directory for update archive in %1").arg(this->extensionsInstallPath()));
      d->ActiveTasks.remove(task);
      emit activeTasksChanged();
      return;
    }

    // Save update metadata
    const QString& metadataPath = QString("%1/.updates/%2.s4ext").arg(this->extensionsInstallPath(), extensionName);
    Self::writeExtensionDescriptionFile(metadataPath, task->metadata());

    // Create update archive
    const QString& archiveName = task->archiveName();
    const QString& archivePath = QString("%1/.updates/%2").arg(this->extensionsInstallPath(), archiveName);

    QFile file(archivePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
      //: %1 is the path name, %2 is the error message
      d->critical(tr("Could not write file: '%1' (%2)").arg(archivePath).arg(file.errorString()));
      d->ActiveTasks.remove(task);
      emit activeTasksChanged();
      return;
    }

    file.write(reply->readAll());
    file.close();

    // Schedule install of update
    iter->ArchiveName = archivePath;
    this->scheduleExtensionForUpdate(extensionName);
  }
  d->ActiveTasks.remove(task);
  emit activeTasksChanged();
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::scheduleExtensionForUpdate(const QString& extensionName)
{
  Q_D(qSlicerExtensionsManagerModel);

  // If the user has installed some extension updates then we clear the new extension update
  // notification until the next check.
  QSettings extensionSettings(this->extensionsSettingsFilePath(), QSettings::IniFormat);
  extensionSettings.setValue("Extensions/UpdatesAvailable", false);
  emit extensionUpdatesAvailable(false);

  QString error;
  if (!d->checkExtensionSettingsPermissions(error))
  {
    d->critical(error);
    return false;
  }

  if (!this->isExtensionInstalled(extensionName))
  {
    // Cannot update unknown extension
    return false;
  }
  if (this->isExtensionScheduledForUninstall(extensionName))
  {
    // Cannot update if scheduled to be uninstalled
    return false;
  }

  // Get current mapping of scheduled updates
  QSettings settings(this->extensionsSettingsFilePath(), QSettings::IniFormat);
  QVariantMap scheduled = settings.value("Extensions/ScheduledForUpdate").toMap();

  if (scheduled.contains(extensionName))
  {
    // Already scheduled for update
    return true;
  }

  const UpdateDownloadInformation& updateInfo = d->AvailableUpdates.value(extensionName);
  if (updateInfo.ArchiveName.isEmpty())
  {
    if (updateInfo.ExtensionId.isEmpty())
    {
      //: %1 is extension name
      d->critical(tr("Missing download information for %1 extension").arg(extensionName));
      return false;
    }

    if (updateInfo.DownloadSize != 0)
    {
      // Already being downloaded
      return true;
    }

    qSlicerExtensionDownloadTask* const task = d->downloadExtensionByName(extensionName);
    if (!task)
    {
      //: %1 is extension name
      d->critical(tr("Failed to retrieve metadata for %1 extension").arg(extensionName));
      return false;
    }

    connect(task,
            SIGNAL(finished(qSlicerExtensionDownloadTask*)),
            this,
            SLOT(onUpdateDownloadFinished(qSlicerExtensionDownloadTask*)));
    connect(task,
            SIGNAL(progress(qSlicerExtensionDownloadTask*, qint64, qint64)),
            this,
            SLOT(onUpdateDownloadProgress(qSlicerExtensionDownloadTask*, qint64, qint64)));

    this->onUpdateDownloadProgress(task, 0, -1);
    return true;
  }

  // Add to scheduled updates
  scheduled[extensionName] = updateInfo.ArchiveName;
  settings.setValue("Extensions/ScheduledForUpdate", scheduled);

  //: %1 is extension name
  d->info(tr("%1 extension scheduled for update").arg(extensionName));
  emit this->extensionScheduledForUpdate(extensionName);

  return true;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::cancelExtensionScheduledForUpdate(const QString& extensionName)
{
  Q_D(qSlicerExtensionsManagerModel);

  QString error;
  if (!d->checkExtensionSettingsPermissions(error))
  {
    d->critical(error);
    return false;
  }
  if (!this->isExtensionScheduledForUpdate(extensionName))
  {
    return false;
  }
  d->removeExtensionFromScheduledForUpdateList(extensionName);

  emit this->extensionCancelledScheduleForUpdate(extensionName);

  return true;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModelPrivate::updateExtension(const QString& extensionName, const QString& archiveFile)
{
  Q_Q(qSlicerExtensionsManagerModel);

  QString error;
  if (!this->checkExtensionSettingsPermissions(error))
  {
    this->critical(error);
    return false;
  }

  QStandardItem* item = this->extensionItem(extensionName);
  if (!item)
  {
    //: %1 is extension name
    critical(qSlicerExtensionsManagerModel::tr("Failed to update %1 extension").arg(extensionName));
    return false;
  }

  if (!q->isExtensionScheduledForUpdate(extensionName))
  {
    //: %1 is extension name
    critical(qSlicerExtensionsManagerModel::tr("Failed to update %1 extension: it is not scheduled for update")
               .arg(extensionName));
    return false;
  }

  // Prepare to remove old version
  const QString& installPath = q->extensionInstallPath(extensionName);
  const QString& descriptionFile = q->extensionDescriptionFile(extensionName);
  bool success = true;

  // Remove old version
  if (QFile::exists(installPath))
  {
    success = ctk::removeDirRecursively(installPath);
  }
  if (QFile::exists(descriptionFile))
  {
    success = success && QFile::remove(descriptionFile);
  }
  success = success && this->Model.removeRow(item->row());

  // Read metadata for new version
  const QString& metadataPath = QString("%1/.updates/%2.s4ext").arg(q->extensionsInstallPath(), extensionName);
  const ExtensionMetadataType extensionMetadata =
    qSlicerExtensionsManagerModel::parseExtensionDescriptionFile(metadataPath);

  // Install new version
  success = success && q->installExtension(extensionName, extensionMetadata, archiveFile);

  if (success)
  {
    success = success && QFile::remove(archiveFile);
    success = success && QFile::remove(metadataPath);
    this->removeExtensionFromScheduledForUpdateList(extensionName);
  }

  emit q->extensionUpdated(extensionName);

  return success;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::scheduleExtensionForUninstall(const QString& extensionName)
{
  Q_D(qSlicerExtensionsManagerModel);

  QString error;
  if (!d->checkExtensionSettingsPermissions(error))
  {
    d->critical(error);
    return false;
  }

  if (!this->isExtensionInstalled(extensionName))
  {
    // Cannot uninstall unknown extension
    return false;
  }

  // Get current list of scheduled uninstalls
  QSettings settings(this->extensionsSettingsFilePath(), QSettings::IniFormat);
  QStringList scheduled = settings.value("Extensions/ScheduledForUninstall").toStringList();

  if (scheduled.contains(extensionName))
  {
    // Already scheduled for uninstall; nothing to do
    return true;
  }

  // Ensure extension not scheduled for update; cancel update if needed
  this->cancelExtensionScheduledForUpdate(extensionName);

  // Add to scheduled uninstalls
  scheduled.append(extensionName);
  settings.setValue("Extensions/ScheduledForUninstall", scheduled);

  d->removeExtensionSettings(extensionName);

  emit this->extensionScheduledForUninstall(extensionName);

  return true;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::cancelExtensionScheduledForUninstall(const QString& extensionName)
{
  Q_D(qSlicerExtensionsManagerModel);

  QString error;
  if (!d->checkExtensionSettingsPermissions(error))
  {
    d->critical(error);
    return false;
  }
  if (!this->isExtensionScheduledForUninstall(extensionName))
  {
    return false;
  }
  d->removeExtensionFromScheduledForUninstallList(extensionName);
  d->addExtensionSettings(extensionName);
  emit this->extensionCancelledScheduleForUninstall(extensionName);

  return true;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::uninstallExtension(const QString& extensionName)
{
  Q_D(qSlicerExtensionsManagerModel);

  QString error;
  if (!d->checkExtensionSettingsPermissions(error))
  {
    d->critical(error);
    return false;
  }

  QStandardItem* item = d->extensionItem(extensionName);
  if (!item)
  {
    //: %1 is extension name
    d->critical(tr("Failed to uninstall %1 extension").arg(extensionName));
    return false;
  }

  if (this->isExtensionLoaded(extensionName))
  {
    //: %1 is extension name
    d->critical(tr("Failed to uninstall %1 extension: extension is already loaded").arg(extensionName));
    return false;
  }

  bool bookmarked = this->isExtensionBookmarked(extensionName);

  bool success = true;
  if (QDir(this->extensionInstallPath(extensionName)).exists())
  {
    success = success && ctk::removeDirRecursively(this->extensionInstallPath(extensionName));
  }
  if (bookmarked)
  {
    // Keep the extension description around, just update in the extension description that is no longer installed
    QStandardItem* item = const_cast<QStandardItem*>(
      d->extensionItem(extensionName, qSlicerExtensionsManagerModelPrivate::InstalledColumn));
    if (item)
    {
      item->setData(false, qSlicerExtensionsManagerModelPrivate::InstalledRole);
      item->setData(false, Qt::DisplayRole);
    }
  }
  else
  {
    // Not installed or bookmarked - remove from managed extensions
    success = success && d->Model.removeRow(item->row());
  }
  success = success && d->removeExtensionDescriptionFile(extensionName);

  if (success)
  {
    d->removeExtensionSettings(extensionName);
    d->removeExtensionFromScheduledForUninstallList(extensionName);
  }

  emit this->extensionUninstalled(extensionName);

  return success;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModelPrivate::removeExtensionDescriptionFile(const QString& extensionName)
{
  Q_Q(qSlicerExtensionsManagerModel);
  bool success = true;

  // Remove s4ext file
  success = success && QFile::remove(q->extensionDescriptionFile(extensionName));

  // Remove icon file
  const QDir installDir(q->extensionsInstallPath());
  const QFileInfoList& iconEntries = installDir.entryInfoList(QStringList() << extensionName + "-icon.*");
  foreach (const QFileInfo& iconEntry, iconEntries)
  {
    success = success && QFile::remove(iconEntry.absoluteFilePath());
  }

  return success;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::updateScheduledExtensions()
{
  QStringList updatedExtensions;
  return this->updateScheduledExtensions(updatedExtensions);
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::updateScheduledExtensions(QStringList& updatedExtensions)
{
  Q_D(qSlicerExtensionsManagerModel);
  bool result = true;

  QSettings settings(this->extensionsSettingsFilePath(), QSettings::IniFormat);
  const QVariantMap scheduledUpdates = settings.value("Extensions/ScheduledForUpdate").toMap();

  const QVariantMap::const_iterator end = scheduledUpdates.end();
  for (QVariantMap::const_iterator iter = scheduledUpdates.begin(); iter != end; ++iter)
  {
    const QString& extensionName = iter.key();
    const bool success = d->updateExtension(extensionName, iter->toString());
    if (success)
    {
      updatedExtensions << extensionName;
    }
    else if (!d->AvailableUpdates.contains(extensionName))
    {
      // If update failed, add information to available updates so user can
      // cancel the update
      UpdateDownloadInformation updateInfo;
      updateInfo.ArchiveName = iter->toString();
      d->AvailableUpdates.insert(extensionName, updateInfo);
    }
    result = result && success;
  }
  return result;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::uninstallScheduledExtensions()
{
  QStringList uninstalledExtensions;
  return this->uninstallScheduledExtensions(uninstalledExtensions);
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::uninstallScheduledExtensions(QStringList& uninstalledExtensions)
{
  bool result = true;
  foreach (const QString& extensionName, this->scheduledForUninstallExtensions())
  {
    const bool success = this->uninstallExtension(extensionName);
    if (success)
    {
      uninstalledExtensions << extensionName;
    }
    result = result && success;
  }
  return result;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::updateModel()
{
  Q_D(qSlicerExtensionsManagerModel);

  QString extensionDescriptionPath = this->extensionsInstallPath();

  d->Model.clear();

  QSettings settings;
  QStringList bookmarkedExtensions = settings.value("Extensions/Bookmarked").toStringList();

  // First just gather extensions, we'll sort and group them before adding to the model
  QStringList installedExtensions;
  QMap<QString, ExtensionMetadataType> extensionsMetadataFromDescriptionFiles;
  foreach (const QFileInfo& fileInfo, d->extensionDescriptionFileInfos(extensionDescriptionPath))
  {
    ExtensionMetadataType metadata = Self::parseExtensionDescriptionFile(fileInfo.absoluteFilePath());
    QString extensionName = metadata["extensionname"].toString();
    bool loaded = d->LoadedExtensions.contains(extensionName);
    metadata["loaded"] = loaded;
    bool bookmarked = bookmarkedExtensions.contains(extensionName);
    metadata["bookmarked"] = bookmarked;
    bool installed = metadata["installed"].toBool();
    if (!installed && !bookmarked)
    {
      // not installed and not bookmarked (for example, because list of bookmarked extensions changed)
      // remove the extension description file now
      d->removeExtensionDescriptionFile(extensionName);
      continue;
    }
    extensionsMetadataFromDescriptionFiles[extensionName] = metadata;
    if (installed)
    {
      installedExtensions << extensionName;
    }
  }

  // Add installed extensions, sorted alphabetically
  QStringList managedExtensions;
  managedExtensions << bookmarkedExtensions;
  managedExtensions << installedExtensions;
  managedExtensions.removeDuplicates();
  managedExtensions.sort(Qt::CaseInsensitive);
  foreach (const QString& extensionName, managedExtensions)
  {
    if (extensionsMetadataFromDescriptionFiles.contains(extensionName))
    {
      // This is an installed or bookmarked extension with an extension description (s4ext) file.
      d->addExtensionModelRow(extensionsMetadataFromDescriptionFiles[extensionName]);
    }
    else
    {
      // This is a bookmarked extension without an extension description (s4ext) file.
      // Use metadata received from the server.
      ExtensionMetadataType metadata = d->ExtensionsMetadataFromServer.value(extensionName);
      metadata["extensionname"] = extensionName;
      metadata["loaded"] = false;
      metadata["installed"] = false;
      metadata["bookmarked"] = true;
      d->addExtensionModelRow(metadata);
    }
  }

  emit this->modelUpdated();
}

// --------------------------------------------------------------------------
CTK_GET_CPP(qSlicerExtensionsManagerModel, QString, extensionsSettingsFilePath, ExtensionsSettingsFilePath)

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::setExtensionsSettingsFilePath(const QString& extensionsSettingsFilePath)
{
  Q_D(qSlicerExtensionsManagerModel);
  if (d->ExtensionsSettingsFilePath == extensionsSettingsFilePath)
  {
    return;
  }
  d->ExtensionsSettingsFilePath = extensionsSettingsFilePath;
  emit this->extensionsSettingsFilePathChanged(extensionsSettingsFilePath);
}

// --------------------------------------------------------------------------
CTK_GET_CPP(qSlicerExtensionsManagerModel, QString, slicerRevision, SlicerRevision)

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::setSlicerRevision(const QString& revision)
{
  this->setSlicerRequirements(revision, this->slicerOs(), this->slicerArch());
}

// --------------------------------------------------------------------------
CTK_GET_CPP(qSlicerExtensionsManagerModel, QString, slicerOs, SlicerOs)

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::setSlicerOs(const QString& os)
{
  this->setSlicerRequirements(this->slicerRevision(), os, this->slicerArch());
}

// --------------------------------------------------------------------------
CTK_GET_CPP(qSlicerExtensionsManagerModel, QString, slicerArch, SlicerArch)

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::setSlicerArch(const QString& arch)
{
  this->setSlicerRequirements(this->slicerRevision(), this->slicerOs(), arch);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::setSlicerRequirements(const QString& revision,
                                                          const QString& os,
                                                          const QString& arch)
{
  Q_D(qSlicerExtensionsManagerModel);
  if (d->SlicerRevision == revision && d->SlicerOs == os && d->SlicerArch == arch)
  {
    return;
  }
  QString previousSlicerRevision = d->SlicerRevision;
  d->SlicerRevision = revision;

  QString previousSlicerOs = d->SlicerOs;
  d->SlicerOs = os;

  QString previousSlicerArch = d->SlicerArch;
  d->SlicerArch = arch;

  if (previousSlicerRevision != d->SlicerRevision)
  {
    emit this->slicerRevisionChanged(d->SlicerRevision);
  }
  if (previousSlicerOs != d->SlicerOs)
  {
    emit this->slicerOsChanged(d->SlicerOs);
  }
  if (previousSlicerArch != d->SlicerArch)
  {
    emit this->slicerArchChanged(d->SlicerArch);
  }
  emit this->slicerRequirementsChanged(d->SlicerRevision, d->SlicerOs, d->SlicerArch);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::identifyIncompatibleExtensions()
{
  Q_D(const qSlicerExtensionsManagerModel);
  foreach (const QString& extensionName, this->installedExtensions())
  {
    QStringList reasons = this->isExtensionCompatible(extensionName, d->SlicerRevision, d->SlicerOs, d->SlicerArch);
    if (!reasons.isEmpty())
    {
      //: %1 is extension name
      reasons.prepend(tr("Extension %1 is incompatible").arg(extensionName));
      qCritical() << reasons.join("\n  ");
      this->setExtensionEnabled(extensionName, false);
      emit this->extensionIdentifedAsIncompatible(extensionName);
    }
  }
}

// --------------------------------------------------------------------------
CTK_GET_CPP(qSlicerExtensionsManagerModel, QString, slicerVersion, SlicerVersion)

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::setSlicerVersion(const QString& slicerVersion)
{
  Q_D(qSlicerExtensionsManagerModel);
  if (d->SlicerVersion == slicerVersion)
  {
    return;
  }
  d->SlicerVersion = slicerVersion;
  emit this->slicerVersionChanged(slicerVersion);
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::isExtensionCompatible(const QString& extensionName,
                                                                 const QString& slicerRevision,
                                                                 const QString& slicerOs,
                                                                 const QString& slicerArch) const
{
  if (extensionName.isEmpty())
  {
    return QStringList() << tr("'extensionName' parameter is not specified");
  }
  ExtensionMetadataType metadata = this->extensionMetadata(extensionName);
  return qSlicerExtensionsManagerModelPrivate::isExtensionCompatible(metadata, slicerRevision, slicerOs, slicerArch);
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::isExtensionCompatible(const QString& extensionName) const
{
  return this->isExtensionCompatible(extensionName, this->slicerRevision(), this->slicerOs(), this->slicerArch());
}

// --------------------------------------------------------------------------
const QStandardItemModel* qSlicerExtensionsManagerModel::model() const
{
  Q_D(const qSlicerExtensionsManagerModel);
  return &d->Model;
}

// --------------------------------------------------------------------------
QHash<QString, QString> qSlicerExtensionsManagerModel::serverToExtensionDescriptionKey(int serverAPI)
{
  QHash<QString, QString> serverToExtensionDescriptionKey;

  //  | Model Columns Id     | Model Column Name | s4ext Key          | Girder_v1 key        |
  //  |----------------------|-------------------|--------------------|----------------------|
  //  | IdColumn             | extension_id      |                    | _id                  |
  //  | NameColumn           | extensionname     |                    | meta.baseName        |
  //  | ScmColumn            | scm               | scm                | meta.repository_type |
  //  | ScmUrlColumn         | scmurl            | scmurl             | meta.repository_url  |
  //  | SlicerRevisionColumn | slicer_revision   |                    | meta.app_revision    |
  //  | RevisionColumn       | revision          | scmrevision        | meta.revision        |
  //  | ReleaseColumn        | release           |                    |                      |
  //  | ArchColumn           | arch              |                    | meta.arch            |
  //  | OsColumn             | os                |                    | meta.os              |
  //  | DependsColumn        | depends           | depends            | meta.dependency      |
  //  | HomepageColumn       | homepage          | homepage           | meta.homepage        |
  //  | IconUrlColumn        | iconurl           | iconurl            | meta.icon_url        |
  //  | CategoryColumn       | category          | category           | meta.category        |
  //  | StatusColumn         | status            | status             |                      |
  //  | ContributorsColumn   | contributors      | contributors       | meta.contributors    |
  //  | DescriptionColumn    | description       | description        | meta.description     |
  //  | ScreenshotsColumn    | screenshots       | screenshoturls     | meta.screenshots     |
  //  | EnabledColumn        | enabled           | enabled            |                      |
  //  | ArchiveNameColumn    | archivename       |                    |                      |
  //  | MD5Column            | md5               |                    |                      |
  //  | UpdatedColumn        | updated           | updated            | updated              | package update date and
  //  time in ISO format | InstalledColumn      | installed         | installed          |                      | |
  //  BookmarkedColumn     | bookmarked        | bookmarked         |                      | |                      | |
  //  build_subdirectory |                      | |                      |                   |                    |
  //  created              | |                      |                   |                    | baseParentId         | |
  //  |                   |                    | baseParentType       | |                      |                   | |
  //  creatorId            | |                      |                   |                    | description          | |
  //  |                   |                    | folderId             | |                      |                   | |
  //  lowerName            | |                      |                   |                    | meta.app_id          | |
  //  |                   |                    | name                 | |                      |                   | |
  //  size                 |

  if (serverAPI == Self::Girder_v1)
  {
    serverToExtensionDescriptionKey.insert("_id", "extension_id");
    serverToExtensionDescriptionKey.insert("meta.baseName", "extensionname");
    serverToExtensionDescriptionKey.insert("meta.repository_type", "scm");
    serverToExtensionDescriptionKey.insert("meta.repository_url", "scmurl");
    serverToExtensionDescriptionKey.insert("meta.app_revision", "slicer_revision");
    serverToExtensionDescriptionKey.insert("meta.revision", "revision");
    // release
    serverToExtensionDescriptionKey.insert("meta.arch", "arch");
    serverToExtensionDescriptionKey.insert("meta.os", "os");
    serverToExtensionDescriptionKey.insert("meta.dependency", "depends");
    serverToExtensionDescriptionKey.insert("meta.homepage", "homepage");
    serverToExtensionDescriptionKey.insert("meta.icon_url", "iconurl");
    serverToExtensionDescriptionKey.insert("meta.category", "category");
    // status
    serverToExtensionDescriptionKey.insert("meta.contributors", "contributors");
    serverToExtensionDescriptionKey.insert("meta.description", "description");
    serverToExtensionDescriptionKey.insert("meta.screenshots", "screenshots");
    // enabled
    // archivename
    // md5
    serverToExtensionDescriptionKey.insert("updated", "updated");
  }
  else
  {
    qWarning() << Q_FUNC_INFO << " failed: missing implementation for serverAPI" << serverAPI;
  }
  return serverToExtensionDescriptionKey;
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel::ExtensionMetadataType qSlicerExtensionsManagerModel::convertExtensionMetadata(
  const ExtensionMetadataType& extensionMetadata,
  int serverAPI)
{
  ExtensionMetadataType updatedExtensionMetadata;
  QHash<QString, QString> serverToExtensionDescriptionKey = Self::serverToExtensionDescriptionKey(serverAPI);
  foreach (const QString& key, extensionMetadata.keys())
  {
    updatedExtensionMetadata.insert(serverToExtensionDescriptionKey.value(key, key), extensionMetadata.value(key));
  }
  return updatedExtensionMetadata;
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::serverKeysToIgnore(int serverAPI)
{
  if (serverAPI == Self::Girder_v1)
  {
    return QStringList() << "baseParentId"
                         << "baseParentType"
                         << "created"
                         << "creatorId"
                         << "description"
                         << "folderId"
                         << "lowerName"
                         << "meta.app_id"
                         << "name"
                         << "size";
  }
  else
  {
    qWarning() << Q_FUNC_INFO << " failed: missing implementation for serverAPI" << serverAPI;
    return QStringList();
  }
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel::ExtensionMetadataType qSlicerExtensionsManagerModel::filterExtensionMetadata(
  const ExtensionMetadataType& extensionMetadata,
  int serverAPI)
{
  QStringList extensionMetadataKeys = serverToExtensionDescriptionKey(serverAPI).values();

  ExtensionMetadataType filteredExtensionMetadata = extensionMetadata;
  foreach (const QString& key, Self::serverKeysToIgnore(serverAPI))
  {
    // Do not remove entry if the corresponding key may be returned
    // by "convertExtensionMetadata()".
    if (extensionMetadataKeys.contains(key))
    {
      continue;
    }
    filteredExtensionMetadata.remove(key);
  }
  return filteredExtensionMetadata;
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::readArrayValues(QSettings& settings,
                                                           const QString& arrayName,
                                                           const QString fieldName)
{
  Q_ASSERT(!arrayName.isEmpty());
  Q_ASSERT(!fieldName.isEmpty());
  QStringList listOfValues;
  int size = settings.beginReadArray(arrayName);
  for (int i = 0; i < size; ++i)
  {
    settings.setArrayIndex(i);
    listOfValues << settings.value(fieldName).toString();
  }
  settings.endArray();
  return listOfValues;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::writeArrayValues(QSettings& settings,
                                                     const QStringList& values,
                                                     const QString& arrayName,
                                                     const QString fieldName)
{
  Q_ASSERT(!arrayName.isEmpty());
  Q_ASSERT(!fieldName.isEmpty());
  settings.remove(arrayName);
  settings.beginWriteArray(arrayName);
  for (int i = 0; i < values.size(); ++i)
  {
    settings.setArrayIndex(i);
    settings.setValue(fieldName, values.at(i));
  }
  settings.endArray();
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::extractExtensionArchive(const QString& extensionName,
                                                            const QString& archiveFile,
                                                            const QString& destinationPath)
{
  Q_D(qSlicerExtensionsManagerModel);

  if (extensionName.isEmpty())
  {
    //: %1 is extension name
    d->critical(tr("Corrupted %1 extension package").arg(extensionName));
    return false;
  }

  QString error;
  if (!d->checkExtensionsInstallDestinationPath(destinationPath, error))
  {
    //: %1 is extension name, %2 is extension package file path, %3 is destination path
    d->critical(tr("Failed to extract %1 extension archive %2 into directory %3")
                  .arg(extensionName)
                  .arg(archiveFile)
                  .arg(destinationPath));
    d->critical(error);
    return false;
  }

  QDir extensionsDir(destinationPath);

  ctk::removeDirRecursively(extensionsDir.filePath(extensionName));

  // Make extension output directory
  extensionsDir.mkdir(extensionName);

  // Extract into <extensionsPath>/<extensionName>/<archiveBaseName>/
  extensionsDir.cd(extensionName);
  QString archiveBaseName = d->extractArchive(extensionsDir, archiveFile);
  if (archiveBaseName.isEmpty())
  {
    // extractArchive has logged the error
    return false;
  }
  extensionsDir.cdUp();

  // Rename <extensionName>/<archiveBaseName> into <extensionName>
  // => Such operation can't be done directly, we need intermediate steps ...

  QString dstPath(extensionsDir.absolutePath() + "/" + extensionName);
  QString srcPath(extensionsDir.absolutePath() + "/" + extensionName + "/" + archiveBaseName);
  QString intermediatePath(extensionsDir.absolutePath() + "/" + extensionName + "-XXXXXX");

  //  Step1:
  //  <extensionName>/<archiveBaseName>[/<Slicer_BUNDLE_LOCATION>/<Slicer_EXTENSIONS_DIRBASENAME>-<slicerRevision>/<extensionName>]
  //  -> <extensionName>-XXXXXX
  QString srcPathToCopy(srcPath);
  if (this->slicerOs() == Slicer_OS_MAC_NAME)
  {
    srcPathToCopy = srcPathToCopy + "/" Slicer_BUNDLE_LOCATION "/" Slicer_EXTENSIONS_DIRBASENAME "-"
                    + this->slicerRevision() + "/" + extensionName;
  }

  // Remove intermediate directory (might be created and left there if running out of disk space)
  // as it would make copyDirRecursively fail.
  ctk::removeDirRecursively(intermediatePath);

  if (!ctk::copyDirRecursively(srcPathToCopy, intermediatePath))
  {
    d->critical(tr("Failed to copy directory %1 into directory %2").arg(srcPathToCopy).arg(intermediatePath));
    return false;
  }

  //  Step2: <extensionName>-XXXXXX -> <extensionName>
  if (!ctk::copyDirRecursively(intermediatePath, dstPath))
  {
    d->critical(tr("Failed to copy directory %1 into directory %2").arg(intermediatePath).arg(dstPath));
    return false;
  }

  //  Step3: Remove <extensionName>-XXXXXX
  if (!ctk::removeDirRecursively(intermediatePath))
  {
    d->critical(tr("Failed to remove directory %1").arg(intermediatePath));
    return false;
  }

  //  Step4: Remove  <extensionName>/<archiveBaseName>
  if (!ctk::removeDirRecursively(srcPath))
  {
    d->critical(tr("Failed to remove directory %1").arg(srcPath));
    return false;
  }

  return true;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::writeExtensionDescriptionFile(const QString& file,
                                                                  const ExtensionMetadataType& metadata)
{
  QFile outputFile(file);
  if (!outputFile.open(QFile::WriteOnly | QFile::Truncate))
  {
    return false;
  }
  QTextStream outputStream(&outputFile);
  foreach (const QString& key, metadata.keys())
  {
    if (key == "extensionname" // name is defined by the s4ext filename
        || key == "bookmarked" // bookmarked extensions are stored in application settings
        || key == "loaded")    // this is just temporary state information
    {
      continue;
    }
    outputStream << QString("%1 %2\n").arg(key).arg(metadata.value(key).toString());
  }
  outputFile.close();
  return true;
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel::ExtensionMetadataType qSlicerExtensionsManagerModel::parseExtensionDescriptionFile(
  const QString& file)
{
  ExtensionMetadataType metadata;

  QFile inputFile(file);
  if (!inputFile.open(QFile::ReadOnly))
  {
    return metadata;
  }

  QTextStream inputStream(&inputFile);
  for (QString line(""); !line.isNull(); line = inputStream.readLine())
  {
    QChar commentCharacter('#');
    if (line.startsWith(commentCharacter) || line.isEmpty())
    {
      continue;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QStringList components = line.split(" ", Qt::SkipEmptyParts);
#else
    QStringList components = line.split(" ", QString::SkipEmptyParts);
#endif
    if (components.size() == 0)
    {
      continue;
    }
    QString key = components.takeFirst();
    QString value = components.join(" ");
    metadata.insert(key, value);
  }
  if (metadata.count() > 0)
  {
    metadata.insert("extensionname", QFileInfo(file).baseName());
    // In earlier installations
    if (!metadata.contains("installed"))
    {
      metadata["installed"] = "true";
    }
  }

  return metadata;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::exportExtensionList(QString& exportFilePath)
{
  Q_D(qSlicerExtensionsManagerModel);

  QString extensionDescriptionPath = this->extensionsInstallPath();

  if (!QDir(extensionDescriptionPath).exists())
  {
    return false;
  }

  QFile exportFile(exportFilePath);
  if (!exportFile.open(QFile::WriteOnly))
  {
    qWarning() << "Failed to create export file" << exportFilePath;
    return false;
  }
  QTextStream exportStream(&exportFile);

  foreach (const QFileInfo& fileInfo, d->extensionDescriptionFileInfos(extensionDescriptionPath))
  {
    QFile inputFile(fileInfo.absoluteFilePath());
    if (!inputFile.open(QFile::ReadOnly))
    {
      qWarning() << "Failed to open" << fileInfo.absoluteFilePath();
      break;
    }

    QTextStream inputStream(&inputFile);
    QString description = inputStream.readAll();
    exportStream << "\n";
    exportStream << "#####################################\n";
    exportStream << "# File: " << fileInfo.fileName() << "\n";
    exportStream << "#####################################\n";
    exportStream << description;
  }

  return true;
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::checkInstallPrerequisites() const
{
  Q_D(const qSlicerExtensionsManagerModel);
  QStringList errors;
  QString error;
  if (!d->checkExtensionSettingsPermissions(error))
  {
    d->critical(error);
    errors << error;
  }
  if (!d->checkExtensionsInstallDestinationPath(this->extensionsInstallPath(), error))
  {
    d->critical(error);
    errors << error;
  }
  return errors;
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::activeTasks() const
{
  Q_D(const qSlicerExtensionsManagerModel);
  return d->ActiveTasks.values();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::aboutToLoadExtensions()
{
  Q_D(qSlicerExtensionsManagerModel);

  // Extensions that are currently installed are about to be loaded, so set
  // "loaded" flag based on current "installed" flag.
  // We may install extensions later, but they will not be loaded immediately
  // (only on the next application restart).
  d->LoadedExtensions.clear();
  for (int rowIdx = 0; rowIdx < d->Model.rowCount(); ++rowIdx)
  {
    QStandardItem* installedItem = d->Model.item(rowIdx, qSlicerExtensionsManagerModelPrivate::InstalledColumn);
    bool installed = installedItem->data(qSlicerExtensionsManagerModelPrivate::InstalledRole).toBool();
    QStandardItem* loadedItem = d->Model.item(rowIdx, qSlicerExtensionsManagerModelPrivate::LoadedColumn);
    loadedItem->setData(installed, qSlicerExtensionsManagerModelPrivate::LoadedRole);
    loadedItem->setData(installed, Qt::DisplayRole);
    // Save the loaded extension into a list in case we later do a full model update.
    if (installed)
    {
      d->LoadedExtensions << d->Model.item(rowIdx, qSlicerExtensionsManagerModelPrivate::NameColumn)->text();
    }
  }
}

// --------------------------------------------------------------------------
QUrl qSlicerExtensionsManagerModel::extensionsListUrl() const
{
  Q_D(const qSlicerExtensionsManagerModel);

  QUrl url = this->frontendServerUrl();
  int serverAPI = this->serverAPI();

  if (serverAPI == qSlicerExtensionsManagerModel::Girder_v1)
  {
    url.setPath(url.path() + QString("/catalog/All/%1/%2").arg(d->SlicerRevision).arg(d->SlicerOs));
  }
  else
  {
    qWarning() << Q_FUNC_INFO << " failed: missing implementation for serverAPI" << serverAPI;
  }

  return url;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::waitForAllTasksCompletion(int timeoutMsec /*=-1*/) const
{
  Q_D(const qSlicerExtensionsManagerModel);

  QElapsedTimer timer;

  // busy wait loop - it should be fine, as it should not take long and
  // and only rarely used (e.g., when a module installs some required extensions)
  while (true)
  {
    if (d->ActiveTasks.empty())
    {
      return true;
    }
    if (timeoutMsec >= 0 && timer.elapsed() > timeoutMsec)
    {
      return false;
    }
    // Wait a bit to reduce CPU burden
    if (QApplication::instance())
    {
      QApplication::instance()->processEvents(QEventLoop::ExcludeUserInputEvents);
      QThread::msleep(10);
    }
  }
}
