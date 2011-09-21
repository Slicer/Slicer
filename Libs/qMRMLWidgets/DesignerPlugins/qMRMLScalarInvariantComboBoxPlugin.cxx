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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qMRMLScalarInvariantComboBoxPlugin.h"
#include "qMRMLScalarInvariantComboBox.h"

//------------------------------------------------------------------------------
qMRMLScalarInvariantComboBoxPlugin
::qMRMLScalarInvariantComboBoxPlugin(QObject* parentObject)
  : QObject(parentObject)
{
}

//------------------------------------------------------------------------------
QWidget *qMRMLScalarInvariantComboBoxPlugin::createWidget(QWidget* parentWidget)
{
  qMRMLScalarInvariantComboBox* newWidget = new qMRMLScalarInvariantComboBox(parentWidget);
  return newWidget;
}

//------------------------------------------------------------------------------
QString qMRMLScalarInvariantComboBoxPlugin::domXml() const
{
  return "<widget class=\"qMRMLScalarInvariantComboBox\" \
          name=\"MRMLScalarInvariantComboBox\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QIcon qMRMLScalarInvariantComboBoxPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

//------------------------------------------------------------------------------
QString qMRMLScalarInvariantComboBoxPlugin::includeFile() const
{
  return "qMRMLScalarInvariantComboBox.h";
}

//------------------------------------------------------------------------------
bool qMRMLScalarInvariantComboBoxPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qMRMLScalarInvariantComboBoxPlugin::name() const
{
  return "qMRMLScalarInvariantComboBox";
}
