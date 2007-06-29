function nrrdStrct = tensorTransformToGradientSpace(slicerStrct, measurementFrame_orig)
% transform diffusion tensor from gradient space into Slicer ijk space
% this should be consistent with the vtkNRRDReader in Slicer
% does: RASToIJKRotationMatrix * mf * tn_work *mf_trans * RASToIJKRotationMatrix_trans

t_slicer = slicerStrct.data;
mf = measurementFrame_orig;
sd = slicerStrct.spacedirections;

rasToijk=inv(sd);

% normalize rasToijk columnwise
RASToIJKRotationMatrix = rasToijk./repmat([norm(rasToijk(:,1)), ...
        norm(rasToijk(:,2)), norm(rasToijk(:,3))], 3, 1)

RASToIJKRotationMatrix_trans = RASToIJKRotationMatrix';
mf_trans = mf';

RASToIJKRotationMatrix_inv = inv(RASToIJKRotationMatrix);
mf_inv = inv(mf);
RASToIJKRotationMatrix_trans_inv = inv(RASToIJKRotationMatrix');
mf_trans_inv = inv(mf');

mf_inv_times_RASToIJKRotationMatrix_inv = mf_inv * RASToIJKRotationMatrix_inv;
RASToIJKRotationMatrix_trans_inv_times_mf_trans_inv = RASToIJKRotationMatrix_trans_inv * mf_trans_inv;

wb = waitbar(0, 'Transforming data...');
t_orig = ones(size(t_slicer));

% do transform backwards
for j=1:slicerStrct.sizes(2)
    waitbar(j/slicerStrct.sizes(2));
    for k=1:slicerStrct.sizes(3)
        for l=1:slicerStrct.sizes(4)
            current_t_slicer = squeeze(t_slicer(2:end, j, k, l));
            % blow it up to be a 3x3 matrix
            t_slicer_work = [current_t_slicer(1:3)'; current_t_slicer(2)', ...
                current_t_slicer(4:5)'; current_t_slicer(3)', current_t_slicer(5:6)'];
            t_orig_work =  mf_inv_times_RASToIJKRotationMatrix_inv * t_slicer_work * ...
                RASToIJKRotationMatrix_trans_inv_times_mf_trans_inv;
            t_orig(2:end, j, k, l) = [t_orig_work(1, 1:3), ...
                t_orig_work(2, 2:3), t_orig_work(3,3)]'; 
        end
    end
end
    
close(wb)
slicerStrct.data = t_orig;

% now adapt measurement frame in the nrrd structure
% I believe this should be the inverse of RASToIJKRotationMatrix, but for
% now, be consistent with the nrrdreader:
slicerStrct.measurementframe = measurementFrame_orig;
nrrdStrct = slicerStrct;
return

