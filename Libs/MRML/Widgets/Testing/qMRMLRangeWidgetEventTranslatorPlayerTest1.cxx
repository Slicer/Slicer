/*=========================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Benjamin LONG, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

=========================================================================*/

// Qt includes
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>
// Slicer includes
#include "vtkSlicerConfigure.h"

// CTK includes
#include "ctkCallback.h"
#include "ctkEventTranslatorPlayerWidget.h"
#include "ctkQtTestingUtility.h"

// qMRML includes
#include "qMRMLRangeWidget.h"

// VTK includes
#include "qMRMLWidget.h"

// STD includes
#include <cstdlib>
#include <iostream>

namespace
{
//-----------------------------------------------------------------------------
void checkFinalWidgetState(void* data)
{
  qMRMLRangeWidget* const widget = reinterpret_cast<qMRMLRangeWidget*>(data);

  CTKCOMPARE(widget->minimumValue(), 0.00);
  CTKCOMPARE(widget->maximumValue(), 85.00);
}
} // namespace

//-----------------------------------------------------------------------------
int qMRMLRangeWidgetEventTranslatorPlayerTest1(int argc, char* argv[])
{
  qMRMLWidget::preInitializeApplication();
  const QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  const QString xmlDirectory = QString(argv[1]) + "/Libs/MRML/Widgets/Testing/";

  // ------------------------
  ctkEventTranslatorPlayerWidget etpWidget;
  ctkQtTestingUtility* const testUtility = new ctkQtTestingUtility(&etpWidget);
  etpWidget.setTestUtility(testUtility);

  // Test case 1
  qMRMLRangeWidget* const widget = new qMRMLRangeWidget();
  etpWidget.addTestCase(widget, xmlDirectory + "qMRMLRangeWidgetEventTranslatorPlayerTest1.xml", &checkFinalWidgetState);

  // ------------------------
  if (!app.arguments().contains("-I"))
  {
    QTimer::singleShot(0, &etpWidget, SLOT(play()));
  }

  etpWidget.show();
  return app.exec();
}
