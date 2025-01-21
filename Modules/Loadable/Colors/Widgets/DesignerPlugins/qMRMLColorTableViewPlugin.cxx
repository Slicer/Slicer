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

#include "qMRMLColorTableViewPlugin.h"
#include "qMRMLColorTableView.h"

qMRMLColorTableViewPlugin::qMRMLColorTableViewPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qMRMLColorTableViewPlugin::createWidget(QWidget *_parent)
{
  qMRMLColorTableView* _widget = new qMRMLColorTableView(_parent);
  return _widget;
}

QString qMRMLColorTableViewPlugin::domXml() const
{
  return "<widget class=\"qMRMLColorTableView\" \
          name=\"MRMLColorTableView\">\n"
          "</widget>\n";
}

QIcon qMRMLColorTableViewPlugin::icon() const
{
  return QIcon(":Icons/table.png");
}

QString qMRMLColorTableViewPlugin::includeFile() const
{
  return "qMRMLColorTableView.h";
}

bool qMRMLColorTableViewPlugin::isContainer() const
{
  return false;
}

QString qMRMLColorTableViewPlugin::name() const
{
  return "qMRMLColorTableView";
}
