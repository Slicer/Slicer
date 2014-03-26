#ifndef __vtkModelMirrorLogic_h
#define __vtkModelMirrorLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include "vtkMRMLScene.h"

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// VTK includes
#include "vtkObject.h"
#include "vtkMatrix4x4.h"
#include "vtkPolyData.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLModelMirrorNode.h"

// STD includes
#include <string>
#include <vector>
#include <map>
#include <iterator>

#include "vtkSlicerModelMirrorModuleLogicExport.h"

// TODO Node registration needs to be done in the Logic. See RegisterNodes

class vtkIntArray;

class VTK_SLICER_MODELMIRROR_MODULE_LOGIC_EXPORT vtkModelMirrorLogic : public vtkSlicerModuleLogic
{
  public:
  static vtkModelMirrorLogic *New();
  vtkTypeMacro(vtkModelMirrorLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // These methods are used to turn observers on/off when module is entered/exited.
  virtual void Enter ( );
  virtual void Exit ( ) ;

  // Description:
  // Get/Set MRML node storing parameter values
  vtkGetObjectMacro (ModelMirrorNode, vtkMRMLModelMirrorNode);
  virtual void SetModelMirrorNode( vtkMRMLModelMirrorNode *node )
      {
        vtkSetMRMLNodeMacro ( this->ModelMirrorNode, node );
      }

  // Description:
  // Get/Set MRML node storing parameter values
  void SetAndObserveModelMirrorNode(vtkMRMLModelMirrorNode *n)
      {
      vtkSetAndObserveMRMLNodeMacro( this->ModelMirrorNode, n);
      }

  virtual vtkIntArray* NewObservableEvents();

  vtkGetObjectMacro ( MirrorMatrix, vtkMatrix4x4);

  vtkGetObjectMacro ( MirrorTransformNode, vtkMRMLLinearTransformNode );
  vtkSetObjectMacro ( MirrorTransformNode, vtkMRMLLinearTransformNode );


  virtual void CreateMirrorModel ();
  virtual void CreateMirrorTransform ( );
  virtual void DeleteMirrorTransform ( );
  virtual void CreateMirrorMatrix( );
  virtual int HardenTransform();
  virtual int  FlipNormals();
  virtual int PositionInHierarchy();

 protected:
  vtkModelMirrorLogic();
  ~vtkModelMirrorLogic();
  vtkModelMirrorLogic(const vtkModelMirrorLogic&);
  void operator=(const vtkModelMirrorLogic&);

  bool Visited;
  bool Raised;
  vtkMRMLModelMirrorNode *ModelMirrorNode;
  vtkMRMLLinearTransformNode *MirrorTransformNode;
  vtkMatrix4x4 *MirrorMatrix;

};


#endif

