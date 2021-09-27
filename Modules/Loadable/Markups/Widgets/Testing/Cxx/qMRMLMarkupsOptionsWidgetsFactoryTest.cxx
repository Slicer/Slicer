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
#include "qMRMLMarkupsOptionsWidgetsFactory.h"
#include "qMRMLMarkupsCurveSettingsWidget.h"
#include "qMRMLMarkupsAngleMeasurementsWidget.h"

// Markups tests includes
#include "qMRMLMarkupsMalformedWidget.h"

// CTK includes
#include "ctkTest.h"

// Qt includes
#include <QScopedPointer>
#include <QSignalSpy>
#include <QTest>

//------------------------------------------------------------------------------
class qMRMLMarkupsOptionsWidgetsFactoryTester : public QObject
{
  Q_OBJECT

  private slots:

    // Test registration of a valid additional widget without making use of it
    void testOptionsWidgetRegistration1();

    // Test registration of a nullptr
    void testOptionsWidgetRegistration2();

    // Test registration of a malformed widget
    void testOptionsWidgetRegistration3();

    // Test registration of a valid additional wiget and unregister malformed
    // widget
    void testOptionsWidgetRegistration4();

    // Test registration of a valid additional wiget and unregister another type
    // of widget
    void testOptionsWidgetRegistration5();

    // Test registration of a valid additional wiget making use of it.
    // Deallocation is performed by widget going out of scope
    void testOptionsWidgetRegistration6();

    // NOTE: This test should be the last one! Test registration of a valid
    // additional wiget. Deallocation happens by termination of factory.
    void testOptionsWidgetRegistration7();
};


//------------------------------------------------------------------------------
// In this test we register a valid optional widget
void qMRMLMarkupsOptionsWidgetsFactoryTester::
testOptionsWidgetRegistration1()
{
  auto factory = qMRMLMarkupsOptionsWidgetsFactory::instance();
  QSignalSpy registeredSignalSpy(factory, SIGNAL(optionsWidgetRegistered()));
  QSignalSpy unregisteredSignalSpy(factory, SIGNAL(optionsWidgetUnregistered()));

  // Test registration of a valid widget
  factory->registerOptionsWidget(new qMRMLMarkupsCurveSettingsWidget());
  QCOMPARE(registeredSignalSpy.count(), 1); // Make sure the signal triggers only 1 time.
  auto registeredWidgets = factory->registeredOptionsWidgetsClassNames();
  QVERIFY(registeredWidgets.length() == 1);

  // Test unregistration of a valid widget
  factory->unregisterOptionsWidget(new qMRMLMarkupsCurveSettingsWidget());
  QCOMPARE(unregisteredSignalSpy.count(), 1); // Make sure the signal triggers only 1 time.
  registeredWidgets = factory->registeredOptionsWidgetsClassNames();
  QVERIFY(registeredWidgets.length() == 0);
}

//------------------------------------------------------------------------------
// In this test we register a nullptr widget and try to unregister
void qMRMLMarkupsOptionsWidgetsFactoryTester::
testOptionsWidgetRegistration2()
{
  auto factory = qMRMLMarkupsOptionsWidgetsFactory::instance();
  QSignalSpy registeredSignalSpy(factory, SIGNAL(optionsWidgetRegistered()));
  QSignalSpy unregisteredSignalSpy(factory, SIGNAL(optionsWidgetUnregistered()));

  // Test registration of a valid widget
  factory->registerOptionsWidget(nullptr);
  QCOMPARE(registeredSignalSpy.count(), 0); // Make sure the signal does not trigger
  auto registeredWidgets = factory->registeredOptionsWidgetsClassNames();
  QVERIFY(registeredWidgets.length() == 0);

  // Test unregistration of a valid widget
  factory->unregisterOptionsWidget(nullptr);
  QCOMPARE(unregisteredSignalSpy.count(), 0); // Make sure the signal does not trigger
  registeredWidgets = factory->registeredOptionsWidgetsClassNames();
  QVERIFY(registeredWidgets.length() == 0);
}

//------------------------------------------------------------------------------
// In this test we register a malformed widget
void qMRMLMarkupsOptionsWidgetsFactoryTester::
testOptionsWidgetRegistration3()
{
  auto factory = qMRMLMarkupsOptionsWidgetsFactory::instance();
  QSignalSpy registeredSignalSpy(factory, SIGNAL(optionsWidgetRegistered()));
  QSignalSpy unregisteredSignalSpy(factory, SIGNAL(optionsWidgetUnregistered()));

  // Test registration of an invalid widget
  factory->registerOptionsWidget(new qMRMLMarkupsMalformedWidget());
  QCOMPARE(registeredSignalSpy.count(), 0); // Make sure the signal triggers only 1 time.
  auto registeredWidgets = factory->registeredOptionsWidgetsClassNames();
  QVERIFY(registeredWidgets.length() == 0);

  // Test unregistration of an invalid widget
  factory->unregisterOptionsWidget(new qMRMLMarkupsMalformedWidget());
  QCOMPARE(unregisteredSignalSpy.count(), 0); // Make sure the signal triggers only 1 time.
  registeredWidgets = factory->registeredOptionsWidgetsClassNames();
  QVERIFY(registeredWidgets.length() == 0);
}

//------------------------------------------------------------------------------
// In this test we register a valid widget and try to unregister a malformed one
void qMRMLMarkupsOptionsWidgetsFactoryTester::
testOptionsWidgetRegistration4()
{
  auto factory = qMRMLMarkupsOptionsWidgetsFactory::instance();
  QSignalSpy registeredSignalSpy(factory, SIGNAL(optionsWidgetRegistered()));
  QSignalSpy unregisteredSignalSpy(factory, SIGNAL(optionsWidgetUnregistered()));

  // Test registration of an invalid widget
  factory->registerOptionsWidget(new qMRMLMarkupsCurveSettingsWidget);
  QCOMPARE(registeredSignalSpy.count(), 1); // Make sure the signal triggers only 1 time.
  auto registeredWidgets = factory->registeredOptionsWidgetsClassNames();
  QVERIFY(registeredWidgets.length() == 1);

  // Test unregistration of an invalid widget
  factory->unregisterOptionsWidget(new qMRMLMarkupsMalformedWidget);
  QCOMPARE(unregisteredSignalSpy.count(), 0); // Make sure the signal triggers only 1 time.
  registeredWidgets = factory->registeredOptionsWidgetsClassNames();
  QVERIFY(registeredWidgets.length() == 1);

  // Test unregistration of a valid widget
  factory->unregisterOptionsWidget(new qMRMLMarkupsCurveSettingsWidget);
  QCOMPARE(unregisteredSignalSpy.count(), 1); // Make sure the signal triggers only 1 time.
  registeredWidgets = factory->registeredOptionsWidgetsClassNames();
  QVERIFY(registeredWidgets.length() == 0);
}

//------------------------------------------------------------------------------
// In this test we register a valid widget and try to unregister a valid different type
void qMRMLMarkupsOptionsWidgetsFactoryTester::
testOptionsWidgetRegistration5()
{
  auto factory = qMRMLMarkupsOptionsWidgetsFactory::instance();
  QSignalSpy registeredSignalSpy(factory, SIGNAL(optionsWidgetRegistered()));
  QSignalSpy unregisteredSignalSpy(factory, SIGNAL(optionsWidgetUnregistered()));

  // Test registration of an invalid widget
  factory->registerOptionsWidget(new qMRMLMarkupsCurveSettingsWidget);
  QCOMPARE(registeredSignalSpy.count(), 1); // Make sure the signal triggers only 1 time.
  auto registeredWidgets = factory->registeredOptionsWidgetsClassNames();
  QVERIFY(registeredWidgets.length() == 1);

  // Test unregistration of an invalid widget
  factory->unregisterOptionsWidget(new qMRMLMarkupsAngleMeasurementsWidget);
  QCOMPARE(unregisteredSignalSpy.count(), 0); // Make sure the signal triggers only 1 time.
  registeredWidgets = factory->registeredOptionsWidgetsClassNames();
  QVERIFY(registeredWidgets.length() == 1);

  // Test unregistration of a valid widget
  factory->unregisterOptionsWidget(new qMRMLMarkupsCurveSettingsWidget);
  QCOMPARE(unregisteredSignalSpy.count(), 1); // Make sure the signal triggers only 1 time.
  registeredWidgets = factory->registeredOptionsWidgetsClassNames();
  QVERIFY(registeredWidgets.length() == 0);
}

//------------------------------------------------------------------------------
// In this test we register a valid widget and make use of it
void qMRMLMarkupsOptionsWidgetsFactoryTester::
testOptionsWidgetRegistration6()
{
  auto factory = qMRMLMarkupsOptionsWidgetsFactory::instance();
  QSignalSpy registeredSignalSpy(factory, SIGNAL(optionsWidgetRegistered()));
  QSignalSpy unregisteredSignalSpy(factory, SIGNAL(optionsWidgetUnregistered()));
  QScopedPointer<QWidget> widgetPtr(new QWidget);

  // Test registration of an invalid widget
  factory->registerOptionsWidget(new qMRMLMarkupsCurveSettingsWidget);
  QCOMPARE(registeredSignalSpy.count(), 1); // Make sure the signal triggers only 1 time.
  auto registeredWidgets = factory->registeredOptionsWidgetsClassNames();
  QVERIFY(registeredWidgets.length() == 1);
  auto widget = factory->createWidget(registeredWidgets[0]);
  QVERIFY(widget!=nullptr);
  widget->setParent(widgetPtr.data());

  // Test unregistration of a valid widget
  factory->unregisterOptionsWidget(new qMRMLMarkupsCurveSettingsWidget);
  QCOMPARE(unregisteredSignalSpy.count(), 1); // Make sure the signal triggers only 1 time.
  registeredWidgets = factory->registeredOptionsWidgetsClassNames();
  QVERIFY(registeredWidgets.length() == 0);
}

//------------------------------------------------------------------------------
// In this test we register a valid widget and make use of it
void qMRMLMarkupsOptionsWidgetsFactoryTester::
testOptionsWidgetRegistration7()
{
  auto factory = qMRMLMarkupsOptionsWidgetsFactory::instance();
  QSignalSpy registeredSignalSpy(factory, SIGNAL(optionsWidgetRegistered()));

  // Test registration of an invalid widget
  factory->registerOptionsWidget(new qMRMLMarkupsCurveSettingsWidget);
  QCOMPARE(registeredSignalSpy.count(), 1); // Make sure the signal triggers only 1 time.
  auto registeredWidgets = factory->registeredOptionsWidgetsClassNames();
  QVERIFY(registeredWidgets.length() == 1);
}

//------------------------------------------------------------------------------
CTK_TEST_MAIN(qMRMLMarkupsOptionsWidgetsFactoryTest)
#include "moc_qMRMLMarkupsOptionsWidgetsFactoryTest.cxx"
