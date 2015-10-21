
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

Q_DECLARE_METATYPE(QNetworkReply::NetworkError)

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
void qSlicerSslTester::testHttpsConnection()
{
  QFETCH(QString, url);
  QFETCH(QNetworkReply::NetworkError, expectedNetworkError);
  QFETCH(int, expectedStatusCode);

  qSlicerCoreApplication::loadCaCertificates(
        QProcessEnvironment::systemEnvironment().value("SLICER_HOME"));

  QNetworkAccessManager * manager = new QNetworkAccessManager(this);

  QEventLoop eventLoop;
  QObject::connect(manager, SIGNAL(finished(QNetworkReply*)),
                   &eventLoop, SLOT(quit()));
  QObject::connect(manager, SIGNAL(sslErrors(QNetworkReply*, QList<QSslError>)),
            &eventLoop, SLOT(quit()));

  QNetworkReply * reply = manager->get(QNetworkRequest(QUrl(url)));
  eventLoop.exec();

  QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

  QCOMPARE(reply->error(), expectedNetworkError);
  QCOMPARE(statusCode.toInt(), expectedStatusCode);
}

// ----------------------------------------------------------------------------
void qSlicerSslTester::testHttpsConnection_data()
{
  QTest::addColumn<QString>("url");
  QTest::addColumn<QNetworkReply::NetworkError>("expectedNetworkError");
  QTest::addColumn<int>("expectedStatusCode");

  QTest::newRow("invalid-HostNotFoundError-0")
      << "http://i.n.v.a.l.i.d"
      << QNetworkReply::HostNotFoundError << 0;

  QTest::newRow("clear-NoError-200")
      << "http://slicer.org"
      << QNetworkReply::NoError << 200;

  QTest::newRow("secured-NoError-200")
      << "https://www.eff.org/https-everywhere"
      << QNetworkReply::NoError << 200;
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qSlicerSslTest)
#include "moc_qSlicerSslTest.cxx"
