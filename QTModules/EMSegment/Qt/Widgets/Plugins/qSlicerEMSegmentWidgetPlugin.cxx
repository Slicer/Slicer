/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qSlicerEMSegmentWidgetPlugin.h"
#include "qSlicerEMSegmentWidget.h"

//-----------------------------------------------------------------------------
qSlicerEMSegmentWidgetPlugin::qSlicerEMSegmentWidgetPlugin(QObject *newParent)
        : QObject(newParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qSlicerEMSegmentWidgetPlugin::createWidget(QWidget *newParent)
{
  qSlicerEMSegmentWidget* _widget = new qSlicerEMSegmentWidget(newParent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentWidgetPlugin::domXml() const
{
  return "<widget class=\"qSlicerEMSegmentWidget\" \
          name=\"SlicerEMSegmentWidget\">\n"
          " <property name=\"geometry\">\n"
          "  <rect>\n"
          "   <x>0</x>\n"
          "   <y>0</y>\n"
          "   <width>200</width>\n"
          "   <height>200</height>\n"
          "  </rect>\n"
          " </property>\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QIcon qSlicerEMSegmentWidgetPlugin::icon() const
{
  return QIcon(":/Icons/widget.png");
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentWidgetPlugin::includeFile() const
{
  return "qSlicerEMSegmentWidget.h";
}

//-----------------------------------------------------------------------------
bool qSlicerEMSegmentWidgetPlugin::isContainer() const
{
  return true;
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentWidgetPlugin::name() const
{
  return "qSlicerEMSegmentWidget";
}
