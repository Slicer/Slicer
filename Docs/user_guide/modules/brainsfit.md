```{include} ../../_moduledescriptions/BRAINSFitOverview.md
```

```{include} ../../_moduledescriptions/BRAINSFitParameters.md
```

## Use cases

Most frequently used for these scenarios and recommended registration settings.

### Same Subject: Longitudinal

For this case we're registering a baseline T1 scan with a follow-up T1 scan on the same subject a year later.

First, set the fixed and moving volumes:

    --fixedVolume test.nii.gz \
    --movingVolume test2.nii.gz \

Next, set the output transform and volume:

    --outputVolume testT1LongRegFixed.nii.gz \
    --outputTransform longToBase.xform \

Since the input scans are of the same subject we can assume very little has changed in the last year, so we'll use a Rigid registration.

    --transformType Rigid \

:::{note}

If the registration is poor or there are reasons to expect anatomical changes (tumor growth, rapid disease progression, etc.) additional transforms may be needed.  In that case they can be added in a comma separated list, such as "Rigid,ScaleVersor3D,ScaleSkewVersor3D,Affine,BSpline". Available methods are:
- Rigid
- ScaleVersor3D
- ScaleSkewVersor3D
- Affine
- BSpline

:::

Example: multiple registration methods

    --transformType Rigid,ScaleVersor3D,ScaleSkewVersor3D,Affine,BSpline \

The scans are the same modality so we'll use --histogramMatch to match the intensity profiles as this tends to help the registration.  If there are lesions or tumors that vary between images this may not be a good idea, since it will make it harder to detect differences between the images.

    --histogramMatch \

To start with the best possible initial alignment we use --initializeTransformMode. The available transform modes are:
- useCenterOfHeadAlign
- useCenterOfROIAlign
- useMomentsAlign
- useGeometryAlign

We're working with human heads so we pick useCenterOfHeadAlign, which detects the center of head even with varying amounts of neck or shoulders present.

    --initializeTransformMode useCenterOfHeadAlign \

ROI masks normally improve registration but we haven't generated any so we turn on --maskProcessingMode ROIAUTO (other options are NOMASK and ROI).

    --maskProcessingMode ROIAUTO \

The registration generally performs better if we include some background in the mask to make the tissue boundary very clear.  The parameter that expands the mask outside the brain is ROIAutoDilateSize (under "Registration Debugging Parameters" if using the GUI).  These values are in millimeters and a good starting value is 3.

    --ROIAutoDilateSize 3 \

Lastly, we set the interpolation mode to be Linear, which is a decent tradeoff between quality and speed.  If the best possible interpolation is needed regardless of processing time, select WindowedSync instead.

    --interpolationMode Linear

The full command is:

```console
BRAINSFit --fixedVolume test.nii.gz \
    --movingVolume test2.nii.gz \
    --outputVolume testT1LongRegFixed.nii.gz \
    --outputTransform longToBase.xform \
    --transformType Rigid \
    --histogramMatch \
    --initializeTransformMode useCenterOfHeadAlign \
    --maskProcessingMode ROIAUTO \
    --ROIAutoDilateSize 3 \
    --interpolationMode Linear
```


### Same Subject: MultiModal

For this use case we're registering a T1 scan with a T2 scan collected in the same session.  The two images are again available on the [Midas site](http://midas.kitware.com/item/view/483) as test.nii.gz and standard.nii.gz

First we set the fixed and moving volumes as well as the output transform and output volume names.

    --fixedVolume test.nii.gz \
    --movingVolume standard.nii.gz \
    --outputVolume testT2RegT1.nii.gz \
    --outputTransform T2ToT1.xform \

Since these are the same subject, same session we'll use a Rigid registration.

    --transformType Rigid \

The scans are different modalities so we absolutely DO NOT want to use --histogramMatch to match the intensity profiles! This would try to map T2 intensities into T1 intensities resulting in an image that is neither, and hence useless.

To start with the best possible initial alignment we use --initializeTransformMode. We're working with human heads so we pick useCenterOfHeadAlign, which detects the center of head even with varying amounts of neck or shoulders present.

    --initializeTransformMode useCenterOfHeadAlign \

ROI masks normally improve registration but we haven't generated any so we turn on --maskProcessingMode ROIAUTO (other options are NOMASK and ROI).

    --maskProcessingMode ROIAUTO \

The registration generally performs better if we include some background in the mask to make the tissue boundary very clear.  The parameter that expands the mask outside the brain is ROIAutoDilateSize (under "Registration Debugging Parameters" if using the GUI).  These values are in millimeters and a good starting value is 3.

    --ROIAutoDilateSize 3 \

Lastly, we set the interpolation mode to be Linear, which is a decent tradeoff between quality and speed.  If the best possible interpolation is needed regardless of processing time, select WindowedSync instead.

    --interpolationMode Linear


The full command is:

```console
BRAINSFit --fixedVolume test.nii.gz \
    --movingVolume standard.nii.gz \
    --outputVolume testT2RegT1.nii.gz \
    --outputTransform T2ToT1.xform \
    --transformType Rigid \
    --initializeTransformMode useCenterOfHeadAlign \
    --maskProcessingMode ROIAUTO \
    --ROIAutoDilateSize 3 \
    --interpolationMode Linear
```

### Mouse Registration

Here we'll register brains from two different mice together.  The fixed and moving mouse brains used in this example are available on the [Midas site](http://midas.kitware.com/item/view/483) as mouseFixed.nii.gz and mouseMoving.nii.gz.

First we set the fixed and moving volumes as well as the output transform and output volume names.

    --fixedVolume mouseFixed.nii.gz \
    --movingVolume mouseMoving.nii.gz \
    --outputVolume movingRegFixed.nii.gz \
    --outputTransform movingToFixed.xform \

Since the subjects are different we are going to use transforms all the way through BSpline.

:::{note}

Building up transforms one type at a time can't hurt and might help, so we're including all transforms from Rigid through BSpline in the transformType parameter.

:::

    --transformType Rigid,ScaleVersor3D,ScaleSkewVersor3D,Affine,BSpline \

The scans are the same modality so we'll use --histogramMatch.

    --histogramMatch \

To start with the best possible initial alignment we use --initializeTransformMode but we aren't working with human heads this time, so we can't pick useCenterOfHeadAlign! Instead we pick useMomentsAlign which does a reasonable job of selecting the centers of mass.

    --initializeTransformMode useMomentsAlign \

ROI masks normally improve registration but we haven't generated any so we turn on --maskProcessingMode ROIAUTO.

    --maskProcessingMode ROIAUTO \

Since the mouse brains are much smaller than human brains there are a few advanced parameters we'll need to tweak, ROIAutoClosingSize and ROIAutoDilateSize (both under Registration Debugging Parameters if using the GUI).  These values are in millimeters and a good starting value for mice is 0.9.

    --ROIAutoClosingSize 0.9 \
    --ROIAutoDilateSize 0.9 \

Lastly, we set the interpolation mode to be Linear, which is a decent tradeoff between quality and speed.  If the best possible interpolation is needed regardless of processing time, select WindowedSync instead.

    --interpolationMode Linear

The full command is:
```console
BRAINSFit --fixedVolume mouseFixed.nii.gz \
    --movingVolume mouseMoving.nii.gz \
    --outputVolume movingRegFixed.nii.gz \
    --outputTransform movingToFixed.xform \
    --transformType Rigid,ScaleVersor3D,ScaleSkewVersor3D,Affine,BSpline \
    --histogramMatch \
    --initializeTransformMode useMomentsAlign \
    --maskProcessingMode ROIAUTO \
    --ROIAutoClosingSize 0.9 \
    --ROIAutoDilateSize 0.9 \
    --interpolationMode Linear
```

## References

- [BRAINSFit: Mutual Information Registrations of Whole-Brain 3D Images](https://hdl.handle.net/1926/1291), Using the Insight Toolkit, Johnson H.J., Harris G., Williams K., The Insight Journal, 2007.
- [Source code on github](https://github.com/BRAINSia/BRAINSTools/tree/main/BRAINSFit)
