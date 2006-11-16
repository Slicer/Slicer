/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: ConfidenceConnected.cxx,v $
  Language:  C++
  Date:      $Date: 2005/11/19 16:31:50 $
  Version:   $Revision: 1.36 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif



//  Software Guide : BeginCommandLineArgs
//  INPUTS: {BrainProtonDensitySlice.png}
//  OUTPUTS: {ConfidenceConnectedOutput1.png}
//  60 116
//  Software Guide : EndCommandLineArgs
//  Software Guide : BeginCommandLineArgs
//  INPUTS: {BrainProtonDensitySlice.png}
//  OUTPUTS: {ConfidenceConnectedOutput2.png}
//  81 112
//  Software Guide : EndCommandLineArgs
//  Software Guide : BeginCommandLineArgs
//  INPUTS: {BrainProtonDensitySlice.png}
//  OUTPUTS: {ConfidenceConnectedOutput3.png}
//  107 69
//  Software Guide : EndCommandLineArgs

// Software Guide : BeginLatex
//
// \index{itk::FloodFillIterator!In Region Growing}
// \index{itk::ConfidenceConnectedImageFilter}
// \index{itk::ConfidenceConnectedImageFilter!header}
//
// The following example illustrates the use of the
// \doxygen{ConfidenceConnectedImageFilter}. The criterion used by the
// ConfidenceConnectedImageFilter is based on simple statistics of the
// current region. First, the algorithm computes the mean and standard
// deviation of intensity values for all the pixels currently included in the
// region. A user-provided factor is used to multiply the standard deviation
// and define a range around the mean. Neighbor pixels whose intensity values
// fall inside the range are accepted and included in the region. When no
// more neighbor pixels are found that satisfy the criterion, the algorithm
// is considered to have finished its first iteration. At that point, the
// mean and standard deviation of the intensity levels are recomputed using
// all the pixels currently included in the region. This mean and standard
// deviation defines a new intensity range that is used to visit current
// region neighbors and evaluate whether their intensity falls inside the
// range.  This iterative process is repeated until no more pixels are added
// or the maximum number of iterations is reached. The following equation
// illustrates the inclusion criterion used by this filter,
//
// \begin{equation}
// I(\mathbf{X}) \in [ m - f \sigma , m + f \sigma ]
// \end{equation}
//
// where $m$ and $\sigma$ are the mean and standard deviation of the  region
// intensities, $f$ is a factor defined by the user, $I()$ is the image and
// $\mathbf{X}$ is the position of the particular neighbor pixel being
// considered for inclusion in the region.
//
// Let's look at the minimal code required to use this algorithm. First, the
// following header defining the \doxygen{ConfidenceConnectedImageFilter} class
// must be included.
//
// Software Guide : EndLatex 


// Software Guide : BeginCodeSnippet
#include "itkConfidenceConnectedImageFilter.h"
// Software Guide : EndCodeSnippet


#include "itkImage.h"
#include "itkCastImageFilter.h"


//  Software Guide : BeginLatex
//
//  Noise present in the image can reduce the capacity of this filter to grow
//  large regions. When faced with noisy images, it is usually convenient to
//  pre-process the image by using an edge-preserving smoothing filter. Any of
//  the filters discussed in Section~\ref{sec:EdgePreservingSmoothingFilters}
//  can be used to this end. In this particular example we use the
//  \doxygen{CurvatureFlowImageFilter}, hence we need to include its header
//  file.
//
//  Software Guide : EndLatex 

// Software Guide : BeginCodeSnippet
#include "itkCurvatureFlowImageFilter.h"
// Software Guide : EndCodeSnippet


#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkPluginFilterWatcher.h"

#include "ConfidenceConnectedCLP.h"

int main( int argc, char *argv[] )
{
  PARSE_ARGS;

  //  Software Guide : BeginLatex
  //  
  //  We now define the image type using a pixel type and a particular
  //  dimension. In this case the \code{float} type is used for the pixels due
  //  to the requirements of the smoothing filter. 
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  typedef   float           InternalPixelType;
  const     unsigned int    Dimension = 3;
  typedef itk::Image< InternalPixelType, Dimension >  InternalImageType;
  // Software Guide : EndCodeSnippet

  typedef unsigned char OutputPixelType;
  typedef itk::Image< OutputPixelType, Dimension > OutputImageType;

  typedef itk::CastImageFilter< InternalImageType, OutputImageType >
    CastingFilterType;
  CastingFilterType::Pointer caster = CastingFilterType::New();
                        

  // We instantiate reader and writer types
  //
  typedef  itk::ImageFileReader< InternalImageType > ReaderType;
  typedef  itk::ImageFileWriter<  OutputImageType  > WriterType;

  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  reader->SetFileName( inputVolume.c_str() );
  writer->SetFileName( outputVolume.c_str() );


  //  Software Guide : BeginLatex
  //  
  //  The smoothing filter type is instantiated using the image type as
  //  a template parameter.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  typedef itk::CurvatureFlowImageFilter< InternalImageType, InternalImageType >
    CurvatureFlowImageFilterType;
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  //  
  //  Next the filter is created by invoking the \code{New()} method and
  //  assigning the result to a \doxygen{SmartPointer}.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  CurvatureFlowImageFilterType::Pointer smoothing = 
                         CurvatureFlowImageFilterType::New();
  itk::PluginFilterWatcher watcher1(smoothing, "Curvature flow",
                                    CLPProcessInformation, 0.5, 0.0);
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  //  
  //  We now declare the type of the region growing filter. In this case it is
  //  the ConfidenceConnectedImageFilter. 
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  typedef itk::ConfidenceConnectedImageFilter<InternalImageType, InternalImageType> 
    ConnectedFilterType;
  // Software Guide : EndCodeSnippet

  //  Software Guide : BeginLatex
  //  
  //  Then, we construct one filter of this class using the \code{New()}
  //  method.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  ConnectedFilterType::Pointer confidenceConnected = ConnectedFilterType::New();
  itk::PluginFilterWatcher watcher2(confidenceConnected, "Segmenting",
                                    CLPProcessInformation, 0.5, 0.5);
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  //  
  //  Now it is time to create a simple, linear pipeline. A file reader is
  //  added at the beginning of the pipeline and a cast filter and writer are
  //  added at the end. The cast filter is required here to convert
  //  \code{float} pixel types to integer types since only a few image file
  //  formats support \code{float} types.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  smoothing->SetInput( reader->GetOutput() );
  confidenceConnected->SetInput( smoothing->GetOutput() );
  caster->SetInput( confidenceConnected->GetOutput() );
  writer->SetInput( caster->GetOutput() );
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  //
  //  The CurvatureFlowImageFilter requires defining two parameters.  The
  //  following are typical values for $2D$ images. However they may have to
  //  be adjusted depending on the amount of noise present in the input
  //  image.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  smoothing->SetNumberOfIterations( smoothingIterations );
  smoothing->SetTimeStep( timestep );
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  //
  //  The ConfidenceConnectedImageFilter requires defining two parameters.
  //  First, the factor $f$ that the defines how large the range of
  //  intensities will be. Small values of the multiplier will restrict the
  //  inclusion of pixels to those having very similar intensities to those
  //  in the current region. Larger values of the multiplier will relax the
  //  accepting condition and will result in more generous growth of the
  //  region. Values that are too large will cause the region to grow into
  //  neighboring regions that may actually belong to separate anatomical
  //  structures.
  //
  //  \index{itk::ConfidenceConnectedImageFilter!SetMultiplier()}
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  confidenceConnected->SetMultiplier( multiplier );
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  //
  //  The number of iterations is specified based on the homogeneity of the
  //  intensities of the anatomical structure to be segmented. Highly
  //  homogeneous regions may only require a couple of iterations. Regions
  //  with ramp effects, like MRI images with inhomogeneous fields, may
  //  require more iterations. In practice, it seems to be more important to
  //  carefully select the multiplier factor than the number of iterations.
  //  However, keep in mind that there is no reason to assume that this
  //  algorithm should converge to a stable region. It is possible that by
  //  letting the algorithm run for more iterations the region will end up
  //  engulfing the entire image.
  //
  //  \index{itk::ConfidenceConnectedImageFilter!SetNumberOfIterations()}
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  confidenceConnected->SetNumberOfIterations( 5 );
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  //
  //  The output of this filter is a binary image with zero-value pixels
  //  everywhere except on the extracted region. The intensity value to be
  //  set inside the region is selected with the method
  //  \code{SetReplaceValue()}
  //
  //  \index{itk::ConfidenceConnectedImageFilter!SetReplaceValue()}
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  confidenceConnected->SetReplaceValue( 255 );
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  //
  //  The initialization of the algorithm requires the user to provide a seed
  //  point. It is convenient to select this point to be placed in a
  //  \emph{typical} region of the anatomical structure to be segmented. A
  //  small neighborhood around the seed point will be used to compute the
  //  initial mean and standard deviation for the inclusion criterion. The
  //  seed is passed in the form of a \doxygen{Index} to the \code{SetSeed()}
  //  method.
  //
  //  \index{itk::ConfidenceConnectedImageFilter!SetSeed()}
  //  \index{itk::ConfidenceConnectedImageFilter!SetInitialNeighborhoodRadius()}
  //
  //  Software Guide : EndLatex 

  InternalImageType::IndexType  index;
  
  for (int i=0; i<seed.size(); ++i)
    {
    index[0] = seed[i][0];
    index[1] = seed[i][1];
    index[2] = seed[i][2];

    confidenceConnected->AddSeed(index);
    }

  
  //  Software Guide : BeginLatex
  //  
  //  The size of the initial neighborhood around the seed is defined with the
  //  method \code{SetInitialNeighborhoodRadius()}. The neighborhood will be
  //  defined as an $N$-dimensional rectangular region with $2r+1$ pixels on
  //  the side, where $r$ is the value passed as initial neighborhood radius. 
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  confidenceConnected->SetInitialNeighborhoodRadius( 2 );
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  //  
  //  The invocation of the \code{Update()} method on the writer triggers the
  //  execution of the pipeline.  It is recommended to place update calls in a
  //  \code{try/catch} block in case errors occur and exceptions are thrown.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  try
    {
    writer->Update();
    }
  catch( itk::ExceptionObject & excep )
    {
    std::cerr << "Exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    }
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  //
  //  Let's now run this example using as input the image
  //  \code{BrainProtonDensitySlice.png} provided in the directory
  //  \code{Examples/Data}. We can easily segment the major anatomical
  //  structures by providing seeds in the appropriate locations. For example
  //
  //  \begin{center}
  //  \begin{tabular}{|l|c|c|}
  //  \hline
  //  Structure & Seed Index & Output Image \\ \hline
  //  White matter & $(60,116)$ & Second from left in Figure \ref{fig:ConfidenceConnectedOutput} \\ \hline
  //  Ventricle    & $(81,112)$ & Third  from left in Figure \ref{fig:ConfidenceConnectedOutput} \\ \hline
  //  Gray matter  & $(107,69)$ & Fourth from left in Figure \ref{fig:ConfidenceConnectedOutput} \\ \hline
  //  \end{tabular}
  //  \end{center}
  //
  // \begin{figure} \center
  // \includegraphics[width=0.24\textwidth]{BrainProtonDensitySlice.eps}
  // \includegraphics[width=0.24\textwidth]{ConfidenceConnectedOutput1.eps}
  // \includegraphics[width=0.24\textwidth]{ConfidenceConnectedOutput2.eps}
  // \includegraphics[width=0.24\textwidth]{ConfidenceConnectedOutput3.eps}
  // \itkcaption[ConfidenceConnected segmentation results]{Segmentation results
  // for the ConfidenceConnected filter for various seed points.}
  // \label{fig:ConfidenceConnectedOutput}
  // \end{figure}
  //
  //  Note that the gray matter is not being completely segmented.  This
  //  illustrates the vulnerability of the region growing methods when the
  //  anatomical structures to be segmented do not have a homogeneous
  //  statistical distribution over the image space. You may want to
  //  experiment with different numbers of iterations to verify how the
  //  accepted region will extend.
  //
  //  Software Guide : EndLatex 


  return 0;
}




