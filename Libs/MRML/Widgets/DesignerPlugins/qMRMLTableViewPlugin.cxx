/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

#include "qMRMLTableViewPlugin.h"
#include "qMRMLTableView.h"

//------------------------------------------------------------------------------
qMRMLTableViewPlugin::qMRMLTableViewPlugin(QObject *_parent)
: QObject(_parent)
{
}

//------------------------------------------------------------------------------
QWidget *qMRMLTableViewPlugin::createWidget(QWidget *_parent)
{
qMRMLTableView* _widget = new qMRMLTableView(_parent);
return _widget;
}

//------------------------------------------------------------------------------
QString qMRMLTableViewPlugin::domXml() const
{
return "<widget class=\"qMRMLTableView\" \
name=\"MRMLTableView\">\n"
"</widget>\n";
}

//------------------------------------------------------------------------------
QString qMRMLTableViewPlugin::includeFile() const
{
return "qMRMLTableView.h";
}

//------------------------------------------------------------------------------
bool qMRMLTableViewPlugin::isContainer() const
{
return false;
}

//------------------------------------------------------------------------------
QString qMRMLTableViewPlugin::name() const
{
return "qMRMLTableView";
}
