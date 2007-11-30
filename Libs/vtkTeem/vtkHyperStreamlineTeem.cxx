#include "vtkHyperStreamlineTeem.h"

#include "vtkCellArray.h"
#include "vtkDataSet.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkImageData.h"


vtkCxxRevisionMacro(vtkHyperStreamlineTeem, "$Revision: 1.5 $");
vtkStandardNewMacro(vtkHyperStreamlineTeem);


vtkHyperStreamlineTeem::vtkHyperStreamlineTeem()
{
}

vtkHyperStreamlineTeem::~vtkHyperStreamlineTeem()
{
}



void vtkHyperStreamlineTeem::Execute()
{
  this->DebugOn();
  
  // This approach may cause problems depending on how slicer handles multiple datasets
  // because swapping back and forth between two dataset will cause new fibercontexts to be
  // regenerated each time. Is this a problem? Dunno. Needs to be tested... but should be
  // fine for now (for beta-testing purposes) where only 1 dataset used.
  static tenFiberContext *context = NULL;
  
  if( !context )
  {
    context = ProduceFiberContext();
  }
  else if( DatasetOrSettingsChanged() )
    {
      // TODO: Clean up after previous context here
      context = ProduceFiberContext();
    }
  
  StartFiberFrom( StartPosition, context );
  
  vtkDebugMacro( << "Done!");
  this->DebugOff();
}


void vtkHyperStreamlineTeem::StartFiberFrom( const vtkFloatingPointType position[3], tenFiberContext *context )
{
  vtkDebugMacro( << "Starting fiber from ("<< position[0] <<","<< position[1] <<"," << position[2] <<")");
  
  double start[3];
  start[0] = position[0];
  start[1] = position[1];
  start[2] = position[2];
  //real2index( position, start );
  
  Nrrd *output = nrrdNew();
  if (tenFiberTrace( context, output, start )) {
    //char *err = biffGetDone(TEN);
    //FILE *file = fopen( "C:\\Development\\error.txt", "wt" );
    //fprintf (file, "Error: %s\n\n", err );
    //fclose( file );
    vtkDebugMacro( << "Error tracing using tenFiberTrace" );
    return;
  }
  vtkDebugMacro( << "Found "  << output->axis[1].size << " points in fiber" );
  
  if ( output->data )
  {
    VisualizeFibers( output );
  }
  
  vtkDebugMacro( << "Cleaning up");
  nrrdNuke( output );
}

void vtkHyperStreamlineTeem::VisualizeFibers( const Nrrd *fibers )
{
  Streamers = new vtkTractographyArray[1];
  NumberOfStreamers = 1;
  Streamers[0].Direction = 1;
  
  const int fibercount = fibers->axis[1].size;
  size_t pos[2];
  for( int fiber = 0; fiber < fibercount; fiber++ )
  {
      pos[1] = fiber;
      vtkFloatingPointType indexPoints[3];
      
      for( int axis = 0; axis < 3; axis++ )
      {
          pos[0] = axis;
          nrrdSample_nva( &indexPoints[axis], fibers, pos );
      }
      
      vtkTractographyPoint *point = Streamers[0].InsertNextTractographyPoint();
      point->X[0] = indexPoints[0];
      point->X[1] = indexPoints[1];
      point->X[2] = indexPoints[2];
      //index2real( indexPoints, point->X );
      //vtkDebugMacro( << "Point in fiber(" << indexPoints[0] << "," << indexPoints[1] << "," << indexPoints[2] << ")");
      
      // Measure distance between points
      if( fiber > 1 )
      {
          vtkTractographyPoint *prevPoint = Streamers[0].GetTractographyPoint( fiber-2 );
          point->D = prevPoint->D + vtkMath::Distance2BetweenPoints( point->X, prevPoint->X );
      }
      else
      {
          point->D = 0.0;
      }
#if (VTK_MAJOR_VERSION >= 5)
      point->CellId = this->GetPolyDataInput(0)->FindPoint(point->X);
#else
      point->CellId = this->GetInput()->FindPoint( point->X );
#endif
    }
  
  vtkDebugMacro( << "Building lines");
  BuildLines(vtkDataSet::SafeDownCast(this->GetInput(0)),this->GetOutput());
}





tenFiberContext *vtkHyperStreamlineTeem::ProduceFiberContext()
{
  vtkDebugMacro( << "Producing new tenFiberContext" );

  vtkImageData *dataset = (vtkImageData*) this->GetInput();
  const int *size = dataset->GetDimensions();
  double *orig = dataset->GetOrigin();
  double *spcs = dataset->GetSpacing();
  size_t sizet[NRRD_DIM_MAX];
  int cent[NRRD_DIM_MAX];
  double spcdir[NRRD_SPACE_DIM_MAX][NRRD_SPACE_DIM_MAX];

  Nrrd *nrrd = nrrdNew();
  sizet[0] = 7;
  sizet[1] = size[0];
  sizet[2] = size[1];
  sizet[3] = size[2];
  nrrdAlloc_nva( nrrd, nrrdTypeFloat, 4, sizet );
  nrrdSpaceSet( nrrd, nrrdSpace3DRightHanded );
  nrrdSpaceOriginSet( nrrd, orig );
  ELL_3V_SET( spcdir[0], AIR_NAN, AIR_NAN, AIR_NAN );
  ELL_3V_SET( spcdir[1], spcs[0], 0.0, 0.0);
  ELL_3V_SET( spcdir[2], 0.0, spcs[1], 0.0);
  ELL_3V_SET( spcdir[3], 0.0, 0.0, spcs[2]);
  nrrdAxisInfoSet_nva( nrrd, nrrdAxisInfoSpaceDirection, spcdir );
  cent[0] = nrrdCenterUnknown;
  cent[1] = nrrdCenterCell;
  cent[2] = nrrdCenterCell;
  cent[3] = nrrdCenterCell;
  nrrdAxisInfoSet_nva( nrrd, nrrdAxisInfoCenter, cent);


  float* data = (float*) nrrd->data;
  vtkDataArray *array = dataset->GetPointData()->GetTensors();
  {
    int position[3];
    vtkFloatingPointType tensor[3][3];
    for( position[2] = 0; position[2] < size[2]; position[2]++ )
      for( position[1] = 0; position[1] < size[1]; position[1]++ )
    for( position[0] = 0; position[0] < size[0]; position[0]++ )
      {
        array->GetTuple( dataset->ComputePointId( position ), (vtkFloatingPointType*) tensor );
            
        *(data++) = 1.0f; // Confidence mask
        
        *(data++) = (float) tensor[0][0]; // Dxx
        *(data++) = (float) tensor[0][1]; // Dxy
        *(data++) = (float) tensor[0][2]; // Dxz
        *(data++) = (float) tensor[1][1]; // Dyy
        *(data++) = (float) tensor[1][2]; // Dyz
        *(data++) = (float) tensor[2][2]; // Dzz
      }
  }
  
  tenFiberContext *context = tenFiberContextNew( nrrd );
  if ( !context ) {
    //char *err = biffGetDone(TEN);
    //FILE *file = fopen( "C:\\Development\\error1.txt", "wt" );
    //fprintf (file, "Error: %s\n\n", err );
    //fclose( file );
    vtkDebugMacro( << "Error creating new fiber-context tenFiberContextNew" );
    return NULL;
  }


  const NrrdKernel *kernel;
  double params[NRRD_KERNEL_PARMS_NUM];
  nrrdKernelParse( &kernel, params, "tent" );

  int E = 0;
  if (!E) E |= tenFiberStopSet( context, tenFiberStopNumSteps, 100 );
  if (!E) E |= tenFiberTypeSet( context, tenFiberTypeEvec1 );
  if (!E) E |= tenFiberKernelSet( context, kernel, params );
  if (!E) E |= tenFiberIntgSet( context, tenFiberIntgRK4 );
  if (!E) E |= tenFiberParmSet( context, tenFiberParmStepSize, 0.1 );
  if (!E) E |= tenFiberParmSet( context, tenFiberParmUseIndexSpace, AIR_FALSE );
  if (!E) E |= tenFiberUpdate( context );
  if (E) {
    //char *err = biffGetDone(TEN);
    //FILE *file = fopen( "C:\\Development\\error.txt", "wt" );
    //fprintf (file, "Error: %s\n\n", err );
    //fclose( file );
    vtkDebugMacro( << "Error setting parameters for ten" );
    return NULL;
  }

  return context;
}

// Should return true if the tensor dataset or settings has changed since the previous
// time this function was called, for instance if the user opened a second tensor dataset and
// did some streamline visualization on it.
bool vtkHyperStreamlineTeem::DatasetOrSettingsChanged()
{
  return false; // TODO: implement this in a meaningfull manner!
}



void vtkHyperStreamlineTeem::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

