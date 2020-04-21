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

// Self includes
#include "qSlicerExtensionDownloadTask.h"

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

// QtCore includes
#include "qSlicerExtensionsManagerModel.h"

//-----------------------------------------------------------------------------
class qSlicerExtensionDownloadTaskPrivate
{
public:
  QNetworkReply* Reply;
  QString ExtensionName;
  QString ArchiveName;
  QVariantMap Metadata;
};

/*
  QString extensionName = extensionMetadata.value("extensionname").toString();
  QString archiveName = extensionMetadata.value("archivename").toString();
*/

//-----------------------------------------------------------------------------
qSlicerExtensionDownloadTask::qSlicerExtensionDownloadTask(
  QNetworkReply* reply, QObject* parent)
  : QObject(parent), d_ptr(new qSlicerExtensionDownloadTaskPrivate)
{
  Q_D(qSlicerExtensionDownloadTask);

  reply->setParent(this);

  connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
          this, SLOT(emitProgress(qint64,qint64)));
  connect(reply, SIGNAL(finished()), this, SLOT(emitFinished()));
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(emitError(QNetworkReply::NetworkError)));

  d->Reply = reply;
}

//-----------------------------------------------------------------------------
qSlicerExtensionDownloadTask::~qSlicerExtensionDownloadTask() = default;

//-----------------------------------------------------------------------------
QVariantMap qSlicerExtensionDownloadTask::metadata() const
{
  Q_D(const qSlicerExtensionDownloadTask);
  return d->Metadata;
}

//-----------------------------------------------------------------------------
void qSlicerExtensionDownloadTask::setMetadata(const QVariantMap& md)
{
  Q_D(qSlicerExtensionDownloadTask);
  d->Metadata = md;
  if (d->ExtensionName.isEmpty())
    {
    d->ExtensionName = md.value("extensionname").toString();
    }
  if (d->ArchiveName.isEmpty())
    {
    d->ArchiveName = md.value("archivename").toString();
    }
}

//-----------------------------------------------------------------------------
QString qSlicerExtensionDownloadTask::extensionName() const
{
  Q_D(const qSlicerExtensionDownloadTask);
  return d->ExtensionName;
}

//-----------------------------------------------------------------------------
void qSlicerExtensionDownloadTask::setExtensionName(const QString& name)
{
  Q_D(qSlicerExtensionDownloadTask);
  d->ExtensionName = name;
}

//-----------------------------------------------------------------------------
QString qSlicerExtensionDownloadTask::archiveName() const
{
  Q_D(const qSlicerExtensionDownloadTask);
  return d->ArchiveName;
}

//-----------------------------------------------------------------------------
void qSlicerExtensionDownloadTask::setArchiveName(const QString& name)
{
  Q_D(qSlicerExtensionDownloadTask);
  d->ArchiveName = name;
}

//-----------------------------------------------------------------------------
QNetworkReply* qSlicerExtensionDownloadTask::reply() const
{
  Q_D(const qSlicerExtensionDownloadTask);
  return d->Reply;
}

//-----------------------------------------------------------------------------
void qSlicerExtensionDownloadTask::emitProgress(qint64 received, qint64 total)
{
  emit this->progress(this, received, total);
}

//-----------------------------------------------------------------------------
void qSlicerExtensionDownloadTask::emitFinished()
{
  emit this->finished(this);
}

//-----------------------------------------------------------------------------
void qSlicerExtensionDownloadTask::emitError(QNetworkReply::NetworkError error)
{
  emit this->error(this, error);
}
