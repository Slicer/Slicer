
#ifndef _LLSBiasCorrector_txx
#define _LLSBiasCorrector_txx

#include "LLSBiasCorrector.h"

#include "vnl/vnl_math.h"

#include <float.h>
#include <math.h>

#include <iostream>

// Use the normal equation? Less accurate, but requires much less memory
#define LLSBIAS_USE_NORMAL_EQUATION 1

#define EXPP(x) (exp((x)/100.0) - 1.0)
#define LOGP(x) (100.0 * log((x)+1.0))
#define MUL100(x) (100.0 * x)
//#define EXPP(x) (exp(x) - 1)
//#define LOGP(x) (log((x)+1))

////////////////////////////////////////////////////////////////////////////////

static inline
double
mypow(double x, unsigned int n)
{
  double p = 1.0;
  for (unsigned int i = 0; i < n; i++)
    p *= x;
  return p;
}

////////////////////////////////////////////////////////////////////////////////

template <class TInputImage, class TProbabilityImage>
LLSBiasCorrector <TInputImage, TProbabilityImage>
::LLSBiasCorrector()
{

  m_InputImages.Clear();

  m_MaxDegree = 4;

  m_SampleSpacing = 4.0;
  m_WorkingSpacing = 1.0;

  m_ClampBias = false;
  m_MaximumBiasMagnitude = 5.0;

  m_ReferenceClassIndex = 0;

  m_XMu[0] = 0.0;
  m_XMu[1] = 0.0;
  m_XMu[2] = 0.0;

  m_XStd[0] = 1.0;
  m_XStd[1] = 1.0;
  m_XStd[2] = 1.0;

}

template <class TInputImage, class TProbabilityImage>
LLSBiasCorrector <TInputImage, TProbabilityImage>
::~LLSBiasCorrector()
{
  m_Mask = 0;

  m_Basis.set_size(0, 0);

  m_Covariances.Clear();
  m_Probabilities.Clear();
  m_InputImages.Clear();
}

template <class TInputImage, class TProbabilityImage>
void
LLSBiasCorrector <TInputImage, TProbabilityImage>
::CheckInputs()
{

  //if (m_MaxDegree == 0)
  // itkExceptionMacro(<< "Max bias degree is zero");

  if (m_InputImages.GetSize() == 0)
    itkExceptionMacro(<< "No input image specified");

  if (m_InputImages[0]->GetImageDimension() != 3)
    itkExceptionMacro(<< "Input dimension invalid: only supports 3D images");

  if (m_Probabilities.GetSize() < 1)
    itkExceptionMacro(<< "Must have one or more class probabilities");

  InputImageSizeType size =
    m_InputImages[0]->GetLargestPossibleRegion().GetSize();

  for (unsigned int i = 1; i < m_InputImages.GetSize(); i++)
  {
    InputImageSizeType size_i =
      m_InputImages[i]->GetLargestPossibleRegion().GetSize();
    if (size != size_i)
      itkExceptionMacro(<< "Image sizes do not match");
  }

  for (unsigned int i = 0; i < m_Probabilities.GetSize(); i++)
  {
    if (m_Probabilities[i]->GetImageDimension() != 3)
      itkExceptionMacro(<< "Probability [" << i << "] has invalid dimension: only supports 3D images");
    ProbabilityImageSizeType psize =
      m_Probabilities[i]->GetLargestPossibleRegion().GetSize();
    if (size[0] != psize[0] || size[1] != psize[1] || size[2] != psize[2])
      itkExceptionMacro(<< "Image data and probabilities 3D size mismatch");
  }

}


template <class TInputImage, class TProbabilityImage>
void
LLSBiasCorrector <TInputImage, TProbabilityImage>
::SetEMSMeans(double** LogMu, int numClasses, int numChannels)
{
  // Allocate
  m_Means = MatrixType(numChannels, numClasses, 0.0);

  for (int l = 0; l < numClasses; l++)
  {
    for (int n = 0; n < numChannels; n++)
    {
       m_Means(n,l) = 100.0 * LogMu[l][n];
//       m_Means(n,l) = EXPP(LogMu[l][n]);
    }
  }
}


template <class TInputImage, class TProbabilityImage>
void
LLSBiasCorrector <TInputImage, TProbabilityImage>
::SetEMSCovariances(double*** LogCovariance, int numClasses, int numChannels)
{
  for (int l = 0; l < numClasses; l++)
  {
   // Allocate
    MatrixType cov(numChannels, numChannels, 0.0);

    for (int m = 0; m < numChannels; m++)
    {
      for (int n = 0; n < numChannels; n++)
      {
        cov(n,m) = 100.0 * 100.0 * LogCovariance[l][m][n];
//        cov(n,m) = EXPP(LogCovariance[l][m][n]);
      }
    }

    m_Covariances.Append(cov);
  }
}


template <class TInputImage, class TProbabilityImage>
void
LLSBiasCorrector <TInputImage, TProbabilityImage>
::ComputeDistributions()
{

  itkDebugMacro(<< "LLSBiasCorrector: Computing means and variances...");

  unsigned int numChannels = m_InputImages.GetSize();
  unsigned int numClasses = m_Probabilities.GetSize();

  // Allocate
  m_Means = MatrixType(numChannels, numClasses, 0.0);
  m_Covariances.Clear();

  InputImageSizeType size =
    m_Probabilities[0]->GetLargestPossibleRegion().GetSize();

  InputImageIndexType ind;

  // Compute skips along each dimension
  InputImageSpacingType spacing = m_InputImages[0]->GetSpacing();
  unsigned skips[3];
  skips[0] = (unsigned int)fabs(m_SampleSpacing / spacing[0]);
  skips[1] = (unsigned int)fabs(m_SampleSpacing / spacing[1]);
  skips[2] = (unsigned int)fabs(m_SampleSpacing / spacing[2]);

  if (skips[0] == 0)
    skips[0] = 1;
  if (skips[1] == 0)
    skips[1] = 1;
  if (skips[2] == 0)
    skips[2] = 1;

  // Compute the means
  for (unsigned int iclass = 0; iclass < numClasses; iclass++)
  {

    for (unsigned int ichan = 0; ichan < numChannels; ichan++)
    {
      double mu = 0;
      double sumClassProb = DBL_EPSILON;

      for (ind[2] = 0; ind[2] < (long)size[2]; ind[2] += skips[2])
        for (ind[1] = 0; ind[1] < (long)size[1]; ind[1] += skips[1])
          for (ind[0] = 0; ind[0] < (long)size[0]; ind[0] += skips[0])
          {
            mu += 
              m_Probabilities[iclass]->GetPixel(ind)
              *
              MUL100(m_InputImages[ichan]->GetPixel(ind));
            sumClassProb += m_Probabilities[iclass]->GetPixel(ind);
          }

      if (sumClassProb < 1e-20)
        continue;

      mu /= sumClassProb;

      m_Means(ichan, iclass) = mu;
    }
  }

  // Compute the covariances
  for (unsigned int iclass = 0; iclass < numClasses; iclass++)
  {

    MatrixType cov(numChannels, numChannels, 0.0);

    for (unsigned int r = 0; r < numChannels; r++)
    {
      double mu1 = m_Means(r, iclass);

      for (unsigned int c = r; c < numChannels; c++)
      {
        double var = 0.0;

        double mu2 = m_Means(c, iclass);

        double sumClassProb = DBL_EPSILON;

        for (ind[2] = 0; ind[2] < (long)size[2]; ind[2] += skips[2])
          for (ind[1] = 0; ind[1] < (long)size[1]; ind[1] += skips[1])
            for (ind[0] = 0; ind[0] < (long)size[0]; ind[0] += skips[0])
            {
              double diff1 = MUL100(m_InputImages[r]->GetPixel(ind)) - mu1;
              double diff2 = MUL100(m_InputImages[c]->GetPixel(ind)) - mu2;
              var += m_Probabilities[iclass]->GetPixel(ind) * (diff1*diff2);
              sumClassProb += m_Probabilities[iclass]->GetPixel(ind);
            }

        if (sumClassProb < 1e-20)
          continue;

        var /= sumClassProb;

        cov(r, c) = var;
        cov(c, r) = var;

      }

    }


    for (unsigned int ichan = 0; ichan < numChannels; ichan++)
      cov(ichan, ichan) += 1e-10;

    m_Covariances.Append(cov);
  }

  itkDebugMacro(<< "Means:" << std::endl << m_Means);
  itkDebugMacro(<< "Covariances:" << std::endl)
  for (unsigned int iclass = 0; iclass < numClasses; iclass++)
    itkDebugMacro(<< m_Covariances[iclass] << std::endl);

}

template <class TInputImage, class TProbabilityImage>
void
LLSBiasCorrector <TInputImage, TProbabilityImage>
::SetMeans(const MatrixType& mu)
{
  m_Means = mu;
}

template <class TInputImage, class TProbabilityImage>
void
LLSBiasCorrector <TInputImage, TProbabilityImage>
::SetCovariances(const DynArray<MatrixType>& covs)
{
  m_Covariances = covs;
}

template <class TInputImage, class TProbabilityImage>
void
LLSBiasCorrector <TInputImage, TProbabilityImage>
::SetMaxDegree(unsigned int n)
{
  itkDebugMacro(<< "SetMaxDegree");

  m_MaxDegree = n;

  // Hack: update basis equations
  if (!m_Mask.IsNull())
    this->SetMask(m_Mask);
  if (m_Probabilities.GetSize() > 0)
    this->SetProbabilities(m_Probabilities);
}

template <class TInputImage, class TProbabilityImage>
void
LLSBiasCorrector <TInputImage, TProbabilityImage>
::SetSampleSpacing(double s)
{
  itkDebugMacro(<< "SetSampleSpacing");

  m_SampleSpacing = s;

  // Hack: update basis equations
  if (!m_Mask.IsNull())
    this->SetMask(m_Mask);
  if (m_Probabilities.GetSize() > 0)
    this->SetProbabilities(m_Probabilities);
}

template <class TInputImage, class TProbabilityImage>
void
LLSBiasCorrector <TInputImage, TProbabilityImage>
::SetMask(MaskImageType* mask)
{

  m_Mask = mask;

  InputImageSizeType size =
    m_Mask->GetLargestPossibleRegion().GetSize();

  // Image index for iterations
  InputImageIndexType ind;

  // Compute skips along each dimension
  InputImageSpacingType spacing = m_Mask->GetSpacing();
  unsigned int skips[3];
  skips[0] = (unsigned int)(m_SampleSpacing / spacing[0]);
  skips[1] = (unsigned int)(m_SampleSpacing / spacing[1]);
  skips[2] = (unsigned int)(m_SampleSpacing / spacing[2]);

  if (skips[0] == 0)
    skips[0] = 1;
  if (skips[1] == 0)
    skips[1] = 1;
  if (skips[2] == 0)
    skips[2] = 1;

  itkDebugMacro(<< "Sample skips: " << skips[0] << " x " << skips[1] << " x " << skips[2]);

  unsigned int numCoefficients =
    (m_MaxDegree+1) * (m_MaxDegree+2)/2 * (m_MaxDegree+3)/3;

  // Number of pixels with non-zero weights, downsampled
  unsigned numEquations = 0;
  for (ind[2] = 0; ind[2] < (long)size[2]; ind[2] += skips[2])
    for (ind[1] = 0; ind[1] < (long)size[1]; ind[1] += skips[1])
      for (ind[0] = 0; ind[0] < (long)size[0]; ind[0] += skips[0])
      {
        if (m_Mask->GetPixel(ind) != 0)
          numEquations++;
      }

  itkDebugMacro(<< "Linear system size = " << numEquations << " x " << numCoefficients);

  // Make sure that number of equations >= number of unknowns
  if (numEquations < numCoefficients)
    itkExceptionMacro(<< "Number of unknowns exceed number of equations");

  // Create basis matrix

  itkDebugMacro(<< "Computing polynomial basis functions...");

  m_Basis.set_size(numEquations, numCoefficients);

  // Coordinate scaling and offset parameters
  m_XMu[0] = 0.0;
  m_XMu[1] = 0.0;
  m_XMu[2] = 0.0;

  for (ind[2] = 0; ind[2] < (long)size[2]; ind[2] += skips[2])
    for (ind[1] = 0; ind[1] < (long)size[1]; ind[1] += skips[1])
      for (ind[0] = 0; ind[0] < (long)size[0]; ind[0] += skips[0])
      {
        if (m_Mask->GetPixel(ind) == 0)
          continue;

        m_XMu[0] += ind[0];
        m_XMu[1] += ind[1];
        m_XMu[2] += ind[2];
      }
  m_XMu[0] /= numEquations;
  m_XMu[1] /= numEquations;
  m_XMu[2] /= numEquations;

  m_XStd[0] = 0.0;
  m_XStd[1] = 0.0;
  m_XStd[2] = 0.0;

  for (ind[2] = 0; ind[2] < (long)size[2]; ind[2] += skips[2])
    for (ind[1] = 0; ind[1] < (long)size[1]; ind[1] += skips[1])
      for (ind[0] = 0; ind[0] < (long)size[0]; ind[0] += skips[0])
      {
        if (m_Mask->GetPixel(ind) == 0)
          continue;

        double diff;

        diff = ind[0] - m_XMu[0];
        m_XStd[0] += diff*diff;
        diff = ind[1] - m_XMu[1];
        m_XStd[1] += diff*diff;
        diff = ind[2] - m_XMu[2];
        m_XStd[2] += diff*diff;
      }

  m_XStd[0] /= numEquations;
  m_XStd[1] /= numEquations;
  m_XStd[2] /= numEquations;

  m_XStd[0] = sqrt(m_XStd[0]);
  m_XStd[1] = sqrt(m_XStd[1]);
  m_XStd[2] = sqrt(m_XStd[2]);

  // Image coordinate values
  double xc, yc, zc;

  // Row and column indices
  unsigned int r;
  unsigned int c;

  // Fill in polynomial basis values
  r = 0;

  for (ind[2] = 0; ind[2] < (long)size[2]; ind[2] += skips[2])
    for (ind[1] = 0; ind[1] < (long)size[1]; ind[1] += skips[1])
      for (ind[0] = 0; ind[0] < (long)size[0]; ind[0] += skips[0])
      {
        if (r >= numEquations)
          break;

        if (m_Mask->GetPixel(ind) == 0)
          continue;

        c = 0;
        for (unsigned int order = 0; order <= m_MaxDegree; order++)
          for (unsigned int xorder = 0; xorder <= order; xorder++)
            for (unsigned int yorder = 0; yorder <= (order-xorder); yorder++)
            {
              int zorder = order - xorder - yorder;

              xc = (ind[0] - m_XMu[0]) / m_XStd[0];
              yc = (ind[1] - m_XMu[1]) / m_XStd[1];
              zc = (ind[2] - m_XMu[2]) / m_XStd[2];

              m_Basis(r, c) =
                mypow(xc,xorder) * mypow(yc,yorder) * mypow(zc,zorder);
              c++;
            }

        r++;


      } // for 0

}

template <class TInputImage, class TProbabilityImage>
void
LLSBiasCorrector <TInputImage, TProbabilityImage>
::SetProbabilities(DynArray<ProbabilityImagePointer> probs)
{

  itkDebugMacro(<< "SetProbabilities");

  if (probs.GetSize() < 1)
    itkExceptionMacro(<<"Need one or more probabilities");

  for (unsigned int i = 0; i < probs.GetSize(); i++)
  {
    if (probs[i].IsNull())
      itkExceptionMacro(<<"One of input probabilities not initialized");
  }

  m_Probabilities = probs;

}

template <class TInputImage, class TProbabilityImage>
void
LLSBiasCorrector <TInputImage, TProbabilityImage>
::CorrectImages(
  DynArray<InputImagePointer>& inputs,
  DynArray<InputImagePointer>& outputs,
  bool fullRes)
{
  ConverterType::Pointer converter = ConverterType::New();
  converter->SetOutputMinimum(0);
  converter->SetOutputMaximum(65355);
  WriterType::Pointer writer = WriterType::New();


  if (inputs.GetSize() != outputs.GetSize())
    itkExceptionMacro(<< "Number of output images != input images");

  // For convenience
  m_InputImages = inputs;

  // Verify input
  this->CheckInputs();

  InputImageSizeType size =
    m_InputImages[0]->GetLargestPossibleRegion().GetSize();

  // Image index for iterations
  InputImageIndexType ind;

  // Compute means and variances
  if (GetComputeMean()) {
    this->ComputeDistributions();
  }
  itkDebugMacro(<< "LLSBiasCorrector: Computing means and variances already done.");
  itkDebugMacro(<< "Means:" << std::endl << m_Means);
  for (unsigned int iclass = 0; iclass < m_Probabilities.GetSize(); iclass++)
    itkDebugMacro(<< "Cov[" << iclass << "] " << m_Covariances[iclass] << std::endl);

  // Compute skips along each dimension
  InputImageSpacingType spacing = m_InputImages[0]->GetSpacing();

  unsigned int sampleofft[3];
  sampleofft[0] = (unsigned int)fabs(m_SampleSpacing / spacing[0]);
  sampleofft[1] = (unsigned int)fabs(m_SampleSpacing / spacing[1]);
  sampleofft[2] = (unsigned int)fabs(m_SampleSpacing / spacing[2]);

  if (sampleofft[0] < 1)
    sampleofft[0] = 1;
  if (sampleofft[1] < 1)
    sampleofft[1] = 1;
  if (sampleofft[2] < 1)
    sampleofft[2] = 1;

  itkDebugMacro(
     << "Sample offsets: " << sampleofft[0] << " x " << sampleofft[1] << " x " << sampleofft[2]);

  unsigned int workingofft[3];
  workingofft[0] = (unsigned int)fabs(m_WorkingSpacing / spacing[0]);
  workingofft[1] = (unsigned int)fabs(m_WorkingSpacing / spacing[1]);
  workingofft[2] = (unsigned int)fabs(m_WorkingSpacing / spacing[2]);

  if (workingofft[0] < 1)
    workingofft[0] = 1;
  if (workingofft[1] < 1)
    workingofft[1] = 1;
  if (workingofft[2] < 1)
    workingofft[2] = 1;

  itkDebugMacro(<< "Working offsets: " << workingofft[0] << " x " << workingofft[1] << " x " << workingofft[2]);

  unsigned int numChannels = inputs.GetSize();

  unsigned int numClasses = m_Probabilities.GetSize();

  unsigned int numCoefficients =
    (m_MaxDegree+1) * (m_MaxDegree+2)/2 * (m_MaxDegree+3)/3;

  itkDebugMacro(<< numClasses << " classes\n");
  itkDebugMacro(<< numCoefficients << " coefficients\n");

  itkDebugMacro(<< "Computing inverse covars...\n");
  DynArray<MatrixType> invCovars;
  for (unsigned int iclass = 0; iclass < numClasses; iclass++)
    invCovars.Append(MatrixInverseType(m_Covariances[iclass]));

  // Create matrices and vectors
  // lhs = replicated basis polynomials for each channel, weighted by inv cov
  // rhs = difference image between original and reconstructed mean image

  itkDebugMacro(<< "Creating matrices for LLS...");

  unsigned int numEquations = m_Basis.rows();

  itkDebugMacro(
    << numEquations << " equations, " << numCoefficients << " coefficients");

  // Compute  orthogonal transpose component of basis
  itkDebugMacro(<< "Computing ortho part of basis");
#if 1
  // Note: vnl_qr gives Q mxm and R mxn for A mxn
  MatrixQRType qr(m_Basis);

  // Get economy size R (square)
  MatrixType Rfull = qr.R();
  MatrixType R(numCoefficients, numCoefficients, 0);
  for (unsigned int r = 0; r < numCoefficients; r++)
    for (unsigned int c = r; c < numCoefficients; c++)
      R(r, c) = Rfull(r, c);
  Rfull.set_size(1, 1);

  // Hack to get mxn Q from vnl_qr, Q'*Q = id nxn
  MatrixType basisT = m_Basis * MatrixInverseType(R);

  basisT.inplace_transpose(); // basisT = Q'
#else
  // Do this instead for ordinary weighted LSQ
  MatrixType basisT = m_Basis.transpose();
#endif

#if LLSBIAS_USE_NORMAL_EQUATION
  MatrixType lhs(numCoefficients*numChannels, numCoefficients*numChannels);
  MatrixType rhs(numCoefficients*numChannels, 1);
#else
  MatrixType lhs(numEquations*numChannels, numCoefficients*numChannels);
  MatrixType rhs(numEquations*numChannels, 1);
#endif

  // Image coordinate values
  double xc, yc, zc;

  itkDebugMacro(<< "Fill rhs");

  rhs.fill(0.0);

  // Compute ratio between original and flat image, weighted using posterior
  // probability and inverse covariance
  for (unsigned int ichan = 0; ichan < numChannels; ichan++)
  {
    MatrixType R_i(numEquations, 1, 0.0);

    for (unsigned int jchan = 0; jchan < numChannels; jchan++)
    {
      unsigned int eq = 0;

      for (ind[2] = 0; ind[2] < (long)size[2]; ind[2] += sampleofft[2])
        for (ind[1] = 0; ind[1] < (long)size[1]; ind[1] += sampleofft[1])
          for (ind[0] = 0; ind[0] < (long)size[0]; ind[0] += sampleofft[0])
          {
  
            if (eq >= numEquations)
              break;

            if (m_Mask->GetPixel(ind) == 0)
              continue;

            // Compute reconstructed intensity, weighted by prob * invCov
            double sumW = DBL_EPSILON;
            double recon = 0;
            for (unsigned int iclass = 0; iclass < numClasses; iclass++)
            {
              MatrixType invCov = invCovars[iclass];
              double w =
                m_Probabilities[iclass]->GetPixel(ind)
                *
                invCov(ichan, jchan);
              sumW += w;
              recon += w * m_Means(jchan, iclass);
            }

            recon /= sumW;

            double bias = MUL100(m_InputImages[jchan]->GetPixel(ind)) - recon;
            R_i(eq, 0) += sumW * bias;

            eq++;

          } // for ind[0]

    } // for jchan

#if LLSBIAS_USE_NORMAL_EQUATION
    R_i = basisT * R_i;
    for (unsigned int row = 0; row < numCoefficients; row++)
      rhs(ichan*numCoefficients+row, 0) = R_i(row, 0);
#else
    for (unsigned int row = 0; row < numEquations; row++)
      rhs(ichan*numEquations+row, 0) = R_i(row, 0);
#endif

  } // for ichan

  itkDebugMacro(<< "Fill lhs");

  // Compute LHS using replicated basis entries, weighted using posterior
  // probability and inverse covariance
  for (unsigned int ichan = 0; ichan < numChannels; ichan++)
  {

    for (unsigned int jchan = 0; jchan < numChannels; jchan++)
    {
      MatrixType Wij_A(numEquations, numCoefficients, 0.0);

      unsigned int eq = 0;

      for (ind[2] = 0; ind[2] < (long)size[2]; ind[2] += sampleofft[2])
        for (ind[1] = 0; ind[1] < (long)size[1]; ind[1] += sampleofft[1])
          for (ind[0] = 0; ind[0] < (long)size[0]; ind[0] += sampleofft[0])
          {
  
            if (eq >= numEquations)
              break;

            if (m_Mask->GetPixel(ind) == 0)
              continue;

            double sumW = DBL_EPSILON;
            for (unsigned int iclass = 0; iclass < numClasses; iclass++)
            {
              MatrixType invCov = invCovars[iclass];
              double w =
                m_Probabilities[iclass]->GetPixel(ind)
                *
                invCov(ichan, jchan);
              sumW += w;
            }

            for (unsigned int col = 0; col < numCoefficients; col++)
              Wij_A(eq, col) = sumW * m_Basis(eq, col);

            eq++;

          } // for ind[0];

#if LLSBIAS_USE_NORMAL_EQUATION
          MatrixType lhs_ij = basisT * Wij_A;
          for (unsigned int row = 0; row < numCoefficients; row++)
            for (unsigned int col = 0; col < numCoefficients; col++)
              lhs(row+ichan*numCoefficients, col+jchan*numCoefficients) =
                lhs_ij(row, col);
#else
          for (unsigned int row = 0; row < numEquations; row++)
            for (unsigned int col = 0; col < numCoefficients; col++)
              lhs(row+ichan*numEquations, col+jchan*numCoefficients) =
                Wij_A(row, col);
#endif

    } // for jchan

  } // for ichan

  // Clear memory for the basis transpose
  basisT.set_size(0, 0);

  itkDebugMacro(<< "Solve " << lhs.rows() << " x " << lhs.columns());

  // Use VNL to solve linear system
  MatrixType coeffs;
  {
    MatrixQRType qr(lhs);
    coeffs = qr.solve(rhs);
    // SVD more expensive, should be more accurate
    //MatrixSVDType svd(lhs);
    //coeffs = svd.solve(rhs);
  }

  itkDebugMacro(<< "Bias field coeffs after LLS:" << std::endl  << coeffs);

  // Remove bias
  itkDebugMacro(<< "Correcting input images...");

  if (fullRes)
  {
    workingofft[0] = 1;
    workingofft[1] = 1;
    workingofft[2] = 1;
  }

  double logMax = LOGP(m_MaximumBiasMagnitude);
  double logMin = -1.0 * logMax;

  for (unsigned int ichan = 0; ichan < numChannels; ichan++)
  {
    InputImagePointer input = inputs[ichan];
    InputImagePointer output = outputs[ichan];

    // Compute the original mean intensity for ref class
    double sumP = 1e-20;
    double inputMu = 0;
    for (ind[2] = 0; ind[2] < (long)size[2]; ind[2] += workingofft[2])
      for (ind[1] = 0; ind[1] < (long)size[1]; ind[1] += workingofft[1])
        for (ind[0] = 0; ind[0] < (long)size[0]; ind[0] += workingofft[0])
        {
          double p = m_Probabilities[m_ReferenceClassIndex]->GetPixel(ind);
          //inputMu += p * input->GetPixel(ind); // TODO, this might be correct
          inputMu += p * exp(input->GetPixel(ind))-1.0;
          sumP += p;
        }
    inputMu /= sumP; 

    output->SetRegions(input->GetLargestPossibleRegion());
    output->CopyInformation(input);
    output->Allocate();
    output->FillBuffer(0);

    // Compute the log transformed bias field
    InternalImagePointer biasField = InternalImageType::New();
    biasField->CopyInformation(input);
    biasField->SetRegions(input->GetLargestPossibleRegion());
    biasField->Allocate();

    double maxBias = 0.0;
    double minBias = 0.0;

    double outputMu = 0;

    for (ind[2] = 0; ind[2] < (long)size[2]; ind[2] += workingofft[2])
      for (ind[1] = 0; ind[1] < (long)size[1]; ind[1] += workingofft[1])
        for (ind[0] = 0; ind[0] < (long)size[0]; ind[0] += workingofft[0])
        {

          double fit = 0.0;

          unsigned int c = ichan*numCoefficients;
          for (unsigned int order = 0; order <= m_MaxDegree; order++)
            for (unsigned int xorder = 0; xorder <= order; xorder++)
              for (unsigned int yorder = 0; yorder <= (order-xorder); yorder++)
              {
                int zorder = order - xorder - yorder;

                xc = (ind[0] - m_XMu[0]) / m_XStd[0];
                yc = (ind[1] - m_XMu[1]) / m_XStd[1];
                zc = (ind[2] - m_XMu[2]) / m_XStd[2];

                double poly =
                  mypow(xc,xorder) * mypow(yc,yorder) * mypow(zc,zorder);

                //fit += coeffs[c] * poly;
                fit += coeffs(c, 0) * poly;

                c++;
              }

          if (m_ClampBias)
          {
            if (fit < logMin)
              fit = logMin;
            if (fit > logMax)
              fit = logMax;
          }

          if (vnl_math_isnan(fit))
            fit = 0.0;
          if (vnl_math_isinf(fit))
            fit = 0.0;

          if (m_Mask->GetPixel(ind) != 0)
          {
            if (fit > maxBias)
              maxBias = fit;
            if (fit < minBias)
              minBias = fit;
          }

          biasField->SetPixel(ind, (InternalImagePixelType)fit);

        } // for ind[0]


    // Write biasfield
    converter->SetInput(biasField);
    converter->Update();

    std::ostringstream oss;
    oss << "/tmp/bias_" << rand() << "_" << ichan << ".nhdr" << std::ends;
    std::cout << "Write bias-ed image..." << oss.str().c_str() << std::endl;
    writer->SetFileName(oss.str().c_str());
    writer->SetInput(converter->GetOutput());
    writer->Update();


    // Correct image using (clamped) bias field)
    for (ind[2] = 0; ind[2] < (long)size[2]; ind[2] += workingofft[2])
      for (ind[1] = 0; ind[1] < (long)size[1]; ind[1] += workingofft[1])
        for (ind[0] = 0; ind[0] < (long)size[0]; ind[0] += workingofft[0])
        {
          double logb = biasField->GetPixel(ind);

          if (logb > maxBias)
            logb = maxBias;
          if (logb < minBias)
            logb = minBias;

          double logd = MUL100(m_InputImages[ichan]->GetPixel(ind)) - logb;
//          double logd = m_InputImages[ichan]->GetPixel(ind) - logb;
          double d = EXPP(logd);
          //double d = m_InputImages[ichan]->GetPixel(ind) / (logb + 1e-20);

          if (vnl_math_isnan(d))
            d = 0.0;
          if (vnl_math_isinf(d))
            d = 0.0;

          double p = m_Probabilities[m_ReferenceClassIndex]->GetPixel(ind);
          outputMu += p * d;

          output->SetPixel(ind, (InputImagePixelType)d);
//          output->SetPixel(ind, (InputImagePixelType)logd);
        } // for ind[0]

/**/
    outputMu /= sumP;

    double resRatio = inputMu / (outputMu + 1e-20);

    // Rescale so output mean for ref class stays the same
    for (ind[2] = 0; ind[2] < (long)size[2]; ind[2] += workingofft[2])
      for (ind[1] = 0; ind[1] < (long)size[1]; ind[1] += workingofft[1])
        for (ind[0] = 0; ind[0] < (long)size[0]; ind[0] += workingofft[0])
        {
          double v = output->GetPixel(ind);
          output->SetPixel(ind, v * resRatio);
        }
/**/

  } // for ichan

  // Remove internal references to input images when done
  m_InputImages.Clear();

//  writer->Delete();
//  converter->Delete();
}

#endif
