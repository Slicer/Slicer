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

#ifndef __qSlicerApplicationUpdateManager_h
#define __qSlicerApplicationUpdateManager_h

// Qt includes
#include <QObject>
#include <QUrl>
#include <QUuid>

// QtGUI includes
#include "qSlicerBaseQTCoreExport.h"

class qSlicerApplicationUpdateManagerPrivate;

/// \brief Class querying application updates on the download server
///
/// Automatic update check is enabled/disabled by ApplicationUpdates/Enabled application setting,
/// enabled by default.
///
/// Application update feature can be excluded from the build by setting
/// CMake variable Slicer_BUILD_APPLICATIONUPDATE_SUPPORT=OFF.
///
class Q_SLICER_BASE_QTCORE_EXPORT qSlicerApplicationUpdateManager : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString slicerRevision READ slicerRevision)
  Q_PROPERTY(QString slicerOs READ slicerOs)
  Q_PROPERTY(QString slicerArch READ slicerArch)
  /// Periodically check for updates on the server.
  Q_PROPERTY(bool autoUpdateCheck READ autoUpdateCheck WRITE setAutoUpdateCheck NOTIFY autoUpdateCheckChanged)
  Q_PROPERTY(bool isUpdateAvailable READ isUpdateAvailable NOTIFY updateAvailable)
  Q_PROPERTY(QString latestReleaseVersion READ latestReleaseVersion)
  Q_PROPERTY(QUrl applicationDownloadPageUrl READ applicationDownloadPageUrl)

public:
  /// Superclass typedef
  typedef QObject Superclass;

  /// Pimpl typedef
  typedef qSlicerApplicationUpdateManagerPrivate Pimpl;

  /// Self typedef
  typedef qSlicerApplicationUpdateManager Self;

  /// Constructor
  explicit qSlicerApplicationUpdateManager(QObject* parent = nullptr);

  /// Destructor
  ~qSlicerApplicationUpdateManager() override;

  /// \brief Application update server URL used to perform API calls.
  ///
  /// If set, it returns the value from application settings value `ApplicationUpdate/ServerUrl`.
  ///
  /// It is set to `https://download.slicer.org` by default.
  ///
  /// For local testing during development:
  ///
  /// - download the json file from `https://download.slicer.org/find?os=win&stability=release`
  /// - save this json file with the filename `find` in a local folder
  /// - specify this local folder (such as `file:///c:/temp/slicerupdatetest/`) as `Update server URL` in application
  /// settings.
  ///
  Q_INVOKABLE QUrl serverUrl() const;

  bool autoUpdateCheck() const;

  /// \brief Check for updates. Reads the last cached value if recently queries from the server.
  /// Emits updateAvailable(bool) signal if update has become available/unavailable.
  /// \param force set to true to force query from the server.
  /// \param waitForCompletion set to true to wait for the server response
  /// \return if waitForCompletion is enabled then this returns true if query was successful
  bool checkForUpdate(bool force, bool waitForCompletion);

  /// Return time of last successful update of extensions metadata from the server.
  /// If there has not been any updates then it the object is set to null.
  QDateTime lastUpdateCheckTime() const;

  /// \brief Return True if an updated version is available for the application
  /// Does not call checkForUpdate().
  bool isUpdateAvailable() const;

  /// Return latest application release version (e.g., 5.0.3)
  QString latestReleaseVersion() const;

  /// Return latest application release version (e.g., 30893)
  QString latestReleaseRevision() const;

  /// Return URL string of the application download website
  /// (html page that can be displayed for the user).
  QUrl applicationDownloadPageUrl() const;

  /// Get Slicer revision.
  /// \sa setSlicerRequirements
  QString slicerRevision() const;

  /// Get Slicer operating system.
  /// \sa setSlicerRequirements
  QString slicerOs() const;

  /// Get Slicer architecture.
  /// \sa setSlicerRequirements
  QString slicerArch() const;

  /// \brief Set Slicer revision, operating system and architecture.
  ///
  /// Signal slicerRevisionChanged(), slicerArchChanged() and slicerArchChanged() are emitted
  /// only if the corresponding value is updated.
  ///
  /// The slicerRequirementsChanged() signal is emitted only once it at least one of the
  /// three properties has been updated.
  ///
  /// The updateAvailable(bool) signal is emitted with the update availability
  /// determined based on the new requirements.
  void setSlicerRequirements(const QString& revision, const QString& os, const QString& arch);

  /// Return true if application update is enabled in application settings
  /// (ApplicationUpdates/Enabled). Enabled by default.
  static bool isApplicationUpdateEnabled();

public slots:

  /// Checks for application update (server query is not forced, does not wait for completion)
  /// \sa checkForUpdate(bool, bool)
  void checkForUpdate();

  /// Enable/disable automatic periodic update checks on the server.
  void setAutoUpdateCheck(bool enable);

signals:

  /// Emitted when application update availability has changed
  /// or an update check has been completed.
  void updateAvailable(bool isAvailable);

  /// Emitted when availability update check is completed.
  void updateCheckCompleted(bool success);

  /// @{
  /// Emitted when Slicer application revision, operating system, or architecture is set.
  void slicerRequirementsChanged(const QString& revision, const QString& os, const QString& arch);
  /// @}

  /// Emitted when autoUpdateCheck property is changed
  void autoUpdateCheckChanged();

protected slots:

  bool onReleaseInfoQueryFinished(const QUuid& requestId);
  void refreshUpdateAvailable();

protected:
  QScopedPointer<qSlicerApplicationUpdateManagerPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerApplicationUpdateManager);
  Q_DISABLE_COPY(qSlicerApplicationUpdateManager);
};

#endif
