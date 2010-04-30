#
# Example run for BRAINSFit
echo Registering test2.nii.gz to test.nii.gz.
echo Using Affine transform type
echo Output image registered.nii.gz
BRAINSFit --FixedImage test.nii.gz --MovingImage test2.nii.gz --OutputImage registered.nii.gz --FitTransformType Affine
