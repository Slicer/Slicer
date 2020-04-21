
// Qt includes
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslSocket>

// CTK includes
#include <ctkTest.h>

// Slicer includes
#include "qSlicerCoreApplication.h"

// ----------------------------------------------------------------------------
class qSlicerSslTester: public QObject
{
  Q_OBJECT
  typedef qSlicerSslTester Self;

private slots:
  void testSupportsSsl();
  void testLoadCaCertificates();
  void testHttpsConnection();
  void testHttpsConnection_data();
};

// ----------------------------------------------------------------------------
void qSlicerSslTester::testSupportsSsl()
{
  QVERIFY(QSslSocket::supportsSsl());
}

// ----------------------------------------------------------------------------
void qSlicerSslTester::testLoadCaCertificates()
{
  QVERIFY(qSlicerCoreApplication::loadCaCertificates(
            QProcessEnvironment::systemEnvironment().value("SLICER_HOME")));
}

// ----------------------------------------------------------------------------
class SslEventLoop : public QEventLoop
{
  Q_OBJECT
public:
  SslEventLoop() = default;
public slots:
  void onSslErrors(QNetworkReply* reply, const QList<QSslError>& sslErrors)
  {
    Q_UNUSED(reply);
    foreach(const QSslError& sslError, sslErrors)
      {
      this->SslErrors << sslError.error();
      this->SslErrorStrings << sslError.errorString();
      }
    this->quit();
  }
public:
  QList<QSslError::SslError> SslErrors;
  QStringList SslErrorStrings;
};

Q_DECLARE_METATYPE(QList<QSslError::SslError>)

// ----------------------------------------------------------------------------
void qSlicerSslTester::testHttpsConnection()
{
  QFETCH(QString, url);
  QFETCH(QList<QSslError::SslError>, expectedSslErrors);
  QFETCH(QNetworkReply::NetworkError, expectedNetworkError);
  QFETCH(int, expectedStatusCode);

  qSlicerCoreApplication::loadCaCertificates(
        QProcessEnvironment::systemEnvironment().value("SLICER_HOME"));

  QNetworkAccessManager * manager = new QNetworkAccessManager(this);

  SslEventLoop eventLoop;
  QObject::connect(manager, SIGNAL(finished(QNetworkReply*)),
                   &eventLoop, SLOT(quit()));
  QObject::connect(manager, SIGNAL(sslErrors(QNetworkReply*, QList<QSslError>)),
            &eventLoop, SLOT(onSslErrors(QNetworkReply*, QList<QSslError>)));

  QNetworkReply * reply = manager->get(QNetworkRequest(QUrl(url)));
  eventLoop.exec();

  QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

  QCOMPARE(eventLoop.SslErrors, expectedSslErrors);
  QCOMPARE(reply->error(), expectedNetworkError);
  QCOMPARE(statusCode.toInt(), expectedStatusCode);
}

// ----------------------------------------------------------------------------
void qSlicerSslTester::testHttpsConnection_data()
{
  QTest::addColumn<QString>("url");
  QTest::addColumn<QList<QSslError::SslError> >("expectedSslErrors");
  QTest::addColumn<QNetworkReply::NetworkError>("expectedNetworkError");
  QTest::addColumn<int>("expectedStatusCode");

  QTest::newRow("invalid-HostNotFoundError-0")
      << "http://i.n.v.a.l.i.d"
      << (QList<QSslError::SslError>())
      << QNetworkReply::HostNotFoundError << 0;

  QTest::newRow("slicer-clear-with-redirect-NoError-301")
      << "http://slicer.org"
      << (QList<QSslError::SslError>())
      << QNetworkReply::NoError << 301;

  QTest::newRow("slicer-secured-MovedPermanently-301")
      << "https://slicer.org"
      << (QList<QSslError::SslError>())
      << QNetworkReply::NoError << 301;

  QTest::newRow("other-secured-NoError-200")
      << "https://www.eff.org/https-everywhere"
      << (QList<QSslError::SslError>())
      << QNetworkReply::NoError << 200;
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qSlicerSslTest)
#include "moc_qSlicerSslTest.cxx"
