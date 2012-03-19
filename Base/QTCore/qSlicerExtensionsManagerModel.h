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

#ifndef __qSlicerExtensionsManagerModel_h
#define __qSlicerExtensionsManagerModel_h

// Qt includes
#include <QHash>
#include <QStringList>
#include <QUrl>
#include <QSettings>
#include <QVariantMap>

// QtGUI includes
#include "qSlicerBaseQTCoreExport.h"

class QNetworkReply;
class qSlicerExtensionsManagerModelPrivate;
class QStandardItemModel;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerExtensionsManagerModel : public QObject
{
  Q_OBJECT
  Q_PROPERTY(int numberOfInstalledExtensions READ numberOfInstalledExtensions)
  Q_PROPERTY(QStringList installedExtensions READ installedExtensions)
  Q_PROPERTY(QStringList enabledExtensions READ enabledExtensions)
  Q_PROPERTY(QUrl serverUrl READ serverUrl)
  Q_PROPERTY(QUrl serverUrlWithPackagePath READ serverUrlWithPackagePath)
  Q_PROPERTY(QUrl serverUrlWithExtensionsStorePath READ serverUrlWithExtensionsStorePath)
  Q_PROPERTY(QString extensionsInstallPath READ extensionsInstallPath)
  Q_PROPERTY(bool newExtensionEnabledByDefault READ newExtensionEnabledByDefault WRITE setNewExtensionEnabledByDefault)
  Q_PROPERTY(QString launcherSettingsFilePath READ launcherSettingsFilePath WRITE setLauncherSettingsFilePath)
  Q_PROPERTY(QString slicerRevision READ slicerRevision WRITE setSlicerRevision)
  Q_PROPERTY(QString slicerOs READ slicerOs WRITE setSlicerOs)
  Q_PROPERTY(QString slicerArch READ slicerArch WRITE setSlicerArch)
  Q_PROPERTY(QString slicerVersion READ slicerVersion WRITE setSlicerVersion)
public:
  /// Superclass typedef
  typedef QObject Superclass;

  /// Pimpl typedef
  typedef qSlicerExtensionsManagerModelPrivate Pimpl;

  /// Self typedef
  typedef qSlicerExtensionsManagerModel Self;

  /// Constructor
  explicit qSlicerExtensionsManagerModel(QObject* parent = 0);

  /// Destructor
  virtual ~qSlicerExtensionsManagerModel();

  /// \brief Extension metadata typedef
  typedef QVariantMap ExtensionMetadataType;

  QUrl serverUrl()const;
  QUrl serverUrlWithPackagePath()const;
  QUrl serverUrlWithExtensionsStorePath()const;

  QString extensionsInstallPath()const;

  Q_INVOKABLE QString extensionInstallPath(const QString& extensionName) const;

  Q_INVOKABLE QStringList extensionModulePaths(const QString& extensionName)const;

  Q_INVOKABLE QString extensionDescriptionFile(const QString& extensionName) const;

  void setNewExtensionEnabledByDefault(bool value);
  bool newExtensionEnabledByDefault()const;

  Q_INVOKABLE ExtensionMetadataType extensionMetadata(const QString& extensionName)const;

  /// \brief Return True if the \a extensionName is installed
  /// \sa installExtension, numberOfInstalledExtensions, installedExtensions, extensionInstalled
  Q_INVOKABLE bool isExtensionInstalled(const QString& extensionName)const;

  /// \brief Return the number of installed extensions
  /// \sa installExtension, isExtensionInstalled, installedExtensions, extensionInstalled
  int numberOfInstalledExtensions()const;

  /// \brief Return names of all installed extensions
  /// \sa installExtension, numberOfInstalledExtensions, isExtensionInstalled, extensionInstalled
  QStringList installedExtensions()const;

  /// \brief Return True if the \a extensionName is enabled
  /// \sa setExtensionEnabled, extensionEnabledChanged, enabledExtensions
  Q_INVOKABLE bool isExtensionEnabled(const QString& extensionName)const;

  /// \brief Return names of all enabled extensions
  /// \sa setExtensionEnabled, extensionEnabledChanged, isExtensionEnabled
  QStringList enabledExtensions()const;

  QString launcherSettingsFilePath()const;
  void setLauncherSettingsFilePath(const QString& launcherSettingsFilePath);

  QString slicerRevision()const;
  void setSlicerRevision(const QString& revision);

  QString slicerOs()const;
  void setSlicerOs(const QString& os);

  QString slicerArch()const;
  void setSlicerArch(const QString& arch);

  void setSlicerRequirements(const QString& revision, const QString& os, const QString& arch);

  QString slicerVersion()const;
  void setSlicerVersion(const QString& version);

  Q_INVOKABLE void imcompatibleExtensions() const;

  /// \brief Check if \a extensionName is compatibile with the system identified
  /// by \a slicerRevision, \a slicerOs and \a slicerArch.
  /// @return Return the reason justifying the incompatibility or an empty string if the extension
  /// is compatibile.
  Q_INVOKABLE QString isExtensionCompatible(const QString& extensionName, const QString& slicerRevision,
                                            const QString& slicerOs, const QString& slicerArch) const;

  /// \brief Query the extension server and retrieve the metadata associated with \a extensionId
  /// \sa setServerUrl
  Q_INVOKABLE ExtensionMetadataType retrieveExtensionMetadata(const QString& extensionId);

  /// \sa downloadExtension, uninstallExtension
  Q_INVOKABLE bool installExtension(const QString& extensionName,
                                    const ExtensionMetadataType &extensionMetadata,
                                    const QString &archiveFile);

  /// Return the item model used internally
  Q_INVOKABLE const QStandardItemModel * model()const;

  static QStringList serverKeysToIgnore();

  static ExtensionMetadataType filterExtensionMetadata(const ExtensionMetadataType &extensionMetadata);

  static QStringList readArrayValues(QSettings& settings,
                                     const QString& arrayName, const QString fieldName);

  static void writeArrayValues(QSettings& settings, const QStringList& values,
                               const QString& arrayName, const QString fieldName);

  static bool extractExtensionArchive(const QString& extensionName,
                                      const QString& archiveFile,
                                      const QString &destinationPath);

  static bool writeExtensionDescriptionFile(const QString& file,
                                            const ExtensionMetadataType& metadata);

  static ExtensionMetadataType parseExtensionDescriptionFile(const QString& file);

public slots:

  void setExtensionEnabled(const QString& extensionName, bool value);

  /// \brief Download and install \a extensionId
  /// The \a extensionId correponds to the identifier used on the extension server itself.
  /// \sa installExtension, uninstallExtension
  void downloadAndInstallExtension(const QString& extensionId);

  /// \brief Uninstall \a extensionName
  /// \note The directory containing the extension will be deleted.
  /// \sa downloadExtension, installExtension
  bool uninstallExtension(const QString& extensionName);

  bool exportExtensionList(QString& exportFilePath);

  void updateModel();

signals:

  void modelUpdated();

  void extensionInstalled(const QString& extensionName);

  void extensionUninstalled(const QString& extensionName);

  void extensionEnabledChanged(const QString& extensionName, bool value);

  void slicerRequirementsChanged(const QString& revision, const QString& os, const QString& arch);

protected slots:

  /// \sa downloadAndInstallExtension
  void onDownloadFinished(QNetworkReply* reply);

protected:
  QScopedPointer<qSlicerExtensionsManagerModelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerExtensionsManagerModel);
  Q_DISABLE_COPY(qSlicerExtensionsManagerModel);
};

// Metatype already declared in qSlicerIO.h
//Q_DECLARE_METATYPE(qSlicerExtensionsManagerModel::ExtensionMetadataType)

#endif
