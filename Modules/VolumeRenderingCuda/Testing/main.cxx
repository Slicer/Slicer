#include <stdio.h>

//#include "CUDA_renderAlgo.h"
#include "ntkTimer.h"

int main(int argc, char** argv){

  //char* fileName = "heart256.raw";
  //if (argc > 1)
  //  fileName = argv[1];

  //ntkTimer *timer=new ntkTimer();

  //unsigned char* inputBuffer=(unsigned char*)malloc(256*256*256*sizeof(unsigned char));
  //unsigned char* outputBuffer;

  //FILE *fp;
  //fp=fopen(fileName,"r");
  //fread(inputBuffer, sizeof(unsigned char), 256*256*256, fp);
  //fclose(fp);

  //// Setting transformation matrix. This matrix will be used to do rotation and translation on ray tracing.

  //float color[6]={255,255,255,1,1,1};
  //float minmax[6]={0,255,0,255,0,255};
  //float lightVec[3]={0, 0, 1};
  //float rotationMatrix[4][4]={{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};

  //// Initialization. Prepare and allocate GPU memory to accomodate 3D data and Result image.

  //timer->Start();
  //CUDArenderAlgo_init(256,256,256,1024,768);
  //timer->End();
  //timer->printDetailedElapsedTime("Initialization.");

  //// Load 3D data into GPU memory.

  //timer->Start();
  //CUDArenderAlgo_loadData(inputBuffer, 256,256,256);
  //timer->End();
  //timer->printDetailedElapsedTime("Load data from CPU to GPU.");
  //
  //// Do rendering. 

  //timer->Start();
  //CUDArenderAlgo_doRender((float*)rotationMatrix, color, minmax, lightVec, 
  //      256,256,256,    //3D data size
  //      1024,768,       //result image size
  //      0,0,0,          //translation of data in x,y,z direction
  //      1, 1, 1,        //voxel dimension
  //      90, 255,        //min and max threshold
  //      -100);          //slicing distance from center of 3D data
  //      
  //timer->End();
  //timer->printDetailedElapsedTime("Volume rendering.");

  //// Get the resulted image.

  //timer->Start();
  //CUDArenderAlgo_getResult((unsigned char**)&outputBuffer, 1024,768);
  //timer->End();
  //timer->printDetailedElapsedTime("Copy result from GPU to CPU.");
  //
  //fp=fopen("output.raw","w");
  //fwrite(outputBuffer, sizeof(unsigned char), 1024*768*4, fp);
  //fclose(fp);

  //// Free allocated GPU memory.
  //
  //CUDArenderAlgo_delete();
  //free(inputBuffer);
}
