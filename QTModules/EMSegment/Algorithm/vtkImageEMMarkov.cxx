/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageEMMarkov.cxx,v $
  Date:      $Date: 2006/12/08 23:28:23 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
#include "vtkImageEMMarkov.h"
#include "vtkObjectFactory.h"

//------------------------------------------------------------------------------
vtkImageEMMarkov* vtkImageEMMarkov::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageEMMarkov");
  if(ret)
  {
    return (vtkImageEMMarkov*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageEMMarkov;
}

//----------------------------------------------------------------------------
vtkImageEMMarkov::vtkImageEMMarkov()
{
  this->NumClasses = 0;       // Number of Classes - has to be set to 0 bc currently nothing is set
  this->StartSlice = 1;       // First Slide to be segmented
  this->EndSlice = 1;         // Last Slide to be segmented  
  this->Error = 1;             // Error flag -> if Error < 0 than error occured
  this->ImgTestNo = -1;       // Segment an image test picture (-1 => No, > 0 =>certain Test pictures)
  this->ImgTestDivision = -1; // Number of divisions/colors of the picture
  this->ImgTestPixel = -1;    // Pixel length on one division (pixel == -1 => max pixel length for division)
  this->Label = NULL;this->LabelNumber = NULL;this->ClassProbability = NULL;
}

vtkImageEMMarkov::~vtkImageEMMarkov(){
  this->DeleteVariables();
}

//----------------------------------------------------------------------------
void vtkImageEMMarkov::ComputeInputUpdateExtent(int inExt[6], int outExt[6])
{
  this->GetInput()->GetWholeExtent(inExt);
}


//----------------------------------------------------------------------------
void vtkImageEMMarkov::PrintSelf(ostream& os, vtkIndent indent)
{
}

// To chage anything about output us this executed before Thread
//----------------------------------------------------------------------------
void vtkImageEMMarkov::ExecuteInformation(vtkImageData *inData, vtkImageData *outData) 
{
  
  int ext[6];
  vtkFloatingPointType spacing[3], origin[3];
  spacing[0] = 1.0;
  spacing[1] = 1.0;
  spacing[2] = 1.0;

  origin[0] = 0.0;
  origin[1] = 0.0;
  origin[2] = 0.0;


  // Check if extend is sset correctly 
  ext[0] = ext[2] = ext[4] = 0;
  ext[1] = ext[3] = this->NumClasses-1;
  ext[5] = 5;
   
  outData->SetOrigin(origin);
  outData->SetNumberOfScalarComponents(1);
  outData->SetWholeExtent(ext);
  outData->SetSpacing(spacing);
  outData->SetScalarType(VTK_FLOAT);
}





//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class T>
static void vtkImageEMMarkovExecute(vtkImageEMMarkov *self,vtkImageData *in1Data, T *in1Ptr,int inExt[6],vtkImageData *outData, float *outPtr,int outExt[6], int maxZ)
{
  int idxR, idxY, idxZ;
  int maxY;
  int inIncX, inIncY, inIncZ;
  int outIncX, outIncY, outIncZ;
  int rowLength;
  int StartEndSlice;

  // find the region to loop over
  rowLength = (inExt[1] - inExt[0]+1)*in1Data->GetNumberOfScalarComponents();
  maxY = inExt[3] - inExt[2] + 1; // outExt[3/2] = Relative Maximum/Minimum Y index  
  StartEndSlice = self->GetEndSlice() - self->GetStartSlice() + 1;

  // Get increments to march through data 
  in1Data->GetContinuousIncrements(inExt, inIncX, inIncY, inIncZ);
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);

  // The Slices of the images 
  int ***Volume = new int**[StartEndSlice];
  for (idxZ = 0; idxZ < StartEndSlice; idxZ++) {
    Volume[idxZ]  = new int*[maxY];
    for (idxY = 0; idxY < maxY; idxY++)
      Volume[idxZ][idxY] = new int[rowLength];
  }
  // Loop through input pixels 

  // 1.) Read thorugh beginning slides we do not want to read through for input and output
  in1Ptr += ((rowLength+inIncY)*maxY + inIncZ)*(self->GetStartSlice() -1); 

  // 2.) Read thorugh the slides that should be segmented 
  if (self->GetImgTestNo() < 1) {
    for (idxZ = 0; idxZ < StartEndSlice ; idxZ++) {
      for (idxY = 0; idxY < maxY; idxY++) {
        for (idxR = 0; idxR < rowLength; idxR++) {
          Volume[idxZ][idxY][idxR] = ((int) * in1Ptr);
          in1Ptr++;
        }
        in1Ptr += inIncY;
      }
      in1Ptr += inIncZ;
    }
  } else {
    self->setMatrix3DTest(Volume,StartEndSlice,maxY,rowLength,self->GetImgTestNo(),self->GetImgTestDivision(),self->GetImgTestPixel());
  } 
  // 3.) Run Algorith to find out Matrix
  self->TrainMarkovMatrix(Volume, maxY, rowLength, outPtr);
  // 4.) Delete variables
  for (idxZ = 0; idxZ < StartEndSlice; idxZ++) {
    for (idxY = 0; idxY < maxY; idxY++)
      delete []Volume[idxZ][idxY];
    delete[] Volume[idxZ];
  }
  delete[] Volume;
}

//----------------------------------------------------------------------------
// This method is passed a input and output datas, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the datas data types.

// void vtkImageEMMarkov::ThreadedExecute(vtkImageData *inData, vtkImageData *outData,int outExt[6], int id)
void vtkImageEMMarkov::ExecuteData(vtkDataObject *)
{
  void *inPtr;
  void *outPtr;

  int inExt[6];
  int maxZ;
  int outExt[6];
  // Necessary  for VTK
  this->ComputeInputUpdateExtent(inExt,outExt);
 // vtk4
  vtkImageData *inData  = this->GetInput();
  vtkImageData *outData = this->GetOutput();
  outData->SetExtent(this->GetOutput()->GetWholeExtent());
  outData->AllocateScalars();
  outData->GetWholeExtent(outExt);
  // vtk4

  vtkDebugMacro(<< "Execute: inData = " << inData << ", outData = " << outData);
 
  if (inData == NULL) {
    vtkErrorMacro(<< "Input " << 0 << " must be specified.");
    return;
  }

  inPtr = inData->GetScalarPointerForExtent(inExt);
  outPtr = outData->GetScalarPointerForExtent(outExt);
  maxZ = inExt[5] - inExt[4] + 1; // outExt[5/4] = Relative Maximum/Minimum Slice index  

  // Making sure values are set correctly
  if ((this->StartSlice < 1) || (this->StartSlice > this->EndSlice) || (this->EndSlice > maxZ)) {
    vtkErrorMacro(<< "Start Slice,"<<this->StartSlice<< ", or EndSlice," << this->EndSlice << ", not defined correctly !");
    return;
  }
  
  if (outData->GetScalarType() != VTK_FLOAT) {
     vtkErrorMacro(<< "Output image has to be of Data Type VTK_FLOAT.");
     return;
  }
  switch (inData->GetScalarType()) {
    vtkTemplateMacro8(vtkImageEMMarkovExecute, this, inData, (VTK_TT *)(inPtr),inExt, outData, (float *)(outPtr),outExt, maxZ);
  default:
    vtkErrorMacro(<< "Execute: Unknown ScalarType");
    return;
  }
}

//----------------------------------------------------------------------------
// We need to create a special SetNumClass, bc whenver NumClasses are set, we 
// also have to set arrays which depend on the number of classes e.g. prob
//----------------------------------------------------------------------------

void vtkImageEMMarkov::SetNumClasses(int NumberOfClasses)
{
  if (this->NumClasses == NumberOfClasses) return;
  this->DeleteVariables();

  if (NumberOfClasses > 0 ) {

    // Create new space for variables
    this->Label             = new int*[NumberOfClasses];
    this->LabelNumber       = new int[NumberOfClasses];
    this->ClassProbability  = new double[NumberOfClasses];
    // Set all initial values to -1 and 0
    for (int x= 0; x < NumberOfClasses; x++) {
      *this->Label++                = NULL;
      *this->LabelNumber++          = -1;
      *this->ClassProbability++     = 0.0;
    }
    this->Label -= NumberOfClasses; this->LabelNumber -= NumberOfClasses;this->ClassProbability  -= NumberOfClasses;
  } else {
    this->Label = NULL;this->LabelNumber = NULL;this->ClassProbability = NULL;
  }
  this->NumClasses = NumberOfClasses;
}

//----------------------------------------------------------------------------
// Define the arrays 
//----------------------------------------------------------------------------

double vtkImageEMMarkov::GetProbability(int index) {
  if ((index < 0) || (index >= this->NumClasses)) {
    vtkErrorMacro(<< "Error:vtkImageEMSegm::GetProbability: Index exceeds dimensions : " << index);
    this->Error = -6;
    return -6;
  }
  return this->ClassProbability[index];
}

void vtkImageEMMarkov::SetLabelNumber(int index, int num){
  if ((index<0) || (index >= this->NumClasses) || (num < 0) ){
    vtkErrorMacro(<< "Error:vtkImageEMMarkov::SetLabelNumber: Incorrect index (" << index << ") or number ("<<num<<") < 0 !");
    this->Error = -7;
    return;
  }
  if (this->Label[index]) { delete[] Label[index]; }
  this->Label[index] = new int[num];
  for (int i = 0 ; i < num; i++)  this->Label[index][i] = -1;
  this->LabelNumber[index] = num;
}

void vtkImageEMMarkov::SetLabel(int index, int Label){
  if ((index<0) || (index >= this->NumClasses) || (Label < 0) ){
    vtkErrorMacro(<< "Error:vtkImageEMMarkov::SetMu: Incorrect index (" << index << ") or mu ("<<Label<<") < 0 !");
    this->Error = -7;
    return;
  }
  int i= 0;
  while (this->Label[index][i] > -1) i++ ;
  this->Label[index][i] = Label;
}

//----------------------------------------------------------------------------
// Given the input volumes it derives a Markov Matrix

void vtkImageEMMarkov::TrainMarkovMatrix(int ***Image, int Ydim, int Xdim, float *outPtr) {
  // Nothing to do
  if (this->NumClasses == 0 ) return;
  std::cerr << "vtkImageEMMarkov::TrainMarkovMatrix" << endl; 
  int z,x,y,i,j, search;
  int NumSlices = this->EndSlice - this->StartSlice +1;
  int missfits = 0;
  double NormMarkov, NormProb;
  int outDimXY = this->NumClasses*this->NumClasses, 
      index, flag;
  // Check if we have overlapping labels 
  i = flag = 0;

  index = 0;
  for (z = 0 ; z < 6; z++) {
     for (y=0; y < outDimXY ; y++) {
      outPtr[index] = 0.0;
      index ++;
     }
  }

  while ((i < this->NumClasses) && !flag) {
    j = 0;
    while ((j < this->NumClasses) && !flag) {
      if (j!=i) { 
    z = 0;
    while ((z < this->LabelNumber[j]) && !flag) {
      y = 0; 
      while ((y < this->LabelNumber[i]) && !flag) {
        if (this->Label[i][y] == this->Label[j][z]) flag = 1;
        y ++;
      }
          z ++;
    }
      }
      j ++;
    }
    i ++;
  }

  if (flag) {
    std::cerr << "vtkImageEMMarkov::TrainMarkovMatrix::Error: Cannot compute CIM Matrix, because classes does not have unique labels ! "<< endl;
    return; 
  }
  

  // Transfer labels to their parent classes if necessary
  for (z=0; z < NumSlices; z++) {
    for (y=0; y < Ydim; y++) {
      for (x=0; x < Xdim; x++) {
        i = 0;
        search = 1;
        while ((i < this->NumClasses) && search) {
          j = 0;
          while ((j < this->LabelNumber[i]) && search){
        if (this->Label[i][j] == Image[z][y][x]) {
          Image[z][y][x] = i; 
              search = 0;
        }
        j++;
      }
          i++;
    }
    if ((i == this->NumClasses) && search) Image[z][y][x] = -1;
      }
    }
  }
  // Define Markov Matrix and stationary Prior for classes
  
  for (z=0; z < NumSlices; z++) {
    for (y=0; y < Ydim; y++) {
      for (x=0; x < Xdim; x++) {
        if (Image[z][y][x] > -1 ) {
          // This is the y component in the Markov matirx M[z][y][x]
      index = this->NumClasses * Image[z][y][x] ;
          // This way the matrix is defined correctly : Y-Dim = Current Pixel, X-Dim = Neighbouring Pixel 
          // Relationship between pixel and neighbour to the west

          if ((x > 0) && (Image[z][y][x-1] > -1)) outPtr[index + Image[z][y][x-1]] += 1.0;
      index += outDimXY;
    
          // Relationship between pixel and neighbour to the North
          if ((y < (Ydim-1)) && (Image[z][y+1][x] > -1)) outPtr[index + Image[z][y+1][x]] += 1.0;
          index += outDimXY;
    
      // Relationship between pixel and pixel of the next slice
          if ((z < (NumSlices-1)) && (Image[z+1][y][x] > -1)) outPtr[index + Image[z+1][y][x]] += 1.0;
          index += outDimXY;
    
      // Relationship between pixel and neighbour to the east
          if ((x < (Xdim-1)) && (Image[z][y][x+1] > -1)) outPtr[index + Image[z][y][x+1]] += 1.0;
          index += outDimXY;
        
          // Picture is displayed upside down on the screen -> y= 1 -> down, y = Ymax-> up
          // Relationship between pixel and neighbour to the South
          if ((y > 0)  && (Image[z][y-1][x] > -1)) outPtr[index + Image[z][y-1][x]] += 1.0;
          index += outDimXY;

          // Relationship between pixel and pixel of the previous slice
          if ((z > 0) && (Image[z-1][y][x] > -1)) outPtr[index + Image[z-1][y][x]] += 1.0; 
          // Define class probablities/stationary priors
          this->ClassProbability[Image[z][y][x]] += 1;
    } else {
          missfits ++;
    }
      }
    }
  }
  // Rows have to be normalized to 1 !
  // Normalize and round up MarkovMatrix and stationary prios
  if (missfits) {std::cerr << "vtkImageEMMarkov::TrainMarkovMatrix: Number of missifts: " <<  missfits << endl;}
  NormProb = Ydim*Xdim*NumSlices - missfits;
  index = 0;
  for (y=0; y < this->NumClasses; y++) {
    if (NormProb > 0) {
      this->ClassProbability[y] /= NormProb;
    }
    index = this->NumClasses * y; 
    for (z=0; z < 6; z++) {
      NormMarkov = 0.0;

      for (x=0; x < this->NumClasses; x++) { 
        NormMarkov +=  outPtr[index];
        index ++;
      } 
      if (NormMarkov > 0.0) { 
    for (x= this->NumClasses; x > 0; x--) { 
        index --;
            outPtr[index] = int(outPtr[index]/NormMarkov*1000+0.5)/1000.0;
    }
      } else { 
    // M[z][y][y] = 1.0 and rest = 0.0
      index -= this->NumClasses;
          outPtr[index + y] =  1.0;
      }
      index += outDimXY;
    }
  }
   std::cerr << "End vtkImageEMMarkov::TrainMarkovMatrix" << endl; 
}

//----------------------------------------
// Special Vector and Matrix functions

void vtkImageEMMarkov::DeleteVariables() {
  int y;
  if (this->NumClasses > 0 ){
    for (y=0; y < this->NumClasses; y++) {
      if (this->Label[y]) delete[] this->Label[y]; 
    }
    delete[] this->Label; 
    
    delete[] this->LabelNumber; 
    delete[] this->ClassProbability;

    this->Label = NULL;this->LabelNumber = NULL;this->ClassProbability = NULL;
  } 
}

// Setting up Test Matrix to test 
// test = 1 => Creates horizontal stripped matrix with image values from 0 to 255 
// test = 2 => Creates squared matrix with image values from 0 to 255 
// test = 3 => Creates vertical stripped matrix with image values from 0 to 255 
// division = the number of colors that the pixture should be devided in 
//            if pixel == -1 => the number of divisions of the picture
// pixel = pixel length of one divison

void vtkImageEMMarkov::setMatrix3DTest(int ***mat3D,int maxZ, int maxY, int maxX, int test,int division,int pixel){
  int z;
  if (division < 2) {
    for (z = 0 ; z <maxZ; z++)
      this->setMatrix(mat3D[z],maxY,maxX, 0);
    return;
  }
  for (z =0 ; z < maxZ; z++) 
    this->setMatrixTest(mat3D[z],maxY, maxX, test,division,pixel,z+1);
}
void vtkImageEMMarkov::setMatrixTest(int **mat, int maxY, int maxX, int test,int division, int pixel, int offset){
  int y;
  double YScale = 0;
  if (division < 2) {
      this->setMatrix(mat,maxY,maxX,0);
      return;
  }

  if (pixel > 0) YScale = double(pixel);
  else YScale = maxY / double(division);

  int xoffset= offset % division; 
  int offsetadd = (division >2 ? 2:1);
  maxY++;
  for (y=1; y < maxY; y++) {
    if ((test < 3) && (int(fmod(y,YScale)) == 1) && (int(fmod(y-1,YScale)) == 0)) xoffset = (xoffset + offsetadd) % division;
    this->setVectorTest(mat[y-1],maxX,test,division,pixel,xoffset);
  }
}

void vtkImageEMMarkov::setMatrix(int **mat, int maxY, int maxX, int val){
  int y,x;
  for (y = 0; y < maxY; y++){ 
    for (x = 0; x < maxX; x++) mat[y][x] = val;
  }
}

void vtkImageEMMarkov::setVectorTest(int *vec, int maxX,int test,int division,int pixel, int offset){
  int x;
  int ImgColor = 0;
  if (division < 2) {
    for (x = 0; x < maxX; x++) vec[x] = 0;
    return;
  }
  double ImgScale = 255/double(division-1);
  double XScale ;

  if (pixel > 0) XScale = double(pixel);
  else XScale = double(maxX) /double(division);

  if (test < 2) ImgColor = int(ImgScale*(offset%division));
  maxX++;
  for (x =1 ; x < maxX; x++) {
    if (test> 1) ImgColor = int(ImgScale*((int((x-1)/XScale)+offset)%division));
    vec[x-1] = ImgColor;
  }
}

