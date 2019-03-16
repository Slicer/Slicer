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

#ifndef __qMRMLTableView_p_h
#define __qMRMLTableView_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Qt includes
class QToolButton;

// VTK includes
#include <vtkWeakPointer.h>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>
class ctkPopupWidget;

// qMRML includes
#include "qMRMLTableView.h"

class vtkMRMLTableViewNode;
class vtkMRMLTableNode;
class vtkMRMLColorLogic;
class vtkMRMLColorNode;
class vtkMRMLDoubleArrayNode;
class vtkObject;
class vtkStringArray;

//-----------------------------------------------------------------------------
class qMRMLTableViewPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLTableView);
protected:
  qMRMLTableView* const q_ptr;
public:
  qMRMLTableViewPrivate(qMRMLTableView& object);
  ~qMRMLTableViewPrivate() override;

  virtual void init();

  void setMRMLScene(vtkMRMLScene* scene);
  vtkMRMLScene *mrmlScene();

  bool verifyTableModelAndNode(const char* methodName) const;

public slots:
  /// Handle MRML scene event
  void startProcessing();
  void endProcessing();

  void updateWidgetFromViewNode();

  /// slot when the view is configured to look at a different table node
  //void onTableNodeChanged();

protected:

  // Generate a string of options for a bar table
  QString barOptions(vtkMRMLTableNode*);

  vtkMRMLScene*                      MRMLScene;
  vtkMRMLTableViewNode*              MRMLTableViewNode;

  QToolButton*                       PinButton;
  ctkPopupWidget*                    PopupWidget;
};

#endif
