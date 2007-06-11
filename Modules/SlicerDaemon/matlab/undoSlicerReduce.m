function nrrdStrct = undoSlicerReduce(header)
% do: mf_inv * rastovtk_inv * ts2_work * rastovtk_T_inv * mf_T_inv

ts = header.data;
mf = header.measurementframe;
d = header.spacedirections;
rasToijk=inv(d);

c=[1 0 0; 0 -1 0; 0 0 1];
cr = c*rasToijk;
rastovtk = cr./repmat([norm(cr(1,:)); norm(cr(2,:)); norm(cr(3,:))], 1, 3);

rastovtktrans = rastovtk';
mftrans = mf';

mf_inv = inv(mf);
rastovtk_inv = inv(rastovtk);
mf_T_inv = inv(mf');
rastovtk_T_inv = inv(rastovtk');

mf_inv_times_rastovtk_inv = mf_inv * rastovtk_inv;
rastovtk_T_inv_times_mf_T_inv = rastovtk_T_inv * mf_T_inv;

wb = waitbar(0, 'Transforming data...');
tn_again = ones(size(ts));

% do transform backwards
for j=1:header.sizes(2)
    waitbar(j/header.sizes(2));
    for k=1:header.sizes(3)
        for l=1:header.sizes(4)
            current_ts = squeeze(ts(2:end, j, k, l));
            ts_work = [current_ts(1:3)'; current_ts(2)', current_ts(4:5)'; current_ts(3)', current_ts(5:6)'];
            %tn_again_work = mf_inv * rastovtk_inv * ts_work * rastovtk_T_inv * mf_T_inv ;
            tn_again_work = mf_inv_times_rastovtk_inv * ts_work * rastovtk_T_inv_times_mf_T_inv;
            tn_again(2:end, j, k, l) = [tn_again_work(1, 1:3), tn_again_work(2, 2:3), tn_again_work(3,3)]';
           
        end
    end
end
    
close(wb)
header.data = tn_again;
nrrdStrct = header;
return
