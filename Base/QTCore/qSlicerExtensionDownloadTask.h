/*==============================================================================

  Program: 3D Slicer

  Copyright 2014 Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Matthew Woehlke, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerExtensionDownloadTask_h
#define __qSlicerExtensionDownloadTask_h

// Qt includes
#include <QNetworkReply>

// QtGUI includes
#include "qSlicerBaseQTCoreExport.h"

class qSlicerExtensionDownloadTaskPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerExtensionDownloadTask : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QVariantMap metadata READ metadata WRITE setMetadata)
  Q_PROPERTY(QString extensionName READ extensionName WRITE setExtensionName)
  Q_PROPERTY(QString archiveName READ archiveName WRITE setArchiveName)
  Q_PROPERTY(QNetworkReply* reply READ reply)

public:
  /// Constructor.
  ///
  /// The task takes ownership of the reply and will delete it when the task
  /// is destroyed.
  explicit qSlicerExtensionDownloadTask(QNetworkReply* reply,
                                        QObject* parent = nullptr);

  /// Destructor.
  ~qSlicerExtensionDownloadTask() override;

  /// Get extension metadata.
  QVariantMap metadata() const;

  /// Set extension metadata.
  ///
  /// This sets the extension metadata that is associated with the task. If
  /// provided by the metadata, this also sets the extension name and archive
  /// name, if not previously set.
  ///
  /// \sa setExtensionName, setArchiveName
  void setMetadata(const QVariantMap&);

  /// Get extension name.
  QString extensionName() const;

  /// Set extension name.
  void setExtensionName(const QString&);

  /// Get archive name.
  QString archiveName() const;

  /// Set archive name.
  void setArchiveName(const QString&);

  /// Get associated network reply.
  QNetworkReply* reply() const;

signals:
  void finished(qSlicerExtensionDownloadTask*);
  void error(qSlicerExtensionDownloadTask*, QNetworkReply::NetworkError);
  void progress(qSlicerExtensionDownloadTask*, qint64 received, qint64 total);

protected slots:
  void emitFinished();
  void emitError(QNetworkReply::NetworkError);
  void emitProgress(qint64, qint64);

protected:
  QScopedPointer<qSlicerExtensionDownloadTaskPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerExtensionDownloadTask);
  Q_DISABLE_COPY(qSlicerExtensionDownloadTask);
};

#endif
