# Modules

:::{note}

This documentation is still a work in progress. Additional module documentation is available on the [Slicer wiki](https://www.slicer.org/wiki/Documentation/Nightly).

:::

**Main modules**

```{toctree}
:maxdepth: 1
data.md
dicom.md
markups.md
models.md
sceneviews.md
segmentations.md
segmenteditor.md
transforms.md
viewcontrollers.md
volumerendering.md
volumes.md
welcome.md
```

## Informatics
```{toctree}
:maxdepth: 1
colors.md
plots.md
sampledata.md
tables.md
terminologies.md
texts.md
dataprobe.md
```
%comparevolumes.md

## Registration
```{toctree}
:maxdepth: 1
acpctransform.md
brainsfit.md
brainsresample.md
brainsresize.md
fiducialregistration.md
landmarkregistration.md
performmetrictest.md
reformat.md
```

## Segmentation
```{toctree}
:maxdepth: 1
segmentations.md
segmenteditor.md
brainsroiauto.md
```

## Quantification
```{toctree}
:maxdepth: 1
markups.md
petstandarduptakevaluecomputation.md
segmentstatistics.md
```

## Sequences
```{toctree}
:maxdepth: 1
sequences.md
cropvolumesequences.md
multivolumeimporter.md
multivolumeexplorer.md
```

## Diffusion
```{toctree}
:maxdepth: 1
dmriinstall.md
dwiconvert.md
brainsdwicleanup.md
```

## Filtering
```{toctree}
:maxdepth: 1
addscalarvolumes.md
castscalarvolume.md
curvatureanisotropicdiffusion.md
gaussianblurimagefilter.md
gradientanisotropicdiffusion.md
grayscalefillholeimagefilter.md
grayscalegrindpeakimagefilter.md
maskscalarvolume.md
medianimagefilter.md
multiplyscalarvolumes.md
n4itkbiasfieldcorrection.md
checkerboardfilter.md
extractskeleton.md
histogrammatching.md
imagelabelcombine.md
simplefilters.md
subtractscalarvolumes.md
thresholdscalarvolume.md
votingbinaryholefillingimagefilter.md
```

## Utilities
```{toctree}
:maxdepth: 1
brainsdeface.md
brainsstriprotation.md
brainstransformconvert.md
dicompatcher.md
endoscopy.md
screencapture.md
```

## Surface Models
```{toctree}
:maxdepth: 1
dynamicmodeler.md
grayscalemodelmaker.md
labelmapsmoothing.md
mergemodels.md
modelmaker.md
models.md
modeltolabelmap.md
probevolumewithmodel.md
surfacetoolbox.md
```

## Converters
```{toctree}
:maxdepth: 1
createdicomseries.md
cropvolume.md
orientscalarvolume.md
vectortoscalarvolume.md
resampledtivolume.md
resamplescalarvectordwivolume.md
```

## Developer Tools

These modules are intended for module developers.

```{toctree}
:maxdepth: 1
cameras.md
eventbroker.md
executionmodeltour.md
extensionwizard.md
webserver.md
```

## Testing

These modules are for testing correctness and performance of the application.

```{toctree}
:maxdepth: 1
performancetests.md
selftests.md
```

## Legacy and retired modules

Deprecated modules are not recommended to be used anymore, typically because other modules have replaced them, and they are planned to be removed in the future.

```{toctree}
:maxdepth: 1
annotations.md
resamplescalarvolume.md
robuststatisticssegmenter.md
simpleregiongrowingsegmentation.md
```

Retired modules have been already removed from the application.

```{toctree}
:maxdepth: 1
retiredmodules.md
```
