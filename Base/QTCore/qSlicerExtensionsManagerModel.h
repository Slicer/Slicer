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
#include <QUuid>
#include <QVariantMap>

// CTK includes
#include <ctkErrorLogModel.h>

// QtGUI includes
#include "qSlicerBaseQTCoreExport.h"
#include "qSlicerExtensionDownloadTask.h"

class QNetworkReply;
class qSlicerExtensionsManagerModelPrivate;
class QStandardItemModel;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerExtensionsManagerModel : public QObject
{
  Q_OBJECT
  Q_PROPERTY(int numberOfInstalledExtensions READ numberOfInstalledExtensions NOTIFY modelUpdated)
  Q_PROPERTY(int installedExtensionsCount READ installedExtensionsCount NOTIFY modelUpdated)
  Q_PROPERTY(QStringList installedExtensions READ installedExtensions NOTIFY modelUpdated)
  Q_PROPERTY(QStringList enabledExtensions READ enabledExtensions NOTIFY modelUpdated)
  Q_PROPERTY(bool newExtensionEnabledByDefault READ newExtensionEnabledByDefault WRITE setNewExtensionEnabledByDefault NOTIFY newExtensionEnabledByDefaultChanged)
  Q_PROPERTY(bool interactive READ interactive WRITE setInteractive NOTIFY interactiveChanged)
  Q_PROPERTY(QString extensionsSettingsFilePath READ extensionsSettingsFilePath WRITE setExtensionsSettingsFilePath NOTIFY extensionsSettingsFilePathChanged)
  Q_PROPERTY(QString extensionsHistorySettingsFilePath READ extensionsHistorySettingsFilePath WRITE setExtensionsHistorySettingsFilePath NOTIFY extensionsHistorySettingsFilePathChanged)
  Q_PROPERTY(QString slicerRevision READ slicerRevision WRITE setSlicerRevision NOTIFY slicerRevisionChanged)
  Q_PROPERTY(QString slicerOs READ slicerOs WRITE setSlicerOs NOTIFY slicerOsChanged)
  Q_PROPERTY(QString slicerArch READ slicerArch WRITE setSlicerArch NOTIFY slicerArchChanged)
  Q_PROPERTY(QString slicerVersion READ slicerVersion WRITE setSlicerVersion NOTIFY slicerVersionChanged)
public:
  /// Superclass typedef
  typedef QObject Superclass;

  /// Pimpl typedef
  typedef qSlicerExtensionsManagerModelPrivate Pimpl;

  /// Self typedef
  typedef qSlicerExtensionsManagerModel Self;

  /// Constructor
  explicit qSlicerExtensionsManagerModel(QObject* parent = nullptr);

  /// Destructor
  ~qSlicerExtensionsManagerModel() override;

  /// \brief Extension metadata typedef
  typedef QVariantMap ExtensionMetadataType;

  enum ServerAPI
    {
    Midas_v1 = 0,
    Girder_v1,
    ServerAPI_Last
    };

  /// Return current serverAPI.
  ///
  /// \sa serverAPIFromString()
  int serverAPI() const;

  /// Return a string representation of the \a serverAPI.
  ///
  /// \sa serverAPIFromString()
  static QString serverAPIToString(int serverAPI);

  /// Return ServerAPI from the string \a str.
  ///
  /// \sa serverAPIToString()
  static int serverAPIFromString(const QString& str);

  /// \brief Backend server URL used to perform API calls.
  ///
  /// If set, it returns the value associated with `SLICER_EXTENSIONS_MANAGER_SERVER_URL`
  /// environment variable. Otherwise, it returns the extensions settings value `Extensions/ServerUrl`.
  ///
  /// \sa extensionsSettingsFilePath()
  Q_INVOKABLE QUrl serverUrl()const;

  /// \brief Frontend server URL displaying extension manager web page.
  ///
  /// If set, it returns the value associated with `SLICER_EXTENSIONS_MANAGER_FRONTEND_SERVER_URL`
  /// environment variable. Otherwise, it returns the extensions settings value `Extensions/FrontendServerUrl`.
  ///
  /// \sa extensionsSettingsFilePath()
  Q_INVOKABLE QUrl frontendServerUrl()const;

  Q_INVOKABLE QString extensionsInstallPath()const;

  Q_INVOKABLE QString extensionInstallPath(const QString& extensionName) const;

  Q_INVOKABLE QStringList extensionModulePaths(const QString& extensionName)const;

  Q_INVOKABLE QString extensionDescriptionFile(const QString& extensionName) const;

  void setNewExtensionEnabledByDefault(bool value);
  bool newExtensionEnabledByDefault()const;

  /// If set to true (by default) then the user may be asked to confirm installation of additional dependencies.
  /// If set to false then no blocking popups are displayed and dependencies are installed automatically.
  void setInteractive(bool value);
  bool interactive()const;

  Q_INVOKABLE ExtensionMetadataType extensionMetadata(const QString& extensionName)const;

  /// \brief Return True if the \a extensionName is installed
  /// \sa installExtension, installedExtensionsCount, installedExtensions, extensionInstalled
  Q_INVOKABLE bool isExtensionInstalled(const QString& extensionName)const;

  /// \brief Return the number of installed extensions
  /// \sa installExtension, isExtensionInstalled, installedExtensions, extensionInstalled
  int installedExtensionsCount()const;

  /// \brief Return the number of installed extensions. Deprecated, use installedExtensionsCount instead.
  /// \sa installExtension, isExtensionInstalled, installedExtensions, extensionInstalled
  int numberOfInstalledExtensions()const;

  /// \brief Return names of all installed extensions
  /// \sa installExtension, installedExtensionsCount, isExtensionInstalled, extensionInstalled
  QStringList installedExtensions()const;

  /// \brief Return True if the \a extensionName is enabled
  /// \sa setExtensionEnabled, extensionEnabledChanged, enabledExtensions
  Q_INVOKABLE bool isExtensionEnabled(const QString& extensionName)const;

  /// Get the names of all extensions scheduled for update.
  ///
  /// \sa scheduleExtensionForUpdate, isExtensionScheduledForUpdate,
  ///     extensionScheduledForUpdate
  QStringList scheduledForUpdateExtensions() const;

  /// Check if an update is known to be available for the specified extension.
  ///
  /// \return \c true if a previous check for updates has determined that an
  ///         update is available for the specified extension.
  ///
  /// \sa checkForUpdates
  Q_INVOKABLE bool isExtensionUpdateAvailable(const QString& extensionName)const;

  /// Test if extension is scheduled to be updated.
  ///
  /// \return \c true if \p extensionName is scheduled to be updated.
  ///
  /// \sa updateScheduledExtensions();
  Q_INVOKABLE bool isExtensionScheduledForUpdate(const QString& extensionName)const;

  /// \brief Return names of all extensions scheduled for uninstall
  /// \sa scheduleExtensionForUninstall, isExtensionScheduledForUninstall, extensionScheduledForUninstall
  QStringList scheduledForUninstallExtensions() const;

  /// \brief Return True if the \a extensionName is scheduled to be uninstalled
  /// \sa uninstallScheduledExtensions();
  Q_INVOKABLE bool isExtensionScheduledForUninstall(const QString& extensionName)const;

  /// \brief Return names of all enabled extensions
  /// \sa setExtensionEnabled, extensionEnabledChanged, isExtensionEnabled
  QStringList enabledExtensions()const;

  /// \brief Set/Get extension settings file path.
  ///
  /// Signal extensionsSettingsFilePathChanged() is emitted when a new path is set.
  QString extensionsSettingsFilePath()const;
  void setExtensionsSettingsFilePath(const QString& extensionsSettingsFilePath);

  /// \brief Set/Get extension history settings file path.
  ///
  /// Signal extensionsHistorySettingsFilePathChanged() is emitted when a new path is set.
  QString extensionsHistorySettingsFilePath()const;
  void setExtensionsHistorySettingsFilePath(const QString& extensionsHistorySettingsFilePath);

  QVariantMap extensionsHistoryInformation()const;

  /// \brief Set/Get Slicer revision.
  ///
  /// Signal slicerRevisionChanged() is emitted when a revision is set.
  QString slicerRevision()const;
  void setSlicerRevision(const QString& revision);

  /// \brief Set/Get Slicer operating system.
  ///
  /// Signal slicerOsChanged() is emitted when a new operating system is set.
  QString slicerOs()const;
  void setSlicerOs(const QString& os);

  /// \brief Set/Get Slicer architecture.
  ///
  /// Signal slicerArchChanged() is emitted when a new architecture is set.
  QString slicerArch()const;
  void setSlicerArch(const QString& arch);

  /// \brief Convenience function setting Slicer revision, operating system and architecture.
  ///
  /// Signal slicerRevisionChanged(), slicerArchChanged() and slicerArchChanged() are emitted
  /// only if the corresponding value is updated.
  ///
  /// The, signal slicerRequirementsChanged() is emitted only once it at least one of the
  /// three properties has been updated.
  void setSlicerRequirements(const QString& revision, const QString& os, const QString& arch);

  QString slicerVersion()const;
  void setSlicerVersion(const QString& version);

  /// \brief Check if \a extensionName is compatible with the system identified
  /// by \a slicerRevision, \a slicerOs and \a slicerArch.
  /// @return Return the reasons justifying the incompatibility or an empty list if the extension
  /// is compatible.
  Q_INVOKABLE QStringList isExtensionCompatible(const QString& extensionName, const QString& slicerRevision,
                                                const QString& slicerOs, const QString& slicerArch) const;

  /// \brief Check if \a extensionName is compatible.
  /// An extension is considered incompatible when the version of Slicer used
  /// to build the extension is different from the version of Slicer attempting
  /// to load the extension.
  /// \sa isExtensionCompatible(const QString&, const QString&, const QString&)
  /// \sa setSlicerRevision, setSlicerOs, setSlicerArch, setSlicerRequirements
  Q_INVOKABLE QStringList isExtensionCompatible(const QString& extensionName) const;

  /// \brief Query the extension server and retrieve the metadata associated with \a extensionId
  /// \sa setServerUrl
  Q_INVOKABLE ExtensionMetadataType retrieveExtensionMetadata(const QString& extensionId);

  /// \brief Query the extension server and retrieve the metadata associated with \a extensionName
  /// \sa setServerUrl
  Q_INVOKABLE ExtensionMetadataType retrieveExtensionMetadataByName(const QString& extensionName);

  /// Install extension from the specified archive file.
  ///
  /// This attempts to install an extension given only the archive file
  /// containing the extension. The archive file is inspected in order to
  /// determine the extension name.
  ///
  /// \sa installExtension(const QString&,ExtensionMetadataType,const QString&)
  Q_INVOKABLE bool installExtension(const QString &archiveFile);

  /// Install extension.
  ///
  /// This attempts to install an extension with the specified name and
  /// metadata from the specified archive file. If the metadata is empty, the
  /// metadata from the extension description contained in the archive is used.
  ///
  /// \sa downloadExtension, isExtensionScheduledForUninstall, extensionScheduledForUninstall
  Q_INVOKABLE bool installExtension(const QString& extensionName,
                                    ExtensionMetadataType extensionMetadata,
                                    const QString &archiveFile);

  /// \brief Extract \a archiveFile into \a destinationPath/extensionName directory
  Q_INVOKABLE bool extractExtensionArchive(const QString& extensionName,
                                           const QString& archiveFile,
                                           const QString &destinationPath);

  /// Return the item model used internally
  Q_INVOKABLE const QStandardItemModel * model()const;

  static QHash<QString, QString> serverToExtensionDescriptionKey(int serverAPI = Self::Midas_v1);

  static QStringList serverKeysToIgnore(int serverAPI = Self::Midas_v1);

  static ExtensionMetadataType filterExtensionMetadata(const ExtensionMetadataType &extensionMetadata, int serverAPI = Self::Midas_v1);

  static QStringList readArrayValues(QSettings& settings,
                                     const QString& arrayName, const QString fieldName);

  static void writeArrayValues(QSettings& settings, const QStringList& values,
                               const QString& arrayName, const QString fieldName);

  static bool writeExtensionDescriptionFile(const QString& file,
                                            const ExtensionMetadataType& metadata);

  static ExtensionMetadataType parseExtensionDescriptionFile(const QString& file);

public slots:

  /// \brief Enable or disable an extension.
  /// Tell the application to load (or skip the loading) of \a extensionName
  /// by adding (or removing) all associated module paths to the application settings.
  void setExtensionEnabled(const QString& extensionName, bool value);

  /// \brief Download and install \a extensionId
  /// The \a extensionId corresponds to the identifier used on the extension server itself.
  /// \sa installExtension, scheduleExtensionForUninstall, uninstallScheduledExtensions
  bool downloadAndInstallExtension(const QString& extensionId);

  /// \brief Schedule \a extensionName of uninstall
  /// Tell the application to uninstall \a extensionName when it will restart
  /// An extension scheduled for uninstall can be effectively uninstalled by calling
  /// uninstallScheduledExtensions()
  /// \sa isExtensionScheduledForUninstall, uninstallScheduledExtensions
  bool scheduleExtensionForUninstall(const QString& extensionName);

  /// \brief Cancel the uninstallation of \a extensionName
  /// Tell the application to keep \a extensionName installed
  /// \sa scheduleExtensionForUninstall
  bool cancelExtensionScheduledForUninstall(const QString& extensionName);

  /// Check for updates to installed extensions.
  ///
  /// This checks each installed extension to see if it is the latest version.
  /// If \p installUpdates is \c true, available updates will be automatically
  /// scheduled for installation.
  void checkForUpdates(bool installUpdates);

  /// Schedule \p extensionName to be updated (reinstalled).
  ///
  /// This records \p extensionName in the list of extensions scheduled to be
  /// updated (which is done by reinstalling the extension at next startup).
  ///
  /// \sa isExtensionScheduledForUpdate, updateScheduledExtensions
  bool scheduleExtensionForUpdate(const QString& extensionName);

  /// \brief Cancel the uninstallation of \a extensionName
  /// Tell the application to keep \a extensionName installed
  /// \sa scheduleExtensionForUninstall
  bool cancelExtensionScheduledForUpdate(const QString& extensionName);

  /// Update extensions scheduled for update.
  ///
  /// \param updatedExtensions
  ///   QStringList which received the list of extensions which are
  ///   successfully updated.
  /// \return \c true if all scheduled extensions are successfully updated.
  ///
  /// \sa scheduleExtensionForUpdate, isExtensionScheduledForUpdate
  bool updateScheduledExtensions(QStringList &updatedExtensions);

  /// Update extensions scheduled for update.
  ///
  /// \return \c true if all scheduled extensions are successfully updated.
  ///
  /// \sa scheduleExtensionForUpdate, isExtensionScheduledForUpdate
  bool updateScheduledExtensions();

  /// Uninstall extensions scheduled for uninstall.
  ///
  /// \param uninstalledExtensions
  ///   QStringList which received the list of extensions which are
  ///   successfully uninstalled.
  /// \return \c true if all scheduled extensions are successfully uninstalled.
  ///
  /// \sa scheduleExtensionForUninstall, isExtensionScheduledForUninstall,
  bool uninstallScheduledExtensions(QStringList &uninstalledExtensions);

  /// Uninstall extensions scheduled for uninstall.
  ///
  /// \return \c true if all scheduled extensions are successfully uninstalled.
  ///
  /// \sa scheduleExtensionForUninstall, isExtensionScheduledForUninstall,
  bool uninstallScheduledExtensions();

  void gatherExtensionsHistoryInformationOnStartup();

  QVariantMap getExtensionHistoryInformation();

  void identifyIncompatibleExtensions();

  bool exportExtensionList(QString& exportFilePath);

  QStringList checkInstallPrerequisites() const;

  void updateModel();

  /// Number of operations in progress
  QStringList activeTasks() const;

signals:

  void downloadStarted(QNetworkReply * reply);

  void downloadFinished(QNetworkReply * reply);

  void updateDownloadProgress(const QString& extensionName,
                              qint64 received, qint64 total);

  void modelUpdated();

  void extensionUpdateAvailable(const QString& extensionName);

  void extensionInstalled(const QString& extensionName);

  void extensionUpdated(const QString& extensionName);

  void extensionScheduledForUninstall(const QString& extensionName);

  void extensionCancelledScheduleForUninstall(const QString& extensionName);

  void extensionScheduledForUpdate(const QString& extensionName);

  void extensionCancelledScheduleForUpdate(const QString& extensionName);

  void extensionUninstalled(const QString& extensionName);

  void extensionEnabledChanged(const QString& extensionName, bool value);

  void extensionIdentifedAsIncompatible(const QString& extensionName);

  void newExtensionEnabledByDefaultChanged(bool value);

  void slicerRequirementsChanged(const QString& revision, const QString& os, const QString& arch);
  void slicerArchChanged(const QString& slicerArch);
  void slicerOsChanged(const QString& slicerOs);
  void slicerRevisionChanged(const QString& slicerRevision);

  void interactiveChanged(bool interactive);

  void slicerVersionChanged(const QString& slicerVersion);

  void messageLogged(const QString& text, ctkErrorLogLevel::LogLevels level) const;

  void extensionHistoryGatheredOnStartup(const QVariantMap&);

  void installDownloadProgress(const QString& extensionName, qint64 received, qint64 total);

  void extensionsSettingsFilePathChanged(const QString& extensionsSettingsFilePath);

  void extensionsHistorySettingsFilePathChanged(const QString& extensionsHistorySettingsFilePath);

protected slots:

  void onInstallDownloadProgress(qSlicerExtensionDownloadTask* task, qint64 received, qint64 total);

  /// \sa downloadAndInstallExtension
  void onInstallDownloadFinished(qSlicerExtensionDownloadTask* task);

  /// \sa scheduleExtensionForUpdate
  void onUpdateDownloadFinished(qSlicerExtensionDownloadTask* task);

  void onUpdateDownloadProgress(qSlicerExtensionDownloadTask* task,
                                qint64 received, qint64 total);

  void onUpdateCheckFinished(const QUuid& requestId);

  void onUpdateCheckComplete(const QUuid& requestId,
                             const QList<QVariantMap>& results);
  void onUpdateCheckFailed(const QUuid& requestId);

protected:
  QScopedPointer<qSlicerExtensionsManagerModelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerExtensionsManagerModel);
  Q_DISABLE_COPY(qSlicerExtensionsManagerModel);
};

Q_DECLARE_METATYPE(qSlicerExtensionsManagerModel::ServerAPI);

// Metatype already declared in qSlicerIO.h
//Q_DECLARE_METATYPE(qSlicerExtensionsManagerModel::ExtensionMetadataType)

#endif
