/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital) and was supported by The Research Council of Norway
  through the ALive project (grant nr. 311393).

==============================================================================*/

// This tests the markups additional options wigets factory.

// Markups widgets includes
#include "qSlicerMarkupsAdditionalOptionsWidgetsFactory.h"
#include "qSlicerMarkupsCurveSettingsWidget.h"
#include "qSlicerMarkupsAngleMeasurementsWidget.h"

// Markups tests includes
#include "qSlicerMarkupsMalformedWidget.h"

// CTK includes
#include "ctkTest.h"

// Qt includes
#include <QScopedPointer>
#include <QSignalSpy>
#include <QTest>

//------------------------------------------------------------------------------
class qSlicerMarkupsAdditionalOptionsWidgetsFactoryTester : public QObject
{
  Q_OBJECT

  private slots:

    // Test registration of a valid additional widget without making use of it
    void testAdditionalOptionsWidgetRegistration1();

    // Test registration of a nullptr
    void testAdditionalOptionsWidgetRegistration2();

    // Test registration of a malformed widget
    void testAdditionalOptionsWidgetRegistration3();

    // Test registration of a valid additional wiget and unregister malformed
    // widget
    void testAdditionalOptionsWidgetRegistration4();

    // Test registration of a valid additional wiget and unregister another type
    // of widget
    void testAdditionalOptionsWidgetRegistration5();

    // Test registration of a valid additional wiget making use of it.
    // Deallocation is performed by widget going out of scope
    void testAdditionalOptionsWidgetRegistration6();

    // NOTE: This test should be the last one! Test registration of a valid
    // additional wiget. Deallocation happens by termination of factory.
    void testAdditionalOptionsWidgetRegistration7();
};


//------------------------------------------------------------------------------
// In this test we register a valid optional widget
void qSlicerMarkupsAdditionalOptionsWidgetsFactoryTester::
testAdditionalOptionsWidgetRegistration1()
{
  auto factory = qSlicerMarkupsAdditionalOptionsWidgetsFactory::instance();
  QSignalSpy registeredSignalSpy(factory, SIGNAL(additionalOptionsWidgetRegistered()));
  QSignalSpy unregisteredSignalSpy(factory, SIGNAL(additionalOptionsWidgetUnregistered()));

  // Test registration of a valid widget
  factory->registerAdditionalOptionsWidget(new qSlicerMarkupsCurveSettingsWidget());
  QCOMPARE(registeredSignalSpy.count(), 1); // Make sure the signal triggers only 1 time.
  auto registeredWidgets = factory->additionalOptionsWidgets();
  QVERIFY(registeredWidgets.length() == 1);

  // Test unregistration of a valid widget
  factory->unregisterAdditionalOptionsWidget(new qSlicerMarkupsCurveSettingsWidget());
  QCOMPARE(unregisteredSignalSpy.count(), 1); // Make sure the signal triggers only 1 time.
  registeredWidgets = factory->additionalOptionsWidgets();
  QVERIFY(registeredWidgets.length() == 0);
}

//------------------------------------------------------------------------------
// In this test we register a nullptr widget and try to unregister
void qSlicerMarkupsAdditionalOptionsWidgetsFactoryTester::
testAdditionalOptionsWidgetRegistration2()
{
  auto factory = qSlicerMarkupsAdditionalOptionsWidgetsFactory::instance();
  QSignalSpy registeredSignalSpy(factory, SIGNAL(additionalOptionsWidgetRegistered()));
  QSignalSpy unregisteredSignalSpy(factory, SIGNAL(additionalOptionsWidgetUnregistered()));

  // Test registration of a valid widget
  factory->registerAdditionalOptionsWidget(nullptr);
  QCOMPARE(registeredSignalSpy.count(), 0); // Make sure the signal does not trigger
  auto registeredWidgets = factory->additionalOptionsWidgets();
  QVERIFY(registeredWidgets.length() == 0);

  // Test unregistration of a valid widget
  factory->unregisterAdditionalOptionsWidget(nullptr);
  QCOMPARE(unregisteredSignalSpy.count(), 0); // Make sure the signal does not trigger
  registeredWidgets = factory->additionalOptionsWidgets();
  QVERIFY(registeredWidgets.length() == 0);
}

//------------------------------------------------------------------------------
// In this test we register a malformed widget
void qSlicerMarkupsAdditionalOptionsWidgetsFactoryTester::
testAdditionalOptionsWidgetRegistration3()
{
  auto factory = qSlicerMarkupsAdditionalOptionsWidgetsFactory::instance();
  QSignalSpy registeredSignalSpy(factory, SIGNAL(additionalOptionsWidgetRegistered()));
  QSignalSpy unregisteredSignalSpy(factory, SIGNAL(additionalOptionsWidgetUnregistered()));

  // Test registration of an invalid widget
  factory->registerAdditionalOptionsWidget(new qSlicerMarkupsMalformedWidget);
  QCOMPARE(registeredSignalSpy.count(), 0); // Make sure the signal triggers only 1 time.
  auto registeredWidgets = factory->additionalOptionsWidgets();
  QVERIFY(registeredWidgets.length() == 0);

  // Test unregistration of an invalid widget
  factory->unregisterAdditionalOptionsWidget(new qSlicerMarkupsMalformedWidget);
  QCOMPARE(unregisteredSignalSpy.count(), 0); // Make sure the signal triggers only 1 time.
  registeredWidgets = factory->additionalOptionsWidgets();
  QVERIFY(registeredWidgets.length() == 0);
}

//------------------------------------------------------------------------------
// In this test we register a valid widget and try to unregister a malformed one
void qSlicerMarkupsAdditionalOptionsWidgetsFactoryTester::
testAdditionalOptionsWidgetRegistration4()
{
  auto factory = qSlicerMarkupsAdditionalOptionsWidgetsFactory::instance();
  QSignalSpy registeredSignalSpy(factory, SIGNAL(additionalOptionsWidgetRegistered()));
  QSignalSpy unregisteredSignalSpy(factory, SIGNAL(additionalOptionsWidgetUnregistered()));

  // Test registration of an invalid widget
  factory->registerAdditionalOptionsWidget(new qSlicerMarkupsCurveSettingsWidget);
  QCOMPARE(registeredSignalSpy.count(), 1); // Make sure the signal triggers only 1 time.
  auto registeredWidgets = factory->additionalOptionsWidgets();
  QVERIFY(registeredWidgets.length() == 1);

  // Test unregistration of an invalid widget
  factory->unregisterAdditionalOptionsWidget(new qSlicerMarkupsMalformedWidget);
  QCOMPARE(unregisteredSignalSpy.count(), 0); // Make sure the signal triggers only 1 time.
  registeredWidgets = factory->additionalOptionsWidgets();
  QVERIFY(registeredWidgets.length() == 1);

  // Test unregistration of a valid widget
  factory->unregisterAdditionalOptionsWidget(new qSlicerMarkupsCurveSettingsWidget);
  QCOMPARE(unregisteredSignalSpy.count(), 1); // Make sure the signal triggers only 1 time.
  registeredWidgets = factory->additionalOptionsWidgets();
  QVERIFY(registeredWidgets.length() == 0);
}

//------------------------------------------------------------------------------
// In this test we register a valid widget and try to unregister a valid different type
void qSlicerMarkupsAdditionalOptionsWidgetsFactoryTester::
testAdditionalOptionsWidgetRegistration5()
{
  auto factory = qSlicerMarkupsAdditionalOptionsWidgetsFactory::instance();
  QSignalSpy registeredSignalSpy(factory, SIGNAL(additionalOptionsWidgetRegistered()));
  QSignalSpy unregisteredSignalSpy(factory, SIGNAL(additionalOptionsWidgetUnregistered()));

  // Test registration of an invalid widget
  factory->registerAdditionalOptionsWidget(new qSlicerMarkupsCurveSettingsWidget);
  QCOMPARE(registeredSignalSpy.count(), 1); // Make sure the signal triggers only 1 time.
  auto registeredWidgets = factory->additionalOptionsWidgets();
  QVERIFY(registeredWidgets.length() == 1);

  // Test unregistration of an invalid widget
  factory->unregisterAdditionalOptionsWidget(new qSlicerMarkupsAngleMeasurementsWidget);
  QCOMPARE(unregisteredSignalSpy.count(), 0); // Make sure the signal triggers only 1 time.
  registeredWidgets = factory->additionalOptionsWidgets();
  QVERIFY(registeredWidgets.length() == 1);

  // Test unregistration of a valid widget
  factory->unregisterAdditionalOptionsWidget(new qSlicerMarkupsCurveSettingsWidget);
  QCOMPARE(unregisteredSignalSpy.count(), 1); // Make sure the signal triggers only 1 time.
  registeredWidgets = factory->additionalOptionsWidgets();
  QVERIFY(registeredWidgets.length() == 0);
}

//------------------------------------------------------------------------------
// In this test we register a valid widget and make use of it
void qSlicerMarkupsAdditionalOptionsWidgetsFactoryTester::
testAdditionalOptionsWidgetRegistration6()
{
  auto factory = qSlicerMarkupsAdditionalOptionsWidgetsFactory::instance();
  QSignalSpy registeredSignalSpy(factory, SIGNAL(additionalOptionsWidgetRegistered()));
  QSignalSpy unregisteredSignalSpy(factory, SIGNAL(additionalOptionsWidgetUnregistered()));
  QScopedPointer<QWidget> widgetPtr(new QWidget);

  // Test registration of an invalid widget
  factory->registerAdditionalOptionsWidget(new qSlicerMarkupsCurveSettingsWidget);
  QCOMPARE(registeredSignalSpy.count(), 1); // Make sure the signal triggers only 1 time.
  auto registeredWidgets = factory->additionalOptionsWidgets();
  QVERIFY(registeredWidgets.length() == 1);
  registeredWidgets[0]->setParent(widgetPtr.data());

  // Test unregistration of a valid widget
  factory->unregisterAdditionalOptionsWidget(new qSlicerMarkupsCurveSettingsWidget);
  QCOMPARE(unregisteredSignalSpy.count(), 1); // Make sure the signal triggers only 1 time.
  registeredWidgets = factory->additionalOptionsWidgets();
  QVERIFY(registeredWidgets.length() == 0);
}

//------------------------------------------------------------------------------
// In this test we register a valid widget and make use of it
void qSlicerMarkupsAdditionalOptionsWidgetsFactoryTester::
testAdditionalOptionsWidgetRegistration7()
{
  auto factory = qSlicerMarkupsAdditionalOptionsWidgetsFactory::instance();
  QSignalSpy registeredSignalSpy(factory, SIGNAL(additionalOptionsWidgetRegistered()));

  // Test registration of an invalid widget
  factory->registerAdditionalOptionsWidget(new qSlicerMarkupsCurveSettingsWidget);
  QCOMPARE(registeredSignalSpy.count(), 1); // Make sure the signal triggers only 1 time.
  auto registeredWidgets = factory->additionalOptionsWidgets();
  QVERIFY(registeredWidgets.length() == 1);
}

//------------------------------------------------------------------------------
CTK_TEST_MAIN(qSlicerMarkupsAdditionalOptionsWidgetsFactoryTest)
#include "moc_qSlicerMarkupsAdditionalOptionsWidgetsFactoryTest.cxx"
