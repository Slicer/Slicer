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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

#include "qSlicerMarkupsROIWidgetPlugin.h"
#include "qSlicerMarkupsROIWidget.h"

//-----------------------------------------------------------------------------
qSlicerMarkupsROIWidgetPlugin::qSlicerMarkupsROIWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qSlicerMarkupsROIWidgetPlugin::createWidget(QWidget *_parent)
{
  qSlicerMarkupsROIWidget* _widget = new qSlicerMarkupsROIWidget(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qSlicerMarkupsROIWidgetPlugin::domXml() const
{
  return "<widget class=\"qSlicerMarkupsROIWidget\" \
          name=\"MRMLMarkupsROIWidget\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qSlicerMarkupsROIWidgetPlugin::includeFile() const
{
  return "qSlicerMarkupsROIWidget.h";
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsROIWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qSlicerMarkupsROIWidgetPlugin::name() const
{
  return "qSlicerMarkupsROIWidget";
}
