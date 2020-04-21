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
#include <QDir>
#include <QFileInfo>
#include <QFileInfo>
#include <QNetworkCookie>
#include <QSettings>
#include <QStringList>

// QtCore includes
#include "qSlicerPersistentCookieJar.h"

//-----------------------------------------------------------------------------
class qSlicerPersistentCookieJarPrivate
{
  Q_DECLARE_PUBLIC(qSlicerPersistentCookieJar);
protected:
  qSlicerPersistentCookieJar* const q_ptr;
public:
  qSlicerPersistentCookieJarPrivate(qSlicerPersistentCookieJar& object);

  void init();

  QString FilePath;
};

//-----------------------------------------------------------------------------
// qSlicerPersistentCookieJarPrivate methods

// --------------------------------------------------------------------------
qSlicerPersistentCookieJarPrivate::
qSlicerPersistentCookieJarPrivate(qSlicerPersistentCookieJar& object) :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerPersistentCookieJarPrivate::init()
{
  QSettings::Format savedFormat = QSettings::defaultFormat();
  QSettings::setDefaultFormat(QSettings::IniFormat);
  this->FilePath = QFileInfo(QFileInfo(QSettings().fileName()).dir(), "cookies.ini").filePath();
  QSettings::setDefaultFormat(savedFormat);
}

//-----------------------------------------------------------------------------
// qSlicerPersistentCookieJar methods

//-----------------------------------------------------------------------------
qSlicerPersistentCookieJar::qSlicerPersistentCookieJar(QObject * parent)
  :Superclass(parent), d_ptr(new qSlicerPersistentCookieJarPrivate(*this))
{
  Q_D(qSlicerPersistentCookieJar);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerPersistentCookieJar::~qSlicerPersistentCookieJar() = default;

//-----------------------------------------------------------------------------
QString qSlicerPersistentCookieJar::filePath()const
{
  Q_D(const qSlicerPersistentCookieJar);
  return d->FilePath;
}

//-----------------------------------------------------------------------------
void qSlicerPersistentCookieJar::setFilePath(const QString& filePath)
{
  Q_D(qSlicerPersistentCookieJar);
  d->FilePath = filePath;
}

//-----------------------------------------------------------------------------
QList<QNetworkCookie> qSlicerPersistentCookieJar::cookiesForUrl( const QUrl & url)const
{
  Q_D(const qSlicerPersistentCookieJar);
  QSettings settings(d->FilePath, QSettings::IniFormat);
  QList<QNetworkCookie> cookieList;
  settings.beginGroup(url.host());
  QStringList keys = settings.childKeys();
  foreach(const QString& key, keys)
    {
    cookieList << QNetworkCookie(key.toUtf8(), settings.value(key).toString().toUtf8());
    }
  return cookieList;
}

//-----------------------------------------------------------------------------
bool qSlicerPersistentCookieJar::
setCookiesFromUrl(const QList<QNetworkCookie> & cookieList, const QUrl & url)
{
  Q_D(qSlicerPersistentCookieJar);
  QSettings settings(d->FilePath, QSettings::IniFormat);
  settings.beginGroup(url.host());
  foreach(const QNetworkCookie& cookie, cookieList)
    {
    settings.setValue(cookie.name(), cookie.value());
    }
  settings.sync();
  return settings.status() == QSettings::NoError;
}
