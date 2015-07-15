/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Laurent Chauvin, Brigham and Women's
  Hospital, based on a template developed by Jean-Christophe Fillion-Robin,
  Kitware Inc. The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 5R42CA137886.
  The file was then updated for the Markups module by Nicole Aucoin, BWH.

==============================================================================*/

#include "qSlicerSimpleMarkupsWidgetPlugin.h"
#include "qSlicerSimpleMarkupsWidget.h"

//------------------------------------------------------------------------------
qSlicerSimpleMarkupsWidgetPlugin
::qSlicerSimpleMarkupsWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qSlicerSimpleMarkupsWidgetPlugin
::createWidget(QWidget *_parent)
{
  qSlicerSimpleMarkupsWidget* _widget
    = new qSlicerSimpleMarkupsWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qSlicerSimpleMarkupsWidgetPlugin
::domXml() const
{
  return "<widget class=\"qSlicerSimpleMarkupsWidget\" \
          name=\"SimpleMarkupsWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qSlicerSimpleMarkupsWidgetPlugin
::includeFile() const
{
  return "qSlicerSimpleMarkupsWidget.h";
}

//------------------------------------------------------------------------------
bool qSlicerSimpleMarkupsWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qSlicerSimpleMarkupsWidgetPlugin
::name() const
{
  return "qSlicerSimpleMarkupsWidget";
}
