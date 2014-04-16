/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/


#include "qMRMLTransformDisplayNodeWidgetPlugin.h"
#include "qMRMLTransformDisplayNodeWidget.h"

//------------------------------------------------------------------------------
qMRMLTransformDisplayNodeWidgetPlugin::qMRMLTransformDisplayNodeWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{
}

//------------------------------------------------------------------------------
QWidget *qMRMLTransformDisplayNodeWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLTransformDisplayNodeWidget* _widget = new qMRMLTransformDisplayNodeWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qMRMLTransformDisplayNodeWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLTransformDisplayNodeWidget\" \
          name=\"MRMLTransformDisplayNodeWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qMRMLTransformDisplayNodeWidgetPlugin::includeFile() const
{
  return "qMRMLTransformDisplayNodeWidget.h";
}

//------------------------------------------------------------------------------
bool qMRMLTransformDisplayNodeWidgetPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qMRMLTransformDisplayNodeWidgetPlugin::name() const
{
  return "qMRMLTransformDisplayNodeWidget";
}
