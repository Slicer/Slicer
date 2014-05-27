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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "qMRMLNodeAttributeTableWidgetPlugin.h"
#include "qMRMLNodeAttributeTableWidget.h"

//-----------------------------------------------------------------------------
qMRMLNodeAttributeTableWidgetPlugin::qMRMLNodeAttributeTableWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLNodeAttributeTableWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLNodeAttributeTableWidget* _widget = new qMRMLNodeAttributeTableWidget(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qMRMLNodeAttributeTableWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLNodeAttributeTableWidget\" \
          name=\"MRMLNodeAttributeTableWidget\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qMRMLNodeAttributeTableWidgetPlugin::includeFile() const
{
  return "qMRMLNodeAttributeTableWidget.h";
}

//-----------------------------------------------------------------------------
bool qMRMLNodeAttributeTableWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLNodeAttributeTableWidgetPlugin::name() const
{
  return "qMRMLNodeAttributeTableWidget";
}
