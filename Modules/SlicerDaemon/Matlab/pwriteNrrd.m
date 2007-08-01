
function pwriteNrrd( p, header ,transformDT)

% write a nrrd image to a pipe opened by popenw

header

popenw(p,double(['NRRD0001', 10]),'char');

popenw(p,double(['content: from pwriteNrrd', 10]),'char');

popenw(p,double('type: '),'char');
popenw(p,double([header.type, 10]),'char');

str = sprintf ('dimension: %d', header.dimension);
popenw(p,double([str, 10]),'char');

popenw(p,double('space: '),'char');
popenw(p,double([header.space, 10]),'char');


popenw(p,double('space origin: ('),'char');
for looper = 1:3
    str = sprintf ('%f', header.spaceorigin(looper)); popenw(p,double(str),'char');
    if (looper < 3)
        popenw(p,double(','),'char');
    end
end
popenw(p,double(')'),'char');
popenw(p,10,'char');

popenw(p,double('sizes:'),'char');
for looper = 1:max(size(header.sizes))
    str = sprintf (' %d', header.sizes(looper));
    popenw(p,double(str),'char');
end
popenw(p,10,'char');

popenw(p,double('space directions: '),'char');
if (strcmp(header.kinds(1), '3D-masked-symmetric-matrix'))
%if ( (length(header.spacedirections)==10) && isnan(header.spacedirections(1)) )
    % assume tensor data
%    header.spacedirections = header.spacedirections(2:end);
    popenw(p,double('none '),'char');
end
for outerlooper = 1:3
    popenw(p,double('('),'char');
    for looper = 1:3
       % str = sprintf ( '%f', header.spacedirections( (outerlooper-1) * 3 + looper ) );
       str = sprintf ( '%f', header.spacedirections(looper,outerlooper));  
        popenw(p,double(str),'char');
        if (looper < 3)
            popenw(p,double(','),'char');
        end
    end
    popenw(p,double(') '),'char');
end
popenw(p,10,'char');

popenw(p,double('kinds:'),'char');
for looper = 1:max(size(header.kinds))
    str = sprintf (' %s', char(header.kinds(looper)));
    popenw(p,double(str),'char');
end
popenw(p,10,'char');


if (strcmp(header.kinds(1), '3D-masked-symmetric-matrix'))
    % assume tensor data
    popenw(p,double('measurement frame: '),'char');
    for outerlooper = 1:3
        popenw(p,double('('),'char');
        for looper = 1:3
            %str = sprintf ( '%f', header.measurementframe( (outerlooper-1) * 3 + looper ) );
            str = sprintf ( '%f', header.measurementframe(looper,outerlooper) );
            popenw(p,double(str),'char');
            if (looper < 3)
                popenw(p,double(','),'char');
            end
        end
        popenw(p,double(') '),'char');
    end
    popenw(p,10,'char');
end

popenw(p,double(['encoding: raw', 10]),'char');

popenw(p,double('endian: '),'char');
popenw(p,double([header.endian, 10]),'char');


popenw(p,10,'char');
popenType = nrrd2popenType( header.type );

if (transformDT & strcmp(header.kinds(1),'3D-masked-symmetric-matrix'))
    header = tensorTransformToSlicerSpace(header);
end

% popenw expects double data since the mex-way to access parameters only
% works for double-pointers (see function mxGetPr)
popenw(p,double(header.data), popenType);

return
