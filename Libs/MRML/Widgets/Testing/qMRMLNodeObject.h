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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef qMRMLNodeObject_h
#define qMRMLNodeObject_h

#include <QObject>
#include <vtkMRMLNode.h>

class qMRMLNodeObject : public QObject
{
  Q_OBJECT
public:
  qMRMLNodeObject(vtkMRMLNode* node, QObject* parent = nullptr);

  void setProcessEvents(bool process);
  bool processEvents()const;

  void setMessage(const QString& message);
  QString message()const;

public slots:
  void modify();

protected:
  vtkMRMLNode* Node;
  bool         ProcessEvents;
  QString      Message;
};

#endif
