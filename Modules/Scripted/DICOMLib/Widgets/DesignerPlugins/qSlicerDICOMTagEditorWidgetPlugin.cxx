/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

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

#include "qSlicerDICOMTagEditorWidgetPlugin.h"
#include "qSlicerDICOMTagEditorWidget.h"

//-----------------------------------------------------------------------------
qSlicerDICOMTagEditorWidgetPlugin::qSlicerDICOMTagEditorWidgetPlugin(QObject* pluginParent)
  : QObject(pluginParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qSlicerDICOMTagEditorWidgetPlugin::createWidget(QWidget* parentWidget)
{
  qSlicerDICOMTagEditorWidget* pluginWidget =
    new qSlicerDICOMTagEditorWidget(parentWidget);
  return pluginWidget;
}

//-----------------------------------------------------------------------------
QString qSlicerDICOMTagEditorWidgetPlugin::domXml() const
{
  return "<widget class=\"qSlicerDICOMTagEditorWidget\" \
          name=\"DICOMTagEditorWidget\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qSlicerDICOMTagEditorWidgetPlugin::includeFile() const
{
  return "qSlicerDICOMTagEditorWidget.h";
}

//-----------------------------------------------------------------------------
bool qSlicerDICOMTagEditorWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qSlicerDICOMTagEditorWidgetPlugin::name() const
{
  return "qSlicerDICOMTagEditorWidget";
}
