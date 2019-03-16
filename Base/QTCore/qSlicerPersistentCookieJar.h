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

#ifndef __qSlicerPersistentCookieJar_h
#define __qSlicerPersistentCookieJar_h

// Qt includes
#include <QNetworkCookieJar>

// QtCore includes
#include "qSlicerBaseQTCoreExport.h"

class qSlicerPersistentCookieJarPrivate;

/// qSlicerPersistentCookieJar provides a mechanism allowing to store persistently cookies
/// when associated with an instance of QNetworkAccessManager.
///
/// The cookies will be storted in a INI config file. By default, the config file will
/// be located in the directory associated with the current application settings and will be
/// named cookies.ini. This could be overwritten using qSlicerPersistentCookieJar::setFilePath
///
/// A cookie jar can be associated with a QNetworkAccessManager using method QNetworkAccessManager::setCookieJar
///
/// \sa QSettings::IniFormat, QNetworkAccessManager::setCookieJar
class Q_SLICER_BASE_QTCORE_EXPORT qSlicerPersistentCookieJar: public QNetworkCookieJar
{
public:
  typedef QNetworkCookieJar Superclass;
  qSlicerPersistentCookieJar(QObject *parent = nullptr);
  ~qSlicerPersistentCookieJar() override;

  QString filePath()const;
  void setFilePath(const QString& filePath);

  QList<QNetworkCookie> cookiesForUrl(const QUrl & url) const override;
  bool setCookiesFromUrl(const QList<QNetworkCookie> & cookieList, const QUrl & url) override;

protected:
  QScopedPointer<qSlicerPersistentCookieJarPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerPersistentCookieJar);
  Q_DISABLE_COPY(qSlicerPersistentCookieJar);
};

#endif

