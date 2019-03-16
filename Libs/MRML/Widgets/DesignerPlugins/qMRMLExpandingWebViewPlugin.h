/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Nicole Aucoin, BWH
==============================================================================*/

#ifndef __qMRMLExpandingWebViewPlugin_h
#define __qMRMLExpandingWebViewPlugin_h

#include "qMRMLWidgetsAbstractPlugin.h"

class QMRML_WIDGETS_PLUGINS_EXPORT qMRMLExpandingWebViewPlugin
  : public QObject,
  public qMRMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLExpandingWebViewPlugin(QObject *_parent = nullptr);

  QWidget *createWidget(QWidget *_parent) override;
  QString domXml() const override;
  QString includeFile() const override;
  bool isContainer() const override;
  QString name() const override;

};

#endif
