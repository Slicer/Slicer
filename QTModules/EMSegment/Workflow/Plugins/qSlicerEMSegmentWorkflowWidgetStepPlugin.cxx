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

  This file was originally developed by Danielle Pace, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qSlicerEMSegmentWorkflowWidgetStepPlugin.h"
#include "qSlicerEMSegmentWorkflowWidgetStep.h"

//-----------------------------------------------------------------------------
qSlicerEMSegmentWorkflowWidgetStepPlugin::qSlicerEMSegmentWorkflowWidgetStepPlugin(QObject *newParent)
        : QObject(newParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qSlicerEMSegmentWorkflowWidgetStepPlugin::createWidget(QWidget *newParent)
{
  qSlicerEMSegmentWorkflowWidgetStep* _widget = new qSlicerEMSegmentWorkflowWidgetStep(newParent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentWorkflowWidgetStepPlugin::domXml() const
{
  return "<widget class=\"qSlicerEMSegmentWorkflowWidgetStep\" \
          name=\"SlicerEMSegmentWorkflowWidgetStep\">\n"
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
QIcon qSlicerEMSegmentWorkflowWidgetStepPlugin::icon() const
{
  return QIcon(":/Icons/widget.png");
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentWorkflowWidgetStepPlugin::includeFile() const
{
  return "qSlicerEMSegmentWorkflowWidgetStep.h";
}

//-----------------------------------------------------------------------------
bool qSlicerEMSegmentWorkflowWidgetStepPlugin::isContainer() const
{
  return true;
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentWorkflowWidgetStepPlugin::name() const
{
  return "qSlicerEMSegmentWorkflowWidgetStep";
}
