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

#include "qMRMLMarkupsROIWidgetPlugin.h"
#include "qMRMLMarkupsROIWidget.h"

//-----------------------------------------------------------------------------
qMRMLMarkupsROIWidgetPlugin::qMRMLMarkupsROIWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLMarkupsROIWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLMarkupsROIWidget* _widget = new qMRMLMarkupsROIWidget(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qMRMLMarkupsROIWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLMarkupsROIWidget\" \
          name=\"MRMLMarkupsROIWidget\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qMRMLMarkupsROIWidgetPlugin::includeFile() const
{
  return "qMRMLMarkupsROIWidget.h";
}

//-----------------------------------------------------------------------------
bool qMRMLMarkupsROIWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLMarkupsROIWidgetPlugin::name() const
{
  return "qMRMLMarkupsROIWidget";
}
