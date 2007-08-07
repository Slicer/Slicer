function nrrdStrct = tensorTransformToSlicerSpace(nrrdTensor)
% Transform diffusion tensor from gradient space into Slicer ijk space.
% This function relies on the fact, that the input diffusion data is in
% gradient space. Otherwise the transformation result won't be in Slicer
% space.
% This should be quite consistent with the vtkNRRDReader in Slicer, except
% for the fact that in Matlab all computations are done with double
% precision, in the vtkNrrdReader at the moment it's done in part in float
% precision.
% This is the transsformation that is applied: RASToIJKRotationMatrix * mf
% * tn_work *mf_trans * RASToIJKRotationMatrix_trans
% The measurement frame of the nrrd structure will be adapted.

t_orig = double(nrrdTensor.data);
mf = double(nrrdTensor.measurementframe);
sd = double(nrrdTensor.spacedirections);

rasToijk=(inv(sd));

% normalize rasToijk columnwise
RASToIJKRotationMatrix = rasToijk./repmat([norm(rasToijk(:,1)), ...
        norm(rasToijk(:,2)), norm(rasToijk(:,3))], 3, 1);

RASToIJKRotationMatrix_trans = RASToIJKRotationMatrix';
mf_trans = mf';
RASToIJKRotationMatrix_times_mf =RASToIJKRotationMatrix * mf;
mftrans_times_RASToIJKRotationMatrix_trans = mf_trans * RASToIJKRotationMatrix_trans;

wb = waitbar(0, 'Transforming data...');
t_slicer = ones(size(t_orig));

for j=1:nrrdTensor.sizes(2)
    waitbar(j/nrrdTensor.sizes(2));
    for k=1:nrrdTensor.sizes(3)
        for l=1:nrrdTensor.sizes(4)
            current_t_orig = squeeze(t_orig(2:end, j, k, l));
            % blow it up to be a 3x3 matrix
            t_orig_work = [current_t_orig(1:3)'; current_t_orig(2)', ...
                current_t_orig(4:5)'; current_t_orig(3)', current_t_orig(5:6)'];
            t_slicer_work = RASToIJKRotationMatrix_times_mf * t_orig_work * ...
                mftrans_times_RASToIJKRotationMatrix_trans;
            t_slicer(2:end, j, k, l) = [t_slicer_work(1, 1:3), ...
                t_slicer_work(2, 2:3), t_slicer_work(3,3)]'; 
        end
    end
end
    
close(wb)
nrrdTensor.data = single(t_slicer);

% now adapt measurement frame in the nrrd structure

nrrdTensor.measurementframe = inv(RASToIJKRotationMatrix);
nrrdStrct = nrrdTensor;
return

