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
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>
#include <QStandardItemModel>
#include <QTemporaryFile>
#include <QTextStream>
#include <QUrl>
#include <QUrlQuery>

// CTK includes
#include <ctkScopedCurrentDir.h>
#include <ctkPimpl.h>
#include <ctkUtils.h>

// qRestAPI includes
#include <qMidasAPI.h>

// QtCore includes
#include "qSlicerExtensionsManagerModel.h"
#include "vtkSlicerConfigure.h"
#include "vtkSlicerVersionConfigure.h"

// MRML includes
#include "vtkArchive.h"


// --------------------------------------------------------------------------
namespace
{

// --------------------------------------------------------------------------
struct UpdateCheckInformation
{
  QString ExtensionName;
  QString InstalledVersion;
  bool InstallAutomatically;
};

// --------------------------------------------------------------------------
struct UpdateDownloadInformation
{
  UpdateDownloadInformation(const QString& extensionId = QString())
    : ExtensionId(extensionId) {}

  QString ExtensionId;
  QString ArchiveName;
  qint64 DownloadSize{0};
  qint64 DownloadProgress{0};
};

// --------------------------------------------------------------------------
class QStandardItemModelWithRole : public QStandardItemModel
{
public:
  QHash<int, QByteArray> roleNames() const override
  {
    return this->CustomRoleNames;
  }
  QHash<int,QByteArray> CustomRoleNames;
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
    MD5Column
    };

  enum ItemDataRole{
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
    MD5Role
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

  QFileInfoList extensionDescriptionFileInfos(const QString& extensionDescriptionPath)const;

  void addExtensionModelRow(const ExtensionMetadataType &metadata);
  QStandardItem* extensionItem(const QString& extensionName, int column = Self::NameColumn)const;

  QStringList columnNames()const;

  void addExtensionPathToApplicationSettings(const QString& extensionName);
  void removeExtensionPathFromApplicationSettings(const QString& extensionName);

  void addExtensionPathToLauncherSettings(const QString& extensionName);
  void removeExtensionPathFromLauncherSettings(const QString& extensionName);

  bool checkExtensionsInstallDestinationPath(const QString &destinationPath, QString& error)const;

  bool checkExtensionSettingsPermissions(QString &error)const;
  void addExtensionSettings(const QString& extensionName);
  void removeExtensionSettings(const QString& extensionName);

  void removeExtensionFromScheduledForUpdateList(const QString& extensionName);
  void removeExtensionFromScheduledForUninstallList(const QString& extensionName);

  QString extractArchive(const QDir& extensionsDir, const QString &archiveFile);

  qSlicerExtensionDownloadTask* downloadExtension(const QString& extensionId);

  /// Update (reinstall) specified extension.
  ///
  /// This updates the specified extension
  ///
  /// \param extensionName Name of the extension.
  /// \param
  /// \sa downloadExtension, installExtension
  bool updateExtension(const QString& extensionName, const QString& archiveFile);

  /// \brief Uninstall \a extensionName
  /// \note The directory containing the extension will be deleted.
  /// \sa downloadExtension, installExtension
  bool uninstallExtension(const QString& extensionName);

  QStringList extensionLibraryPaths(const QString& extensionName)const;
  QStringList extensionQtPluginPaths(const QString& extensionName)const;
  QStringList extensionPaths(const QString& extensionName)const;

#ifdef Slicer_USE_PYTHONQT
  QStringList extensionPythonPaths(const QString& extensionName)const;
#endif
  static bool validateExtensionMetadata(const ExtensionMetadataType &extensionMetadata);

  static QStringList isExtensionCompatible(
      const ExtensionMetadataType& metadata, const QString& slicerRevision,
      const QString& slicerOs, const QString& slicerArch);

  void saveExtensionDescription(const QString& extensionDescriptionFile, const ExtensionMetadataType &allExtensionMetadata);
  void saveExtensionToHistorySettings(const QString& extensionsHistorySettingsFile, const ExtensionMetadataType &extensionMetadata);
  void scheduleExtensionHistorySettingRemoval(const QString& extensionsHistorySettingsFile, const ExtensionMetadataType &extensionMetadata);
  void addExtensionHistorySetting(const QString& extensionsHistorySettingsFile, const ExtensionMetadataType &extensionMetadata, const QString& settingsPath);
  void cancelExtensionHistorySettingRemoval(const QString& extensionsHistorySettingsFile, const QString& extensionName);
  void removeScheduledExtensionHistorySettings(const QString& extensionsHistorySettingsFile);
  QVariantMap getExtensionsInfoFromPreviousInstallations(const QString& extensionsHistorySettingsFile);
  void gatherExtensionsHistoryInformationOnStartup();


  qSlicerExtensionsManagerModel::ExtensionMetadataType retrieveExtensionMetadata(
    const qMidasAPI::ParametersType& parameters);

  void initializeColumnIdToNameMap(int columnIdx, const char* columnName);
  QHash<int, QString> ColumnIdToName;
  QStringList ColumnNames;

  bool NewExtensionEnabledByDefault;

  QNetworkAccessManager NetworkManager;
  qMidasAPI CheckForUpdatesApi;
  QHash<QUuid, UpdateCheckInformation> CheckForUpdatesRequests;

  QHash<QString, UpdateDownloadInformation> AvailableUpdates;

  QString ExtensionsSettingsFilePath;
  QString ExtensionsHistorySettingsFilePath;

  QString SlicerRevision;
  QString SlicerOs;
  QString SlicerArch;

  QString SlicerVersion;

  QStandardItemModelWithRole Model;
};

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModelPrivate::qSlicerExtensionsManagerModelPrivate(qSlicerExtensionsManagerModel& object)
  : q_ptr(&object)
  , NewExtensionEnabledByDefault(true)
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

  // See http://www.developer.nokia.com/Community/Wiki/Using_QStandardItemModel_in_QML
  QHash<int, QByteArray> roleNames;
  int columnIdx = 0;
  foreach(const QString& columnName, this->columnNames())
    {
    roleNames[Qt::UserRole + 1 + columnIdx] = columnName.toUtf8();
    ++columnIdx;
    }

  this->Model.CustomRoleNames = roleNames;

  QObject::connect(q, SIGNAL(slicerRequirementsChanged(QString,QString,QString)),
                   q, SLOT(identifyIncompatibleExtensions()));

  QObject::connect(q, SIGNAL(modelUpdated()),
                   q, SLOT(identifyIncompatibleExtensions()));

  QObject::connect(&this->CheckForUpdatesApi,
                   SIGNAL(resultReceived(QUuid,QList<QVariantMap>)),
                   q, SLOT(onUpdateCheckComplete(QUuid,QList<QVariantMap>)));

  QObject::connect(&this->CheckForUpdatesApi,
                   SIGNAL(errorReceived(QUuid,QString)),
                   q, SLOT(onUpdateCheckFailed(QUuid)));
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
  if(level == ctkErrorLogLevel::Fatal)
    {
    qFatal("%s", qPrintable(text));
    }
  else if (level == ctkErrorLogLevel::Critical)
    {
    qCritical() << text;
    }
  else if (level == ctkErrorLogLevel::Warning)
    {
    qWarning() << text;
    }
  else
    {
    qDebug() << text;
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
QFileInfoList qSlicerExtensionsManagerModelPrivate::extensionDescriptionFileInfos(const QString& extensionDescriptionPath)const
{
  QDir extensionDescriptionDir(extensionDescriptionPath);
  QFileInfoList fileInfos = extensionDescriptionDir.entryInfoList(QStringList() << "*.s4ext",
                                                                  QDir::Files | QDir::Readable,
                                                                  QDir::Name | QDir::IgnoreCase);
  return fileInfos;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::addExtensionModelRow(const ExtensionMetadataType &metadata)
{
  QList<QStandardItem*> itemList;

  foreach(const QString& key, this->columnNames())
    {
    QString value = metadata.value(key).toString();
    QStandardItem * item = new QStandardItem(value);
    item->setEditable(false);
    item->setData(value, this->role(key.toUtf8()));
    itemList << item;
    }

  this->Model.invisibleRootItem()->appendRow(itemList);
}

// --------------------------------------------------------------------------
QStandardItem* qSlicerExtensionsManagerModelPrivate::extensionItem(const QString& extensionName, int column)const
{
  QList<QStandardItem*> foundItems =
      this->Model.findItems(extensionName, Qt::MatchExactly, Self::NameColumn);
  if (foundItems.count() != 1)
    {
    return nullptr;
    }
  return this->Model.item(foundItems.at(0)->row(), column);
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModelPrivate::columnNames()const
{
  return this->ColumnNames;
}

// --------------------------------------------------------------------------
namespace
{

// --------------------------------------------------------------------------
bool hasPath(const QStringList& paths, const QString& pathToCheck)
{
  QDir dirToCheck(pathToCheck);
  foreach(const QString& path, paths)
    {
    if (dirToCheck == QDir(path))
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
  foreach(const QString& pathToAppend, pathsToAppend)
    {
    if (!hasPath(paths, pathToAppend) && shouldExist ? QDir(pathToAppend).exists() : true)
      {
      updatedPaths << pathToAppend;
      }
    }
  return updatedPaths;
}

// --------------------------------------------------------------------------
QStringList removeFromPathList(QStringList& paths, const QString& pathToRemove)
{
  QDir extensionDir(pathToRemove);
  foreach(const QString& path, paths)
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
  foreach(const QString& pathToRemove, pathsToRemove)
    {
    removeFromPathList(updatedPaths, pathToRemove);
    }
  return updatedPaths;
}

} // end of anonymous namespace

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::addExtensionPathToApplicationSettings(const QString& extensionName)
{
  Q_Q(qSlicerExtensionsManagerModel);
  QSettings settings(q->extensionsSettingsFilePath(), QSettings::IniFormat);
  QStringList additionalPaths = settings.value("Modules/AdditionalPaths").toStringList();
  settings.setValue("Modules/AdditionalPaths",
                    appendToPathList(additionalPaths, q->extensionModulePaths(extensionName)));
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::removeExtensionPathFromApplicationSettings(const QString& extensionName)
{
  Q_Q(qSlicerExtensionsManagerModel);
  QSettings settings(q->extensionsSettingsFilePath(), QSettings::IniFormat);
  QStringList additionalPaths = settings.value("Modules/AdditionalPaths").toStringList();
  settings.setValue("Modules/AdditionalPaths",
                    removeFromPathList(additionalPaths, q->extensionModulePaths(extensionName)));
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
    this->warning(qSlicerExtensionsManagerModel::tr("Failed to open extensions settings file %1").arg(this->ExtensionsSettingsFilePath));
    return;
    }

  QStringList libraryPath = qSlicerExtensionsManagerModel::readArrayValues(settings, "LibraryPaths", "path");
  qSlicerExtensionsManagerModel::writeArrayValues(settings,
                         appendToPathList(libraryPath, this->extensionLibraryPaths(extensionName)),
                         "LibraryPaths", "path");

  QStringList paths = qSlicerExtensionsManagerModel::readArrayValues(settings, "Paths", "path");
  qSlicerExtensionsManagerModel::writeArrayValues(settings,
                         appendToPathList(paths, this->extensionPaths(extensionName)),
                         "Paths", "path");

#ifdef Slicer_USE_PYTHONQT
  QStringList pythonPaths = qSlicerExtensionsManagerModel::readArrayValues(settings, "PYTHONPATH", "path");
  qSlicerExtensionsManagerModel::writeArrayValues(settings,
                         appendToPathList(pythonPaths, this->extensionPythonPaths(extensionName)),
                         "PYTHONPATH", "path");
#endif

  QStringList qtPluginPaths = qSlicerExtensionsManagerModel::readArrayValues(settings, "QT_PLUGIN_PATH", "path");
  qSlicerExtensionsManagerModel::writeArrayValues(settings,
                         appendToPathList(qtPluginPaths, this->extensionQtPluginPaths(extensionName)),
                         "QT_PLUGIN_PATH", "path");
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
    this->warning(qSlicerExtensionsManagerModel::tr("Failed to open extensions settings file: %1").arg(this->ExtensionsSettingsFilePath));
    return;
    }

  QStringList libraryPath = qSlicerExtensionsManagerModel::readArrayValues(settings, "LibraryPaths", "path");
  qSlicerExtensionsManagerModel::writeArrayValues(settings,
                         removeFromPathList(libraryPath, this->extensionLibraryPaths(extensionName)),
                         "LibraryPaths", "path");

  QStringList paths = qSlicerExtensionsManagerModel::readArrayValues(settings, "Paths", "path");
  qSlicerExtensionsManagerModel::writeArrayValues(settings,
                         removeFromPathList(paths, this->extensionPaths(extensionName)),
                         "Paths", "path");

#ifdef Slicer_USE_PYTHONQT
  QStringList pythonPaths = qSlicerExtensionsManagerModel::readArrayValues(settings, "PYTHONPATH", "path");
  qSlicerExtensionsManagerModel::writeArrayValues(settings,
                         removeFromPathList(pythonPaths, this->extensionPythonPaths(extensionName)),
                         "PYTHONPATH", "path");
#endif

  QStringList qtPluginPaths = qSlicerExtensionsManagerModel::readArrayValues(settings, "QT_PLUGIN_PATH", "path");
  qSlicerExtensionsManagerModel::writeArrayValues(settings,
                         removeFromPathList(qtPluginPaths, this->extensionQtPluginPaths(extensionName)),
                         "QT_PLUGIN_PATH", "path");
}

#ifdef Q_OS_WIN
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModelPrivate::checkExtensionsInstallDestinationPath(
    const QString& destinationPath, QString& error)const
{
  if (!QDir(destinationPath).exists())
    {
    error = qSlicerExtensionsManagerModel::tr("Extensions install directory does NOT exist: <strong>%1</strong>").arg(destinationPath);
    return false;
    }

  QFileInfo destinationPathInfo(destinationPath);
#ifdef Q_OS_WIN
  struct QtNtfsPermissionLookupHelper
  {
    QtNtfsPermissionLookupHelper(){++qt_ntfs_permission_lookup;}
    ~QtNtfsPermissionLookupHelper(){--qt_ntfs_permission_lookup;}
  };
  QtNtfsPermissionLookupHelper qt_ntfs_permission_lookup_helper;
#endif
  if (!destinationPathInfo.isReadable()
      || !destinationPathInfo.isWritable()
      || !destinationPathInfo.isExecutable())
    {
    error = qSlicerExtensionsManagerModel::tr("Extensions install directory is expected to be "
                        "readable/writable/executable: <strong>%1</strong>").arg(destinationPath);
    return false;
    }

  return true;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModelPrivate::checkExtensionSettingsPermissions(QString& error)const
{
  Q_Q(const qSlicerExtensionsManagerModel);

  QFileInfo settingsFileInfo = QFileInfo(q->extensionsSettingsFilePath());
  if (settingsFileInfo.exists())
    {
    if (!settingsFileInfo.isReadable() || !settingsFileInfo.isWritable())
      {
      error = QString("Extensions settings file is expected to be readable/writable: <strong>%1</strong>").
          arg(q->extensionsSettingsFilePath());
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
void qSlicerExtensionsManagerModelPrivate
::removeExtensionFromScheduledForUpdateList(const QString& extensionName)
{
  Q_Q(qSlicerExtensionsManagerModel);

  QSettings settings(q->extensionsSettingsFilePath(), QSettings::IniFormat);
  QVariantMap scheduled =
    settings.value("Extensions/ScheduledForUpdate").toMap();

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
  bool success = vtkArchive::ExtractTar(qPrintable(archiveFile), /* verbose */ false, /* extract */ true, &extracted_files);
  if(!success)
    {
    this->critical(QString("Failed to extract %1 into %2").arg(archiveFile).arg(extensionsDir.absolutePath()));
    return QString();
    }
  if(extracted_files.size() == 0)
    {
    this->warning(QString("Archive %1 doesn't contain any files !").arg(archiveFile));
    return QString();
    }

  // Compute <topLevelArchiveDir>. We assume all files are extracted in top-level folder.
  QDir extractDirOfFirstFile = QFileInfo(extensionsDir, QString::fromStdString(extracted_files.at(0))).dir();
  QDir topLevelArchiveDir;
  while(extractDirOfFirstFile != extensionsDir)
    {
    topLevelArchiveDir = extractDirOfFirstFile;
    extractDirOfFirstFile.cdUp();
    }

  return topLevelArchiveDir.dirName();
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModelPrivate::extensionLibraryPaths(const QString& extensionName)const
{
  Q_Q(const qSlicerExtensionsManagerModel);
  if (this->SlicerVersion.isEmpty())
    {
    return QStringList();
    }
  QString path = q->extensionInstallPath(extensionName);
  return appendToPathList(QStringList(), QStringList()
                   << path + "/" + QString(Slicer_BIN_DIR).replace(Slicer_VERSION, this->SlicerVersion)
                   << path + "/" + QString(Slicer_LIB_DIR).replace(Slicer_VERSION, this->SlicerVersion)
                   << path + "/" + QString(Slicer_CLIMODULES_LIB_DIR).replace(Slicer_VERSION, this->SlicerVersion)
                   << path + "/" + QString(Slicer_QTLOADABLEMODULES_LIB_DIR).replace(Slicer_VERSION, this->SlicerVersion)
                   << path + "/" + QString(Slicer_THIRDPARTY_LIB_DIR)
                   );
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModelPrivate::extensionQtPluginPaths(const QString& extensionName)const
{
  Q_Q(const qSlicerExtensionsManagerModel);
  if (this->SlicerVersion.isEmpty())
  {
    return QStringList();
  }
  QString path = q->extensionInstallPath(extensionName);
  return appendToPathList(QStringList(), QStringList()
                   << path + "/" + QString(Slicer_QtPlugins_DIR).replace(Slicer_VERSION, this->SlicerVersion)
  );
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModelPrivate::extensionPaths(const QString& extensionName)const
{
  Q_Q(const qSlicerExtensionsManagerModel);
  if (this->SlicerVersion.isEmpty())
    {
    return QStringList();
    }
  QString path = q->extensionInstallPath(extensionName);
  return appendToPathList(QStringList(), QStringList()
                   << path + "/" + QString(Slicer_CLIMODULES_BIN_DIR).replace(Slicer_VERSION, this->SlicerVersion)
                   << path + "/" + QString(Slicer_THIRDPARTY_BIN_DIR)
                   );
}

#ifdef Slicer_USE_PYTHONQT
// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModelPrivate::extensionPythonPaths(const QString& extensionName)const
{
  Q_Q(const qSlicerExtensionsManagerModel);
  if (this->SlicerVersion.isEmpty())
    {
    return QStringList();
    }
  QString path = q->extensionInstallPath(extensionName);
  return appendToPathList(QStringList(), QStringList()
                          << path + "/" + QString(Slicer_QTSCRIPTEDMODULES_LIB_DIR).replace(Slicer_VERSION, this->SlicerVersion)
                          << path + "/" + QString(Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR).replace(Slicer_VERSION, this->SlicerVersion)
                          << path + "/" + QString(PYTHON_SITE_PACKAGES_SUBDIR)
                          );
}
#endif

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModelPrivate::validateExtensionMetadata(
    const ExtensionMetadataType &extensionMetadata)
{
  bool valid = true;
  QStringList expectedNonEmptyKeys;
  foreach(const QString& key, QStringList()
          << "productname"  << "name" << "item_id"
          )
    {
    valid = valid && !extensionMetadata.value(key).toString().isEmpty();
    }
  return valid;
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModelPrivate::isExtensionCompatible(
    const ExtensionMetadataType& metadata, const QString& slicerRevision,
    const QString& slicerOs, const QString& slicerArch)
{
  if (slicerRevision.isEmpty())
    {
    return QStringList() << qSlicerExtensionsManagerModel::tr("slicerRevision is not specified");
    }
  if (slicerOs.isEmpty())
    {
    return QStringList() << qSlicerExtensionsManagerModel::tr("slicerOs is not specified");
    }
  if (slicerArch.isEmpty())
    {
    return QStringList() << qSlicerExtensionsManagerModel::tr("slicerArch is not specified");
    }
  QStringList reasons;
  QString extensionSlicerRevision = metadata.value("slicer_revision").toString();
  if (slicerRevision != extensionSlicerRevision)
    {
    reasons << qSlicerExtensionsManagerModel::tr("extensionSlicerRevision [%1] is different from slicerRevision [%2]").arg(extensionSlicerRevision).arg(slicerRevision);
    }
  QString extensionArch = metadata.value("arch").toString();
  if (slicerArch != extensionArch)
    {
    reasons << qSlicerExtensionsManagerModel::tr("extensionArch [%1] is different from slicerArch [%2]").arg(extensionArch).arg(slicerArch);
    }
  QString extensionOs = metadata.value("os").toString();
  if (slicerOs != extensionOs)
    {
    reasons << qSlicerExtensionsManagerModel::tr("extensionOs [%1] is different from slicerOs [%2]").arg(extensionOs).arg(slicerOs);
    }
  return reasons;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::saveExtensionDescription(
    const QString& extensionDescriptionFile, const ExtensionMetadataType& allExtensionMetadata)
{
//  QStringList expectedKeys;
//  expectedKeys << "extensionname" << "scm" << "scmurl" << "arch" << "os"
//               << "depends" << "homepage" << "category"
//               << "description" << "enabled"
//               << "revision" << "slicer_revision";

//  // TODO Make sure expected keys are provided
//  // TODO Tuple in metadata should be ordered as the expectedKeys. May be could use QList<QPair<> >
//  ExtensionMetadataType updatedMetadata;
//  foreach(const QString& key, expectedKeys)
//    {
//    updatedMetadata.insert(key, allExtensionMetadata[key]);
//    }

  qSlicerExtensionsManagerModel::writeExtensionDescriptionFile(extensionDescriptionFile, allExtensionMetadata);
}


// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::saveExtensionToHistorySettings(
  const QString& extensionsHistorySettingsFile, const ExtensionMetadataType &extensionMetadata)
{
  this->addExtensionHistorySetting(extensionsHistorySettingsFile, extensionMetadata, "ExtensionsHistory/Revisions/" + this->SlicerRevision);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::scheduleExtensionHistorySettingRemoval(
  const QString& extensionsHistorySettingsFile, const ExtensionMetadataType &extensionMetadata)
{
  this->addExtensionHistorySetting(extensionsHistorySettingsFile, extensionMetadata, "ExtensionsHistory/ScheduledForRemoval");
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::addExtensionHistorySetting(
  const QString& extensionsHistorySettingsFile, const ExtensionMetadataType &extensionMetadata, const QString& settingsPath)
{
  QSettings settings(extensionsHistorySettingsFile, QSettings::IniFormat);
  QStringList settingsInfoList = settings.value(settingsPath).toStringList();
  settingsInfoList << extensionMetadata.value("extensionname").toString();
  settingsInfoList.removeDuplicates();
  settings.setValue(settingsPath, settingsInfoList);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::cancelExtensionHistorySettingRemoval(
  const QString& extensionsHistorySettingsFile, const QString& extensionName)
{
  QSettings settings(extensionsHistorySettingsFile, QSettings::IniFormat);
  QStringList settingsInfoList = settings.value("ExtensionsHistory/ScheduledForRemoval").toStringList();
  settingsInfoList.removeOne(extensionName);
  settingsInfoList.removeDuplicates();
  settings.setValue("ExtensionsHistory/ScheduledForRemoval", settingsInfoList);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::removeScheduledExtensionHistorySettings(
  const QString& extensionsHistorySettingsFile)
{
  QSettings settings(extensionsHistorySettingsFile, QSettings::IniFormat);
  QStringList scheduledForRemovalList = settings.value("ExtensionsHistory/ScheduledForRemoval").toStringList();
  QStringList historyList = settings.value("ExtensionsHistory/Revisions/" + this->SlicerRevision).toStringList();
  for (int i = 0; i < scheduledForRemovalList.length(); i++)
    {
    historyList.removeOne(scheduledForRemovalList.at(i));
    }
  historyList.removeDuplicates();
  settings.setValue("ExtensionsHistory/Revisions/" + this->SlicerRevision, historyList);
  settings.setValue("ExtensionsHistory/ScheduledForRemoval", "");
}

// --------------------------------------------------------------------------
QVariantMap qSlicerExtensionsManagerModelPrivate::getExtensionsInfoFromPreviousInstallations(
  const QString& extensionsHistorySettingsFile)
{
  Q_Q(qSlicerExtensionsManagerModel);
  QVariantMap extensionsHistoryInformation;
  QSettings settings(extensionsHistorySettingsFile, QSettings::IniFormat);
  settings.beginGroup("ExtensionsHistory/Revisions");
  QStringList revisions = settings.childKeys();
  revisions.sort(); //revisions sorted ascending
  int lastRevision = -1;
  for (int i = revisions.length() - 1; i >= 0; i--) //the revision with the highest number not equal the current one is considered to be the last revision
    {
    if (revisions[i] != this->SlicerRevision)
      {
      lastRevision = i;
      break;
      }
    }
  if (lastRevision == -1)
    {
    return extensionsHistoryInformation;
    }

  for (int i = 0; i < revisions.length(); i++)
    {
    QVariantMap curExtensionInfo;
    const QStringList& extensionNames = settings.value(revisions.at(i)).toStringList();
    for (int j = 0; j < extensionNames.length(); j++)
      {
      QString extensionName = extensionNames.at(j);
      QString extensionId = "";

      curExtensionInfo.insert("UsedLastInRevision", revisions.at(i));
      if (i == lastRevision || (extensionsHistoryInformation.contains(extensionName) &&
        extensionsHistoryInformation.value(extensionName).toMap().value("WasInstalledInLastRevision").toBool()))
        {
        curExtensionInfo.insert("WasInstalledInLastRevision", true);
        }
      else
        {
        curExtensionInfo.insert("WasInstalledInLastRevision", false);
        }
      curExtensionInfo.insert("IsInstalled", q->isExtensionInstalled(extensionName));
      bool isCompatible = true;

      if (!q->isExtensionInstalled(extensionName))
        {
        qMidasAPI::ParametersType parameters;
        parameters["productname"] = extensionName;
        parameters["slicer_revision"] = q->slicerRevision();
        parameters["os"] = q->slicerOs();
        parameters["arch"] = q->slicerArch();
        const ExtensionMetadataType& metaData = retrieveExtensionMetadata(parameters);
        extensionId = metaData.value("extension_id").toString();     //retrieve updated extension id for not installed extensions
        isCompatible = (this->isExtensionCompatible(metaData, this->SlicerRevision, this->SlicerOs, this->SlicerArch).length() == 0);
        }
      else
        {
        const ExtensionMetadataType& metaData = q->extensionMetadata(extensionName);
        extensionId = metaData.value("extension_id").toString();
        isCompatible = (q->isExtensionCompatible(extensionName).length() == 0);
        }
      curExtensionInfo.insert("ExtensionId", extensionId);
      curExtensionInfo.insert("IsCompatible", isCompatible);
      extensionsHistoryInformation.insert(extensionName, curExtensionInfo);
      }
    }
  return extensionsHistoryInformation;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::gatherExtensionsHistoryInformationOnStartup()
{
  Q_Q(qSlicerExtensionsManagerModel);
  emit q->extensionHistoryGatheredOnStartup(q->getExtensionHistoryInformation());
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::initializeColumnIdToNameMap(int columnIdx, const char* columnName)
{
  this->ColumnIdToName[columnIdx] = columnName;
  this->ColumnNames << columnName;
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel::ExtensionMetadataType qSlicerExtensionsManagerModelPrivate
::retrieveExtensionMetadata(const qMidasAPI::ParametersType& parameters)
{
  Q_Q(const qSlicerExtensionsManagerModel);

  bool ok = false;
  QList<QVariantMap> results = qMidasAPI::synchronousQuery(
        ok, q->serverUrl().toString(),
        "midas.slicerpackages.extension.list", parameters);
  if (!ok || results.count() != 1)
    {
    this->critical(results[0]["queryError"].toString());
    return ExtensionMetadataType();
    }
  ExtensionMetadataType result = results.at(0);

  if (!qSlicerExtensionsManagerModelPrivate::validateExtensionMetadata(result))
    {
    return ExtensionMetadataType();
    }

  ExtensionMetadataType updatedExtensionMetadata;
  foreach(const QString& key, result.keys())
    {
    updatedExtensionMetadata.insert(
      q->serverToExtensionDescriptionKey().value(key, key), result.value(key));
    }

  return updatedExtensionMetadata;
}

// --------------------------------------------------------------------------
// qSlicerExtensionsManagerModel methods

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
QUrl qSlicerExtensionsManagerModel::serverUrl()const
{
  QSettings settings(this->extensionsSettingsFilePath(), QSettings::IniFormat);
  return QUrl(settings.value("Extensions/ServerUrl").toString());
  //HS Uncomment the following line for debugging and comment above line.
  //return QUrl("http://10.171.2.133:8080");
}

// --------------------------------------------------------------------------
QUrl qSlicerExtensionsManagerModel::serverUrlWithPackagePath()const
{
  QUrl url(this->serverUrl());
  url.setPath(url.path() + "/slicerpackages");
  return url;
}

// --------------------------------------------------------------------------
QUrl qSlicerExtensionsManagerModel::serverUrlWithExtensionsStorePath()const
{
  QUrl url(this->serverUrl());
  url.setPath(url.path() + "/slicerappstore");
  return url;
}

// --------------------------------------------------------------------------
QString qSlicerExtensionsManagerModel::extensionsInstallPath()const
{
  QSettings settings(this->extensionsSettingsFilePath(), QSettings::IniFormat);
  return settings.value("Extensions/InstallPath").toString();
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
QStringList qSlicerExtensionsManagerModel::extensionModulePaths(const QString& extensionName)const
{
  Q_D(const qSlicerExtensionsManagerModel);
  QString path = this->extensionInstallPath(extensionName);
  return appendToPathList(QStringList(), QStringList()
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
bool qSlicerExtensionsManagerModel::newExtensionEnabledByDefault()const
{
  Q_D(const qSlicerExtensionsManagerModel);
  return d->NewExtensionEnabledByDefault;
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel::ExtensionMetadataType qSlicerExtensionsManagerModel::extensionMetadata(const QString& extensionName)const
{
  Q_D(const qSlicerExtensionsManagerModel);

  ExtensionMetadataType metadata;
  QStandardItem * item = d->extensionItem(extensionName);
  if (!item)
    {
    return metadata;
    }
  int row = item->row();

  foreach(const QString& columnName, d->columnNames())
    {
    // TODO Server should provide us with depends. In the mean time, let's not export it.
    if (columnName == "depends")
      {
      continue;
      }
    metadata.insert(columnName, d->Model.data(
          d->Model.index(row, d->columnNames().indexOf(columnName)), Qt::DisplayRole).toString());
    }

  return metadata;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::isExtensionInstalled(const QString& extensionName) const
{
  Q_D(const qSlicerExtensionsManagerModel);
  QModelIndexList foundIndexes = d->Model.match(
        d->Model.index(0, qSlicerExtensionsManagerModelPrivate::NameColumn),
        qSlicerExtensionsManagerModelPrivate::NameRole, QVariant(extensionName),
        /* hits = */ 1, /* flags= */ Qt::MatchExactly | Qt::MatchWrap);
  Q_ASSERT(foundIndexes.size() < 2);
  return (foundIndexes.size() != 0);
}

// --------------------------------------------------------------------------
int qSlicerExtensionsManagerModel::numberOfInstalledExtensions()const
{
  Q_D(const qSlicerExtensionsManagerModel);
  return d->Model.rowCount();
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::installedExtensions()const
{
  Q_D(const qSlicerExtensionsManagerModel);
  QStringList names;
  for(int rowIdx = 0; rowIdx < d->Model.rowCount(); ++rowIdx)
    {
    names << d->Model.item(rowIdx, qSlicerExtensionsManagerModelPrivate::NameColumn)->text();
    }
  return names;
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

  if(value && !this->isExtensionCompatible(extensionName).isEmpty())
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

  ExtensionMetadataType updatedMetadata = this->extensionMetadata(extensionName);
  updatedMetadata["enabled"] = value;

  d->saveExtensionDescription(this->extensionDescriptionFile(extensionName), updatedMetadata);

  QStandardItem * item = const_cast<QStandardItem*>(
        d->extensionItem(extensionName, qSlicerExtensionsManagerModelPrivate::EnabledColumn));
  Q_ASSERT(item);
  item->setData(value, qSlicerExtensionsManagerModelPrivate::EnabledRole);
  item->setData(value, Qt::DisplayRole);

  emit this->extensionEnabledChanged(extensionName, value);
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::isExtensionEnabled(const QString& extensionName)const
{
  Q_D(const qSlicerExtensionsManagerModel);
  QStandardItem * item = d->extensionItem(extensionName, qSlicerExtensionsManagerModelPrivate::EnabledColumn);
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
bool qSlicerExtensionsManagerModel::isExtensionScheduledForUpdate(
  const QString& extensionName)const
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
bool qSlicerExtensionsManagerModel::isExtensionScheduledForUninstall(
  const QString& extensionName)const
{
  return this->scheduledForUninstallExtensions().contains(extensionName);
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::enabledExtensions()const
{
  Q_D(const qSlicerExtensionsManagerModel);
  QStringList names;
  for(int rowIdx = 0; rowIdx < d->Model.rowCount(); ++rowIdx)
    {
    QString extensionName = d->Model.item(rowIdx, qSlicerExtensionsManagerModelPrivate::NameColumn)->text();
    if (this->isExtensionEnabled(extensionName))
      {
      names << d->Model.item(rowIdx, qSlicerExtensionsManagerModelPrivate::NameColumn)->text();
      }
    }
  return names;
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel::ExtensionMetadataType qSlicerExtensionsManagerModel
::retrieveExtensionMetadata(const QString& extensionId)
{
  Q_D(qSlicerExtensionsManagerModel);

  if (extensionId.isEmpty())
    {
    return ExtensionMetadataType();
    }

  qMidasAPI::ParametersType parameters;
  parameters["extension_id"] = extensionId;

  return d->retrieveExtensionMetadata(parameters);
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel::ExtensionMetadataType qSlicerExtensionsManagerModel
::retrieveExtensionMetadataByName(const QString& extensionName)
{
  Q_D(qSlicerExtensionsManagerModel);

  if (extensionName.isEmpty())
    {
    return ExtensionMetadataType();
    }

  qMidasAPI::ParametersType parameters;
  parameters["productname"] = extensionName;
  parameters["slicer_revision"] = this->slicerRevision();
  parameters["os"] = this->slicerOs();
  parameters["arch"] = this->slicerArch();

  return d->retrieveExtensionMetadata(parameters);
}

// --------------------------------------------------------------------------
qSlicerExtensionDownloadTask*
qSlicerExtensionsManagerModelPrivate::downloadExtension(
  const QString& extensionId)
{
  Q_Q(qSlicerExtensionsManagerModel);

  this->debug(QString("Retrieving extension metadata [ extensionId: %1]").arg(extensionId));
  ExtensionMetadataType extensionMetadata = q->retrieveExtensionMetadata(extensionId);
  if (extensionMetadata.count() == 0)
    {
    return nullptr;
    }

  QString itemId = extensionMetadata["item_id"].toString();

  this->debug(QString("Downloading extension [ itemId: %1]").arg(itemId));
  QUrl downloadUrl(q->serverUrl());
  downloadUrl.setPath(downloadUrl.path() + "/download");
  QUrlQuery urlQuery;
  urlQuery.setQueryItems(
        QList<QPair<QString, QString> >() << QPair<QString, QString>("items", itemId));
  downloadUrl.setQuery(urlQuery);

  QNetworkReply* const reply =
    this->NetworkManager.get(QNetworkRequest(downloadUrl));
  qSlicerExtensionDownloadTask* const task =
    new qSlicerExtensionDownloadTask(reply);

  task->setMetadata(extensionMetadata);
  emit q->downloadStarted(reply);

  return task;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::downloadAndInstallExtension(const QString& extensionId)
{
  Q_D(qSlicerExtensionsManagerModel);
  QString error;
  if (!d->checkExtensionSettingsPermissions(error))
    {
    d->critical(error);
    return false;
    }
  qSlicerExtensionDownloadTask* const task = d->downloadExtension(extensionId);
  if (!task)
    {
    d->critical("Failed to retrieve metadata for extension " + extensionId);
    return false;
    }
  connect(task, SIGNAL(finished(qSlicerExtensionDownloadTask*)),
          this, SLOT(onInstallDownloadFinished(qSlicerExtensionDownloadTask*)));
  connect(task, SIGNAL(progress(qSlicerExtensionDownloadTask*, qint64, qint64)),
          this, SLOT(onInstallDownloadProgress(qSlicerExtensionDownloadTask*, qint64, qint64)));
  return true;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::onInstallDownloadProgress(
  qSlicerExtensionDownloadTask* task, qint64 received, qint64 total)
{
  // Look up the update information
  const QString& extensionName = task->extensionName();

  // Notify observers of download progress
  emit this->installDownloadProgress(extensionName, received, total);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::onInstallDownloadFinished(
  qSlicerExtensionDownloadTask* task)
{
  Q_D(qSlicerExtensionsManagerModel);

  task->deleteLater();

  QNetworkReply* const reply = task->reply();
  QUrl downloadUrl = reply->url();
  Q_ASSERT(QUrlQuery(downloadUrl).hasQueryItem("items"));

  emit this->downloadFinished(reply);

  if (reply->error())
    {
    d->critical(QString("Failed downloading: %1").arg(downloadUrl.toString()));
    return;
    }

  const QString& extensionName = task->extensionName();
  const QString& archiveName = task->archiveName();
  QTemporaryFile file(QString("%1/%2.XXXXXX").arg(QDir::tempPath(), archiveName));
  if (!file.open())
    {
    d->critical(QString("Could not create temporary file for writing: %1").arg(file.errorString()));
    return;
    }
  file.write(reply->readAll());
  file.close();
  const ExtensionMetadataType& extensionMetadata =
    this->filterExtensionMetadata(task->metadata());
  this->installExtension(extensionName, extensionMetadata, file.fileName());
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::installExtension(
  const QString& archiveFile)
{
  Q_D(qSlicerExtensionsManagerModel);

  std::vector<std::string> archiveContents;
  if (!vtkArchive::ListArchive(qPrintable(archiveFile), archiveContents))
    {
    d->critical(
      QString("Failed to list extension archive '%1'").arg(archiveFile));
    return false;
    }
  for (size_t n = 0; n < archiveContents.size(); ++n)
    {
    const std::string& s = archiveContents[n];
    const QString& fileName =
      QString::fromLocal8Bit(s.data(), static_cast<int>(s.size()));

    if (fileName.endsWith(".s4ext"))
      {
      const QFileInfo fi(fileName);
      return this->installExtension(fi.completeBaseName(),
                                    ExtensionMetadataType(), archiveFile);
      }
    }

  d->critical(
    QString("No extension description found in archive '%1'").arg(archiveFile));
  return false;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::installExtension(
  const QString& extensionName, ExtensionMetadataType extensionMetadata,
  const QString& archiveFile)
{
  Q_D(qSlicerExtensionsManagerModel);

  if (extensionName.isEmpty())
    {
    return false;
    }

  if (this->isExtensionInstalled(extensionName))
    {
    return false;
    }

  if (this->extensionsInstallPath().isEmpty())
    {
    d->critical("Extensions/InstallPath setting is not set !");
    return false;
    }

  if (!QDir().mkpath(this->extensionsInstallPath()))
    {
    d->critical(QString("Failed to create extension installation directory %1").arg(this->extensionsInstallPath()));
    return false;
    }

  QString error;
  if (!d->checkExtensionSettingsPermissions(error))
    {
    d->critical(error);
    return false;
    }

  QString extensionDescriptionFile = this->extensionDescriptionFile(extensionName);

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
    extensionMetadata.insert("enabled", d->NewExtensionEnabledByDefault);

    // Copy expected keys from archive description
    QStringList expectedKeys;
    expectedKeys << "category" << "contributors" << "description" << "homepage"
                 << "iconurl" << "screenshots" << "status";

    const ExtensionMetadataType::const_iterator notFound =
      extensionIndexMetadata.constEnd();
    foreach (const QString& key, expectedKeys)
      {
      const ExtensionMetadataType::const_iterator iter =
        extensionIndexMetadata.constFind(key);
      if (iter != notFound)
        {
        extensionMetadata.insert(key, iter.value());
        }
      }

    extensionMetadata.insert("scm",      extensionIndexMetadata.value("scm",         "NA"));
    extensionMetadata.insert("scmurl",   extensionIndexMetadata.value("scmurl",      "NA"));
    extensionMetadata.insert("revision", extensionIndexMetadata.value("scmrevision", "NA"));

    // Fill in keys related to the target Slicer platform
    extensionMetadata.insert("os", this->slicerOs());
    extensionMetadata.insert("arch", this->slicerArch());
    extensionMetadata.insert("slicer_revision", this->slicerRevision());
    }

  // Gather information on dependency extensions
  const QStringList dependencies = extensionIndexMetadata.value("depends").toString().split(" ");
  QHash<QString, ExtensionMetadataType> dependenciesMetadata;
  QStringList unresolvedDependencies;
  foreach (const QString& dependencyName, dependencies)
    {
    if (!dependencyName.isEmpty() && dependencyName != "NA")
      {
      if (this->isExtensionInstalled(dependencyName))
        {
        // Dependency is already installed
        continue;
        }

      qMidasAPI::ParametersType parameters;
      parameters["productname"] = dependencyName;
      parameters["slicer_revision"] = this->slicerRevision();
      parameters["os"] = this->slicerOs();
      parameters["arch"] = this->slicerArch();

      const ExtensionMetadataType& dependencyMetadata =
        d->retrieveExtensionMetadata(parameters);
      if (dependencyMetadata.contains("extension_id"))
        {
        dependenciesMetadata.insert(dependencyName, dependencyMetadata);
        }
      else
        {
        unresolvedDependencies.append(dependencyName);
        }
      }
    }

  // Warn about unresolved dependencies
  if (!unresolvedDependencies.isEmpty())
    {
    QString msg = QString("<p>%1 depends on the following extensions, which could not be found:</p><ul>").arg(extensionName);
    foreach (const QString& dependencyName, unresolvedDependencies)
      {
      msg += QString("<li>%1</li>").arg(dependencyName);
      }
    msg += "</ul><p>The extension may not function properly.</p>";
    QMessageBox::warning(nullptr, "Unresolved dependencies", msg);
    }

  // Prompt to install dependencies (if any)
  if (!dependenciesMetadata.isEmpty())
    {
    QString msg = QString("<p>%1 depends on the following extensions:</p><ul>").arg(extensionName);
    foreach (const QString& dependencyName, dependenciesMetadata.keys())
      {
      msg += QString("<li>%1</li>").arg(dependencyName);
      }
    msg += "</ul><p>Would you like to install them now?</p>";
    const QMessageBox::StandardButton result =
      QMessageBox::question(nullptr, "Install dependencies", msg,
                            QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes)
      {
      // Install dependencies
      msg.clear();
      foreach (const ExtensionMetadataType& dependency, dependenciesMetadata)
        {
        bool res = this->downloadAndInstallExtension(
              dependency.value("extension_id").toString());
        if (!res)
          {
          msg += QString("<li>%1</li>").arg(dependency.value("extensionname").toString());
          }
        }
      if (!msg.isEmpty())
        {
        d->critical(QString("Error while installing dependent extensions:<ul>%1<ul>").arg(msg));
        }
      }
    }

  // Finish installing the extension
  d->saveExtensionDescription(extensionDescriptionFile, extensionMetadata);
  d->addExtensionSettings(extensionName);
  d->addExtensionModelRow(Self::parseExtensionDescriptionFile(extensionDescriptionFile));
  d->saveExtensionToHistorySettings(this->extensionsHistorySettingsFilePath(), extensionMetadata);
  emit this->extensionInstalled(extensionName);

  // Log notice that extension was installed
  const QString& extensionId =
    extensionMetadata.value("extension_id").toString();
  const QString& extensionRevision =
    extensionMetadata.value("revision").toString();

  QString msg = "Installed extension " + extensionName;
  if (!extensionId.isEmpty())
    {
    msg += QString(" (%1)").arg(extensionId);
    }
  if (!extensionRevision.isEmpty())
    {
    msg += QString(" revision %1").arg(extensionRevision);
    }
  d->info(msg);

  return true;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::checkForUpdates(bool installUpdates)
{
  Q_D(qSlicerExtensionsManagerModel);

  d->CheckForUpdatesApi.setServerUrl(this->serverUrl().toString());

  // Loop over extensions
  foreach (const QString& extensionName, this->installedExtensions())
    {
    const ExtensionMetadataType& extensionMetadata =
      this->extensionMetadata(extensionName);
    const QString& extensionId =
      extensionMetadata.value("extension_id").toString();

    // Build parameters to query server about the extension
    qMidasAPI::ParametersType parameters;
    if (!extensionId.isEmpty())
      {
      parameters["extension_id"] = extensionId;
      }
    else
      {
      parameters["productname"] = extensionName;
      parameters["slicer_revision"] = this->slicerRevision();
      parameters["os"] = this->slicerOs();
      parameters["arch"] = this->slicerArch();
      }

    // Issue the query
    const QUuid& requestId =
      d->CheckForUpdatesApi.get("midas.slicerpackages.extension.list",
                                parameters);

    // Store information about the request
    UpdateCheckInformation updateInfo;

    updateInfo.InstallAutomatically = installUpdates;
    updateInfo.ExtensionName = extensionName;
    updateInfo.InstalledVersion =
      extensionMetadata.value("revision").toString();

    d->CheckForUpdatesRequests.insert(requestId, updateInfo);
    }
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::isExtensionUpdateAvailable(
  const QString& extensionName) const
{
  Q_D(const qSlicerExtensionsManagerModel);
  return d->AvailableUpdates.contains(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::onUpdateCheckComplete(
  const QUuid& requestId, const QList<QVariantMap>& results)
{
  Q_D(qSlicerExtensionsManagerModel);

  const UpdateCheckInformation& updateInfo =
    d->CheckForUpdatesRequests.take(requestId);

  // Parse server response
  if (updateInfo.ExtensionName.isEmpty())
    {
    const QString msg(
      "Received response to query %1 with no associated request?");
    d->info(msg.arg(requestId.toString()));
    }
  else if (results.isEmpty())
    {
    const QString msg("Update check for %1 failed: no response from server"
                      " (no such extension known?)");
    d->warning(msg.arg(updateInfo.ExtensionName));
    }
  else
    {
    // Check for valid response (expecting exactly one result)
    if (results.count() > 1)
      {
      const QString msg = "Update check for %1 failed: received unexpected"
                          " multiple responses from the server";
      d->warning(msg.arg(updateInfo.ExtensionName));
      return;
      }

    // Get extension information from response
    const ExtensionMetadataType& extensionMetadata = results.first();

    const QString& extensionId =
      extensionMetadata.value("extension_id").toString();
    const QString& extensionRevision =
      extensionMetadata.value("revision").toString();

    const QString msg("update check for %1 complete:"
                      " '%2' available, '%3' installed");
    d->info(msg.arg(updateInfo.ExtensionName, extensionRevision,
                    updateInfo.InstalledVersion));

    // Check if update is available
    if (!extensionRevision.isEmpty() &&
        extensionRevision != updateInfo.InstalledVersion)
      {
      // Add to known updates
      d->AvailableUpdates.insert(updateInfo.ExtensionName,
                                 UpdateDownloadInformation(extensionId));

      // Immediately start update process if requested
      if (updateInfo.InstallAutomatically)
        {
        this->scheduleExtensionForUpdate(updateInfo.ExtensionName);
        }

      emit this->extensionUpdateAvailable(updateInfo.ExtensionName);
      }
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::onUpdateCheckFailed(const QUuid& requestId)
{
  Q_D(qSlicerExtensionsManagerModel);
  d->CheckForUpdatesRequests.remove(requestId);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::onUpdateDownloadProgress(
  qSlicerExtensionDownloadTask* task, qint64 received, qint64 total)
{
  Q_D(qSlicerExtensionsManagerModel);

  // Look up the update information
  const QString& extensionName = task->extensionName();
  const QHash<QString, UpdateDownloadInformation>::iterator iter =
    d->AvailableUpdates.find(extensionName);

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
void qSlicerExtensionsManagerModel::onUpdateDownloadFinished(
  qSlicerExtensionDownloadTask* task)
{
  Q_D(qSlicerExtensionsManagerModel);

  // Mark task for clean-up
  task->deleteLater();

  // Get network reply
  QNetworkReply* const reply = task->reply();
  QUrl downloadUrl = reply->url();
  Q_ASSERT(QUrlQuery(downloadUrl).hasQueryItem("items"));

  // Notify observers of event
  emit this->downloadFinished(reply);

  // Did the download succeed?
  if (reply->error())
    {
    d->critical("Failed downloading: " + downloadUrl.toString());
    return;
    }

  // Look up the update information
  const QString& extensionName = task->extensionName();
  const QHash<QString, UpdateDownloadInformation>::iterator iter =
    d->AvailableUpdates.find(extensionName);

  if (iter != d->AvailableUpdates.end())
    {
    // Create directory for update archives (a persistent location is desired,
    // since we won't be installing the update Immediately)
    if (!QDir(this->extensionsInstallPath()).mkpath(".updates"))
      {
      d->critical("Could not create directory for update archive");
      return;
      }

    // Save update metadata
    const QString& metadataPath =
      QString("%1/.updates/%2.s4ext").arg(this->extensionsInstallPath(),
                                          extensionName);
    Self::writeExtensionDescriptionFile(metadataPath, task->metadata());

    // Create update archive
    const QString& archiveName = task->archiveName();
    const QString& archivePath =
      QString("%1/.updates/%2").arg(this->extensionsInstallPath(), archiveName);

    QFile file(archivePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
      {
      d->critical("Could not create file for writing: " + file.errorString());
      return;
      }

    file.write(reply->readAll());
    file.close();

    // Schedule install of update
    iter->ArchiveName = archivePath;
    this->scheduleExtensionForUpdate(extensionName);
    }
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::scheduleExtensionForUpdate(
  const QString& extensionName)
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
  QVariantMap scheduled =
    settings.value("Extensions/ScheduledForUpdate").toMap();

  if (scheduled.contains(extensionName))
    {
    // Already scheduled for update
    return true;
    }

  UpdateDownloadInformation& updateInfo =
    d->AvailableUpdates[extensionName];
  if (updateInfo.ArchiveName.isEmpty())
    {
    if (updateInfo.ExtensionId.isEmpty())
      {
      d->critical("Missing download information for extension " +
                  extensionName);
      return false;
      }

    if (updateInfo.DownloadSize != 0)
      {
      // Already being downloaded
      return true;
      }

    qSlicerExtensionDownloadTask* const task =
      d->downloadExtension(updateInfo.ExtensionId);
    if (!task)
      {
      d->critical("Failed to retrieve metadata for extension " +
                  updateInfo.ExtensionId);
      return false;
      }

    connect(task, SIGNAL(finished(qSlicerExtensionDownloadTask*)),
            this, SLOT(onUpdateDownloadFinished(qSlicerExtensionDownloadTask*)));
    connect(task, SIGNAL(progress(qSlicerExtensionDownloadTask*,qint64,qint64)),
            this, SLOT(onUpdateDownloadProgress(qSlicerExtensionDownloadTask*,qint64,qint64)));

    this->onUpdateDownloadProgress(task, 0, -1);
    return true;
    }

  // Add to scheduled updates
  const ExtensionMetadataType metadata = this->extensionMetadata(extensionName);
  scheduled[extensionName] = updateInfo.ArchiveName;
  settings.setValue("Extensions/ScheduledForUpdate", scheduled);

  d->info(extensionName + " scheduled for update");
  emit this->extensionScheduledForUpdate(extensionName);

  return true;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::cancelExtensionScheduledForUpdate(
  const QString& extensionName)
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
bool qSlicerExtensionsManagerModelPrivate::updateExtension(
  const QString& extensionName, const QString& archiveFile)
{
  Q_Q(qSlicerExtensionsManagerModel);

  QString error;
  if (!this->checkExtensionSettingsPermissions(error))
    {
    this->critical(error);
    return false;
    }

  QStandardItem * item = this->extensionItem(extensionName);
  if (!item)
    {
    qCritical() << "Failed to update extension" << extensionName;
    return false;
    }

  if (!q->isExtensionScheduledForUpdate(extensionName))
    {
    qCritical() << "Failed to update extension" << extensionName
                << "- Extension is NOT 'scheduled for update'";
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
  const QString& metadataPath =
    QString("%1/.updates/%2.s4ext").arg(q->extensionsInstallPath(),
                                        extensionName);
  const ExtensionMetadataType extensionMetadata =
    qSlicerExtensionsManagerModel::parseExtensionDescriptionFile(metadataPath);

  // Install new version
  success = success &&
            q->installExtension(extensionName, extensionMetadata, archiveFile);

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
  QStringList scheduled =
    settings.value("Extensions/ScheduledForUninstall").toStringList();

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

  const ExtensionMetadataType& extensionMetadata = this->extensionMetadata(extensionName);
  d->scheduleExtensionHistorySettingRemoval(this->extensionsHistorySettingsFilePath(), extensionMetadata);
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
  d->cancelExtensionHistorySettingRemoval(this->extensionsHistorySettingsFilePath(), extensionName);
  emit this->extensionCancelledScheduleForUninstall(extensionName);

  return true;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModelPrivate::uninstallExtension(const QString& extensionName)
{
  Q_Q(qSlicerExtensionsManagerModel);

  QString error;
  if (!this->checkExtensionSettingsPermissions(error))
    {
    this->critical(error);
    return false;
    }

  QStandardItem * item = this->extensionItem(extensionName);
  if (!item)
    {
    qCritical() << "Failed to uninstall extension" << extensionName;
    return false;
    }

  if (!q->isExtensionScheduledForUninstall(extensionName))
    {
    qCritical() << "Failed to uninstall extension" << extensionName
                << " - Extension is NOT 'scheduled for uninstall'";
    return false;
    }

  bool success = true;
  success = success && ctk::removeDirRecursively(q->extensionInstallPath(extensionName));
  success = success && QFile::remove(q->extensionDescriptionFile(extensionName));
  success = success && this->Model.removeRow(item->row());

  const QDir installDir(q->extensionsInstallPath());
  const QFileInfoList& iconEntries = installDir.entryInfoList(QStringList() << extensionName + "-icon.*");
  foreach (const QFileInfo& iconEntry, iconEntries)
    {
    success = success && QFile::remove(iconEntry.absoluteFilePath());
    }

  if (success)
    {
    this->removeExtensionSettings(extensionName);
    this->removeExtensionFromScheduledForUninstallList(extensionName);
    }

  emit q->extensionUninstalled(extensionName);

  return success;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::updateScheduledExtensions()
{
  QStringList updatedExtensions;
  return this->updateScheduledExtensions(updatedExtensions);
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::updateScheduledExtensions(
  QStringList& updatedExtensions)
{
  Q_D(qSlicerExtensionsManagerModel);
  bool result = true;

  QSettings settings(this->extensionsSettingsFilePath(), QSettings::IniFormat);
  const QVariantMap scheduledUpdates =
    settings.value("Extensions/ScheduledForUpdate").toMap();

  const QVariantMap::const_iterator end = scheduledUpdates.end();
  for(QVariantMap::const_iterator iter = scheduledUpdates.begin();
      iter != end; ++iter)
    {
    const QString& extensionName = iter.key();
    const bool success = d->updateExtension(extensionName, iter->toString());
    if(success)
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
  Q_D(qSlicerExtensionsManagerModel);
  d->removeScheduledExtensionHistorySettings(this->extensionsHistorySettingsFilePath());
  bool result = true;
  foreach(const QString& extensionName, this->scheduledForUninstallExtensions())
    {
    const bool success = d->uninstallExtension(extensionName);
    if(success)
      {
      uninstalledExtensions << extensionName;
      }
    result = result && success;
    }
  return result;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::gatherExtensionsHistoryInformationOnStartup()
{
  Q_D(qSlicerExtensionsManagerModel);
  d->gatherExtensionsHistoryInformationOnStartup();
}

// --------------------------------------------------------------------------
QVariantMap  qSlicerExtensionsManagerModel::getExtensionHistoryInformation()
{
  Q_D(qSlicerExtensionsManagerModel);
  return d->getExtensionsInfoFromPreviousInstallations(extensionsHistorySettingsFilePath());
}


// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::updateModel()
{
  Q_D(qSlicerExtensionsManagerModel);

  QString extensionDescriptionPath = this->extensionsInstallPath();

  d->Model.clear();

  foreach(const QFileInfo& fileInfo, d->extensionDescriptionFileInfos(extensionDescriptionPath))
    {
    ExtensionMetadataType metadata = Self::parseExtensionDescriptionFile(fileInfo.absoluteFilePath());
    d->addExtensionModelRow(metadata);
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
CTK_GET_CPP(qSlicerExtensionsManagerModel, QString, extensionsHistorySettingsFilePath, ExtensionsHistorySettingsFilePath)

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::setExtensionsHistorySettingsFilePath(const QString& extensionsHistorySettingsFilePath)
{
  Q_D(qSlicerExtensionsManagerModel);
  if (d->ExtensionsHistorySettingsFilePath == extensionsHistorySettingsFilePath)
    {
    return;
    }
  d->ExtensionsHistorySettingsFilePath = extensionsHistorySettingsFilePath;
  emit this->extensionsHistorySettingsFilePathChanged(extensionsHistorySettingsFilePath);
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
void qSlicerExtensionsManagerModel::setSlicerRequirements(const QString& revision, const QString& os, const QString& arch)
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
  foreach(const QString& extensionName, this->installedExtensions())
    {
    QStringList reasons = this->isExtensionCompatible(extensionName, d->SlicerRevision, d->SlicerOs, d->SlicerArch);
    if (!reasons.isEmpty())
      {
      reasons.prepend(QString("Extension %1 is incompatible").arg(extensionName));
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
QStringList qSlicerExtensionsManagerModel::isExtensionCompatible(
    const QString& extensionName, const QString& slicerRevision,
    const QString& slicerOs, const QString& slicerArch) const
{
  if (extensionName.isEmpty())
    {
    return QStringList() << tr("extensionName is not specified");
    }
  ExtensionMetadataType metadata = this->extensionMetadata(extensionName);
  return qSlicerExtensionsManagerModelPrivate::isExtensionCompatible(
        metadata, slicerRevision, slicerOs, slicerArch);
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::isExtensionCompatible(const QString& extensionName) const
{
  return this->isExtensionCompatible(
        extensionName, this->slicerRevision(), this->slicerOs(), this->slicerArch());
}

// --------------------------------------------------------------------------
const QStandardItemModel * qSlicerExtensionsManagerModel::model()const
{
  Q_D(const qSlicerExtensionsManagerModel);
  return &d->Model;
}

// --------------------------------------------------------------------------
QHash<QString, QString> qSlicerExtensionsManagerModel::serverToExtensionDescriptionKey()
{
  QHash<QString, QString> serverToExtensionDescriptionKey;
  serverToExtensionDescriptionKey.insert("productname", "extensionname");
  serverToExtensionDescriptionKey.insert("name", "archivename");
  serverToExtensionDescriptionKey.insert("repository_type", "scm");
  serverToExtensionDescriptionKey.insert("repository_url", "scmurl");
  serverToExtensionDescriptionKey.insert("development_status", "status");
  serverToExtensionDescriptionKey.insert("icon_url", "iconurl");
  return serverToExtensionDescriptionKey;
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::serverKeysToIgnore()
{
  return QStringList()
      << "item_id" << "bitstream_id"
      << "submissiontype" << "codebase" << "package"
      << "size" << "date_creation";
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel::ExtensionMetadataType
qSlicerExtensionsManagerModel::filterExtensionMetadata(const ExtensionMetadataType& extensionMetadata)
{
  ExtensionMetadataType filteredExtensionMetadata = extensionMetadata;
  foreach(const QString& key, Self::serverKeysToIgnore())
    {
    filteredExtensionMetadata.remove(key);
    }
  return filteredExtensionMetadata;
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::readArrayValues(
  QSettings& settings, const QString& arrayName, const QString fieldName)
{
  Q_ASSERT(!arrayName.isEmpty());
  Q_ASSERT(!fieldName.isEmpty());
  QStringList listOfValues;
  int size = settings.beginReadArray(arrayName);
  for (int i=0; i < size; ++i)
    {
    settings.setArrayIndex(i);
    listOfValues << settings.value(fieldName).toString();
    }
  settings.endArray();
  return listOfValues;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::writeArrayValues(QSettings& settings, const QStringList& values,
    const QString& arrayName, const QString fieldName)
{
  Q_ASSERT(!arrayName.isEmpty());
  Q_ASSERT(!fieldName.isEmpty());
  settings.remove(arrayName);
  settings.beginWriteArray(arrayName);
  for(int i=0; i < values.size(); ++i)
    {
    settings.setArrayIndex(i);
    settings.setValue(fieldName, values.at(i));
    }
  settings.endArray();
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::extractExtensionArchive(
    const QString& extensionName, const QString& archiveFile, const QString& destinationPath)
{
  Q_D(qSlicerExtensionsManagerModel);

  if (extensionName.isEmpty())
    {
    d->critical("Corrupted extension package");
    return false;
    }

  QString error;
  if (!d->checkExtensionsInstallDestinationPath(destinationPath, error))
    {
    d->critical(QString("Failed to extract archive %1 into directory %2").arg(archiveFile).arg(destinationPath));
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

  //  Step1: <extensionName>/<archiveBaseName>[/<Slicer_BUNDLE_LOCATION>/<Slicer_EXTENSIONS_DIRBASENAME>-<slicerRevision>/<extensionName>] -> <extensionName>-XXXXXX
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
    d->critical(QString("Failed to copy directory %1 into directory %2").arg(srcPathToCopy).arg(intermediatePath));
    return false;
    }

  //  Step2: <extensionName>-XXXXXX -> <extensionName>
  if (!ctk::copyDirRecursively(intermediatePath, dstPath))
    {
    d->critical(QString("Failed to copy directory %1 into directory %2").arg(intermediatePath).arg(dstPath));
    return false;
    }

  //  Step3: Remove <extensionName>-XXXXXX
  if (!ctk::removeDirRecursively(intermediatePath))
    {
    d->critical(QString("Failed to remove directory %1").arg(intermediatePath));
    return false;
    }

  //  Step4: Remove  <extensionName>/<archiveBaseName>
  if (!ctk::removeDirRecursively(srcPath))
    {
    d->critical(QString("Failed to remove directory %1").arg(srcPath));
    return false;
    }

  return true;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::writeExtensionDescriptionFile(const QString& file, const ExtensionMetadataType &metadata)
{
  QFile outputFile(file);
  if (!outputFile.open(QFile::WriteOnly | QFile::Truncate))
    {
    return false;
    }
  QTextStream outputStream(&outputFile);
  foreach(const QString& key, metadata.keys())
    {
    if (key == "extensionname")
      {
      continue;
      }
    outputStream << QString("%1 %2\n").arg(key).arg(metadata.value(key).toString());
    }
  outputFile.close();
  return true;
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel::ExtensionMetadataType
qSlicerExtensionsManagerModel::parseExtensionDescriptionFile(const QString& file)
{
  ExtensionMetadataType metadata;

  QFile inputFile(file);
  if (!inputFile.open(QFile::ReadOnly))
    {
    return metadata;
    }

  QTextStream inputStream(&inputFile);
  for(QString line(""); !line.isNull(); line = inputStream.readLine())
    {
    QChar commentCharacter('#');
    if (line.startsWith(commentCharacter) || line.isEmpty())
      {
      continue;
      }
    QStringList components = line.split(" ", QString::SkipEmptyParts);
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

  foreach(const QFileInfo& fileInfo, d->extensionDescriptionFileInfos(extensionDescriptionPath))
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
