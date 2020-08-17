/*==============================================================================

  Program: 3D Slicer

  Copyright (c)

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MRMLDisplayableManager includes
#include "vtkMRMLModelSliceDisplayableManager.h"
#include "vtkMRMLModelDisplayableManager.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLColorNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLFolderDisplayNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLProceduralColorNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkVersion.h> // must precede reference to VTK_MAJOR_VERSION
#include <vtkActor2D.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCallbackCommand.h>
#include <vtkColorTransferFunction.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkEventBroker.h>
#include <vtkGeneralTransform.h>
#include <vtkLookupTable.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPointLocator.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkWeakPointer.h>

// VTK includes: customization
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
  #include <vtkCompositeDataGeometryFilter.h>
  #include <vtkPlaneCutter.h>
#else
  #include <vtkCutter.h>
#endif
#include <vtkSampleImplicitFunctionFilter.h>

// STD includes
#include <algorithm>
#include <cassert>
#include <set>
#include <map>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLModelSliceDisplayableManager );

//---------------------------------------------------------------------------
class vtkMRMLModelSliceDisplayableManager::vtkInternal
{
public:
  struct Pipeline
    {
    vtkSmartPointer<vtkGeneralTransform> NodeToWorld;
    vtkSmartPointer<vtkTransform> TransformToSlice;
    vtkSmartPointer<vtkTransformPolyDataFilter> Transformer;
    vtkSmartPointer<vtkDataSetSurfaceFilter> SurfaceExtractor;
    vtkSmartPointer<vtkTransformFilter> ModelWarper;
    vtkSmartPointer<vtkPlane> Plane;
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
    vtkSmartPointer<vtkPlaneCutter> Cutter;
    vtkSmartPointer<vtkCompositeDataGeometryFilter> GeometryFilter; // appends multiple cut pieces into a single polydata
#else
    vtkSmartPointer<vtkCutter> Cutter;
#endif
    vtkSmartPointer<vtkSampleImplicitFunctionFilter> SliceDistance;
    vtkSmartPointer<vtkProp> Actor;
    };

  typedef std::map < vtkMRMLDisplayNode*, const Pipeline* > PipelinesCacheType;
  PipelinesCacheType DisplayPipelines;

  typedef std::map < vtkMRMLDisplayableNode*, std::set< vtkMRMLDisplayNode* > > ModelToDisplayCacheType;
  ModelToDisplayCacheType ModelToDisplayNodes;

  // Transforms
  void UpdateDisplayableTransforms(vtkMRMLDisplayableNode *node);
  void GetNodeTransformToWorld(vtkMRMLTransformableNode* node, vtkGeneralTransform* transformToWorld);
  // Slice Node
  void SetSliceNode(vtkMRMLSliceNode* sliceNode);
  void UpdateSliceNode();
  void SetSlicePlaneFromMatrix(vtkMatrix4x4* matrix, vtkPlane* plane);

  // Display Nodes
  void AddDisplayNode(vtkMRMLDisplayableNode*, vtkMRMLDisplayNode*);
  void UpdateDisplayNode(vtkMRMLDisplayNode* displayNode);
  void UpdateDisplayNodePipeline(vtkMRMLDisplayNode*, const Pipeline*);
  void RemoveDisplayNode(vtkMRMLDisplayNode* displayNode);

  // Observations
  void AddObservations(vtkMRMLDisplayableNode* node);
  void RemoveObservations(vtkMRMLDisplayableNode* node);
  bool IsNodeObserved(vtkMRMLDisplayableNode* node);

  // Helper functions
  bool IsVisible(vtkMRMLDisplayNode* displayNode);
  bool UseDisplayNode(vtkMRMLDisplayNode* displayNode);
  bool UseDisplayableNode(vtkMRMLDisplayableNode* displayNode);
  void ClearDisplayableNodes();

  vtkInternal( vtkMRMLModelSliceDisplayableManager* external );
  ~vtkInternal();

private:
  vtkSmartPointer<vtkMatrix4x4> SliceXYToRAS;
  vtkSmartPointer<vtkMRMLSliceNode> SliceNode;
  vtkMRMLModelSliceDisplayableManager* External;
};

//---------------------------------------------------------------------------
// vtkInternal methods
vtkMRMLModelSliceDisplayableManager::vtkInternal
::vtkInternal(vtkMRMLModelSliceDisplayableManager* external)
{
  this->External = external;
  this->SliceXYToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
  this->SliceXYToRAS->Identity();
}

//---------------------------------------------------------------------------
vtkMRMLModelSliceDisplayableManager::vtkInternal
::~vtkInternal()
{
  this->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
bool vtkMRMLModelSliceDisplayableManager::vtkInternal::UseDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  // Check whether DisplayNode should be shown in this view
  bool show = displayNode
              && displayNode->IsA("vtkMRMLModelDisplayNode")
              && ( !displayNode->IsA("vtkMRMLFiberBundleLineDisplayNode") )
              && ( !displayNode->IsA("vtkMRMLFiberBundleGlyphDisplayNode") ) ;
  return show;
}

//---------------------------------------------------------------------------
bool vtkMRMLModelSliceDisplayableManager::vtkInternal::IsVisible(vtkMRMLDisplayNode* displayNode)
{
  if (!displayNode)
    {
    return false;
    }
  if (vtkMRMLSliceLogic::IsSliceModelDisplayNode(displayNode))
    {
    // slice intersections are displayed by vtkMRMLCrosshairDisplayableManager
    return false;
    }
  bool visibleOnNode = true;
  vtkMRMLSliceNode* sliceNode = this->SliceNode;
  if (sliceNode)
    {
    visibleOnNode = displayNode->GetVisibility(sliceNode->GetID());
    }
  else
    {
    visibleOnNode = (displayNode->GetVisibility() == 1);
    }
  return visibleOnNode && (displayNode->GetVisibility2D() != 0) ;
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::SetSliceNode(vtkMRMLSliceNode* sliceNode)
{
  if (!sliceNode || this->SliceNode == sliceNode)
    {
    return;
    }
  this->SliceNode = sliceNode;
  this->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::UpdateSliceNode()
{
  // Update the Slice node transform
  //   then update the DisplayNode pipelines to account for plane location

  this->SliceXYToRAS->DeepCopy( this->SliceNode->GetXYToRAS() );
  PipelinesCacheType::iterator it;
  for (it = this->DisplayPipelines.begin(); it != this->DisplayPipelines.end(); ++it)
    {
    this->UpdateDisplayNodePipeline(it->first, it->second);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::SetSlicePlaneFromMatrix(vtkMatrix4x4* sliceMatrix, vtkPlane* plane)
{
  double normal[3];
  double origin[3];

  // +/-1: orientation of the normal
  const int planeOrientation = 1;
  for (int i = 0; i < 3; i++)
    {
    normal[i] = planeOrientation * sliceMatrix->GetElement(i,2);
    origin[i] = sliceMatrix->GetElement(i,3);
    }

  vtkMath::Normalize(normal);
  plane->SetNormal(normal);
  plane->SetOrigin(origin);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::GetNodeTransformToWorld(vtkMRMLTransformableNode* node, vtkGeneralTransform* transformToWorld)
{
  if (!node || !transformToWorld)
    {
    return;
    }

  vtkMRMLTransformNode* tnode =
    node->GetParentTransformNode();

  transformToWorld->Identity();
  if (tnode)
    {
    tnode->GetTransformToWorld(transformToWorld);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::UpdateDisplayableTransforms(vtkMRMLDisplayableNode* mNode)
{
  // Update the NodeToWorld matrix for all tracked DisplayableNode

  PipelinesCacheType::iterator pipelinesIter;
  std::set<vtkMRMLDisplayNode *> displayNodes = this->ModelToDisplayNodes[mNode];
  std::set<vtkMRMLDisplayNode *>::iterator dnodesIter;
  for ( dnodesIter = displayNodes.begin(); dnodesIter != displayNodes.end(); dnodesIter++ )
    {
    if ( ((pipelinesIter = this->DisplayPipelines.find(*dnodesIter)) != this->DisplayPipelines.end()) )
      {
      this->GetNodeTransformToWorld( mNode, pipelinesIter->second->NodeToWorld);
      this->UpdateDisplayNodePipeline(pipelinesIter->first, pipelinesIter->second);
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::RemoveDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  PipelinesCacheType::iterator actorsIt = this->DisplayPipelines.find(displayNode);
  if(actorsIt == this->DisplayPipelines.end())
    {
    return;
    }
  const Pipeline* pipeline = actorsIt->second;
  this->External->GetRenderer()->RemoveActor(pipeline->Actor);
  delete pipeline;
  this->DisplayPipelines.erase(actorsIt);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::AddDisplayNode(vtkMRMLDisplayableNode* mNode, vtkMRMLDisplayNode* displayNode)
{
  if (!mNode || !displayNode)
    {
    return;
    }

  // Do not add the display node if it is already associated with a pipeline object.
  // This happens when a model node already associated with a display node
  // is copied into an other (using vtkMRMLNode::Copy()) and is added to the scene afterward.
  // Related issue are #3428 and #2608
  PipelinesCacheType::iterator it;
  it = this->DisplayPipelines.find(displayNode);
  if (it != this->DisplayPipelines.end())
    {
    return;
    }

  vtkNew<vtkActor2D> actor;
  if (displayNode->IsA("vtkMRMLModelDisplayNode"))
    {
    actor->SetMapper( vtkNew<vtkPolyDataMapper2D>().GetPointer() );
    }

  // Create pipeline
  Pipeline* pipeline = new Pipeline();
  pipeline->Actor = actor.GetPointer();
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
  pipeline->Cutter = vtkSmartPointer<vtkPlaneCutter>::New();
  pipeline->GeometryFilter = vtkSmartPointer<vtkCompositeDataGeometryFilter>::New();
#else
  pipeline->Cutter = vtkSmartPointer<vtkCutter>::New();
#endif
  pipeline->SliceDistance = vtkSmartPointer<vtkSampleImplicitFunctionFilter>::New();
  pipeline->TransformToSlice = vtkSmartPointer<vtkTransform>::New();
  pipeline->NodeToWorld = vtkSmartPointer<vtkGeneralTransform>::New();
  pipeline->Transformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  pipeline->ModelWarper = vtkSmartPointer<vtkTransformFilter>::New();
  pipeline->SurfaceExtractor = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
  pipeline->Plane = vtkSmartPointer<vtkPlane>::New();

  // Set up pipeline
  pipeline->Transformer->SetTransform(pipeline->TransformToSlice);
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
  pipeline->Transformer->SetInputConnection(pipeline->GeometryFilter->GetOutputPort());
  pipeline->Cutter->SetPlane(pipeline->Plane);
  pipeline->Cutter->BuildTreeOff(); // the cutter crashes for complex geometries if build tree is enabled
  pipeline->Cutter->SetInputConnection(pipeline->ModelWarper->GetOutputPort());
  pipeline->GeometryFilter->SetInputConnection(pipeline->Cutter->GetOutputPort());
#else
  pipeline->Transformer->SetInputConnection(pipeline->Cutter->GetOutputPort());
  pipeline->Cutter->SetCutFunction(pipeline->Plane);
  pipeline->Cutter->SetGenerateCutScalars(0);
  pipeline->Cutter->SetInputConnection(pipeline->ModelWarper->GetOutputPort());
#endif
  // Projection is created from outer surface of volumetric meshes (for polydata surface
  // extraction is just shallow-copy)
  pipeline->SurfaceExtractor->SetInputConnection(pipeline->ModelWarper->GetOutputPort());
  pipeline->SliceDistance->SetImplicitFunction(pipeline->Plane);
  pipeline->SliceDistance->SetInputConnection(pipeline->SurfaceExtractor->GetOutputPort());
  pipeline->Actor->SetVisibility(0);

  // Add actor to Renderer and local cache
  this->External->GetRenderer()->AddActor( pipeline->Actor );
  this->DisplayPipelines.insert( std::make_pair(displayNode, pipeline) );

  // Update cached matrices. Calls UpdateDisplayNodePipeline
  this->UpdateDisplayableTransforms(mNode);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::UpdateDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  // If the DisplayNode already exists, just update.
  //   otherwise, add as new node

  if (!displayNode)
    {
    return;
    }
  PipelinesCacheType::iterator it;
  it = this->DisplayPipelines.find(displayNode);
  if (it != this->DisplayPipelines.end())
    {
    this->UpdateDisplayNodePipeline(displayNode, it->second);
    }
  else
    {
    this->External->AddDisplayableNode( displayNode->GetDisplayableNode() );
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::UpdateDisplayNodePipeline(vtkMRMLDisplayNode* displayNode, const Pipeline* pipeline)
{
  // Sets visibility, set pipeline mesh input, update color
  //   calculate and set pipeline transforms.

  if (!pipeline)
    {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLModelSliceDisplayableManager::"
      "vtkInternal::UpdateDisplayNodePipeline failed: pipeline is invalid");
    return;
    }

  vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(displayNode);
  if (!modelDisplayNode)
    {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLModelSliceDisplayableManager::"
      "vtkInternal::UpdateDisplayNodePipeline failed: vtkMRMLModelDisplayNode display node type is expected");
    pipeline->Actor->SetVisibility(false);
    return;
    }

  // Get display node from hierarchy that applies display properties on branch
  vtkMRMLDisplayableNode* displayableNode = displayNode->GetDisplayableNode();
  vtkMRMLDisplayNode* overrideHierarchyDisplayNode =
    vtkMRMLFolderDisplayNode::GetOverridingHierarchyDisplayNode(displayableNode);

  // Use hierarchy display node if any, and if overriding is allowed for the current display node.
  // If override is explicitly disabled, then do not apply hierarchy visibility or opacity either.
  bool hierarchyVisibility = true;
  double hierarchyOpacity = 1.0;
  if (displayNode->GetFolderDisplayOverrideAllowed())
    {
    if (overrideHierarchyDisplayNode)
      {
      displayNode = overrideHierarchyDisplayNode;
      }

    // Get visibility and opacity defined by the hierarchy.
    // These two properties are influenced by the hierarchy regardless the fact whether there is override
    // or not. Visibility of items defined by hierarchy is off if any of the ancestors is explicitly hidden,
    // and the opacity is the product of the ancestors' opacities.
    // However, this does not apply on display nodes that do not allow overrides (FolderDisplayOverrideAllowed)
    hierarchyVisibility = vtkMRMLFolderDisplayNode::GetHierarchyVisibility(displayableNode);
    hierarchyOpacity = vtkMRMLFolderDisplayNode::GetHierarchyOpacity(displayableNode);
    }

  if (!hierarchyVisibility || !this->IsVisible(displayNode))
    {
    pipeline->Actor->SetVisibility(false);
    return;
    }

  vtkPointSet* pointSet = modelDisplayNode->GetOutputMesh();
  if (!pointSet)
    {
    pipeline->Actor->SetVisibility(false);
    return;
    }

  // Need this to update bounds of the locator, to avoid crash in the cutter
  modelDisplayNode->GetOutputMeshConnection()->GetProducer()->Update();

  if (!pointSet->GetPoints() || pointSet->GetNumberOfPoints() == 0)
    {
    // there are no points, so there is nothing to cut
    pipeline->Actor->SetVisibility(false);
    return;
    }

  pipeline->ModelWarper->SetInputData(pointSet); //why here? +connection?
  pipeline->ModelWarper->SetTransform(pipeline->NodeToWorld);

  // Set Plane Transform
  this->SetSlicePlaneFromMatrix(this->SliceXYToRAS, pipeline->Plane);
  pipeline->Plane->Modified();

  if (modelDisplayNode->GetSliceDisplayMode() == vtkMRMLModelDisplayNode::SliceDisplayProjection
    || modelDisplayNode->GetSliceDisplayMode() == vtkMRMLModelDisplayNode::SliceDisplayDistanceEncodedProjection)
    {

    if (modelDisplayNode->GetSliceDisplayMode() == vtkMRMLModelDisplayNode::SliceDisplayProjection)
      {
      // remove cutter from the pipeline if projection mode is used, we just need to extract surface
      // and flatten the model
      pipeline->Transformer->SetInputConnection(pipeline->SurfaceExtractor->GetOutputPort());
      }
    else
      {
      // replace cutter in the pipeline by surface extraction, slice distance computation,
      // and flattening of the model
      pipeline->Transformer->SetInputConnection(pipeline->SliceDistance->GetOutputPort());
      }

    //  Set Poly Data Transform that projects model to the slice plane
    vtkNew<vtkMatrix4x4> rasToSliceXY;
    vtkMatrix4x4::Invert(this->SliceXYToRAS, rasToSliceXY.GetPointer());
    // Project all points to the slice plane (slice Z coordinate = 0)
    rasToSliceXY->SetElement(2, 0, 0);
    rasToSliceXY->SetElement(2, 1, 0);
    rasToSliceXY->SetElement(2, 2, 0);
    pipeline->TransformToSlice->SetMatrix(rasToSliceXY.GetPointer());
    }
  else
    {
    // show intersection in the slice view
    // include clipper in the pipeline
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
    pipeline->Transformer->SetInputConnection(pipeline->GeometryFilter->GetOutputPort());
#else
    pipeline->Transformer->SetInputConnection(pipeline->Cutter->GetOutputPort());

    // optimization for slice to slice intersections which are 1 quad polydatas
    // no need for 50^3 default locator divisions
    if (pointSet->GetPoints() != nullptr && pointSet->GetNumberOfPoints() <= 4)
    {
      vtkNew<vtkPointLocator> locator;
      double *bounds = pointSet->GetBounds();
      locator->SetDivisions(2, 2, 2);
      locator->InitPointInsertion(pointSet->GetPoints(), bounds);
      pipeline->Cutter->SetLocator(locator.GetPointer());
    }
#endif
    pipeline->Cutter->SetInputConnection(pipeline->ModelWarper->GetOutputPort());

#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
    // If there is no input or if the input has no points, the vtkTransformPolyDataFilter will display an error message
    // on every update: "No input data".
    // To prevent the error, if the input is empty then the actor should not be visible since there is nothing to display.
    pipeline->GeometryFilter->Update();
    if (!pipeline->GeometryFilter->GetOutput() || pipeline->GeometryFilter->GetOutput()->GetNumberOfPoints() < 1)
      {
      pipeline->Actor->SetVisibility(false);
      return;
      }
#endif

    //  Set Poly Data Transform
    vtkNew<vtkMatrix4x4> rasToSliceXY;
    vtkMatrix4x4::Invert(this->SliceXYToRAS, rasToSliceXY.GetPointer());
    pipeline->TransformToSlice->SetMatrix(rasToSliceXY.GetPointer());
    }

  // Update pipeline actor
  vtkActor2D* actor = vtkActor2D::SafeDownCast(pipeline->Actor);
  vtkPolyDataMapper2D* mapper = vtkPolyDataMapper2D::SafeDownCast(actor->GetMapper());

  if (mapper)
    {
    mapper->SetInputConnection( pipeline->Transformer->GetOutputPort() );

    if (modelDisplayNode->GetSliceDisplayMode() == vtkMRMLModelDisplayNode::SliceDisplayDistanceEncodedProjection)
      {
      vtkMRMLColorNode* colorNode = modelDisplayNode->GetDistanceEncodedProjectionColorNode();
      vtkSmartPointer<vtkScalarsToColors> lut = nullptr;
      vtkSmartPointer<vtkMRMLProceduralColorNode> proceduralColor = vtkMRMLProceduralColorNode::SafeDownCast(colorNode);
      if (proceduralColor)
        {
        lut = vtkScalarsToColors::SafeDownCast(proceduralColor->GetColorTransferFunction());
        }
      else
        {
        vtkSmartPointer<vtkLookupTable> dNodeLUT = vtkSmartPointer<vtkLookupTable>::Take(colorNode ? colorNode->CreateLookupTableCopy() : nullptr);
        if (dNodeLUT)
          {
          mapper->SetScalarRange(displayNode->GetScalarRange());
          lut->SetAlpha(hierarchyOpacity * displayNode->GetSliceIntersectionOpacity());
          }
        }

      if (lut != nullptr)
        {
        mapper->SetLookupTable(lut.GetPointer());
        mapper->SetScalarRange(lut->GetRange());
        mapper->SetScalarVisibility(true);
        }
      else
        {
        mapper->SetScalarVisibility(false);
        }
      }
    else if (displayNode->GetScalarVisibility())
      {
      // Check if using point data or cell data
      vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(displayableNode);
      if (vtkMRMLModelDisplayableManager::IsCellScalarsActive(displayNode, modelNode))
        {
        mapper->SetScalarModeToUseCellData();
        }
      else
        {
        mapper->SetScalarModeToUsePointData();
        }

      if (displayNode->GetScalarRangeFlag() == vtkMRMLDisplayNode::UseDirectMapping)
        {
        mapper->SetColorModeToDirectScalars();
        mapper->SetLookupTable(nullptr);
        }
      else
        {
        mapper->SetColorModeToMapScalars();

        // The renderer uses the lookup table scalar range to
        // render colors. By default, UseLookupTableScalarRange
        // is set to false and SetScalarRange can be used on the
        // mapper to map scalars into the lookup table. When set
        // to true, SetScalarRange has no effect and it is necessary
        // to force the scalarRange on the lookup table manually.
        // Whichever way is used, the look up table range needs
        // to be changed to render the correct scalar values, thus
        // one lookup table can not be shared by multiple mappers
        // if any of those mappers needs to map using its scalar
        // values range. It is therefore necessary to make a copy
        // of the colorNode vtkLookupTable in order not to impact
        // that lookup table original range.
        vtkSmartPointer<vtkLookupTable> dNodeLUT = vtkSmartPointer<vtkLookupTable>::Take(displayNode->GetColorNode() ?
          displayNode->GetColorNode()->CreateLookupTableCopy() : nullptr);
        if (dNodeLUT)
          {
          dNodeLUT->SetAlpha(hierarchyOpacity * displayNode->GetSliceIntersectionOpacity());
          }
        mapper->SetLookupTable(dNodeLUT);
        }

      // Set scalar range
      mapper->SetScalarRange(displayNode->GetScalarRange());

      mapper->SetScalarVisibility(true);
      }
    else
      {
      mapper->SetScalarVisibility(false);
      }
    }

  vtkProperty2D* actorProperties = actor->GetProperty();
  actorProperties->SetColor(displayNode->GetColor());
  actorProperties->SetLineWidth(displayNode->GetSliceIntersectionThickness());
  actorProperties->SetOpacity(hierarchyOpacity * displayNode->GetSliceIntersectionOpacity());

  // Opacity of the slice intersection is intentionally not set by
  // actorProperties->SetOpacity(displayNode->GetOpacity()),
  // because most often users only want to make 3D model transparent.
  // Visibility of slice intersections can be tuned by modifying
  // slice intersection thickness or a new SliceIntersectionOpacity attribute
  // could be introduced.

  actor->SetPosition(0,0);
  actor->SetVisibility(true);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::AddObservations(vtkMRMLDisplayableNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;

  if (!broker->GetObservationExist(node, vtkMRMLDisplayableNode::TransformModifiedEvent,
                                          this->External, this->External->GetMRMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkMRMLDisplayableNode::TransformModifiedEvent,
                            this->External, this->External->GetMRMLNodesCallbackCommand() );
    }

  if (!broker->GetObservationExist(node, vtkMRMLDisplayableNode::DisplayModifiedEvent,
                                          this->External, this->External->GetMRMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkMRMLDisplayableNode::DisplayModifiedEvent,
                            this->External, this->External->GetMRMLNodesCallbackCommand() );
    }

  if (!broker->GetObservationExist(node, vtkMRMLModelNode::PolyDataModifiedEvent,
                                          this->External, this->External->GetMRMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkMRMLModelNode::PolyDataModifiedEvent,
                            this->External, this->External->GetMRMLNodesCallbackCommand() );
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::RemoveObservations(vtkMRMLDisplayableNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(
    node, vtkMRMLModelNode::PolyDataModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(
    node, vtkMRMLDisplayableNode::TransformModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
bool vtkMRMLModelSliceDisplayableManager::vtkInternal
::IsNodeObserved(vtkMRMLDisplayableNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkCollection* observations;
  observations = broker->GetObservationsForSubject(node);
  if (observations->GetNumberOfItems() > 0)
    {
    return true;
    }
  else
    {
    return false;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::vtkInternal
::ClearDisplayableNodes()
{
  while(this->ModelToDisplayNodes.size() > 0)
    {
    this->External->RemoveDisplayableNode(this->ModelToDisplayNodes.begin()->first);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLModelSliceDisplayableManager::vtkInternal
::UseDisplayableNode(vtkMRMLDisplayableNode* node)
{
  bool show = node && node->IsA("vtkMRMLModelNode");

  if (!this->SliceNode->GetLayoutName())
    {
    vtkErrorWithObjectMacro(this->External, "No layout name to slice node " << this->SliceNode->GetID());
    return false;
    }
  // Ignore the slice model node for this slice DM.
  std::string cmpstr = std::string(this->SliceNode->GetLayoutName()) + " Volume Slice";
  show = show && ( cmpstr.compare(node->GetName()) );

  return show;
}

//---------------------------------------------------------------------------
// vtkMRMLModelSliceDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLModelSliceDisplayableManager::vtkMRMLModelSliceDisplayableManager()
{
  this->Internal = new vtkInternal(this);
  this->AddingDisplayableNode = 0;
}

//---------------------------------------------------------------------------
vtkMRMLModelSliceDisplayableManager::~vtkMRMLModelSliceDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::AddDisplayableNode(
  vtkMRMLDisplayableNode* node)
{
  if (this->AddingDisplayableNode)
    {
    return;
    }
  // Check if node should be used
  if (!this->Internal->UseDisplayableNode(node))
    {
    return;
    }

  this->AddingDisplayableNode = 1;
  // Add Display Nodes
  int nnodes = node->GetNumberOfDisplayNodes();

  this->Internal->AddObservations(node);

  for (int i=0; i<nnodes; i++)
  {
    vtkMRMLDisplayNode *dnode = node->GetNthDisplayNode(i);
    if ( this->Internal->UseDisplayNode(dnode) )
      {
      this->Internal->ModelToDisplayNodes[node].insert(dnode);
      this->Internal->AddDisplayNode( node, dnode );
      }
    }
  this->AddingDisplayableNode = 0;
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager
::RemoveDisplayableNode(vtkMRMLDisplayableNode* node)
{
  if (!node)
    {
    return;
    }
  vtkInternal::ModelToDisplayCacheType::iterator displayableIt =
    this->Internal->ModelToDisplayNodes.find(node);
  if(displayableIt == this->Internal->ModelToDisplayNodes.end())
    {
    return;
    }

  std::set<vtkMRMLDisplayNode *> dnodes = displayableIt->second;
  std::set<vtkMRMLDisplayNode *>::iterator diter;
  for ( diter = dnodes.begin(); diter != dnodes.end(); ++diter)
    {
    this->Internal->RemoveDisplayNode(*diter);
    }
  this->Internal->RemoveObservations(node);
  this->Internal->ModelToDisplayNodes.erase(displayableIt);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager
::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node->IsA("vtkMRMLModelNode"))
    {
    return;
    }

  // Escape if the scene a scene is being closed, imported or connected
  if (this->GetMRMLScene()->IsBatchProcessing())
    {
    this->SetUpdateFromMRMLRequested(true);
    return;
    }

  this->AddDisplayableNode(vtkMRMLDisplayableNode::SafeDownCast(node));
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager
::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if ( node && !node->IsA("vtkMRMLModelNode")
       && !node->IsA("vtkMRMLModelDisplayNode") )
    {
    return;
    }

  vtkMRMLDisplayableNode* modelNode = nullptr;
  vtkMRMLDisplayNode* displayNode = nullptr;

  bool modified = false;
  if ( (modelNode = vtkMRMLDisplayableNode::SafeDownCast(node)) )
    {
    this->RemoveDisplayableNode(modelNode);
    modified = true;
    }
  else if ( (displayNode = vtkMRMLDisplayNode::SafeDownCast(node)) )
    {
    this->Internal->RemoveDisplayNode(displayNode);
    modified = true;
    }
  if (modified)
    {
    this->RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager
::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkMRMLScene* scene = this->GetMRMLScene();

  if ( scene->IsBatchProcessing() )
    {
    return;
    }

  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(caller);

  if ( displayableNode )
    {
    vtkMRMLNode* callDataNode = reinterpret_cast<vtkMRMLDisplayNode *> (callData);
    vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(callDataNode);

    if ( displayNode && (event == vtkMRMLDisplayableNode::DisplayModifiedEvent) )
      {
      this->Internal->UpdateDisplayNode(displayNode);
      this->RequestRender();
      }
    else if ( (event == vtkMRMLDisplayableNode::TransformModifiedEvent)
             || (event == vtkMRMLModelNode::PolyDataModifiedEvent))
      {
      this->Internal->UpdateDisplayableTransforms(displayableNode);
      this->RequestRender();
      }
    }
  else if ( vtkMRMLSliceNode::SafeDownCast(caller) )
      {
      this->Internal->UpdateSliceNode();
      this->RequestRender();
      }
  else
    {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::UpdateFromMRML()
{
  this->SetUpdateFromMRMLRequested(false);

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkDebugMacro( "vtkMRMLModelSliceDisplayableManager->UpdateFromMRML: Scene is not set.");
    return;
    }
  this->Internal->ClearDisplayableNodes();

  vtkMRMLDisplayableNode* mNode = nullptr;
  std::vector<vtkMRMLNode *> mNodes;
  int nnodes = scene ? scene->GetNodesByClass("vtkMRMLDisplayableNode", mNodes) : 0;
  for (int i=0; i<nnodes; i++)
    {
    mNode  = vtkMRMLDisplayableNode::SafeDownCast(mNodes[i]);
    if (mNode && this->Internal->UseDisplayableNode(mNode))
      {
      this->AddDisplayableNode(mNode);
      }
    }
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::UnobserveMRMLScene()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::OnMRMLSceneStartClose()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::OnMRMLSceneEndClose()
{
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::OnMRMLSceneEndBatchProcess()
{
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLModelSliceDisplayableManager::Create()
{
  this->Internal->SetSliceNode(this->GetMRMLSliceNode());
  this->SetUpdateFromMRMLRequested(true);
}
