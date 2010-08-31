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

#include "qMRMLSlicesControllerToolBarPlugin.h"
#include "qMRMLSlicesControllerToolBar.h"

// --------------------------------------------------------------------------
qMRMLSlicesControllerToolBarPlugin::qMRMLSlicesControllerToolBarPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLSlicesControllerToolBarPlugin::createWidget(QWidget *_parent)
{
  qMRMLSlicesControllerToolBar* _widget = new qMRMLSlicesControllerToolBar(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLSlicesControllerToolBarPlugin::domXml() const
{
  return "<widget class=\"qMRMLSlicesControllerToolBar\" \
          name=\"MRMLSlicesControllerToolBar\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qMRMLSlicesControllerToolBarPlugin::icon() const
{
  return QIcon();
}

// --------------------------------------------------------------------------
QString qMRMLSlicesControllerToolBarPlugin::includeFile() const
{
  return "qMRMLSlicesControllerToolBar.h";
}

// --------------------------------------------------------------------------
bool qMRMLSlicesControllerToolBarPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLSlicesControllerToolBarPlugin::name() const
{
  return "qMRMLSlicesControllerToolBar";
}

