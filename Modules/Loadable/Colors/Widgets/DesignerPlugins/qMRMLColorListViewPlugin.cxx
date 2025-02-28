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

#include "qMRMLColorListViewPlugin.h"
#include "qMRMLColorListView.h"

qMRMLColorListViewPlugin::qMRMLColorListViewPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qMRMLColorListViewPlugin::createWidget(QWidget *_parent)
{
  qMRMLColorListView* _widget = new qMRMLColorListView(_parent);
  return _widget;
}

QString qMRMLColorListViewPlugin::domXml() const
{
  return "<widget class=\"qMRMLColorListView\" \
          name=\"MRMLColorListView\">\n"
          "</widget>\n";
}

QIcon qMRMLColorListViewPlugin::icon() const
{
  return QIcon(":Icons/listbox.png");
}

QString qMRMLColorListViewPlugin::includeFile() const
{
  return "qMRMLColorListView.h";
}

bool qMRMLColorListViewPlugin::isContainer() const
{
  return false;
}

QString qMRMLColorListViewPlugin::name() const
{
  return "qMRMLColorListView";
}
