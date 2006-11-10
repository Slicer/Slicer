/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

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
//    INPUTS: {BrainProtonDensitySlice.png}
//    OUTPUTS: {MedianImageFilterOutput.png}
//  Software Guide : EndCommandLineArgs

//  Software Guide : BeginLatex
//
//  The \doxygen{MedianImageFilter} is commonly used as a robust approach for
//  noise reduction. This filter is particularly efficient against
//  \emph{salt-and-pepper} noise. In other words, it is robust to the presence
//  of gray-level outliers. MedianImageFilter computes the value of each output
//  pixel as the statistical median of the neighborhood of values around the
//  corresponding input pixel. The following figure illustrates the local
//  effect of this filter in a $2D$ case. The statistical median of the
//  neighborhood on the left is passed as the output value associated with the
//  pixel at the center of the neighborhood.
//
//
//  \begin{center}
//  \begin{picture}(200,46)
//  \put(   5.0,  0.0 ){\framebox(30.0,15.0){25}} 
//  \put(  35.0,  0.0 ){\framebox(30.0,15.0){30}} 
//  \put(  65.0,  0.0 ){\framebox(30.0,15.0){32}} 
//  \put(   5.0, 15.0 ){\framebox(30.0,15.0){27}} 
//  \put(  35.0, 15.0 ){\framebox(30.0,15.0){25}} 
//  \put(  65.0, 15.0 ){\framebox(30.0,15.0){29}} 
//  \put(   5.0, 30.0 ){\framebox(30.0,15.0){28}} 
//  \put(  35.0, 30.0 ){\framebox(30.0,15.0){26}} 
//  \put(  65.0, 30.0 ){\framebox(30.0,15.0){50}} 
//  \put( 100.0, 22.0 ){\vector(1,0){20.0}}
//  \put( 125.0, 15.0 ){\framebox(30.0,15.0){28}} 
//  \end{picture}
//  \end{center}
//
//
//  This filter will work on images of any dimension thanks to the internal
//  use of \doxygen{NeighborhoodIterator} and
//  \doxygen{NeighborhoodOperator}. The size of the neighborhood over which
//  the median is computed can be set by the user.
//
//  \index{itk::MedianImageFilter}
//
//  Software Guide : EndLatex 


#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkPluginFilterWatcher.h"


//  Software Guide : BeginLatex
//
//  The header file corresponding to this filter should be included first.
//
//  \index{itk::MedianImageFilter!header}
//
//  Software Guide : EndLatex 


// Software Guide : BeginCodeSnippet
#include "itkMedianImageFilter.h"
// Software Guide : EndCodeSnippet

#include "MedianImageFilterCLP.h"

int main( int argc, char * argv[] )
{
  //  Software Guide : BeginLatex
  //
  //  Then the pixel and image types of the input and output must be defined.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  typedef   short  InputPixelType;
  typedef   short  OutputPixelType;

  typedef itk::Image< InputPixelType,  3 >   InputImageType;
  typedef itk::Image< OutputPixelType, 3 >   OutputImageType;
  // Software Guide : EndCodeSnippet


  typedef itk::ImageFileReader< InputImageType  >  ReaderType;
  typedef itk::ImageFileWriter< OutputImageType >  WriterType;

  PARSE_ARGS;

  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  reader->SetFileName( inputVolume.c_str() );
  writer->SetFileName( outputVolume.c_str() );

  //  Software Guide : BeginLatex
  //
  //  Using the image types, it is now possible to define the filter type
  //  and create the filter object.
  //
  //  \index{itk::MedianImageFilter!instantiation}
  //  \index{itk::MedianImageFilter!New()}
  //  \index{itk::MedianImageFilter!Pointer}
  // 
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  typedef itk::MedianImageFilter<
               InputImageType, OutputImageType >  FilterType;

  FilterType::Pointer filter = FilterType::New();
  itk::PluginFilterWatcher watcher(filter, "Median Image Filter",
    CLPProcessInformation);
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  //
  //  The size of the neighborhood is defined along every dimension by
  //  passing a \code{SizeType} object with the corresponding values. The
  //  value on each dimension is used as the semi-size of a rectangular
  //  box. For example, in $3D$ a size of \(1,2,1\) will result in a 3 times
  //  5 time 3 neighborhood.
  //
  //  \index{itk::MedianImageFilter!Radius}
  //  \index{itk::MedianImageFilter!Neighborhood}
  //
  //  Software Guide : EndLatex 


  // Software Guide : BeginCodeSnippet
  InputImageType::SizeType indexRadius;
  
  indexRadius[0] = neighborhood[0]; // radius along x
  indexRadius[1] = neighborhood[1]; // radius along y
  indexRadius[2] = neighborhood[2]; // radius along slice

  filter->SetRadius( indexRadius );
  // Software Guide : EndCodeSnippet

  //  Software Guide : BeginLatex
  //
  //  The input to the filter can be taken from any other filter, for example
  //  a reader. The output can be passed down the pipeline to other filters,
  //  for example, a writer. An update call on any downstream filter will
  //  trigger the execution of the median filter.
  //
  //  \index{itk::MedianImageFilter!SetInput()}
  //  \index{itk::MedianImageFilter!GetOutput()}
  //
  //  Software Guide : EndLatex 


  // Software Guide : BeginCodeSnippet
  filter->SetInput( reader->GetOutput() );
  writer->SetInput( filter->GetOutput() );
  writer->Update();
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  // 
  // \begin{figure}
  // \center
  // \includegraphics[width=0.44\textwidth]{BrainProtonDensitySlice.eps}
  // \includegraphics[width=0.44\textwidth]{MedianImageFilterOutput.eps}
  // \itkcaption[Effect of the Median filter.]{Effect of the MedianImageFilter on a
  // slice from a MRI proton density brain image.}
  // \label{fig:MedianImageFilterOutput}
  // \end{figure}
  //
  //  Figure \ref{fig:MedianImageFilterOutput} illustrates the effect of the MedianImageFilter
  //  filter on a slice of MRI brain image using a neighborhood radius of
  //  \(1,1\), which corresponds to a $ 3 \times 3 $ classical neighborhood.
  //  The filtered image demonstrates the moderate tendency of the median
  //  filter to preserve edges.
  //
  //  Software Guide : EndLatex 


  return EXIT_SUCCESS;
}

