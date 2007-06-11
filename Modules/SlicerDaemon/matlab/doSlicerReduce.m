function nrrdStrct = doSlicerReduce(header)
%transform diffusion tensor from gradient space into slicer ijk space
% do: rastovtk * mf * tn_work *mftrans * rastovtktrans

tn = header.data;
mf = header.measurementframe;
d = header.spacedirections;
rasToijk=inv(d);

c=[1 0 0; 0 -1 0; 0 0 1];
cr = c*rasToijk;
rastovtk = cr./repmat([norm(cr(1,:)); norm(cr(2,:)); norm(cr(3,:))], 1, 3);

rastovtktrans = rastovtk';
mftrans = mf';
rastovtk_times_mf = rastovtk * mf;
mftrans_times_rastovtktrans = mftrans * rastovtktrans;

wb = waitbar(0, 'Transforming data...');
ts = ones(size(tn));

% do transform backwards
for j=1:header.sizes(2)
    waitbar(j/header.sizes(2));
    for k=1:header.sizes(3)
        for l=1:header.sizes(4)
            current_tn = squeeze(tn(2:end, j, k, l));
            tn_work = [current_tn(1:3)'; current_tn(2)', current_tn(4:5)'; current_tn(3)', current_tn(5:6)'];
            ts_work = rastovtk_times_mf * tn_work * mftrans_times_rastovtktrans;
            ts(2:end, j, k, l) = [ts_work(1, 1:3), ts_work(2, 2:3), ts_work(3,3)]'; 
        end
    end
end
    
close(wb)
header.data = ts;
nrrdStrct = header;
return

