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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qMRMLSpinBoxPlugin.h"
#include "qMRMLSpinBox.h"

// --------------------------------------------------------------------------
qMRMLSpinBoxPlugin::qMRMLSpinBoxPlugin(QObject *_parent)
: QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLSpinBoxPlugin::createWidget(QWidget *_parent)
{
  qMRMLSpinBox* _widget = new qMRMLSpinBox(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLSpinBoxPlugin::domXml() const
{
  return "<widget class=\"qMRMLSpinBox\" \
                  name=\"MRMLSpinBox\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qMRMLSpinBoxPlugin::icon() const
{
  return QIcon(":/Icons/spinbox.png");
}

// --------------------------------------------------------------------------
QString qMRMLSpinBoxPlugin::includeFile() const
{
  return "qMRMLSpinBox.h";
}

// --------------------------------------------------------------------------
bool qMRMLSpinBoxPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLSpinBoxPlugin::name() const
{
  return "qMRMLSpinBox";
}
