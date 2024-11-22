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

  This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
  and development was supported by the Center for Intelligent Image-guided Interventions (CI3).

==============================================================================*/

#include "qSlicerMarkupsMeasurementsWidgetPlugin.h"
#include "qSlicerMarkupsMeasurementsWidget.h"

//------------------------------------------------------------------------------
qSlicerMarkupsMeasurementsWidgetPlugin
::qSlicerMarkupsMeasurementsWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qSlicerMarkupsMeasurementsWidgetPlugin
::createWidget(QWidget *_parent)
{
  qSlicerMarkupsMeasurementsWidget* _widget
    = new qSlicerMarkupsMeasurementsWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qSlicerMarkupsMeasurementsWidgetPlugin
::domXml() const
{
  return "<widget class=\"qSlicerMarkupsMeasurementsWidget\" \
          name=\"MarkupsMeasurementsWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qSlicerMarkupsMeasurementsWidgetPlugin
::includeFile() const
{
  return "qSlicerMarkupsMeasurementsWidget.h";
}

//------------------------------------------------------------------------------
bool qSlicerMarkupsMeasurementsWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qSlicerMarkupsMeasurementsWidgetPlugin
::name() const
{
  return "qSlicerMarkupsMeasurementsWidget";
}
