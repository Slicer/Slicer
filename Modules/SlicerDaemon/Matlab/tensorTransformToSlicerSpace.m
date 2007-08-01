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



%disp('1. original tensor data: ')
%nrrdTensor.data(1:7)
%fprintf('%.15g %.15g %.15g\n', nrrdTensor.data(2),nrrdTensor.data(3), nrrdTensor.data(4) );
%fprintf('%.15g %.15g %.15g\n', nrrdTensor.data(3),nrrdTensor.data(5), nrrdTensor.data(6) );
%fprintf('%.15g %.15g %.15g\n', nrrdTensor.data(4),nrrdTensor.data(6), nrrdTensor.data(7) );

t_orig = double(nrrdTensor.data);
mf = double(nrrdTensor.measurementframe)
sd = double(nrrdTensor.spacedirections);
disp('IJKTo RAS Matrix: ')
sd

rasToijk=(inv(sd))

% normalize rasToijk columnwise
RASToIJKRotationMatrix = rasToijk./repmat([norm(rasToijk(:,1)), ...
        norm(rasToijk(:,2)), norm(rasToijk(:,3))], 3, 1)

RASToIJKRotationMatrix_trans = RASToIJKRotationMatrix'
mf_trans = mf'
RASToIJKRotationMatrix_times_mf =RASToIJKRotationMatrix * mf;
mftrans_times_RASToIJKRotationMatrix_trans = mf_trans * RASToIJKRotationMatrix_trans;

wb = waitbar(0, 'Transforming data...');
t_slicer = ones(size(t_orig));

% do transform backwards
for j=1:nrrdTensor.sizes(2)
    waitbar(j/nrrdTensor.sizes(2));
    for k=1:nrrdTensor.sizes(3)
        for l=1:nrrdTensor.sizes(4)
           
            if (j==1 & k ==1 & l == 1)
                current_t_orig = squeeze(t_orig(2:end, j, k, l));
                % blow it up to be a 3x3 matrix
                t_orig_work = [current_t_orig(1:3)'; current_t_orig(2)', ...
                    current_t_orig(4:5)'; current_t_orig(3)', current_t_orig(5:6)'];
                mf_times_t_orig_work = mf * t_orig_work;
                %disp('MF * T_orig: ')
                %fprintf('%.15g %.15g %.15g\n',mf_times_t_orig_work')
                %disp('MF * T_orig: ')
                %mf_times_t_orig_work
                mf_times_t_orig_times_mft =mf_times_t_orig_work * mf_trans;
                %disp('\nMF * T_orig*MFT: ')
                %fprintf('%.15g %.15g %.15g\n',mf_times_t_orig_times_mft')
                sd_times_mf_times_t_orig_times_mft =  RASToIJKRotationMatrix * ...
                        mf_times_t_orig_times_mft;
                %disp('\nsd*MF * T_orig*MFT: ') 
                %fprintf('%.15g %.15g %.15g\n',sd_times_mf_times_t_orig_times_mft')
                sd_times_mf_times_t_orig_times_mft_times_sdt = sd_times_mf_times_t_orig_times_mft * ...
                    RASToIJKRotationMatrix_trans;
                %disp('\nsd*MF * T_orig*MFT*sdt: ') 
                %fprintf('%.15g %.15g %.15g\n',sd_times_mf_times_t_orig_times_mft_times_sdt')
                t_slicer_work =sd_times_mf_times_t_orig_times_mft_times_sdt ;
                %t_slicer_work = RASToIJKRotationMatrix *mf_times_t_orig_work * ...
                 %   mf_trans * RASToIJKRotationMatrix_trans;
                t_slicer(2:end, j, k, l) = [t_slicer_work(1, 1:3), ...
                    t_slicer_work(2, 2:3), t_slicer_work(3,3)]';
            end

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

%disp('1. transformed tensor data:')
%fprintf('%.15g %.15g %.15g\n', t_slicer(2),t_slicer(3), t_slicer(4) );
%fprintf('%.15g %.15g %.15g\n', t_slicer(3),t_slicer(5), t_slicer(6) );
%fprintf('%.15g %.15g %.15g\n', t_slicer(4),t_slicer(6), t_slicer(7) );
%t_slicer(1:7)


% now adapt measurement frame in the nrrd structure

nrrdTensor.measurementframe = inv(RASToIJKRotationMatrix);
nrrdStrct = nrrdTensor;
return

