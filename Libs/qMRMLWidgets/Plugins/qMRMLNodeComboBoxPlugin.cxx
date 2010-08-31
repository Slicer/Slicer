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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qMRMLNodeComboBoxPlugin.h"
#include "qMRMLNodeComboBox.h"

qMRMLNodeComboBoxPlugin::qMRMLNodeComboBoxPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qMRMLNodeComboBoxPlugin::createWidget(QWidget *_parent)
{
  qMRMLNodeComboBox* _widget = new qMRMLNodeComboBox(_parent);
  return _widget;
}

QString qMRMLNodeComboBoxPlugin::domXml() const
{
  return "<widget class=\"qMRMLNodeComboBox\" \
          name=\"MRMLNodeComboBox\">\n"
          " <property name=\"geometry\">\n"
          "  <rect>\n"
          "   <x>0</x>\n"
          "   <y>0</y>\n"
          "   <width>200</width>\n"
          "   <height>20</height>\n"
          "  </rect>\n"
          " </property>\n"
          "</widget>\n";
}

QIcon qMRMLNodeComboBoxPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

QString qMRMLNodeComboBoxPlugin::includeFile() const
{
  return "qMRMLNodeComboBox.h";
}

bool qMRMLNodeComboBoxPlugin::isContainer() const
{
  return false;
}

QString qMRMLNodeComboBoxPlugin::name() const
{
  return "qMRMLNodeComboBox";
}
