function nrrdStrct = tensorTransformToSlicerSpace(nrrdTensor)
% transform diffusion tensor from gradient space into Slicer ijk space
% this should be consistent with the vtkNRRDReader in Slicer
% does: RASToIJKRotationMatrix * mf * tn_work *mf_trans * RASToIJKRotationMatrix_trans

t_orig = nrrdTensor.data;
mf = nrrdTensor.measurementframe;
sd = nrrdTensor.spacedirections;

rasToijk=inv(sd);

% normalize rasToijk columnwise
RASToIJKRotationMatrix = rasToijk./repmat([norm(rasToijk(:,1)), ...
        norm(rasToijk(:,2)), norm(rasToijk(:,3))], 3, 1)

RASToIJKRotationMatrix_trans = RASToIJKRotationMatrix';
mf_trans = mf';
RASToIJKRotationMatrix_times_mf =RASToIJKRotationMatrix * mf;
mftrans_times_RASToIJKRotationMatrix_trans = mf_trans * RASToIJKRotationMatrix_trans;

wb = waitbar(0, 'Transforming data...');
t_slicer = ones(size(t_orig));

% do transform backwards
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
nrrdTensor.data = t_slicer;

% now adapt measurement frame in the nrrd structure
% I believe this should be the inverse of RASToIJKRotationMatrix, but for
% now, be consistent with the nrrdreader:
nrrdTensor.measurementframe = inv(RASToIJKRotationMatrix);
nrrdStrct = nrrdTensor;
return

