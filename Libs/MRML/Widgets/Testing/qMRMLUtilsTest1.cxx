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
#include <QApplication>
#include <QDebug>

// Slicer includes
#include "vtkSlicerConfigure.h"

// qMRMLWidgets include
#include <qMRMLUtils.h>

// VTK includes
#include "qMRMLWidget.h"

// STD includes
#include <cstdlib>
#include <cmath>

int qMRMLUtilsTest1( int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  // Test color conversions
  QColor qcolor1 = QColor(255, 127, 0);
  QColor qcolor2;
  double color1[3] = {0.0, 1.0, 0.5};
  double color2[3];
  qMRMLUtils::colorToQColor(nullptr, qcolor1);
  qMRMLUtils::qColorToColor(qcolor1, nullptr);
  qMRMLUtils::colorToQColor(color1, qcolor2);
  if (fabs(qcolor2.redF() - 0.0) > 0.01 ||
      fabs(qcolor2.greenF() - 1.0) > 0.01 ||
      fabs(qcolor2.blueF() - 0.5) > 0.01)
    {
    qDebug() << QString("colorToQColor, expected 0.0, 1.0, 0.5, but got ")
      + QString::number(qcolor2.redF()) + QString(" ")
      + QString::number(qcolor2.greenF()) + QString(" ")
      + QString::number(qcolor2.blueF());
    return EXIT_FAILURE;
    }
  qMRMLUtils::qColorToColor(qcolor1, color2);
  if (fabs(color2[0] - 1.0) > 0.01 ||
      fabs(color2[1] - 0.5) > 0.01 ||
      fabs(color2[2] - 0.0) > 0.01)
    {
    qDebug() << QString("Failed: qColorToColor, expected 1.0, 0.5, 0.0 but got ")
      + QString::number(color2[0]) + QString(" ")
      + QString::number(color2[1]) + QString(" ")
      + QString::number(color2[2]);
    return EXIT_FAILURE;
    }

  //  FIXME
  //  FIXME
  //  FIXME
  //  FIXME   Many other methods to be exercised here...
  //  FIXME
  //  FIXME
  //  FIXME
  //  FIXME
  return EXIT_SUCCESS;
}
