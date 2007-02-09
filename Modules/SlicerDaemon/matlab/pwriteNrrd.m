
function pwriteNrrd( p, header )

% write a nrrd image to a pipe opened by popenw

popenw(p,double(['NRRD0001', 10]),'char');

popenw(p,double(['content: from pwriteNrrd', 10]),'char');

popenw(p,double('type: '),'char');
popenw(p,double([header.type, 10]),'char');

header

str = sprintf ('dimension: %d', header.dimension);
popenw(p,double([str, 10]),'char');

popenw(p,double('space: '),'char');
popenw(p,double([header.space, 10]),'char');

popenw(p,double('sizes:'),'char');
for looper = 1:header.dimension
  str = sprintf (' %d', header.sizes(looper));
  popenw(p,double(str),'char');
end
popenw(p,10,'char');

popenw(p,double('space origin: ('),'char');
for looper = 1:header.dimension
  str = sprintf ('%f', header.spaceorigin(looper)); popenw(p,double(str),'char');
  if (looper < header.dimension)
    popenw(p,double(','),'char');
  end
end
popenw(p,double(')'),'char');
popenw(p,10,'char');


popenw(p,double('space directions: '),'char');
for outerlooper = 1:header.dimension
  popenw(p,double('('),'char');
  for looper = 1:3
    str = sprintf ( '%f', header.spacedirections( (outerlooper-1) * header.dimension + looper ) );
    popenw(p,double(str),'char');
    if (looper < header.dimension)
      popenw(p,double(','),'char');
    end
  end
  popenw(p,double(') '),'char');
end
popenw(p,10,'char');

popenw(p,double(['encoding: raw', 10]),'char');

popenw(p,double('endian: '),'char');
popenw(p,double([header.endian, 10]),'char');


popenw(p,10,'char');
popenType = nrrd2popenType( header.type );

% popenw expects double data since the mex-way to access parameters only
% works for double-pointers (see function mxGetPr)
popenw(p,double(header.data), popenType);

return
