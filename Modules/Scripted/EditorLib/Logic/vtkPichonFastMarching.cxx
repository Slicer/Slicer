/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

// EditorLib includes
#include "vtkPichonFastMarching.h"
#include "vtkPichonFastMarchingPDF.h"

// VTK includes
#include <vtkInformation.h>
#include <vtkDataArray.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkStreamingDemandDrivenPipeline.h>

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

// used to compute the median
int compareInt(const void *a, const void *b)
{
  return  (*(int*)a) - (*(int*)b);
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkPichonFastMarching);

//------------------------------------------------------------------------------
void vtkPichonFastMarching::collectInfoSeed( int index )
{
  int med, inh;
  getMedianInhomo(index, med, inh);

  pdfIntensityIn->addRealization( med );
  pdfInhomoIn->addRealization( inh );
}

// speed at index
float vtkPichonFastMarching::speed( int index )
{
  int I;
  int H;
  static bool warned = false;

  getMedianInhomo( index, I, H );

  float s;

  double pI=pdfIntensityIn->value(I);
  double pH=pdfInhomoIn->value(H);



  s=(float)pow(pI*pI*pH, powerSpeed);
  // make sure speed is not too small
  s*=1e10;

  if(!vtkMath::IsFinite(s))
    {
    s = 1.;
    if(!warned){
      std::cerr << "WARNING: s set to 1.0, since it was not vtkMath::IsFinite()" << std::endl;
      warned = true;
    }
    }


  if( (s<1.0/(INF/1e6)) || vtkMath::IsFinite(s)==0 )
    {
      if(vtkMath::IsFinite(s)==0)
    vtkErrorMacro( "Error in vtkPichonFastMarching::speed(...): vtkMath::IsFinite(s)==0 " << s );
      /*
      else
    vtkErrorMacro( "(s<1.0/(INF/1e6)) " << s );
      */
      s=(float)(1.0/(INF/1e6));
    }

  return s;
}

void vtkPichonFastMarching::setSeed( int index )
{
  //assert( (index>=(1+dimX+dimXY)) && (index<(dimXYZ-1-dimX-dimXY)) );
  if(!( (index>=(1+dimX+dimXY)) && (index<(dimXYZ-1-dimX-dimXY)) ))
    {
      vtkErrorMacro("Error in vtkPichonFastMarching::setSeed(...): !( (index>=(1+dimX+dimXY)) && (index<(dimXYZ-1-dimX-dimXY)) )");
      return;
    }

  if( node[index].status!=fmsFAR )
    {
      // this seed has already been planted
      return;
    }

  // by definition, T=0, and that voxel is known
  node[index].T=0.0;
  node[index].status=fmsKNOWN;

  knownPoints.push_back(index);

  // add all FAR 26-neighbors to TRIAL
  for(int n=1;n<=26;n++)
    {
      FMleaf f;
      f.nodeIndex=index + shiftNeighbor(n);
      if( node[ f.nodeIndex ].status==fmsFAR )
    {
      node[f.nodeIndex].status=fmsTRIAL;
      node[f.nodeIndex].T = (float) ( distanceNeighbor(n) / speed(f.nodeIndex) );

      insert( f ); // insert in minheap
    }
    }
}

inline void vtkPichonFastMarching::getMedianInhomo( int index, int &med, int &inh )
{
  // assert( (index>=(1+dimX+dimXY)) && (index<(dimXYZ-1-dimX-dimXY)) );

  inh = inhomo[index];
  if( inh != (-1) )
    // then the values have already been computed
    {
      med = median[index];
      return;
    }

  // otherwise, just do it
  for(int k=0;k<=26;k++)
      tmpNeighborhood[k] = (int)indata[index + arrayShiftNeighbor[k]];

  qsort( (void*)tmpNeighborhood, 27, sizeof(int), &compareInt );

  inh = inhomo[ index ] = (tmpNeighborhood[21] - tmpNeighborhood[5]);
  med = median[ index ] = tmpNeighborhood[13];

  /*
    // same thing for 125-neighbors

    int p=0;
    for(int px=-2;px<=2;px++)
    for(int py=-2;py<=2;py++)
    for(int pz=-2;pz<=2;pz++)
    {
    int k=index + px + py * dimX + pz * dimXY;
    tmpNeighborhood[p++] = (int)indata[k];
    }

    qsort( (void*)tmpNeighborhood, 125, sizeof(int), &compareInt );

    inh = inhomo[ index ] = (tmpNeighborhood[105] - tmpNeighborhood[20]);
    med = median[ index ] = tmpNeighborhood[63];
  */
}

void vtkPichonFastMarching::initNewExpansion()
{
  if(invalidInputs)
    return;

  pdfIntensityIn->reset();
  pdfInhomoIn->reset();

  // empty interface points
  while(tree.size()>0)
    {
      node[ tree[tree.size()-1].nodeIndex ].status=fmsFAR;
      node[ tree[tree.size()-1].nodeIndex ].T=(float)INF;
      tree.pop_back();
    }

  // empty the list of known points
  while(knownPoints.size()>0)
    {
      knownPoints.pop_back();
    }
  nEvolutions=-1;

  firstCall=true;

  while(seedPoints.size()>0)
    seedPoints.pop_back();

  int index=0;
  for(int k=0;k<dimZ;k++)
    for(int j=0;j<dimY;j++)
      for(int i=0;i<dimX;i++)
    {
      if( (outdata[index]==label) && (node[index].status!=fmsOUT) )
        {
            collectInfoSeed( index );
            for(int n=1;n<nNeighbors;n++)
            if(outdata[index+shiftNeighbor(n)]==0)
              {
                seedPoints.push_back( index+shiftNeighbor(n) );
                }

/*
          bool hasIntensityZeroNeighbor = false;
          for(int n=1;n<nNeighbors;n++)
        if(outdata[index+shiftNeighbor(n)]==0)
          {
            hasIntensityZeroNeighbor=true;
            break;
          }

          if(hasIntensityZeroNeighbor)
        {
          node[index].status=fmsFAR;
          seedPoints.push_back( index );
        }
          else
        {
          node[index].status=fmsDONE;
          node[index].T=0.0;
        }
*/

        }

      index++;
    }
}

int vtkPichonFastMarching::nValidSeeds()
{
  if(invalidInputs)
    return 0;

  return (int)(seedPoints.size()+tree.size());
}

int vtkPichonFastMarching::nKnownPoints()
{
  if(invalidInputs)
    return 0;
  return knownPoints.size();
}

void vtkPichonFastMarchingExecute(vtkPichonFastMarching *self,
                vtkImageData *vtkNotUsed(inData), short *inPtr,
                vtkImageData *vtkNotUsed(outData), short *outPtr,
                int vtkNotUsed(outExt)[6])
{
  if(self->invalidInputs)
    return;
  int n=0;
  int k;

  self->setInData( (short *)inPtr );
  self->setOutData( (short *)outPtr );

  if( !self->initialized )
    {
    self->initialized = true;

    int index=0;
    int lastPercentageProgressBarUpdated=-1;

    for(k=0;k<self->dimZ;k++)
      {
      // update progress bar
      int currentPercentage = GRANULARITY_PROGRESS * index / self->dimXYZ;
      if (currentPercentage > lastPercentageProgressBarUpdated)
        {
        lastPercentageProgressBarUpdated = currentPercentage;
        self->UpdateProgress(float(currentPercentage) / float(GRANULARITY_PROGRESS));
        }
      for (int j = 0; j<self->dimY; j++)
        for (int i = 0; i<self->dimX; i++)
        {
          self->node[index].T = (float)INF;

          if (self->outdata[index] == 0)
            self->node[index].status = fmsFAR;
          else
            self->node[index].status = fmsDONE;

          self->inhomo[index] = -1; // meaning inhomo and median have not been computed there

          if ((i<BAND_OUT) || (j<BAND_OUT) || (k<BAND_OUT) ||
            (i >= (self->dimX - BAND_OUT)) || (j >= (self->dimY - BAND_OUT)) || (k >= (self->dimZ - BAND_OUT)))
          {

            self->node[index].status = fmsOUT;

            // we should never have to look at these values anyway !
            self->inhomo[index] = self->depth;
            self->median[index] = 0;
          }

          index++;
        }
      }

    return;
    }

  if( self->firstCall )
    {
    self->firstCall=false;


    //assert(self->seedPoints.size()>0);
    if(!(self->seedPoints.size()>0))
      {
      vtkErrorWithObjectMacro(self, "Error in vtkPichonFastMarchingExecute: !(self->seedPoints.size()>0)" );
      self->firstCall=true; // we did not complete this step
      return;
      }
    for(k=0;k<(int)self->seedPoints.size();k++)
      self->collectInfoSeed( self->seedPoints[k] );

    self->pdfIntensityIn->update();
    self->pdfInhomoIn->update();
    }

  if(self->nPointsEvolution<=0)
    // then we have nothing to do and we have just been called to update the pipeline
    return;

  // reinitialize the points that were removed by the user
  if( self->nEvolutions>0 )
    if( (self->knownPoints.size()>1) &&
      ((signed)self->knownPoints.size()-1>self->nPointsBeforeLeakEvolution) )
      {
      // reinitialize all the points
      for(k=self->nPointsBeforeLeakEvolution;k<(int)self->knownPoints.size();k++)
        {
        int index = self->knownPoints[k];
        self->node[ index ].status = fmsFAR;
        self->node[ index ].T = (float)INF;

        /*
           we also want to remove the neighbors of these points that would be in TRIAL
           as it is not trivial to remove points from the minheap, we will just set their T
           to infinity to make sure they appear in the back of the heap
         */

        for(n=1;n<=self->nNeighbors;n++)
          {
          int indexN=index+self->shiftNeighbor(n);
          if( self->node[indexN].status==fmsTRIAL )
            {
            self->node[indexN].T=(float)INF;
            self->downTree( self->node[indexN].leafIndex );
            }
          }
        }

      // if the points still have a KNOWN neighbor, put them back in TRIAL
      for(k=self->nPointsBeforeLeakEvolution;k<(int)self->knownPoints.size();k++)
        {
        int index = self->knownPoints[k];
        int indexN;

        bool hasKnownNeighbor =  false;
        for(n=1;n<=self->nNeighbors;n++)
          {
          indexN=index+self->shiftNeighbor(n);
          if( self->node[indexN].status==fmsKNOWN )
            hasKnownNeighbor=true;
          }

        if( (hasKnownNeighbor) && (self->node[index].status!=fmsOUT) )
          {
          FMleaf f;

          self->node[index].T=self->computeT(index);
          self->node[index].status=fmsTRIAL;
          f.nodeIndex=index;

          self->insert( f );
          }
        }

      // remove all the points from the displayed knownPoints
      // ok since (self->knownPoints[self->nEvolutions].size()-1>self->nPointsBeforeLeakEvolution)
      // is true
      while((int)self->knownPoints.size()>self->nPointsBeforeLeakEvolution)
        self->knownPoints.pop_back();
      }

  // start a new evolution
  self->nEvolutions++;

  self->nPointsBeforeLeakEvolution=(int)(self->knownPoints.size()-1);

  // use the seeds
  while(self->seedPoints.size()>0)
    {
    int index=self->seedPoints[self->seedPoints.size()-1];
    self->seedPoints.pop_back();

    self->setSeed( index );
    }

  // check minHeap OK
  self->minHeapIsSorted();

  self->pdfIntensityIn->setUpdateRate(self->nPointsEvolution/100);
  self->pdfInhomoIn->setUpdateRate(self->nPointsEvolution/100);

  for(n=0;n<self->nPointsEvolution;n++)
    {
    if( (n*GRANULARITY_PROGRESS) % self->nPointsEvolution == 0 )
      self->UpdateProgress(float(n)/float(self->nPointsEvolution));

    float T=self->step();

    // all the statistics should be gathered from a band 3 pixels from the interface
    self->pdfIntensityIn->setMemory((int)(5*self->tree.size()));
    self->pdfInhomoIn->setMemory((int)(5*self->tree.size()));

    if( T==INF )
      {
      vtkErrorWithObjectMacro(self, "FastMarching: nowhere else to go. End of evolution." );
      break;
      }
    }

  // check minHeap still OK
  self->minHeapIsSorted();

  self->firstPassThroughShow = true;

  // we've done that,
  // make sure this is reset to 0 so that nothing happen if Update is called
  self->nPointsEvolution=0;
}

void vtkPichonFastMarching::show(float r)
{
  if(invalidInputs)
    return;

  //assert( (r>=0) && (r<=1.0) );
  if(!( (r>=0) && (r<=1.0) ))
    {
      vtkErrorMacro("Error in vtkPichonFastMarching::show(...): !( (r>=0) && (r<=1.0) )");
      return;
    }

  if( nEvolutions<0 )
    return;

  if( knownPoints.size()<1 )
    return;

  int oldIndex = nPointsBeforeLeakEvolution;
  int newIndex = (int)((knownPoints.size()-1)*r);

  if( newIndex > oldIndex )
    for(int index=(oldIndex+1);index<=newIndex;index++)
      {
    if( node[ knownPoints[index] ].status==fmsKNOWN )
        if(outdata[ knownPoints[index] ]==0)
          outdata[ knownPoints[index] ]=label;
      }
  else if( newIndex < oldIndex )
    for(int index=oldIndex;index>newIndex;index--)
      {
    if(node[ knownPoints[index] ].status==fmsKNOWN )
        if(outdata[ knownPoints[index] ]==label)
          outdata[ knownPoints[index] ]=0;
      }

  nPointsBeforeLeakEvolution=newIndex;
  firstPassThroughShow=false;
}

void vtkPichonFastMarching::setActiveLabel(int _label)
{
  this->label=_label;
}

//----------------------------------------------------------------------------
// Description:
// This method is passed a input and output data, and executes the filter
// algorithm to fill the output from the input.
// It just executes a switch statement to call the correct function for
// the datas data types.
void vtkPichonFastMarching::ExecuteDataWithInformation(vtkDataObject *output, vtkInformation* outInfo)
{
  vtkImageData *inData = vtkImageData::SafeDownCast(this->GetInput());
  vtkImageData *outData = this->AllocateOutputData(output, outInfo);

  int outExt[6], s;
  outInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), outExt);
  void *inPtr = inData->GetScalarPointerForExtent(outExt);
  void *outPtr = outData->GetScalarPointerForExtent(outExt);

  int x1;

  x1 = this->GetImageDataInput(0)->GetNumberOfScalarComponents();
  if (x1 != 1)
    {
      vtkErrorMacro("Input has "<<x1<<" instead of 1 scalar component.");
      invalidInputs = true;
      return;
    }

  /* Need short data */
  s = inData->GetScalarType();
  if (s != VTK_SHORT)
    {
      vtkErrorMacro("Input scalars are type "<< s
            << " instead of "<< VTK_SHORT);
      invalidInputs = true;
      return;
    }

  vtkPichonFastMarchingExecute(this, inData, (short *)inPtr,
             outData, (short *)(outPtr), outExt);

}

void vtkPichonFastMarching::setNPointsEvolution( int n )
{
  nPointsEvolution=n;
}

void vtkPichonFastMarching::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageAlgorithm::PrintSelf(os,indent);

  os << indent << "dimX: " << this->dimX << "\n";
  os << indent << "dimY: " << this->dimY << "\n";
  os << indent << "dimZ: " << this->dimZ << "\n";
  os << indent << "dimXY: " << this->dimXY << "\n";
  os << indent << "label: " << this->label << "\n";
}

bool vtkPichonFastMarching::emptyTree()
{
  return (tree.size() == 0);
}

void vtkPichonFastMarching::insert(const FMleaf leaf) {

  // insert element at the back
  tree.push_back( leaf );
  node[ leaf.nodeIndex ].leafIndex=(int)(tree.size()-1);

  // trickle the element up until everything
  // is sorted again
  upTree( (int)(tree.size()-1) );
}

bool vtkPichonFastMarching::minHeapIsSorted()
{
  int N=(int)tree.size();
  int k;

  for(k=(N-1);k>=1;k--)
    {
      if(node[tree[k].nodeIndex].leafIndex!=k)
    {
      vtkErrorMacro( "Error in vtkPichonFastMarching::minHeapIsSorted(): "
             << "tree[" << k << "] : pb leafIndex/nodeIndex (size="
             << (unsigned int)tree.size() << ")" );
    }
    }
  for(k=(N-1);k>=1;k--)
    {
      if( vtkMath::IsFinite( node[tree[k].nodeIndex].T)==0 )
    vtkErrorMacro( "Error in vtkPichonFastMarching::minHeapIsSorted(): "
               << "NaN or Inf value in minHeap : " << node[tree[k].nodeIndex].T );

      if( node[tree[k].nodeIndex].T<node[ (int)(tree[(k-1)/2].nodeIndex) ].T )
    {
      vtkErrorMacro( "Error in vtkPichonFastMarching::minHeapIsSorted(): "
             << "minHeapIsSorted is false! : size=" << (unsigned int)tree.size() << "at leafIndex=" << k
             << " node[tree[k].nodeIndex].T=" << node[tree[k].nodeIndex].T
             << "<node[ (int)(tree[(k-1)/2].nodeIndex) ].T=" << node[ (int)(tree[(k-1)/2].nodeIndex) ].T);

      return false;
    }
    }
  return true;
}

void vtkPichonFastMarching::downTree(int index) {
  /*
   * This routine sweeps downward from leaf 'index',
   * swapping child and parent if the value of the child
   * is smaller than that of the parent. Note that this only
   * guarantees the heap property if the value at the
   * starting index is greater than all its parents.
   */
  int LeftChild = 2 * index + 1;
  int RightChild = 2 * index + 2;

  while (LeftChild < (int)tree.size())
    {
      /*
       * Terminate the process when the current leaf has no
       * children. If no swap occurs at a higher leaf, this
       * condition is forced.
       */

      /*
       * Find the child with the smallest value. The node has at least
       * one child, and so has at least a left child.
       */
      int MinChild = LeftChild;

      /*
       * If the node has a right child, and if the right child
       * has smaller crossing time than the left child, then the
       * right child is the MinChild.
       */
      if (RightChild < (int)tree.size()) {

    if (node[tree[LeftChild].nodeIndex].T>
        node[tree[RightChild].nodeIndex].T)
      MinChild = RightChild;
      }

      /*
       * If the MinChild has smaller T than the current leaf,
       * swap them, and move the current leaf to the MinChild.
       */
      if (node[tree[MinChild].nodeIndex].T<
      node[tree[index].nodeIndex].T)
    {
      FMleaf tmp=tree[index];
      tree[index]=tree[MinChild];
      tree[MinChild]=tmp;

      // make sure pointers remain correct
      node[ tree[MinChild].nodeIndex ].leafIndex = MinChild;
      node[ tree[index].nodeIndex ].leafIndex = index;

      index = MinChild;

      LeftChild = 2 * index + 1;
      RightChild =  LeftChild + 1;
    }
      else
    /*
     * If the current leaf has a lower value than its
     * MinChild, the job is done, force a stop.
     */
    break;
    }
}

void vtkPichonFastMarching::upTree(int index) {
  /*
   * This routine sweeps upward from leaf 'index',
   * swapping child and parent if the value of the child
   * is less than that of the parent. Note that this only
   * guarantees the heap property if the value at the
   * starting leaf is less than all its children.
   */
  while( index>0 )
    {
      int upIndex = (int) (index-1)/2;

      if( node[tree[index].nodeIndex].T <
      node[tree[upIndex].nodeIndex].T )
    {
      // then swap the 2 nodes

      FMleaf tmp=tree[index];
      tree[index]=tree[upIndex];
      tree[upIndex]=tmp;

      // make sure pointers remain correct
      node[ tree[upIndex].nodeIndex ].leafIndex = upIndex;
      node[ tree[index].nodeIndex ].leafIndex = index;

      index = upIndex;
    }
      else
    // then there is nothing left to do
    // force stop
    break;
    }
}

FMleaf vtkPichonFastMarching::removeSmallest()
{

  FMleaf f;
  f=tree[0];

  /*
   * Now move the bottom, rightmost, leaf to the root.
   */
  tree[0]=tree[ tree.size()-1 ];

  // make sure pointers remain correct
  node[ tree[0].nodeIndex ].leafIndex = 0;

  tree.pop_back();

  // trickle the element down until everything
  // is sorted again
  downTree( 0 );

  return f;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

vtkPichonFastMarching::vtkPichonFastMarching()
{
  initialized=false;
  invalidInputs=true;

  node = nullptr;
  inhomo = nullptr;
  median = nullptr;

  pdfIntensityIn = nullptr;
  pdfInhomoIn = nullptr;
}

void vtkPichonFastMarching::init(int _dimX, int _dimY, int _dimZ, double _depth, double _dx, double _dy, double _dz)
{
  powerSpeed = 1.0;

  this->dx=(float)_dx;
  this->dy=(float)_dy;
  this->dz=(float)_dz;

  invDx2 = (float)(1.0/(dx*dx));
  invDy2 = (float)(1.0/(dy*dy));
  invDz2 = (float)(1.0/(dz*dz));

  nNeighbors=6; // 6 or 26
  //note: there seem to be some problems with discr < 0
  //and A==0 when 26

  nEvolutions=-1;

  this->dimX=_dimX;
  this->dimY=_dimY;
  this->dimZ=_dimZ;
  this->dimXY=dimX*dimY;
  this->dimXYZ=dimX*dimY*dimZ;

  arrayShiftNeighbor[0] = 0; // neighbor 0 is the node itself
  arrayDistanceNeighbor[0] = 0.0;

  arrayShiftNeighbor[1] = -dimX;
  arrayDistanceNeighbor[1] = dy;
  arrayShiftNeighbor[2] = +1;
  arrayDistanceNeighbor[2] = dx;
  arrayShiftNeighbor[3] = dimX;
  arrayDistanceNeighbor[3] = dy;
  arrayShiftNeighbor[4] = -1;
  arrayDistanceNeighbor[4] = dx;
  arrayShiftNeighbor[5] = -dimXY;
  arrayDistanceNeighbor[5] = dz;
  arrayShiftNeighbor[6] = dimXY;
  arrayDistanceNeighbor[6] = dz;

  arrayShiftNeighbor[7] =  -dimX+dimXY;
  arrayDistanceNeighbor[7] = sqrt( dy*dy + dz*dz );
  arrayShiftNeighbor[8] =  -dimX-dimXY;
  arrayDistanceNeighbor[8] = sqrt( dy*dy + dz*dz );
  arrayShiftNeighbor[9] =   dimX+dimXY;
  arrayDistanceNeighbor[9] = sqrt( dy*dy + dz*dz );
  arrayShiftNeighbor[10] =  dimX-dimXY;
  arrayDistanceNeighbor[10] = sqrt( dy*dy + dz*dz );
  arrayShiftNeighbor[11] = -1+dimXY;
  arrayDistanceNeighbor[11] = sqrt( dx*dx + dz*dz );
  arrayShiftNeighbor[12] = -1-dimXY;
  arrayDistanceNeighbor[12] = sqrt( dx*dx + dz*dz );
  arrayShiftNeighbor[13] = +1+dimXY;
  arrayDistanceNeighbor[13] = sqrt( dx*dx + dz*dz );
  arrayShiftNeighbor[14] = +1-dimXY;
  arrayDistanceNeighbor[14] = sqrt( dx*dx + dz*dz );
  arrayShiftNeighbor[15] = +1-dimX;
  arrayDistanceNeighbor[15] = sqrt( dx*dx + dy*dy );
  arrayShiftNeighbor[16] = +1+dimX;
  arrayDistanceNeighbor[16] = sqrt( dx*dx + dy*dy );
  arrayShiftNeighbor[17] = -1+dimX;
  arrayDistanceNeighbor[17] = sqrt( dx*dx + dy*dy );
  arrayShiftNeighbor[18] = -1-dimX;
  arrayDistanceNeighbor[18] = sqrt( dx*dx + dy*dy );

  arrayShiftNeighbor[19] = +1-dimX-dimXY;
  arrayDistanceNeighbor[19] = sqrt( dx*dx + dy*dy + dz*dz );
  arrayShiftNeighbor[20] = +1-dimX+dimXY;
  arrayDistanceNeighbor[20] = sqrt( dx*dx + dy*dy + dz*dz );
  arrayShiftNeighbor[21] = +1+dimX-dimXY;
  arrayDistanceNeighbor[21] = sqrt( dx*dx + dy*dy + dz*dz );
  arrayShiftNeighbor[22] = +1+dimX+dimXY;
  arrayDistanceNeighbor[22] = sqrt( dx*dx + dy*dy + dz*dz );
  arrayShiftNeighbor[23] = -1+dimX-dimXY;
  arrayDistanceNeighbor[23] = sqrt( dx*dx + dy*dy + dz*dz );
  arrayShiftNeighbor[24] = -1+dimX+dimXY;
  arrayDistanceNeighbor[24] = sqrt( dx*dx + dy*dy + dz*dz );
  arrayShiftNeighbor[25] = -1-dimX-dimXY;
  arrayDistanceNeighbor[25] = sqrt( dx*dx + dy*dy + dz*dz );
  arrayShiftNeighbor[26] = -1-dimX+dimXY;
  arrayDistanceNeighbor[26] = sqrt( dx*dx + dy*dy + dz*dz );

  this->depth = (int) _depth;

  delete[] node;
  node = new FMnode[ dimX*dimY*dimZ ];
  if(node==nullptr)
    {
      vtkErrorMacro("Error in void vtkPichonFastMarching::init(), not enough memory for allocation of 'node'");
      return;
    }

  delete[] inhomo;
  inhomo = new int[ dimX*dimY*dimZ ];
  if(inhomo==nullptr)
    {
      vtkErrorMacro("Error in void vtkPichonFastMarching::init(), not enough memory for allocation of 'inhomo'");
      return;
    }

  delete[] median;
  median = new int[ dimX*dimY*dimZ ];
  if(median==nullptr)
    {
      vtkErrorMacro("Error in void vtkPichonFastMarching::init(), not enough memory for allocation of 'median'");
      return;
    }

  delete pdfIntensityIn;
  pdfIntensityIn = new PichonFastMarchingPDF( (int) _depth );
  if(pdfIntensityIn==nullptr)
    {
      vtkErrorMacro("Error in void vtkPichonFastMarching::init(), not enough memory for allocation of 'pdfIntensityIn'");
      return;
    }

  delete pdfInhomoIn;
  pdfInhomoIn = new PichonFastMarchingPDF( (int) _depth );
  if(pdfInhomoIn==nullptr)
    {
      vtkErrorMacro("Error in void vtkPichonFastMarching::init(), not enough memory for allocation of 'pdfInhomoIn'");
      return;
    }

  initialized=false; // we will need one pass in the execute
  // function before we are properly initialized

  firstCall = true;

  invalidInputs = false; // so far so good
}

void vtkPichonFastMarching::setInData(short* data)
{
  indata=data;
}

void vtkPichonFastMarching::setOutData(short* data)
{
  outdata=data;
}

vtkPichonFastMarching::~vtkPichonFastMarching()
{
  delete[] node;
  node = nullptr;
  delete[] inhomo;
  inhomo = nullptr;
  delete[] median;
  median = nullptr;

  delete pdfIntensityIn;
  pdfIntensityIn = nullptr;
  delete pdfInhomoIn;
  pdfInhomoIn = nullptr;
}

inline int vtkPichonFastMarching::shiftNeighbor(int n)
{
  //assert(initialized);
  //assert(n>=0 && n<=nNeighbors);

  return arrayShiftNeighbor[n];
}

inline double vtkPichonFastMarching::distanceNeighbor(int n)
{
  //assert(initialized);
  //assert(n>=0 && n<=nNeighbors);

  return arrayDistanceNeighbor[n];
}

int vtkPichonFastMarching::indexFather(int n )
{
  float Tmin = (float)INF;
  int index, indexMin = 0;

  // note: has to be 6 or else topology not consistent and
  // we get weird path to parents using the diagonals
  for(int k=1;k<=6;k++)
  {
    index = n+shiftNeighbor(k);
    if( node[index].T<Tmin )
    {
      Tmin = node[index].T;
      indexMin = index;
    }
  }

  //assert( Tmin < INF );
  // or else there was no initialized neighbor around ?

  return indexMin;
}

float vtkPichonFastMarching::step()
{
  if(invalidInputs)
    return (float)INF;

  int indexN;
  int n;

  FMleaf min;

  /* find point in fmsTRIAL with smallest T, remove it from fmsTRIAL and put
     it in fmsKNOWN */

  static int emptyTreeCnt;
  if( emptyTree() )
    {
      if(emptyTreeCnt == 0)
        {
        vtkErrorMacro( "vtkPichonFastMarching::step empty tree!" << endl );
        }
      emptyTreeCnt++;
      return (float)INF;
    }

  min=removeSmallest();

  if( node[min.nodeIndex].T>=INF )
    {
      vtkErrorMacro( " node[min.nodeIndex].T>=INF " << endl );

      // this would happen if the only points left were artificially put back
      // by the user playing with the slider
      // we do not want to consider those before the expansion has naturally
      // reachjed them.
      return (float)INF;
    }

  int I, H;
  getMedianInhomo( min.nodeIndex, I, H );

  pdfIntensityIn->addRealization( I );
  pdfInhomoIn->addRealization( H );

  node[min.nodeIndex].status=fmsKNOWN;
  knownPoints.push_back(min.nodeIndex);

  /* then we consider all the neighbors */
  for(n=1;n<=nNeighbors;n++)
    {
      /* 'indexN' is the index of the nth neighbor
     of node of index 'index' */
      indexN=min.nodeIndex+shiftNeighbor(n);

      /*
       * Check the status of the neighbors. If
       * they are fmsTRIAL, recompute their crossing time values and
       * adjust their position in the tree with an UpHeap (Note that
       * recomputed value must be less than or equal to the original).
       * If they are fmsFAR, recompute their crossing times, and move
       * them into fmsTRIAL.
       */
      if( node[indexN].status==fmsFAR )
    {
      FMleaf f;
      node[indexN].T=computeT(indexN);
      f.nodeIndex=indexN;

      insert( f );

      node[indexN].status=fmsTRIAL;
    }
      else if( node[indexN].status==fmsTRIAL )
    {
      float t1,  t2;
      t1 = node[indexN].T;

      node[indexN].T=computeT(indexN);

      t2 = node[indexN].T;

      if( t2<t1 )
          upTree( node[indexN].leafIndex );
      else
          downTree( node[indexN].leafIndex );

    }
    }

  return node[min.nodeIndex].T;
}

float vtkPichonFastMarching::computeT(int index )
{
  double A, B, C, Discr;

  A = 0.0;
  B = 0.0;

  double s=speed(index);

  /*
    we don't want anything really small here as it might give us very large T
    and we don't want something not defined (Inf) or larger than our own INF
    ( because at low level the algo relies on Tij < INF to say that Tij is defined
    cf   if ((Dxm>0.0) || (Dxp<0.0)) ))

    this should be cool with a volume of dimension less than 1e6, (volumes are typically 256~=1e2 to 1e3)
  */

  C = -1.0/( s*s );

  double Tij, Txm, Txp, Tym, Typ, Tzm, Tzp, TijNew;

  Tij = node[index].T;

  /* we know that all neighbors are defined
     because this node is not fmsOUT */
  Txm = node[index+shiftNeighbor(4)].T;
  Txp = node[index+shiftNeighbor(2)].T;
  Tym = node[index+shiftNeighbor(1)].T;
  Typ = node[index+shiftNeighbor(3)].T;
  Tzm = node[index+shiftNeighbor(5)].T;
  Tzp = node[index+shiftNeighbor(6)].T;

  double Dxm, Dxp, Dym, Dyp, Dzm, Dzp;

  Dxm = Tij - Txm;
  Dxp = Txp - Tij;
  Dym = Tij - Tym;
  Dyp = Typ - Tij;
  Dzm = Tij - Tzm;
  Dzp = Tzp - Tij;

  /*
   * Set up the quadratic equation for TijNew.
   */
  if ((Dxm>0.0) || (Dxp<0.0)) {
    if (Dxm > -Dxp) {
      A += invDx2;
      B += -2.0 * Txm * invDx2;
      C += Txm * Txm * invDx2;
    }
    else {
      A += invDx2;
      B += -2.0 * Txp * invDx2;
      C += Txp * Txp * invDx2;
    }
  }
  if ((Dym>0.0) || (Dyp<0.0)) {
    if (Dym > -Dyp) {
      A += invDy2;
      B += -2.0 * Tym * invDy2;
      C += Tym * Tym * invDy2;
    }
    else {
      A += invDy2;
      B += -2.0 * Typ * invDy2;
      C += Typ * Typ * invDy2;
    }
  }
  if ((Dzm>0.0) || (Dzp<0.0)) {
    if (Dzm > -Dzp) {
      A += invDz2;
      B += -2.0 * Tzm * invDz2;
      C += Tzm * Tzm * invDz2;
    }
    else {
      A += invDz2;
      B += -2.0 * Tzp * invDz2;
      C += Tzp * Tzp * invDz2;
    }
  }


  Discr = B*B - 4.0*A*C;

  // cases when the quadratic equation is singular
  if ((A==0) || (Discr < 0.0)) {
    int candidateIndex;
    double candidateT;
    Tij=INF;
    s=speed(index);
    for(int n=1;n<=nNeighbors;n++)
      {
    candidateIndex = index + shiftNeighbor(n);
    if( (node[candidateIndex].status==fmsTRIAL)
        || (node[candidateIndex].status==fmsKNOWN) )
      {
        candidateT = node[candidateIndex].T + distanceNeighbor(n)/s;

        if( candidateT<Tij )
          Tij=candidateT;
      }
      }

    //    assert( Tij<INF );
    if(!( Tij<INF ))
      {
    vtkErrorMacro("Error in vtkPichonFastMarching::computeT(...): !( Tij<INF )");
    return (float)INF;
      }

   return (float)Tij;
  }

  /*
   * Solve the quadratic equation. Note that the new crossing
   * must be GREATER than the average of the active neighbors,
   * since only EARLIER elements are active. Therefore the plus
   * sign is appropriate.
   */
  TijNew = (-B + (float)sqrt(Discr))/((float)2.0*A);

  return (float)TijNew;
}

void vtkPichonFastMarching::setRAStoIJKmatrix
(float _m11, float _m12, float _m13, float _m14,
 float _m21, float _m22, float _m23, float _m24,
 float _m31, float _m32, float _m33, float _m34,
 float _m41, float _m42, float _m43, float _m44)
{
  this->m11=_m11;
  this->m12=_m12;
  this->m13=_m13;
  this->m14=_m14;

  this->m21=_m21;
  this->m22=_m22;
  this->m23=_m23;
  this->m24=_m24;

  this->m31=_m31;
  this->m32=_m32;
  this->m33=_m33;
  this->m34=_m34;

  this->m41=_m41;
  this->m42=_m42;
  this->m43=_m43;
  this->m44=_m44;
}

int vtkPichonFastMarching::addSeed( float r, float a, float s )
{
  if(invalidInputs){
    vtkErrorMacro("vtkPichonFastMarching::addSeed failed: no valid inputs specified");
    return 0;
  }

  int I, J, K;

  I = (int) ( m11*r + m12*a + m13*s + m14*1 );
  J = (int) ( m21*r + m22*a + m23*s + m24*1 );
  K = (int) ( m31*r + m32*a + m33*s + m34*1 );

  if ( (I>=1) && (I<(dimX-1))
       &&  (J>=1) && (J<(dimY-1))
       &&  (K>=1) && (K<(dimZ-1)) )
    {
      seedPoints.push_back( I+J*dimX+K*dimXY );

      // use neighbors to create statistics
      for(int n=0;n<=26;n++)
    collectInfoSeed( I+J*dimX+K*dimXY+shiftNeighbor(n) );

      // note: the neighbors will be put in TRIAL by setseed

      return 1;
    } else {
      cout << "Point is outside image volume" << endl;
    }

  return 0; // we're trying to put a seed outside of the volume
}


int vtkPichonFastMarching::addSeedIJK( int I, int J, int K )
{
  if(invalidInputs){
    vtkErrorMacro("vtkPichonFastMarching::addSeedIJK failed: no valid inputs specified");
    return 0;
  }

  if ( (I>=1) && (I<(dimX-1))
       &&  (J>=1) && (J<(dimY-1))
       &&  (K>=1) && (K<(dimZ-1)) )
    {
      seedPoints.push_back( I+J*dimX+K*dimXY );

      // use neighbors to create statistics
      for(int n=0;n<=26;n++)
        collectInfoSeed( I+J*dimX+K*dimXY+shiftNeighbor(n) );

      // note: the neighbors will be put in TRIAL by setseed

      return 1;
    } else {
      cout << "Point is outside image volume" << endl;
    }

  return 0; // we're trying to put a seed outside of the volume
}

int vtkPichonFastMarching::addSeedsFromImage(vtkImageData* label)
{
  // here the assumption is that the label image is always short pixel type
  // The filter is to be used from Editor effects, and the label is always
  // short, so for this use case it's a valid assumption.
  int nSeeds = 0;
  int scalarType = label->GetScalarType();
  if(scalarType == VTK_SHORT || scalarType == VTK_UNSIGNED_SHORT)
  {
    short* bufferPointer = (short*) label->GetPointData()->GetScalars()->GetVoidPointer(0);
    vtkIdType inc[3];
    label->GetIncrements(inc);
    int extent[6];
    label->GetExtent(extent);

    for(int k=0;k<extent[5]+1;k++)
    {
      for(int j=0;j<extent[3]+1;j++)
      {
        for(int i=0;i<extent[1]+1;i++)
        {
          if(bufferPointer[k*inc[2]+j*inc[1]+i])
          {
            this->addSeedIJK(i,j,k);
            nSeeds++;
          }
        }
      }
    }
  }
  else
  {
    std::cerr << "ERROR: vtkPichonFastMarching: seed label image must be of type SHORT or UNSIGNED SHORT!" << std::endl;
    return 0;
  }

  return nSeeds;
}

char *vtkPichonFastMarching::cxxVersionString()
{
  char* text = new char[100];

  sprintf(text, "%d.%d \t(%s)", MAJOR_VERSION, MINOR_VERSION, DATE_VERSION);
  return text;
}

int vtkPichonFastMarching::cxxMajorVersion()
{
  return MAJOR_VERSION;
}

void vtkPichonFastMarching::tweak(char *name, double value)
{
  if( strcmp( name, "sigma2SmoothPDF" )==0 )
    {
      pdfIntensityIn->sigma2SmoothPDF=value;
      pdfInhomoIn->sigma2SmoothPDF=value;
      return;
    }

  if( strcmp( name, "powerSpeed" )==0 )
    {
      powerSpeed=value;
      return;
    }


  vtkErrorMacro("Error in vtkPichonFastMarching::tweak(...): '" << name << "' not recognized !");
}





