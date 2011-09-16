/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageEMGeneral.cxx,v $
  Date:      $Date: 2006/12/15 19:11:06 $
  Version:   $Revision: 1.2 $

=========================================================================auto=*/
#include "vtkImageEMGeneral.h"
#include "vtkImageData.h"
#include "vtkImageWriter.h"
#include "vtkImageClip.h"
#include "vtkImageCast.h"
#include "vtkImageSumOverVoxels.h"
#include "vtkImageAccumulate.h"
#include "vtkMath.h"
#include "vtkImageThreshold.h"
#include "vtkImageMathematics.h"
#include "vtkImageReader.h"
#include "vtkObjectFactory.h"

#include <vtksys/SystemTools.hxx>
#include <sstream>


// Similar to above.  This one computes an
// approximation to the Gaussian of the square
// root of the argument, in other words, the
// argument should already be squared.


// Opens up a new file and writes down result in the file
void vtkImageEMGeneral_WriteVectorMatlabFile (const char *filename, const char *name, unsigned char *vec, int xMax)  {
  int appendFlag = 0;
  FILE *f = (strcmp(filename,"-"))?fopen(filename,((appendFlag)?"a":"w")):stdout;
  if ( f == NULL ) {
    cerr << "Could not open file " << filename << "\n";
    return;
  }
  if (name != NULL) fprintf(f,"%s = [", name);
  xMax --;
  for (int x = 0; x < xMax; x++ )
      fprintf(f,"%d ", vec[x]);
  fprintf(f,"%d", vec[xMax]);
  if (name != NULL) fprintf(f,"];\n");
  fflush(f);
  fclose(f);
}

void vtkImageEMGeneral_WriteVectorMatlabFile (const char *filename, const char *name,float *vec, int xMax)  {
  int appendFlag = 0;
  FILE *f = (strcmp(filename,"-"))?fopen(filename,((appendFlag)?"a":"w")):stdout;
  if ( f == NULL ) {
    cerr << "Could not open file " << filename << "\n";
    return;
  }
  if (name != NULL) fprintf(f,"%s = [", name);
  xMax --;
  for (int x = 0; x < xMax; x++ ) fprintf(f,"%10.6f ", vec[x]);
  fprintf(f,"%10.6f", vec[xMax]);
  if (name != NULL) fprintf(f,"];\n");
  fflush(f);
  fclose(f);
}


// Writes Vector to file in Matlab format if name is specified otherwise just 
// writes the values in the file
void vtkImageEMGeneral_WriteVectorMatlabFile (FILE *f, const char *name, double *vec, int xMax)  {
  if (name != NULL) fprintf(f,"%s = [", name);
  xMax --;
  for (int x = 0; x < xMax; x++ )
      fprintf(f,"%10.6f ", vec[x]);
  fprintf(f,"%10.6f", vec[xMax]);
  if (name != NULL) fprintf(f,"];\n");
}

// Opens up a new file and writes down result in the file
void vtkImageEMGeneral_WriteVectorMatlabFile (const char *filename, const char *varname,double *vec, int xMax)  {
  int appendFlag = 0;
  FILE *f = (strcmp(filename,"-"))?fopen(filename,((appendFlag)?"a":"w")):stdout;
  if ( f == NULL ) {
    cerr << "Could not open file " << filename << "\n";
    return;
  }
  vtkImageEMGeneral_WriteVectorMatlabFile(f,varname,vec,xMax);
  fflush(f);
  fclose(f);
}

// Writes Matrix to file in Matlab format if name is specified otherwise just 
// writes the values in the file
void vtkImageEMGeneral_WriteMatrixMatlabFile (FILE *f, const char *name, double **mat, int imgY, int imgX) 
{
  if (name != NULL) fprintf(f,"%s = [", name);
  for (int y = 0; y < imgY; y++ ) {
    vtkImageEMGeneral_WriteVectorMatlabFile(f,NULL,mat[y],imgX);
    if (y < (imgY-1)) fprintf(f,";\n");
  }
  if (name != NULL) fprintf(f,"];\n");
  fprintf(f,"\n");
}



// Opens up a new file and writes down result in the file
void vtkImageEMGeneral_WriteMatrixMatlabFile (const char *filename, const char *varname, double **mat, int imgY, int imgX)  {
  int appendFlag = 0;
  FILE *f = (strcmp(filename,"-"))?fopen(filename,((appendFlag)?"a":"w")):stdout;
  if ( f == NULL ) {
    cerr << "Could not open file " << filename << "\n";
    return;
  }
  vtkImageEMGeneral_WriteMatrixMatlabFile(f,varname,mat,imgY,imgX);
  fflush(f);
  fclose(f);
}



float vtkImageEMGeneral_qgauss_sqrt(float inverse_sigma, float x)
{
    return EMSEGMENT_ONE_OVER_ROOT_2_PI * inverse_sigma 
    * vtkImageEMGeneral_qnexp2(EMSEGMENT_MINUS_ONE_OVER_2_LOG_2 * inverse_sigma * inverse_sigma * x);
}

// ---------------------------------
// Lookup Table Gauss Function
// --------------------------------
 double vtkImageEMGeneral::LookupGauss(double* table, double lbound, double ubound, double resolution,double value) {
  if (value < lbound) return 0.0;
  if (value >= ubound) return 0.0;
  // printf("%f %f %d \n",value,lbound,int((value-lbound)/resolution)); 
  return table[int((value-lbound)/resolution)];
}

// ---------------------------------
// Lookup Table Gauss Function for multiple Images
// --------------------------------
 double vtkImageEMGeneral::LookupGauss(double* table, double *lbound, double *ubound, double *resolution,double *value,
                         int NumberOfInputImages ) {
  int index=0, offset = 1, i;
  for (i = 0; i< NumberOfInputImages; i++) {
    if (value[i] < lbound[i]) return 0.0;
    if (value[i] >= ubound[i]) return 0.0;
    index  += int((value[i]-lbound[i]) / resolution[i]) *offset;
    offset *= EMSEGMENT_TABLE_SIZE_MULTI;
  }
  return table[index];
}

// ---------------------------------
// Normal Gauss Function
// --------------------------------
 double vtkImageEMGeneral::GeneralGauss(double x,double m,double s) {
  double term = (x-m)/s;
  if  (s > 0 ) return (EMSEGMENT_ONE_OVER_ROOT_2_PI/s * exp(-0.5 *term*term));
  return (m == x ? 1e20:0);
}

// ----------------------------------------
// Normal Gauss Function for Multiple Input 
// ----------------------------------------
 double vtkImageEMGeneral::GeneralGauss(float *x,double *mu,double **inv_cov, double inv_sqrt_det_cov,int n) {
  double *x_m = new double[n];
  double term = 0;
  int i,j; 

  for (i=0; i < n; i++) x_m[i] = double(x[i]) - mu[i];
  for (i=0; i < n; i++) {
    for (j=0; j < n; j++) term += (inv_cov[i][j]*x_m[j]);
    term *= x_m[i];
  }
  delete []x_m;
  return (pow(EMSEGMENT_ONE_OVER_ROOT_2_PI,n)*inv_sqrt_det_cov * exp(-0.5 *term));
}
// -----------------------------------------
// Special Matrix function
// ----------------------------------------

// Convolution and polynomial multiplication . 
// This is assuming u and 'this' have the same dimension
 void vtkImageEMGeneral::convMatrix3D(double*** mat3D, double*** U,int mat3DZlen, int mat3DYlen, int mat3DXlen, double v[],int vLen) {
  int stump = vLen /2;
  int k,j,jMin,jMax,x,y;
  int kMax = mat3DZlen + stump;
  double ***USta = U;
  double *vSta = v;

  for (k = stump; k <  kMax; k++) {
    for (y = 0; y < mat3DYlen; y++) {
    for (x = 0; x < mat3DXlen; x++) 
      (*mat3D)[y][x] = 0;
    }
    jMin = (0 > (k+1 - vLen) ? 0 : (k+1  - vLen));     //  max(0,k+1-vLen):
    jMax = ((k+1) < mat3DZlen ? (k+1) : mat3DZlen);     //  min(k+1,mat3DZlen) 
    // this->mat3D[k-stump] += U[j]*v[k-j];
    U = USta + jMin;  v = vSta + k-jMin; 
    for (j=jMin; j < jMax; j++) {
      for (y = 0; y < mat3DYlen; y++) {
    for (x = 0; x < mat3DXlen; x++)
      (*mat3D)[y][x] += (*U)[y][x] * (*v);
      }
      v--;
      U++;
    }
    mat3D++ ;
  }  
}

// Calculated the determinant for a dim dimensional matrix -> the value is returned 
// Faster with LU decomposition - look in Numerical Recipecs
 double vtkImageEMGeneral::determinant(double **mat,int dim) {
  if (dim < 2) return mat[0][0];
  if (dim < 3) return mat[0][0]*mat[1][1] - mat[0][1]*mat[1][0];
  if (dim < 4) return mat[0][0]*mat[1][1]*mat[2][2] + mat[1][0]*mat[2][1]*mat[0][2] + 
                      mat[2][0]*mat[0][1]*mat[1][2] - mat[0][0]*mat[2][1]*mat[1][2] - 
                      mat[1][0]*mat[0][1]*mat[2][2] - mat[2][0]*mat[1][1]*mat[0][2];
  int j,k,i;
  double result = 0;
  double **submat = new double*[dim-1];
  for (i=0; i< dim-1; i++) submat[i] = new double[dim-1];

  for (j = 0 ; j < dim ; j ++) {
    if (j < 1) {
      for (k=1 ; k < dim; k++) {
    for (i=1; i < dim; i++)
      submat[k-1][i-1] = mat[k][i];  
      }
    } else {
      for (i=1; i < dim; i++) submat[j-1][i-1] = mat[j-1][i];
    }

    result += (j%2 ? -1:1) * mat[0][j]*vtkImageEMGeneral::determinant(submat,dim-1);
  }

  for (i=0; i< dim-1; i++) delete[] submat[i];
  delete[] submat;

  return result;
}

 int vtkImageEMGeneral::InvertMatrix(double **mat, double **inv_mat,int dim) {
  double det;
  if (dim < 2) {
    if (mat[0][0] == 0) return 0;
    inv_mat[0][0] = 1.0 / mat[0][0];
    return 1;
  } 
  if (dim < 3) {
    det = vtkImageEMGeneral::determinant(mat,2);
    if (fabs(det) <  1e-15 ) return 0;
    det = 1.0 / det;
    inv_mat[0][0] = det * mat[1][1];
    inv_mat[1][1] = det * mat[0][0];
    inv_mat[0][1] = -det * mat[0][1];
    inv_mat[1][0] = -det * mat[1][0];
    return 1;
  } 
  return vtkMath::InvertMatrix(mat,inv_mat,dim);
}

// Description:
// Multiplies the Matrix mat with the vector vec => res = mat * vec 
 void vtkImageEMGeneral::MatrixVectorMulti(double **mat,double *vec,double *res,int dim) {
  int x,y;
  for (y= 0 ; y< dim; y++) {
    memset(res, 0, sizeof(double)*dim);
    for (x= 0; x<dim; x++) res[y] += mat[y][x]*vec[x];
  }
}

// -------------------------------------------------------------------------------------------------------------------
// CalculateLogMeanandLogCovariance - for 1 Image
//
// Input: mu              = Vector with Mean values for every tissue class
//        Sigma           = Vector with Sigma values for every tissue class
//        LogMu           = Log Mean Values calculated by the function
//        LogVariance     = Log Variance Values calculated by the function
//        LogTestSequence = Sequence of log(i+1) with i ranges from [0,SequenceMax]
//        SequenceMax     = Maximum "grey value" the mean and sigma value should be computed
//        
// Idea: Calculates the MuLog and SigmaLog values given mu and sigma. To calculate MuLog and SigmaLog we 
//       normally need samples from the image. In our case we do not have any samples, because we do not 
//       know how the image is segmented to get around it we generate a Testsequence T where
//       T[i] =  p(i)*log(i+1) with  
//       p(i) = the probability that grey value i appears in the image given sigma and mu = Gauss(i, Sigma, Mu)
//       and i ranges from [0,SequenceMax]   
// -------------------------------------------------------------------------------------------------------------------

 void vtkImageEMGeneral::CalculateLogMeanandLogCovariance(double *mu, double *Sigma, double *LogMu, double *LogVariance,double *LogTestSequence, int NumberOfClasses, int SequenceMax) { 
  int i,j;
  double term;

  double *ProbSum = new double[NumberOfClasses];
  double *SigmaInverse = new double[NumberOfClasses];
  double  **ProbMatrix = new double*[NumberOfClasses];

  for (i=0; i< NumberOfClasses;i++) {
    SigmaInverse[i] = 1.0/ Sigma[i];
    ProbMatrix[i] = new double[SequenceMax];
  }

  memset(LogMu,       0, NumberOfClasses*sizeof(double));
  memset(LogVariance, 0, NumberOfClasses*sizeof(double));
  memset(ProbSum,     0, NumberOfClasses*sizeof(double));

  // The following is the same in Matlab as sum(p.*log(x+1)) with x =[0:iMax-1] and p =Gauss(x,mu,sigma)
  for (i = 0; i < SequenceMax; i++) {
    LogTestSequence[i] = log(double(i+1));
    for (j=0; j < NumberOfClasses; j++) {
      ProbMatrix[j][i] =  vtkImageEMGeneral::FastGauss(SigmaInverse[j],i - mu[j]);
      LogMu[j] +=  ProbMatrix[j][i]*LogTestSequence[i];
      ProbSum[j] += ProbMatrix[j][i]; 
    }
  } 
  // Normalize Mu over ProbSum
  for (i=0; i < NumberOfClasses; i++) {LogMu[i] /= ProbSum[i];}

  // The following is the same in Matlab as sqrt(sum(p.*(log(x+1)-mulog).*(log(x+1)-mulog))/psum)
  // with x =[0:iMax-1] and p =Gauss(x,mu,sigma) 
  for (i = 0; i < SequenceMax; i++) {
    for (j=0; j < NumberOfClasses; j++) {
      term = LogTestSequence[i] - LogMu[j];
      LogVariance[j] +=  ProbMatrix[j][i]*term*term;
    }
  } 
  // Take the sqrt Kilian Look it up again
  for (i=0; i < NumberOfClasses; i++) { LogVariance[i] =  LogVariance[i] / ProbSum[i]; }

  delete[] SigmaInverse;
  delete[] ProbSum;
  for (i=0; i< NumberOfClasses;i++) delete[] ProbMatrix[i] ;
  delete []ProbMatrix;
}

// -------------------------------------------------------------------------------------------------------------------
// CalculatingPJointDistribution
//
// Input: x                = set input values, x has to be of size numvar and the set variables have to be defined,
//                           e.g. index n and m are set => y[n] and y[m] have to be defined
//        Vleft            = all the vairables that are flexible => e.g Vleft = [1,...,n-1,n+1, .., m-1, m+1 ,..., numvar]
//        mu               = Mean Values of the distribution of size numvar
//        invcov           = The covariance Matrix's inverse of the distribution - has to be of size numvarxnumvar  
//        inv_sqrt_det_cov = The covariance Matrix' determinant squared and inverted
//        SequenceMax      = Test sequence for the different images - has to be of size numvar
//        setvar           = How many veriable are allready fixed 
//        numvar           = Size of Varaible Space V
//        
// Idea: Calculates the joint probability of the  P(i= V/Vleft: y[i] =x[i]) , e.g. P(y[n] = x[n], y[m] = x[m])  
// Explanation: V = {y[1] ... y[numvar]}, V~ = Vleft = V / {y[n],y[m]}, and y = X <=> y element of X 
//              =>  P(y[n] = x[n],y[m] = x[m]) = \sum{k = V~} (\sum{ l = [0..seq[y]]} P(y[n] = x[n],y[m] = x[m],y[k] = l,V~\{ y[k]}) 
// -------------------------------------------------------------------------------------------------------------------
 double vtkImageEMGeneral::CalculatingPJointDistribution(float* x,int *Vleft,double *mu, double **inv_cov, double inv_sqrt_det_cov,int SequenceMax, int setvar,int numvar) {
  double JointProb = 0.0; 
  if (setvar == numvar) {
    if (numvar < 2) JointProb = FastGauss(inv_sqrt_det_cov, double(x[0]) - mu[0]);
      else {
    if (numvar < 3) JointProb = FastGauss2(inv_sqrt_det_cov,x, mu,inv_cov,2);
    else JointProb = vtkImageEMGeneral::GeneralGauss(x,mu,inv_cov,inv_sqrt_det_cov,numvar);
      }
    return JointProb;
  } 
  setvar ++;
  int index = Vleft[numvar - setvar];
  for (int i = 0 ; i < SequenceMax; i++) {
    x[index] = (float)i;
    JointProb += vtkImageEMGeneral::CalculatingPJointDistribution(x,Vleft,mu, inv_cov, inv_sqrt_det_cov,SequenceMax,setvar,numvar);
  }
  return JointProb;
}

// Description:
// Calculates : 
//     /offY->  / *\T /* * * \  /*\  <- offX \.               
// dimY\        |V1|*| * M * | *|V2|         / dimX    where  vec = [* V1 *] = [* V2 *] 
//              \*/  \ * * */   \*/ 
 
 double vtkImageEMGeneral::CalculateVectorMatrixVectorOperation(double** mat, double *vec, int offY,int dimY, int offX, int dimX) {
  double result = 0,ResMatVec;
  int y,x;
  for (y=offY; y < dimY;y++) {
    ResMatVec = 0;
    for (x=offX; x < dimX; x++) ResMatVec += mat[y][x]*vec[x];
    result += vec[y]*ResMatVec;
  }   
  return result; 
}
// Description:                                                            
// Calculates the inner product of <vec, mat[posY:dimY-1][posX]> = vec * mat[posY:dimY-1][posX]
 double vtkImageEMGeneral::InnerproductWithMatrixY(double *vec, double **mat, int posY,int dimY, int posX) {
  double result = 0;
  for (int y = posY ; y < dimY ; y++) result += (*vec++) * mat[y][posX];
  return result;
}

// Description:                                         
// Product of mat[posY][posX:dimY-1]*vec
 double vtkImageEMGeneral::InnerproductWithMatrixX(double **mat, int posY,int posX, int dimX,double* vec) {
  double result = 0;
  for (int x = posX ; x < dimX ; x++) result += mat[posY][x]*(*vec++);
  return result;
}


// ---------------------------------------------------------
// EMGeneral - Just for the Filter itself
// ---------------------------------------------------------

vtkImageEMGeneral* vtkImageEMGeneral::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageEMGeneral");
  if(ret)
  {
    return (vtkImageEMGeneral*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageEMGeneral;
}

// ---------------------------------------------------------
// Math Operation 
// ---------------------------------------------------------

void vtkImageEMGeneral::PrintMatrix(double **mat, int yMax,int xMax) {
  int i;
  for (int y = 0; y < yMax; y++) {
      for (i = 0; i < xMax; i++)
    std::cerr << mat[y][i] << " ";
      std::cerr << endl;
  }
  std::cerr << endl;
}

void vtkImageEMGeneral::PrintMatrix3D(double ***mat, int zMax,int yMax,int xMax) {
  int y,x,z;
  for (z = 0; z < zMax; z++) {
    std::cerr << "mat3D[" << z+1 << "] = ["; 
    for (y = 0; y < yMax; y++) {
      for (x = 0; x < xMax; x++)
    std::cerr << mat[z][y][x] << " ";
      std::cerr << ";" << endl;
    }
    std::cerr << " ]; " << endl;
  }
}


void vtkImageEMGeneral::SquareMatrix(double **Input,double **Output,int dim) {
  int i,j,k;
  for (i = 0 ; i < dim ; i++) {
    memset(Output[i],0,sizeof(double)*dim);
    for (j = 0 ; j < dim ; j++) {
      for (k = 0 ; k < dim ; k++)
        Output[i][j] += Input[k][j]*Input[i][k];
    }
  }
}

//  Smoothes  3D-Matrix
// w(k) = sum(u(j)*v(k+1-j))
// returns Matrix of size r_m
//  void vtkImageEMGeneral::smoothConv(double ***mat3D, int mat3DZlen, int mat3DYlen, int mat3DXlen, double v[],int vLen) {
//    int i,k;
  
//    double *** resultY  = new double**[mat3DZlen]; 
//    double *** resultX  = new double**[mat3DZlen];
//    for (k=0; k < mat3DZlen; k++) {
//        resultY[k]  = new double*[mat3DYlen];
//        resultX[k]  = new double*[mat3DYlen];
//        for (i=0;i < mat3DYlen; i ++) {
//      resultY[k][i]  = new double[mat3DXlen];
//      resultX[k][i]  = new double[mat3DXlen];
//        }
//    }  

//    // First: convolut in Y Direction 
//    for (i = 0; i < mat3DZlen; i++){
//      this->convMatrix(resultY[i],mat3D[i],mat3DYlen,mat3DXlen,v,vLen); 
//    }
//    // Second: convolut in X Direction 
//    for (i = 0; i < mat3DZlen; i++){
//      this->convMatrixT(resultX[i],resultY[i],mat3DYlen,mat3DXlen,v,vLen); 
//    }
//    // Third: in Z direction
//    this->convMatrix3D(mat3D,resultX,mat3DZlen,mat3DYlen,mat3DXlen,v,vLen);

//    for (k=0; k< mat3DZlen; k ++) { 
//      for (i=0; i < mat3DYlen; i++) {
//        delete[] resultY[k][i];
//        delete[] resultX[k][i];
//      }
//      delete[] resultY[k];
//      delete[] resultX[k];
//    }
//    delete[] resultY;
//    delete[] resultX;
//  }

// -------------------------------------------------------------------------------------------------------------------
// CalculateLogMeanandLogCovariance - for multiple Image
//
// Input: Mu                  = Matrix with Mean values for every tissue class (row) and image (column) => [NumberOfClasses]x[NumberOfInputImages]
//        CovMatrix           = Covariance matrix's diagonal for every tissue class (row) and image (column) => [NumberOfClasses]x[NumberOfInputImages]x[NumberOfInputImages]
//        LogMu               = Log Mean Values calculated by the function
//        LogCov              = Log Covariance Values calculated by the function
//        NumberOfInputImages = Number of Input Images 
//        NumberOfClasses     = Number of Classes 
//        SequenceMax         = Maximum "grey value" for every image the mean and sigma value should be computed
// 
// Output: If return value is 0 => could not invert coveriance matrix  
//     
// Idea: Calculates the MuLog Vecotr and CovarianceLog Matrix given mu and CovDiag for every tissue class. To calculate MuLog 
//       and LogCovariance we normally need samples from the image. In our case we do not have any samples, because we do not 
//       know how the image is segmented to get around it we generate a Testsequence T where
//       T[i] =  p(i)*log(i+1) with  
//       p(i) = the probability that grey value i appears in the image given sigma and mu = Gauss(i, Sigma, Mu)
//       and i ranges from [0,SequenceMax]   
// -------------------------------------------------------------------------------------------------------------------
int vtkImageEMGeneral::CalculateLogMeanandLogCovariance(double **Mu, double ***CovMatrix, double **LogMu, double ***LogCov,int NumberOfInputImages, int  NumberOfClasses, int SequenceMax) { 
  std::cerr <<"vtkImageEMGeneral::CalculateLogMeanandLogCovariance start " << endl;
  int i,j,k,l,m;
  int flag = 1;
  int VleftDim = (NumberOfInputImages < 3 ? 1 : NumberOfInputImages -2);
  double inv_sqrt_det_cov;
  double JointProb, JointSum;
  double termJ; 

  int *Vleft                = new int[VleftDim];
  double *LogCovDiag        = new double[NumberOfInputImages];
  double *SqrtCovDiag       = new double[NumberOfInputImages];
  float *x                  = new float[NumberOfInputImages];
  double **inv_cov          = new double*[NumberOfInputImages];
  double *LogTestSequence  = new double[SequenceMax];
  for(i = 0; i < NumberOfInputImages; i++) { 
    inv_cov[i] =  new double[NumberOfInputImages];
  }
  // 1.) Calculate MuLog and the Diagonal Elements of the LogCoveriance Matrix
  for (i = 0; i < NumberOfClasses; i++) {
    for (j=0; j< NumberOfInputImages; j++) SqrtCovDiag[j] = sqrt(CovMatrix[i][j][j]);    
    vtkImageEMGeneral::CalculateLogMeanandLogCovariance(Mu[i], SqrtCovDiag, LogMu[i],LogCovDiag,LogTestSequence,NumberOfInputImages,SequenceMax);
    for (j = 0; j < NumberOfInputImages; j++)
      LogCov[i][j][j] = LogCovDiag[j];
  }
  // 2.) Now calculate the other values of the log covariance matrix
  // Remember: Cov[i][j]= (\sum_{c=[0..N]} \sum_{d=[0..M]} ((c - mu[i])(d- mu[j]) * P(x[i] = c ,x[j] = d))) / ( \sum_{c=[0..N]} \sum_{d=[0..M]} P(x[i] = c,x[j] = d))
  // more general :
  // Cov[i][j]= (\sum_{c=[0..N]} \sum_{d=[0..M]} ((f(c) - mu_f[i])(f(d)- mu_f[j]) * P(x[i] = c,x[j] = d))) / ( \sum_{c=[0..N]} \sum_{d=[0..M]} P(x[i]= c,x[j] = d))
  // where in our case f(x) := log(x+1) =>We (Sandy,Lilla, Dave and me) concluded this work of art after a very long and hard discussion overdays with including 
  // several offical revisions.

  for (i = 0; i < NumberOfClasses; i++) {
    // We know: size(mu) = [NumberOfClasses][NumberOfInputImages], size(cov) = [NumberOfClasses][NumberOfInputImages][NumberOfInputImages] 
    // => we just look at mu[i] (=>size(mu[i]) = [NumberOfInputImages]) and cov[i] (=>size(cov[i]) = [NumberOfInputImages][NumberOfInputImages])

    // 2.a) Calculate for the Gaussian Calculation the covariance Matrix's inverse and the inverse squared covariance matrix determinant
    if (vtkImageEMGeneral::InvertMatrix(CovMatrix[i],inv_cov, NumberOfInputImages) == 0) {
      flag = 0; // => could not invert matrix
      i =  NumberOfClasses;
      cerr << " Could not invert covariance matrix !" << endl;
    }  else { 
      inv_sqrt_det_cov = determinant(CovMatrix[i],NumberOfInputImages);
      if (inv_sqrt_det_cov <= 0.0) {
     flag = 0; // => could not invert matrix
     i =  NumberOfClasses;
     cerr << "Covariance Matrix is not positiv definit !" << endl;
      }
      inv_sqrt_det_cov = 1.0 / sqrt(inv_sqrt_det_cov);
      // 2.b ) Calculate the Joint Probabilites and Log Coveriance Matrix   
      // picking x[j]
      for (j = 0; j < NumberOfInputImages; j++) {
    // Initialise V~ for the first run the two variables are j and j+1
    for(k = 0; k < j; k++) Vleft[k] = k;
    for(k = j+2; k < NumberOfInputImages; k++) Vleft[k-2] = k;

    // picking x[k] 
    for (k = j+1; k < NumberOfInputImages; k++) {
      // Update V~ 
      if (k > j+1) Vleft[k-2] = k-1;
      // Remember covariance matrixes are symmetric => start at j+1    
      JointSum = 0;
      for (l = 0; l < SequenceMax;l++) {
        x[j] = (float)l;
        termJ = LogTestSequence[l] - LogMu[i][j];
        for (m = 0; m < SequenceMax;m++) {
          x[k] = (float)m;
          // Calculating P(x[j] = l,x[k] = m)
          JointProb = vtkImageEMGeneral::CalculatingPJointDistribution(x,Vleft,Mu[i],inv_cov,inv_sqrt_det_cov,SequenceMax,2,NumberOfInputImages);
          LogCov[i][j][k] +=  termJ * (LogTestSequence[m] - LogMu[i][k]) * JointProb;
          JointSum += JointProb;
        }
      } // End of n
      if (JointSum > 0) LogCov[i][j][k] /= JointSum;     
      LogCov[i][k][j] = LogCov[i][j][k];
    }     
      }
    }
  }
  // Delete All veriables
  delete[] Vleft;
  delete[] x;
  delete[] LogCovDiag;
  delete[] SqrtCovDiag;
  delete[] LogTestSequence;
  for(i = 0; i < NumberOfInputImages; i++) delete[] inv_cov[i];
  delete[] inv_cov;
  std::cerr <<"vtkImageEMGeneral::CalculateLogMeanandLogCovariance end" << endl;
  return flag; // Everything went OK => flag == 1
}    

// -------------------------------------------------------------------------------------------------------------------
// CalculateLogMeanandLogCovariance - for 1 Image
//
// Input: mu              = Vector with Mean values for every tissue class
//        Sigma           = Vector with Sigma values for every tissue class
//        logmu           = Log Mean Values calculated by the function
//        logSigma        = Log Sigma Values calculated by the function - this case is standard deviation
//                          or sqrt(variation)
//        NumberOfClasses = Number of classes
//        SequenceMax     = Maximum "grey value" the mean and sigma value should be computed
//        
// Idea: Calculates the MuLog and SigmaLog values given mu and sigma. To calculate MuLog and SigmaLog we 
//       normally need samples from the image. In our case we do not have any samples, because we do not 
//       know how the image is segmented to get around it we generate a Testsequence T where
//       T[i] =  p(i)*log(i+1) with  
//       p(i) = the probability that grey value i appears in the image given sigma and mu = Gauss(i, Sigma, Mu)
//       and i ranges from [0,SequenceMax]   
// -------------------------------------------------------------------------------------------------------------------
void vtkImageEMGeneral::CalculateLogMeanandLogCovariance(double *mu, double *Sigma, double *logmu, double *logSigma, int NumberOfClasses, int SequenceMax) { 
  double *LogTestSequence  = new double[SequenceMax];

  vtkImageEMGeneral::CalculateLogMeanandLogCovariance(mu,Sigma,logmu,logSigma,LogTestSequence,NumberOfClasses,SequenceMax);
  // -> I get the Variance - what I need for the one dimesional case is the standard deviation or sigma value for the gauss curve
  for (int k=0; k< NumberOfClasses;k++) logSigma[k] = sqrt(logSigma[k]); 

  delete [] LogTestSequence;
}

// Specifically defined function for opening files within EM iterations to keep the naming convention  
FILE* vtkImageEMGeneral::OpenTextFile(const char* FileDir, const char FileName[], int Label, int LabelFlag, 
                      const char *LevelName, int LevelNameFlag, int iter, int IterFlag, 
                      const char FileSucessMessage[], char OpenFileName[]) {
  FILE* OpenFile;

  std::stringstream ss;
  ss << FileDir << "/" << FileName;
  if (LabelFlag)     ss << "_C" << setw(2) << Label;
  if (LevelNameFlag) ss << "_L" << LevelName;
  if (IterFlag)      ss << "_I" << setw(2) << iter;
  ss << ".txt";
  strcpy (OpenFileName,ss.str().c_str());

  if (vtksys::SystemTools::MakeDirectory(OpenFileName) == false) return NULL;
  
#ifdef _WIN32
  OpenFile= fopen(OpenFileName, "wb");
#else 
  OpenFile= fopen(OpenFileName, "w");
#endif

  if (OpenFile && FileSucessMessage) std::cerr << FileSucessMessage  << OpenFileName << endl;
  return OpenFile;
}

void* vtkImageEMGeneral::GetPointerToVtkImageData(vtkImageData *Image, int DataType, int Ext[6]) {
 Image->SetWholeExtent(Ext);
 Image->SetExtent(Ext); 
 Image->SetNumberOfScalarComponents(1);
 Image->SetScalarType(DataType); 
 Image->AllocateScalars(); 
 return Image->GetScalarPointerForExtent(Ext);
}

//----------------------------------------------------------------------------
//Could not put it into another file like vtkImageGeneral - then it would seg falt - do not ask me why 
void vtkImageEMGeneral::GEImageReader(vtkImageReader *VOLUME, const char FileName[], int Zmin, int Zmax, int ScalarType) {
  std::cerr << "Load file " <<  FileName << endl;
  VOLUME->ReleaseDataFlagOff();
  VOLUME->SetDataScalarType(ScalarType);
  VOLUME->SetDataSpacing(0.9375,0.9375,1.5);
  VOLUME->SetFilePattern("%s.%03d");
  VOLUME->SetFilePrefix(FileName);
  VOLUME->SetDataExtent(0, 255, 0 ,255 , Zmin , Zmax);
  VOLUME->SetNumberOfScalarComponents(1);
  VOLUME->SetDataByteOrderToLittleEndian();
  VOLUME->Update();
}


// -------------------------------------------------------------------------------------------------------------------
// CalculateGaussLookupTable
// Calculate the Gauss-Lookup-Table for one tissue class 
// -------------------------------------------------------------------------------------------------------------------
int vtkImageEMGeneral::CalculateGaussLookupTable(double *GaussLookupTable,double **ValueTable,double **InvCovMatrix, 
                            double InvSqrtDetCovMatrix, double *ValueVec,int GaussTableIndex,int TableSize,
                            int NumberOfInputImages, int index) {
  double F1,F2;
  int i;
  if (index > 0) {
    for (i=0; i < TableSize;i++) {
      ValueVec[index] = ValueTable[index][i];
      GaussTableIndex = vtkImageEMGeneral::CalculateGaussLookupTable(GaussLookupTable,ValueTable,InvCovMatrix,InvSqrtDetCovMatrix, ValueVec,
                                     GaussTableIndex,TableSize,NumberOfInputImages,index-1); 
    }
  } else {
    if (NumberOfInputImages > 1) {
      if (NumberOfInputImages > 2) {
    // for faster calculations:
    // already fixed values are Vector V, SubMatrixes S1 S2 S3, and Values a 
    // the flexible value is x
    //        T                   T T        T                                         T        T
    // => [x,V ] *|a  S1| * [ x, V ]  = [x,V] * [x* a  + S1*V ,  = X^2 *a + x*(S1*V + V *S2) + V *S3*V 
    //            |S2 S3|                        x* S2 + S3*V ] 
    //
    //                                = F1 + x*F2 + F3*x^2
    //             T                     T
    // with F1 =  V *S3*V1, F2 = S1*V + V *S2 and F3 = a
    F1 = vtkImageEMGeneral::CalculateVectorMatrixVectorOperation(InvCovMatrix,ValueVec, 1,NumberOfInputImages, 1, NumberOfInputImages);
        F2 =  vtkImageEMGeneral::InnerproductWithMatrixX(InvCovMatrix,0,1,NumberOfInputImages,ValueVec+1) 
        + vtkImageEMGeneral::InnerproductWithMatrixY(ValueVec+1,InvCovMatrix,1,NumberOfInputImages,0); 
      } else {
      F1 = ValueVec[1]*ValueVec[1]*InvCovMatrix[1][1];
      F2 = ValueVec[1]*(InvCovMatrix[0][1] + InvCovMatrix[1][0]);
      }
      for(i=0 ; i< TableSize; i++) {
    GaussLookupTable[GaussTableIndex] = vtkImageEMGeneral::FastGaussMulti(InvSqrtDetCovMatrix, 
                                F1 + ValueTable[0][i]*(F2 + InvCovMatrix[0][0]*ValueTable[0][i]), NumberOfInputImages);
    GaussTableIndex ++;
      }

    } else {
      // Calculate Gauss Value for  the half open interval [TableLBound[i] +j/TableResolution[i],TableLBound[i] +(j+1) / TableResolution[i])
      // For mor exact measures
      // GausLookUpTable[i][j] = EMLocal3DSegmenterGauss(-HalfTableSpan + (j+0.5) * TableResolution[i],0,SigmaLog[i]);
      // => Does Not Cahnge a lot => Most influence over acccuracy EMSEGMENT_TABLE_EPSILON
      for (i= 0; i< TableSize; i++) GaussLookupTable[i] =  vtkImageEMGeneral::FastGauss(InvSqrtDetCovMatrix, ValueTable[0][i]);
      GaussTableIndex += TableSize;
    }
  }
  return GaussTableIndex;
}

void vtkImageEMGeneral::TestMatrixFunctions(int MatrixDim,int iter) {
  int i,j,k;
  double **mat = new double*[MatrixDim];
  double **out = new double*[MatrixDim];
  char name[100];
  int a;
  int NumberOfInputImages = 4, 
      NumberOfClasses = 2;
  int SequenceMax = 5000;
  double **Mu   = new double*[NumberOfClasses];
  double **LogMu = new double*[NumberOfClasses];
  double ***CovMatrix = new double**[NumberOfClasses];
  double ***LogCov    = new double**[NumberOfClasses];
  for (i=0;i < NumberOfClasses; i++) {
    Mu[i]        = new double[NumberOfInputImages];
    LogMu[i]     = new double[NumberOfInputImages];
    CovMatrix[i] = new double*[NumberOfInputImages];
    LogCov[i]    = new double*[NumberOfInputImages];
    for (j=0;j < NumberOfInputImages; j++) {
          CovMatrix[i][j] = new double[NumberOfInputImages];
      LogCov[i][j]    = new double[NumberOfInputImages];
      Mu[i][j] = (i+1)*100+j*20;
      for (k=0;k < NumberOfInputImages; k++)  CovMatrix[i][j][k] = ((j==k) ? ((i+1) + k):0.2);
    }
  }
  std::cerr << "Calculate LogMean and Coveriance" << endl;
  std::cerr << "Mu = [" ;
  this->PrintMatrix(Mu,NumberOfClasses,NumberOfInputImages);
  std::cerr << "Covariance" ;
  this->PrintMatrix3D(CovMatrix,NumberOfClasses,NumberOfInputImages,NumberOfInputImages);
  this->CalculateLogMeanandLogCovariance(Mu,CovMatrix, LogMu,LogCov,NumberOfInputImages, NumberOfClasses, SequenceMax); 
  std::cerr << "LogMu = [" ;
  this->PrintMatrix(LogMu,NumberOfClasses,NumberOfInputImages);
  std::cerr << "LogCovariance" ;
  this->PrintMatrix3D(LogCov,NumberOfClasses,NumberOfInputImages,NumberOfInputImages);
  std::cerr <<" Type in a number :";
  cin >> a;

  for (i=0;i < NumberOfClasses; i++) { 
    for (j=0;j < NumberOfInputImages; j++) {
      delete[] CovMatrix[i][j];
      delete[] LogCov[i][j];
    }
    delete[] Mu[i];
    delete[] LogMu[i];
    delete[] CovMatrix[i];
    delete[] LogCov[i];
  }
  delete[] Mu;
  delete[] LogMu;
  delete[] CovMatrix;
  delete[] LogCov;

  for (k=0; k<iter; k++) {
    for (i = 0; i < MatrixDim; i++) { 
      mat[i] = new double[MatrixDim];
      out[i] = new double[MatrixDim];
      for (j=1; j < MatrixDim; j++) mat[i][j] = double(int(vtkMath::Random(0,10)*100))/ 100.0; 
    }
    sprintf(name,"TestDet%d.m",k+1);

    vtkImageEMGeneral_WriteMatrixMatlabFile(name,"mat",mat,MatrixDim,MatrixDim);
    std::cerr << "Result of " << k << endl;
    std::cerr <<" Determinant: " << vtkImageEMGeneral::determinant(mat,MatrixDim) << endl;
    std::cerr <<" Square: " << endl;
    vtkImageEMGeneral::SquareMatrix(mat,out,MatrixDim);
    this->PrintMatrix(out,MatrixDim,MatrixDim);

  }

  for (i = 0; i < MatrixDim; i++) {
    delete[] mat[i];
    delete[] out[i];
  }
  delete[] mat;
  delete[] out;
}

float vtkImageEMGeneral_CountLabel(vtkImageThreshold* trash,vtkImageData * Input, float val) {
  float result;
  trash->SetInput(Input); 
  trash->ThresholdBetween(val,val);
  trash->SetInValue(1.0); 
  trash->SetOutValue(0.0);
  trash->SetOutputScalarType(Input->GetScalarType());
  trash->Update();
  vtkImageAccumulate *Accu = vtkImageAccumulate::New() ;
  Accu->SetInput(trash->GetOutput());
  Accu->SetComponentExtent(0,1,0,0,0,0);
  Accu->SetComponentOrigin(0.0,0.0,0.0); 
  Accu->SetComponentSpacing(1.0,1.0,1.0);
  Accu->Update();  
#if (VTK_MAJOR_VERSION == 4 && VTK_MINOR_VERSION >= 3)
  result = Accu->GetOutput()->GetScalarComponentAsDouble(1,0,0,0);
#else
  result = Accu->GetOutput()->GetScalarComponentAsFloat(1,0,0,0);
#endif
  Accu->Delete();
  return result;
}
// Calculates DICE and Jakobian Simularity Measure 
// Value defines the vooxel with those label to be measured
// Returns  Dice sim measure

float vtkImageEMGeneral::CalcSimularityMeasure (vtkImageData *Image1, vtkImageData *Image2,float val, int PrintRes, int *BoundaryMin, int *BoundaryMax) {
  vtkImageThreshold *Trash1 =  vtkImageThreshold::New(), 
                    *Trash2 =  vtkImageThreshold::New(),
                    *Final  =  vtkImageThreshold::New();

  vtkImageClip      *ROI1 = vtkImageClip::New();
  ROI1->SetInput(Image1);
  ROI1->SetOutputWholeExtent(BoundaryMin[0],BoundaryMax[0],BoundaryMin[1],BoundaryMax[1],BoundaryMin[2],BoundaryMax[2]);
  ROI1->ClipDataOn(); 
  ROI1->Update();

  vtkImageClip      *ROI2 = vtkImageClip::New();
  ROI2->SetInput(Image2);
  ROI2->SetOutputWholeExtent(BoundaryMin[0],BoundaryMax[0],BoundaryMin[1],BoundaryMax[1],BoundaryMin[2],BoundaryMax[2]);
  ROI2->ClipDataOn(); 
  ROI2->Update();

  vtkImageMathematics *MathImg = vtkImageMathematics::New();
  float result;
  float NumMeasure;
  float DivMeasure = vtkImageEMGeneral_CountLabel(Trash1,ROI1->GetOutput(), val); 
  DivMeasure += vtkImageEMGeneral_CountLabel(Trash2,ROI2->GetOutput(), val); 

  // Find out overlapping volume 
  MathImg->SetOperationToAdd();
  MathImg->SetInput(0,Trash1->GetOutput());
  MathImg->SetInput(1,Trash2->GetOutput());
  MathImg->Update();
  NumMeasure = vtkImageEMGeneral_CountLabel(Final,MathImg->GetOutput(),2);
  if (DivMeasure > 0) result = 2.0*NumMeasure / DivMeasure;
  else result = -1.0;
  if (PrintRes) {
    std::cerr << "Label:                 " << val << endl; 
    std::cerr << "Total Union Sum:       " << DivMeasure - NumMeasure << endl; 
    std::cerr << "Total Interaction Sum: " << NumMeasure << endl;
    //  std::cerr << "Jakobien sim measure:  " << ((DivMeasure - NumMeasure) > 0.0 ? NumMeasure / (DivMeasure - NumMeasure) : -1) << endl;
    std::cerr << "Dice sim measure:      " << result << endl;
  }
  ROI1->Delete();
  ROI2->Delete();
  Trash1->Delete();
  Trash2->Delete();
  Final->Delete();
  MathImg->Delete();
  return result;
}  


float vtkImageEMGeneral::CalcSimularityMeasure (vtkImageData *Image1, vtkImageData *Image2,float val, int PrintRes) {
  vtkImageThreshold *Trash1 =  vtkImageThreshold::New(), 
                    *Trash2 =  vtkImageThreshold::New(),
                    *Final  =  vtkImageThreshold::New();

  vtkImageMathematics *MathImg = vtkImageMathematics::New();
  float result;
  float NumMeasure;
  float DivMeasure = vtkImageEMGeneral_CountLabel(Trash1,Image1, val); 
  DivMeasure += vtkImageEMGeneral_CountLabel(Trash2,Image2, val); 

  // Find out overlapping volume 
  MathImg->SetOperationToAdd();
  MathImg->SetInput(0,Trash1->GetOutput());
  MathImg->SetInput(1,Trash2->GetOutput());
  MathImg->Update();
  NumMeasure = vtkImageEMGeneral_CountLabel(Final,MathImg->GetOutput(),2);
  if (DivMeasure > 0) result = 2.0*NumMeasure / DivMeasure;
  else result = -1.0;
  if (PrintRes) {
    std::cerr << "Label:                 " << val << endl; 
    std::cerr << "Total Union Sum:       " << DivMeasure - NumMeasure << endl; 
    std::cerr << "Total Interaction Sum: " << NumMeasure << endl;
    //  std::cerr << "Jakobien sim measure:  " << ((DivMeasure - NumMeasure) > 0.0 ? NumMeasure / (DivMeasure - NumMeasure) : -1) << endl;
    std::cerr << "Dice sim measure:      " << result << endl;
  }
  Trash1->Delete();
  Trash2->Delete();
  Final->Delete();
  MathImg->Delete();
  return result;
}  

// First normalize images  between 0 and 1 and then calcuate dice   
// normalizationType = 0 both images are normalized independently 
//                                = 1 both images are normalized together by using the max and min scalar values across the two images 
//                                = 2 only the first image is normalized 
double vtkImageEMGeneral::CalcSoftSimularityMeasureNormalize (vtkImageData *Image1, vtkImageData *Image2, int normalizationType) {
   double min1 = Image1->GetScalarRange()[0];
   double max1 = Image1->GetScalarRange()[1];
   double min2 = Image2->GetScalarRange()[0]; 
   double max2 = Image2->GetScalarRange()[1];

   //cout << "sfdds1 " <<   min1 << " " << max1 << endl;
   //cout << "sfdds2 " <<   min2 << " " << max2 << endl;

   switch  (normalizationType) {
      case 0: break;
      case 1: if (min1 > min2)  { min1 = min2;
                     } else { min2 = min1; }
                     if (max1 < max2)  { max1 = max2;
                     } else { max2 = max1; } 
                     break;
       case 2: min2 = 0; max2 = 1.0; break;
       default :   
                   vtkErrorMacro("Type " <<   normalizationType << " is currently not installed");
               return -1;
     }

   double norm1 = max1 - min1;
   if (norm1== 0 ) { 
      norm1 = 1; 
    } else {
    norm1 = 1.0/norm1;
   }

   // cout << "sfdds " <<   min1 << " " << max1 << " " << norm1 << endl;


  vtkImageCast* CAST1 = vtkImageCast::New();
  CAST1->SetInput(Image1);  
  CAST1->SetOutputScalarTypeToFloat(); 
  CAST1->Update();

   vtkImageMathematics *SHIFT1 = vtkImageMathematics::New();
   SHIFT1->SetOperationToAddConstant(); 
   SHIFT1->SetInput1(CAST1->GetOutput());
   SHIFT1->SetConstantC(- min1 ); 
   SHIFT1->Update();

   vtkImageMathematics *NORM1 = vtkImageMathematics::New();
   NORM1->SetOperationToMultiplyByK(); 
   NORM1->SetInput1(SHIFT1->GetOutput());
   NORM1->SetConstantK(norm1);
   NORM1->Update();

   double norm2 = max2 - min2;
   if (norm2 == 0) { 
      norm2 = 1; 
    } else {
    norm2 = 1.0/norm2;
   }

  vtkImageCast* CAST2 = vtkImageCast::New();
  CAST2->SetInput(Image2);  
  CAST2->SetOutputScalarTypeToFloat(); 
  CAST2->Update();

   vtkImageMathematics *SHIFT2 = vtkImageMathematics::New();
   SHIFT2->SetOperationToAddConstant(); 
   SHIFT2->SetInput1(CAST2->GetOutput());
   SHIFT2->SetConstantC(- min2 ); 
   SHIFT2->Update();

   vtkImageMathematics *NORM2 = vtkImageMathematics::New();
   NORM2->SetOperationToMultiplyByK(); 
   NORM2->SetInput1(SHIFT2->GetOutput());
   NORM2->SetConstantK(norm2);
   NORM2->Update();

   double value = this->CalcSoftSimularityMeasure (NORM1->GetOutput(), NORM2->GetOutput());
   NORM1->Delete();
   SHIFT1->Delete();
   CAST1->Delete();
   NORM2->Delete();
   SHIFT2->Delete();
   CAST2->Delete();

   return value;
 }

// images are not thresholded - have to be of the same chast and between 0 and 1
double vtkImageEMGeneral::CalcSoftSimularityMeasure (vtkImageData *Image1, vtkImageData *Image2)  {
   if ((Image1->GetScalarRange()[0] < 0) || (Image1->GetScalarRange()[1] > 1.0001) )  {
     vtkErrorMacro("Scalars of first image are out of range [0,1] - Actual Range [" <<Image1->GetScalarRange()[0] <<"," << Image1->GetScalarRange()[1]  << "]" )
     return -1; 
   } 

   if ((Image2->GetScalarRange()[0] < 0) || (Image2->GetScalarRange()[1] > 1.0001) )  {
     vtkErrorMacro("Scalars of second image are out of range [0,1] -Actual Range [" <<Image2->GetScalarRange()[0] <<"," << Image2->GetScalarRange()[1]  << "]" )
     return -1; 
   } 

   if (Image1->GetScalarType()  != Image1->GetScalarType() ) {
     vtkErrorMacro("Scalars types of input are not coherrent")
     return -1; 
   } 

   vtkImageMathematics *UNION = vtkImageMathematics::New();
   UNION->SetOperationToMultiply();
   UNION->SetInput1(Image1);
   UNION->SetInput2(Image2);
   UNION->Update();

   vtkImageSumOverVoxels* VOXELSUM  = vtkImageSumOverVoxels::New();
   VOXELSUM ->SetInput(UNION->GetOutput());
   VOXELSUM ->Update();
   double nominator = 2*VOXELSUM->GetVoxelSum();

   VOXELSUM ->SetInput(Image1);
   VOXELSUM ->Update();
   double denominator = VOXELSUM->GetVoxelSum();

   VOXELSUM ->SetInput(Image2);
   VOXELSUM ->Update();
   denominator += VOXELSUM->GetVoxelSum();

   UNION->Delete();
   VOXELSUM->Delete();

   if (denominator == 0) { 
       return 1;
   }
   
   return    nominator/denominator;
}  

double vtkImageEMGeneral::CalculateMean(vtkImageData *image)  {
  vtkImageSumOverVoxels* sum =  vtkImageSumOverVoxels::New();
  sum->SetInput(image);
  sum->Update();
  int *dim = image->GetDimensions();
  double n = double(dim[0]*dim[1]*dim[2]);
  double mean = sum->GetVoxelSum()/n;
  sum->Delete();
  return mean;
}  

double vtkImageEMGeneral::CalculateCovariance(vtkImageData *image, double mean)  {
  vtkImageMathematics* shift = vtkImageMathematics::New();
  shift->SetInput(image);
  shift->SetOperationToAddConstant();
  shift->SetConstantC(-mean);
  shift->Update();

  vtkImageMathematics* sqr = vtkImageMathematics::New();
  sqr->SetInput(shift->GetOutput());
  sqr->SetOperationToSquare();
  sqr->Update();

  vtkImageSumOverVoxels* sum =  vtkImageSumOverVoxels::New();
  sum->SetInput(sqr->GetOutput());
  sum->Update();
  int *dim = image->GetDimensions();
  double n = double(dim[0]*dim[1]*dim[2]);
  double cov = sum->GetVoxelSum()/(n -1);

  sum->Delete();
  sqr->Delete();
  shift->Delete();

  return cov;
}  

// Assumes that images have the same dimension
double vtkImageEMGeneral::CalculateNCC(vtkImageData *image1, vtkImageData *image2)
{
  int *dim = image1->GetDimensions();
  double n = double(dim[0]*dim[1]*dim[2]);

  double mean1 =  this->CalculateMean(image1);
  double cov1 =  this->CalculateCovariance(image1,mean1)*(n-1);
  double mean2 =  this->CalculateMean(image2);
  double cov2 =  this->CalculateCovariance(image2,mean2)*(n-1);
 
 vtkImageMathematics* shift1 = vtkImageMathematics::New();
  shift1->SetInput(image1);
  shift1->SetOperationToAddConstant();
  shift1->SetConstantC(-mean1);
  shift1->Update();

 vtkImageMathematics* shift2 = vtkImageMathematics::New();
  shift2->SetInput(image2);
  shift2->SetOperationToAddConstant();
  shift2->SetConstantC(-mean2);
  shift2->Update();

  vtkImageMathematics* mul = vtkImageMathematics::New();
  mul->SetInput1(shift1->GetOutput());
  mul->SetInput2(shift2->GetOutput());
  mul->SetOperationToMultiply();
  mul->Update();

  vtkImageSumOverVoxels* numerator =  vtkImageSumOverVoxels::New();
  numerator->SetInput(mul->GetOutput());
  numerator->Update();

  double NCC =  numerator->GetVoxelSum() / (sqrt(cov1) *sqrt(cov2));
 
  numerator->Delete();
  mul->Delete();
  shift2->Delete();
  shift1->Delete();
  return NCC;
  
}
