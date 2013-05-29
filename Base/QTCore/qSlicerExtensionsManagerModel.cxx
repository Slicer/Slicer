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
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>
#include <QStandardItemModel>
#include <QTextStream>
#include <QUrl>

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

// Logic includes
#include "vtkArchive.h"


// --------------------------------------------------------------------------
namespace
{

// --------------------------------------------------------------------------
class QStandardItemModelWithRole : public QStandardItemModel
{
public:
  void setRoleNames(const QHash<int,QByteArray> &roleNames)
  {
    this->QStandardItemModel::setRoleNames(roleNames);
  }
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
    NameColumn = 0,
    ScmColumn,
    ScmUrlColumn,
    SlicerRevisionColumn,
    RevisionColumn,
    ReleaseColumn,
    ArchColumn,
    OsColumn,
    DependsColumn,
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
    NameRole = Qt::UserRole + 1,
    ScmRole,
    ScmUrlRole,
    DependsRole,
    SlicerRevisionRole,
    RevisionRole,
    ReleaseRole,
    ArchRole,
    OsRole,
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

  bool checkExtensionSettingsPermissions()const;
  void addExtensionSettings(const QString& extensionName);
  void removeExtensionSettings(const QString& extensionName);

  void removeExtensionFromScheduledForUninstallList(const QString& extensionName);

  QString extractArchive(const QDir& extensionsDir, const QString &archiveFile);

  /// \brief Uninstall \a extensionName
  /// \note The directory containing the extension will be deleted.
  /// \sa downloadExtension, installExtension
  bool uninstallExtension(const QString& extensionName);

  QStringList extensionLibraryPaths(const QString& extensionName)const;
  QStringList extensionPaths(const QString& extensionName)const;

#ifdef Slicer_USE_PYTHONQT
  QStringList extensionPythonPaths(const QString& extensionName)const;
#endif
  static bool validateExtensionMetadata(const ExtensionMetadataType &extensionMetadata);

  void saveExtensionDescription(const QString& extensionDescriptionFile, const ExtensionMetadataType &allExtensionMetadata);

  void initializeColumnIdToNameMap(int columnIdx, const char* columnName);
  QHash<int, QString> ColumnIdToName;
  QStringList ColumnNames;

  bool NewExtensionEnabledByDefault;

  QNetworkAccessManager NetworkManager;

  QString ExtensionsSettingsFilePath;

  QString SlicerRevision;
  QString SlicerOs;
  QString SlicerArch;

  QString SlicerVersion;

  QStandardItemModelWithRole Model;
};

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModelPrivate::qSlicerExtensionsManagerModelPrivate(qSlicerExtensionsManagerModel& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::init()
{
  Q_Q(qSlicerExtensionsManagerModel);

  this->SlicerVersion = Slicer_VERSION;

  qRegisterMetaType<ExtensionMetadataType>("ExtensionMetadataType");

  this->NewExtensionEnabledByDefault = true;

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
    roleNames[Qt::UserRole + 1 + columnIdx] = columnName.toLatin1();
    ++columnIdx;
    }
  this->Model.setRoleNames(roleNames);

  QObject::connect(&this->NetworkManager, SIGNAL(finished(QNetworkReply*)),
                   q, SLOT(onDownloadFinished(QNetworkReply*)));

  QObject::connect(q, SIGNAL(slicerRequirementsChanged(QString,QString,QString)),
                   q, SLOT(identifyIncompatibleExtensions()));

  QObject::connect(q, SIGNAL(modelUpdated()),
                   q, SLOT(identifyIncompatibleExtensions()));
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::debug(const QString& text) const
{
  this->log(text, ctkErrorLogLevel::Debug);
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
  if(level == ctkErrorLogLevel::Debug)
    {
    qDebug() << text;
    }
  else if (level == ctkErrorLogLevel::Warning)
    {
    qWarning() << text;
    }
  else if (level == ctkErrorLogLevel::Critical)
    {
    qCritical() << text;
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
    item->setData(value, this->role(key.toLatin1()));
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
    return 0;
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
QStringList appendToPathList(const QStringList& paths, const QString& pathToAppend, bool shouldExist = true)
{
  QStringList updatedPaths(paths);
  if (!hasPath(paths, pathToAppend) && shouldExist ? QDir(pathToAppend).exists() : true)
    {
    updatedPaths << pathToAppend;
    }
  return updatedPaths;
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

#if defined(Q_OS_MAC) && defined(Slicer_USE_PYTHONQT)
  QStringList additionalPythonPaths = settings.value("Python/AdditionalPythonPaths").toStringList();
  settings.setValue("Python/AdditionalPythonPaths",
                    appendToPathList(additionalPythonPaths, this->extensionPythonPaths(extensionName)));
#endif
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModelPrivate::removeExtensionPathFromApplicationSettings(const QString& extensionName)
{
  Q_Q(qSlicerExtensionsManagerModel);
  QSettings settings(q->extensionsSettingsFilePath(), QSettings::IniFormat);
  QStringList additionalPaths = settings.value("Modules/AdditionalPaths").toStringList();
  settings.setValue("Modules/AdditionalPaths",
                    removeFromPathList(additionalPaths, q->extensionModulePaths(extensionName)));

#if defined(Q_OS_MAC) && defined(Slicer_USE_PYTHONQT)
  QStringList additionalPythonPaths = settings.value("Python/AdditionalPythonPaths").toStringList();
  settings.setValue("Python/AdditionalPythonPaths",
                    removeFromPathList(additionalPythonPaths, this->extensionPythonPaths(extensionName)));
#endif
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
    this->warning(QString("Failed to open extensions settings file %1").arg(this->ExtensionsSettingsFilePath));
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
  QString sep("<PATHSEP>");
  QString pythonPath = settings.value("EnvironmentVariables/PYTHONPATH").toString();
  QStringList pythonPaths = pythonPath.split(sep);
  settings.setValue("EnvironmentVariables/PYTHONPATH",
                    appendToPathList(pythonPaths, this->extensionPythonPaths(extensionName)).join(sep));
#endif
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
    this->warning(QString("Failed to open extensions settings file: %1").arg(this->ExtensionsSettingsFilePath));
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
  QString sep("<PATHSEP>");
  QString pythonPath = settings.value("EnvironmentVariables/PYTHONPATH").toString();
  QStringList pythonPaths = pythonPath.split(sep);
  settings.setValue("EnvironmentVariables/PYTHONPATH",
                    removeFromPathList(pythonPaths, this->extensionPythonPaths(extensionName)).join(sep));
#endif
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModelPrivate::checkExtensionSettingsPermissions()const
{
  Q_Q(const qSlicerExtensionsManagerModel);

  QFileInfo settingsFileInfo = QFileInfo(q->extensionsSettingsFilePath());
  if (settingsFileInfo.exists())
    {
    if (!settingsFileInfo.isReadable() || !settingsFileInfo.isWritable())
      {
      this->warning(QString("Extensions settings file %1 is expected to be both readable and writable").
                     arg(q->extensionsSettingsFilePath()));
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
  bool success = extract_tar(archiveFile.toLatin1(), /* verbose */ false, /* extract */ true, &extracted_files);
  if(!success)
    {
    this->critical(QString("Failed to extract %1 into %2").arg(archiveFile).arg(extensionsDir.absolutePath()));
    return false;
    }
  if(extracted_files.size() == 0)
    {
    this->warning(QString("Archive %1 doesn't contain any files !").arg(archiveFile));
    return false;
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
void qSlicerExtensionsManagerModelPrivate::initializeColumnIdToNameMap(int columnIdx, const char* columnName)
{
  this->ColumnIdToName[columnIdx] = columnName;
  this->ColumnNames << columnName;
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
qSlicerExtensionsManagerModel::~qSlicerExtensionsManagerModel()
{
}

// --------------------------------------------------------------------------
QUrl qSlicerExtensionsManagerModel::serverUrl()const
{
  QSettings settings(this->extensionsSettingsFilePath(), QSettings::IniFormat);
  return QUrl(settings.value("Extensions/ServerUrl").toString());
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
  d->NewExtensionEnabledByDefault = value;
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
        d->Model.index(0, qSlicerExtensionsManagerModelPrivate::NameColumn), qSlicerExtensionsManagerModelPrivate::NameRole, QVariant(extensionName));
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

  if (!d->checkExtensionSettingsPermissions())
    {
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
QStringList qSlicerExtensionsManagerModel::scheduledForUninstallExtensions() const
{
  QSettings settings(this->extensionsSettingsFilePath(), QSettings::IniFormat);
  return settings.value("Extensions/ScheduledForUninstall").toStringList();
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::isExtensionScheduledForUninstall(const QString& extensionName)const
{
  QSettings settings(this->extensionsSettingsFilePath(), QSettings::IniFormat);
  return settings.value("Extensions/ScheduledForUninstall").toStringList().contains(extensionName);
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

  bool ok = false;
  QList<QVariantMap> results = qMidasAPI::synchronousQuery(
        ok, this->serverUrl().toString(),
        "midas.slicerpackages.extension.list", parameters);
  Q_ASSERT(results.count() == 1);
  if (!ok)
    {
    d->critical(results[0]["queryError"].toString());
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
          this->serverToExtensionDescriptionKey().value(key, key), result.value(key));
    }

  return updatedExtensionMetadata;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::downloadAndInstallExtension(const QString& extensionId)
{
  Q_D(qSlicerExtensionsManagerModel);

  if (!d->checkExtensionSettingsPermissions())
    {
    return;
    }

  d->debug(QString("Retrieving extension metadata [ extensionId: %1]").arg(extensionId));
  ExtensionMetadataType extensionMetadata = this->retrieveExtensionMetadata(extensionId);
  if (extensionMetadata.count() == 0)
    {
    return;
    }

  QString itemId = extensionMetadata["item_id"].toString();

  d->debug(QString("Downloading extension [ itemId: %1]").arg(itemId));
  QUrl downloadUrl(this->serverUrl());
  downloadUrl.setPath(downloadUrl.path() + "/download");
  downloadUrl.setQueryItems(
        QList<QPair<QString, QString> >() << QPair<QString, QString>("items", itemId));

  QNetworkReply * reply = d->NetworkManager.get(QNetworkRequest(downloadUrl));
  reply->setProperty("extension_metadata", extensionMetadata);
  emit this->downloadStarted(reply);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::onDownloadFinished(QNetworkReply* reply)
{
  Q_D(qSlicerExtensionsManagerModel);

  QUrl downloadUrl = reply->url();
  Q_ASSERT(downloadUrl.hasQueryItem("items"));

  emit this->downloadFinished(reply);

  if (reply->error())
    {
    d->critical(QString("Failed downloading: %1").arg(downloadUrl.toString()));
    return;
    }

  ExtensionMetadataType extensionMetadata =
      this->filterExtensionMetadata(reply->property("extension_metadata").toMap());

  QString extensionName = extensionMetadata.value("extensionname").toString();
  QString archiveName = extensionMetadata.value("archivename").toString();

  QFileInfo fileInfo(QDir::tempPath(), archiveName);
  QFile file(fileInfo.absoluteFilePath());
  if (!file.open(QIODevice::WriteOnly))
    {
    d->critical(QString("Could not open %1 for writing: %2").arg(fileInfo.absoluteFilePath()).arg(file.errorString()));
    return;
    }
  file.write(reply->readAll());
  file.close();

  this->installExtension(extensionName, extensionMetadata, fileInfo.absoluteFilePath());
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::installExtension(const QString& extensionName,
                                                     const ExtensionMetadataType& extensionMetadata,
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

  if (!d->checkExtensionSettingsPermissions())
    {
    return false;
    }

  QString extensionDescriptionFile = this->extensionDescriptionFile(extensionName);

  if (!this->extractExtensionArchive(extensionName, archiveFile, this->extensionsInstallPath()))
    {
    return false;
    }

  d->saveExtensionDescription(extensionDescriptionFile, extensionMetadata);
  d->addExtensionSettings(extensionName);
  d->addExtensionModelRow(Self::parseExtensionDescriptionFile(extensionDescriptionFile));

  emit this->extensionInstalled(extensionName);

  return true;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::scheduleExtensionForUninstall(const QString& extensionName)
{
  Q_D(qSlicerExtensionsManagerModel);

  if (!d->checkExtensionSettingsPermissions())
    {
    return false;
    }

  if (!this->isExtensionInstalled(extensionName))
    {
    return false;
    }

  if (this->isExtensionScheduledForUninstall(extensionName))
    {
    return true;
    }
  QSettings settings(this->extensionsSettingsFilePath(), QSettings::IniFormat);
  settings.setValue(
        "Extensions/ScheduledForUninstall",
        settings.value("Extensions/ScheduledForUninstall").toStringList() << extensionName);

  d->removeExtensionSettings(extensionName);

  emit this->extensionScheduledForUninstall(extensionName);

  return true;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::cancelExtensionScheduledForUninstall(const QString& extensionName)
{
  Q_D(qSlicerExtensionsManagerModel);

  if (!d->checkExtensionSettingsPermissions())
    {
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
bool qSlicerExtensionsManagerModelPrivate::uninstallExtension(const QString& extensionName)
{
  Q_Q(qSlicerExtensionsManagerModel);

  if (!this->checkExtensionSettingsPermissions())
    {
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

  if (success)
    {
    this->removeExtensionSettings(extensionName);
    this->removeExtensionFromScheduledForUninstallList(extensionName);
    }

  emit q->extensionUninstalled(extensionName);

  return success;
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
  bool success = true;
  foreach(const QString& extensionName, this->scheduledForUninstallExtensions())
    {
    success = success && d->uninstallExtension(extensionName);
    if(success)
      {
      uninstalledExtensions << extensionName;
      }
    }
  return success;
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
CTK_SET_CPP(qSlicerExtensionsManagerModel, const QString&, setExtensionsSettingsFilePath, ExtensionsSettingsFilePath)

// --------------------------------------------------------------------------
CTK_GET_CPP(qSlicerExtensionsManagerModel, QString, slicerRevision, SlicerRevision)

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::setSlicerRevision(const QString& revision)
{
  Q_D(qSlicerExtensionsManagerModel);
  if (d->SlicerRevision == revision)
    {
    return;
    }
  d->SlicerRevision = revision;

  emit this->slicerRequirementsChanged(d->SlicerRevision, d->SlicerOs, d->SlicerArch);
}

// --------------------------------------------------------------------------
CTK_GET_CPP(qSlicerExtensionsManagerModel, QString, slicerOs, SlicerOs)

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::setSlicerOs(const QString& os)
{
  Q_D(qSlicerExtensionsManagerModel);
  if (d->SlicerOs == os)
    {
    return;
    }
  d->SlicerOs = os;

  emit this->slicerRequirementsChanged(d->SlicerRevision, d->SlicerOs, d->SlicerArch);
}

// --------------------------------------------------------------------------
CTK_GET_CPP(qSlicerExtensionsManagerModel, QString, slicerArch, SlicerArch)

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::setSlicerArch(const QString& arch)
{
  Q_D(qSlicerExtensionsManagerModel);
  if (d->SlicerArch == arch)
    {
    return;
    }
  d->SlicerArch = arch;

  emit this->slicerRequirementsChanged(d->SlicerRevision, d->SlicerOs, d->SlicerArch);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerModel::setSlicerRequirements(const QString& revision, const QString& os, const QString& arch)
{
  Q_D(qSlicerExtensionsManagerModel);
  if (d->SlicerRevision == revision && d->SlicerOs == os && d->SlicerArch == arch)
    {
    return;
    }
  d->SlicerRevision = revision;
  d->SlicerOs = os;
  d->SlicerArch = arch;

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
CTK_SET_CPP(qSlicerExtensionsManagerModel, const QString& , setSlicerVersion, SlicerVersion)

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::isExtensionCompatible(
    const QString& extensionName, const QString& slicerRevision,
    const QString& slicerOs, const QString& slicerArch) const
{
  if (extensionName.isEmpty())
    {
    return QStringList() << tr("extensionName is not specified");
    }
  if (slicerRevision.isEmpty())
    {
    return QStringList() << tr("slicerRevision is not specified");
    }
  if (slicerOs.isEmpty())
    {
    return QStringList() << tr("slicerOs is not specified");
    }
  if (slicerArch.isEmpty())
    {
    return QStringList() << tr("slicerArch is not specified");
    }
  QStringList reasons;
  ExtensionMetadataType metadata = this->extensionMetadata(extensionName);
  QString extensionSlicerRevision = metadata.value("slicer_revision").toString();
  if (slicerRevision != extensionSlicerRevision)
    {
    reasons << tr("extensionSlicerRevision [%1] is different from slicerRevision [%2]").arg(extensionSlicerRevision).arg(slicerRevision);
    }
  QString extensionArch = metadata.value("arch").toString();
  if (slicerArch != extensionArch)
    {
    reasons << tr("extensionArch [%1] is different from slicerArch [%2]").arg(extensionArch).arg(slicerArch);
    }
  QString extensionOs = metadata.value("os").toString();
  if (slicerOs != extensionOs)
    {
    reasons << tr("extensionOs [%1] is different from slicerOs [%2]").arg(extensionOs).arg(slicerOs);
    }
  return reasons;
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
  return serverToExtensionDescriptionKey;
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsManagerModel::serverKeysToIgnore()
{
  return QStringList()
      << "item_id" << "extension_id" << "bitstream_id"
      << "submissiontype" << "codebase" << "package"
      << "size" << "date_creation";
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel::ExtensionMetadataType
qSlicerExtensionsManagerModel::filterExtensionMetadata(const ExtensionMetadataType& extensionMetadata)
{
  QStringList keysToIgnore(Self::serverKeysToIgnore());
  ExtensionMetadataType filteredExtensionMetadata;
  foreach(const QString& key, extensionMetadata.keys())
    {
    if (keysToIgnore.contains(key))
      {
      continue;
      }
    filteredExtensionMetadata.insert(key, extensionMetadata.value(key));
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

#ifdef Q_OS_WIN
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerModel::extractExtensionArchive(
    const QString& extensionName, const QString& archiveFile, const QString& destinationPath)
{
  Q_D(qSlicerExtensionsManagerModel);

  if (extensionName.isEmpty())
    {
    return false;
    }

  if (!QDir(destinationPath).exists())
    {
    d->critical(QString("Failed to extract archive %1 into nonexistent directory %2").arg(archiveFile).arg(destinationPath));
    return false;
    }

  QFileInfo destinationPathInfo(destinationPath);
#ifdef Q_OS_WIN
  ++qt_ntfs_permission_lookup;
#endif
  if (!destinationPathInfo.isReadable()
      || !destinationPathInfo.isWritable()
      || !destinationPathInfo.isExecutable())
    {
    d->critical(QString("Failed to extract archive %1 into directory %2 "
                        "either NON readable, writable or executable").arg(archiveFile).arg(destinationPath));
    return false;
    }
#ifdef Q_OS_WIN
  --qt_ntfs_permission_lookup;
#endif

  QDir extensionsDir(destinationPath);

  ctk::removeDirRecursively(extensionsDir.filePath(extensionName));

  // Make extension output directory
  extensionsDir.mkdir(extensionName);

  // Extract into <extensionsPath>/<extensionName>/<archiveBaseName>/
  extensionsDir.cd(extensionName);
  QString archiveBaseName = d->extractArchive(extensionsDir, archiveFile);
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
  if (!ctk::copyDirRecursively(srcPathToCopy, intermediatePath))
    {
    return false;
    }

  //  Step2: <extensionName>-XXXXXX -> <extensionName>
  if (!ctk::copyDirRecursively(intermediatePath, dstPath))
    {
    return false;
    }

  //  Step3: Remove <extensionName>-XXXXXX
  if (!ctk::removeDirRecursively(intermediatePath))
    {
    return false;
    }

  //  Step4: Remove  <extensionName>/<archiveBaseName>
  if (!ctk::removeDirRecursively(srcPath))
    {
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
