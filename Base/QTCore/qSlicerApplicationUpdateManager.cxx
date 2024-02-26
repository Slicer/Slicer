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
#include <QDebug>
#include <QNetworkAccessManager>
#include <QScopedPointer>
#include <QJSEngine>
#include <QJSValue>
#include <QSettings>
#include <QUrl>

// CTK includes
#include <ctkPimpl.h>
#include <ctkUtils.h>

// qRestAPI includes
#include <qRestAPI.h>
#include <qRestResult.h>

// QtCore includes
#include "qSlicerCoreApplication.h"
#include "qSlicerApplicationUpdateManager.h"
#include "vtkSlicerConfigure.h"

// MRML includes
#include "vtkArchive.h"

// --------------------------------------------------------------------------
// qSlicerApplicationUpdateManagerPrivate methods

//-----------------------------------------------------------------------------
class qSlicerApplicationUpdateManagerPrivate
{
  Q_DECLARE_PUBLIC(qSlicerApplicationUpdateManager);

protected:
  qSlicerApplicationUpdateManager* const q_ptr;

  typedef qSlicerApplicationUpdateManagerPrivate Self;
  qSlicerApplicationUpdateManagerPrivate(qSlicerApplicationUpdateManager& object);
  void init();

  /// Returns true it is due to check if there are any updates for managed extensions.
  bool isUpdateCheckDue() const;

  static bool validateReleaseInfo(const QVariantMap& releaseInfo);

  // By default enable checking of the download server for updates.
  // This is according to common practices in all operating systems
  // most software (including venerable open source apps like vim).
  // Anyone who is particularly concerned is expected to look
  // for ways to disable automatic updates.
  bool AutoUpdateCheck{ true };

  QString SlicerRevision;
  QString SlicerOs;
  QString SlicerArch;

  QNetworkAccessManager NetworkManager;
  qRestAPI ReleaseInfoAPI;
  QUuid ReleaseInfoQueryUID; // if not null then it means that query is in progress

  // ReleaseInfo query results
  QString LatestReleaseVersion;
  QString LatestReleaseRevision;
  bool UpdateAvailable{ false };
};

// --------------------------------------------------------------------------
qSlicerApplicationUpdateManagerPrivate::qSlicerApplicationUpdateManagerPrivate(qSlicerApplicationUpdateManager& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerApplicationUpdateManagerPrivate::init()
{
  Q_Q(qSlicerApplicationUpdateManager);

  QSettings settings;
  this->AutoUpdateCheck = settings.value("ApplicationUpdate/AutoUpdateCheck", this->AutoUpdateCheck).toBool();

  QObject::connect(q, SIGNAL(slicerRequirementsChanged(QString, QString, QString)), q, SLOT(refreshUpdateAvailable()));

  QObject::connect(&this->ReleaseInfoAPI, SIGNAL(finished(QUuid)), q, SLOT(onReleaseInfoQueryFinished(QUuid)));
}

// --------------------------------------------------------------------------
bool qSlicerApplicationUpdateManagerPrivate::isUpdateCheckDue() const
{
  Q_Q(const qSlicerApplicationUpdateManager);

  // stores common settings
  if (!this->AutoUpdateCheck)
  {
    return false;
  }

  // stores time of last update check for this Slicer installation
  QSettings settings;
  if (!settings.contains("ApplicationUpdate/LastUpdateCheckTime"))
  {
    // there has never been an update check
    return true;
  }

  QString lastUpdateCheckTimeStr = settings.value("ApplicationUpdate/LastUpdateCheckTime").toString();
  QDateTime lastUpdateCheckTime = QDateTime::fromString(lastUpdateCheckTimeStr, Qt::ISODate);
  QDateTime currentTime = QDateTime::currentDateTimeUtc();
  // By default, we check for updates once a day
  int updateFrequencyMinutes = settings.value("ApplicationUpdate/AutoUpdateFrequencyMinutes", 24 * 60).toInt();
  qint64 updateFrequencyMsec = qint64(updateFrequencyMinutes) * qint64(60000);
  if (lastUpdateCheckTime.msecsTo(currentTime) < updateFrequencyMsec)
  {
    // not enough time has passed since the last update check
    return false;
  }

  return true;
}

// --------------------------------------------------------------------------
bool qSlicerApplicationUpdateManagerPrivate::validateReleaseInfo(const QVariantMap& releaseInfo)
{
  // Example:
  //  {
  //    "arch": "amd64",
  //    "build_date" : "2022-07-08T09:39:54.992000+00:00",
  //    "build_date_ymd" : "2022-07-08",
  //    "checkout_date" : null,
  //    "checkout_date_ymd" : null,
  //    "codebase" : null,
  //    "download_url" : "/bitstream/62d5d2ebe911182f1dc285b0",
  //    "md5" : null,
  //    "name" : "Slicer_win_amd64_30893",
  //    "os" : "win",
  //    "package" : null,
  //    "product_name" : "Slicer",
  //    "revision" : "30893",
  //    "size" : 222149208,
  //    "stability" : "release",
  //    "version" : "5.0.3"
  //  }
  bool valid = true;
  QStringList requiredNonEmptyKeys;
  requiredNonEmptyKeys << "arch"
                       << "build_date"
                       << "download_url"
                       << "product_name"
                       << "revision"
                       << "version";
  foreach (const QString& key, requiredNonEmptyKeys)
  {
    if (releaseInfo.value(key).toString().isEmpty())
    {
      qWarning() << Q_FUNC_INFO << " failed: required key '" << key << "' is missing from release info.";
      valid = false;
    }
  }
  return valid;
}

// --------------------------------------------------------------------------
// qSlicerApplicationUpdateManager methods

CTK_GET_CPP(qSlicerApplicationUpdateManager, QString, slicerRevision, SlicerRevision)
CTK_GET_CPP(qSlicerApplicationUpdateManager, QString, slicerOs, SlicerOs)
CTK_GET_CPP(qSlicerApplicationUpdateManager, QString, slicerArch, SlicerArch)

CTK_GET_CPP(qSlicerApplicationUpdateManager, bool, isUpdateAvailable, UpdateAvailable);
CTK_GET_CPP(qSlicerApplicationUpdateManager, QString, latestReleaseVersion, LatestReleaseVersion);
CTK_GET_CPP(qSlicerApplicationUpdateManager, QString, latestReleaseRevision, LatestReleaseRevision);

// --------------------------------------------------------------------------
qSlicerApplicationUpdateManager::qSlicerApplicationUpdateManager(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerApplicationUpdateManagerPrivate(*this))
{
  Q_D(qSlicerApplicationUpdateManager);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerApplicationUpdateManager::~qSlicerApplicationUpdateManager() = default;

// --------------------------------------------------------------------------
QUrl qSlicerApplicationUpdateManager::serverUrl() const
{
  QSettings settings;
  return QUrl(settings.value("ApplicationUpdate/ServerUrl").toString());
}

// --------------------------------------------------------------------------
void qSlicerApplicationUpdateManager::setAutoUpdateCheck(bool enable)
{
  Q_D(qSlicerApplicationUpdateManager);
  if (d->AutoUpdateCheck == enable)
  {
    return;
  }
  d->AutoUpdateCheck = enable;
  QSettings settings;
  settings.setValue("ApplicationUpdate/AutoUpdateCheck", enable);
  emit this->autoUpdateCheckChanged();
}

// --------------------------------------------------------------------------
bool qSlicerApplicationUpdateManager::autoUpdateCheck() const
{
  Q_D(const qSlicerApplicationUpdateManager);
  return d->AutoUpdateCheck;
}

// --------------------------------------------------------------------------
void qSlicerApplicationUpdateManager::checkForUpdate()
{
  this->checkForUpdate(false, false);
}

// --------------------------------------------------------------------------
bool qSlicerApplicationUpdateManager::checkForUpdate(bool force, bool waitForCompletion)
{
  Q_D(qSlicerApplicationUpdateManager);

  QSettings settings;
  QString cachedServerUrl = settings.value("ApplicationUpdate/LastUpdateCheckUrl").toString();
  if (cachedServerUrl.isEmpty() || cachedServerUrl != this->serverUrl().toString())
  {
    // no metadata cached for this server URL
    d->LatestReleaseVersion.clear();
    d->LatestReleaseRevision.clear();
  }
  else
  {
    d->LatestReleaseVersion = settings.value("ApplicationUpdate/LatestReleaseVersion").toString();
    d->LatestReleaseRevision = settings.value("ApplicationUpdate/LatestReleaseRevision").toString();
  }

  // Contact the server if cached data is stale
  if (!force && !d->isUpdateCheckDue())
  {
    refreshUpdateAvailable();
    return true;
  }

  if (d->ReleaseInfoQueryUID.isNull())
  {
    // query is not in progress yet, start it
    qRestAPI::Parameters parameters;
    if (this->serverUrl().toString().isEmpty())
    {
      // server address has not been specified, normal at very first startup
      // (default server address is set up by an application settings page)
      this->refreshUpdateAvailable();
      return false;
    }
    // URL example: https://download.slicer.org/find?os=win&stability=release
    d->ReleaseInfoAPI.setServerUrl(this->serverUrl().toString() + QString("/find"));
    parameters["os"] = this->slicerOs();
    parameters["stability"] = QLatin1String("release");
    // Issue the query
    d->ReleaseInfoQueryUID = d->ReleaseInfoAPI.get("", parameters);
  }

  if (!waitForCompletion)
  {
    return true;
  }

  // Temporarily disable onReleaseInfoQueryFinished call via signal/slot
  // because we'll call it directly to get returned result.
  QObject::disconnect(&d->ReleaseInfoAPI, SIGNAL(finished(QUuid)), this, SLOT(onReleaseInfoQueryFinished(QUuid)));

  bool success = this->onReleaseInfoQueryFinished(d->ReleaseInfoQueryUID);

  QObject::connect(&d->ReleaseInfoAPI, SIGNAL(finished(QUuid)), this, SLOT(onReleaseInfoQueryFinished(QUuid)));

  if (!success)
  {
    qWarning() << Q_FUNC_INFO
               << "Check for available application update failed: timed out while waiting for server response from "
               << d->ReleaseInfoAPI.serverUrl();
  }

  return success;
}

// --------------------------------------------------------------------------
QDateTime qSlicerApplicationUpdateManager::lastUpdateCheckTime() const
{
  QSettings settings;
  if (!settings.contains("ApplicationUpdate/LastUpdateCheckTime"))
  {
    // there has never been an update check
    return QDateTime();
  }
  QString lastUpdateCheckTimeStr = settings.value("ApplicationUpdate/LastUpdateCheckTime").toString();
  QDateTime LastUpdateCheckTime = QDateTime::fromString(lastUpdateCheckTimeStr, Qt::ISODate);
  return LastUpdateCheckTime;
}

// --------------------------------------------------------------------------
bool qSlicerApplicationUpdateManager::onReleaseInfoQueryFinished(const QUuid& requestId)
{
  Q_UNUSED(requestId);
  Q_D(qSlicerApplicationUpdateManager);

  QScopedPointer<qRestResult> restResult(d->ReleaseInfoAPI.takeResult(d->ReleaseInfoQueryUID));

  bool success = false;
  QVariantMap releaseInfo;
  if (!restResult.isNull())
  {
    QString responseString = QString(restResult->response());
    QJSEngine scriptEngine;
    QJSValue scriptValue =
      scriptEngine.evaluate("JSON.parse").callWithInstance(QJSValue(), QJSValueList() << responseString);

    QList<QVariantMap> response;
    // e.g. {["key1": "value1", ...]} or {"key1": "value1", ...}
    if (scriptValue.isArray())
    {
      quint32 length = scriptValue.property("length").toUInt();
      for (quint32 i = 0; i < length; ++i)
      {
        qRestAPI::appendScriptValueToVariantMapList(response, scriptValue.property(i));
      }
    }
    else
    {
      qRestAPI::appendScriptValueToVariantMapList(response, scriptValue);
    }

    if (!response.isEmpty())
    {
      if (response.size() > 1)
      {
        qWarning() << Q_FUNC_INFO << "Expected one record but received" << response.size();
      }
      releaseInfo = response[0];
      success = true;
    }

    success = success && qSlicerApplicationUpdateManagerPrivate::validateReleaseInfo(releaseInfo);
  }
  if (!success)
  {
    // Query failed, probably no network connection.
    // Do not pollute the application output with a warning or error message.
    d->ReleaseInfoQueryUID = QUuid();
    this->refreshUpdateAvailable();
    emit updateCheckCompleted(false);
    return false;
  }

  d->LatestReleaseVersion = releaseInfo["version"].toString();
  d->LatestReleaseRevision = releaseInfo["revision"].toString();

  // stores time of last update check for this Slicer installation
  QSettings settings;
  QDateTime currentTime = QDateTime::currentDateTimeUtc();
  settings.setValue("ApplicationUpdate/LastUpdateCheckTime", currentTime.toString(Qt::ISODate));
  settings.setValue("ApplicationUpdate/LastUpdateCheckUrl", this->serverUrl().toString());

  settings.setValue("ApplicationUpdate/LatestReleaseVersion", d->LatestReleaseVersion);
  settings.setValue("ApplicationUpdate/LatestReleaseRevision", d->LatestReleaseRevision);

  d->ReleaseInfoQueryUID = QUuid();

  this->refreshUpdateAvailable();
  emit updateCheckCompleted(true);
  return true;
}

// --------------------------------------------------------------------------
void qSlicerApplicationUpdateManager::setSlicerRequirements(const QString& revision,
                                                            const QString& os,
                                                            const QString& arch)
{
  Q_D(qSlicerApplicationUpdateManager);
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

  this->refreshUpdateAvailable();

  emit this->slicerRequirementsChanged(d->SlicerRevision, d->SlicerOs, d->SlicerArch);
}

// --------------------------------------------------------------------------
QUrl qSlicerApplicationUpdateManager::applicationDownloadPageUrl() const
{
  // Currently the download page is the root folder of the application update servers
  return this->serverUrl();
}

// --------------------------------------------------------------------------
void qSlicerApplicationUpdateManager::refreshUpdateAvailable()
{
  Q_D(qSlicerApplicationUpdateManager);
  bool isUpdateAvailable = false;
  if (!d->LatestReleaseRevision.isEmpty() && !d->SlicerRevision.isEmpty())
  {
    bool isIntegerCurrentRevision = false;
    int currentRevisionNumber = d->SlicerRevision.toInt(&isIntegerCurrentRevision);
    bool isIntegerLatestRevision = false;
    int latestRevisionNumber = d->LatestReleaseRevision.toInt(&isIntegerLatestRevision);
    if (isIntegerCurrentRevision && isIntegerLatestRevision)
    {
      isUpdateAvailable = (currentRevisionNumber < latestRevisionNumber);
    }
  }
  d->UpdateAvailable = isUpdateAvailable;
  emit updateAvailable(d->UpdateAvailable);
}

// --------------------------------------------------------------------------
bool qSlicerApplicationUpdateManager::isApplicationUpdateEnabled()
{
  return QSettings().value("ApplicationUpdates/Enabled", true).toBool();
}
