function tensor_result = transformationTestLoop(tensor)
%Expects a tensor nrrd structure in gradient space, for example the result
%of loadNrrdStructure. 
%transforms tensor data first from gradient space into ijk space using
%tensorTransformToSlicerSpace. Then the result is transformed abck into
%gradient space using the original measurement frame.
%The resulting tensor is compared element by element with the originally
%loaded tensor from file.

disp('Transform tensor data from gradient space into IJK space ...')
tensor_ijk = tensorTransformToSlicerSpace(tensor);

disp('Transform tensor data from IJK space back to original gradient space ...')
tensor_back_to_gradient_space = tensorTransformToGradientSpace(tensor_ijk, tensor.measurementframe);

format long g

fprintf('Overall difference:\n');

tn_total=tensor.data(2:7,:,:,:);
ts_total=tensor_back_to_gradient_space.data(2:7,:,:,:);

true_x = tn_total(tn_total ~=0 );
slicer_x = ts_total(tn_total ~=0 );

err_x = abs( slicer_x - true_x );
per_x = err_x./(abs(true_x));  

fprintf('Absolute:\n');
fprintf('max(err_x): %.15g\n', max(err_x));
fprintf('min(err_x): %g\n', min(err_x));
fprintf('mean(err_x): %g\n', mean(err_x));
fprintf('Relative difference:\n');
fprintf('max(per_x): %g\n', max(per_x));
fprintf('min(per_x): %g\n', min(per_x));
fprintf('mean(per_x): %g\n\n', mean(per_x));

tensor_result = tensor_back_to_gradient_space;
return