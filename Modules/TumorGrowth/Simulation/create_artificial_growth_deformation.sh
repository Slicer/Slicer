#!/bin/bash
# USAGE example: ./create_artificial_growth_deformation.sh original.nhdr copy-deformation_vectors_base.mha 10 final_images
# This will create final_images_#.nhdr images for each iteration. The deformation field is applied successively at each iteration.
# You can also apply the deformation field to the manual_segmentation_roi.mha file to create the deformed segmentations and have the ground truth. For this apply the copy-deformation_vectors_base_roi.mha deformation field since it has the same size as the manual segmentation.

if [ $# -ne 4 ]
then 
    echo "Usage - $0 <initial-image> <deformation-field> <num-of-iterations> <final-image-base(name_days.mha)>"
    exit 1
fi

argv=($@)

initial_image=${argv[0]}
final_image=${argv[3]}
echo "Initial image: $initial_image"
echo "Deformation field: ${argv[1]}"
echo "Number of iterations: ${argv[2]}"

for(( k=1; k<=${argv[2]}; k++ ))
do
  ../../../lib/Slicer3/Plugins/applyDeformationITK $initial_image ${argv[1]} ${final_image}_${k}.nhdr 1 1 
  initial_image=${final_image}_${k}.nhdr
  echo "$initial_image done."
done
