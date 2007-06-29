function [] = saveNrrdStructure( nhdrFileName, nrrdStructure )

% writes a nhdr file structure to a nhdr file
% can only write raw data for now
%
% writeNrrdStructure( nhdrFileName, nrrdStructure )
%
% marc@bwh.harvard.edu
% kquintus@bwh.harvard.edu

[pathstr,matFileName,ext,versn] = fileparts( nhdrFileName );

if (~strcmp( upper(ext), '.NHDR' ) )
    nhdrFileName = sprintf('%s.nhdr',nhdrFileName); 
    [pathstr,matFileName,ext,versn] = fileparts( nhdrFileName );
end

% save data in raw format. Also saves a minimal header, going 
% to overwrite later.
nrrdSave( nhdrFileName, nrrdStructure.data ) 

fid = fopen( nhdrFileName, 'w' );

if (fid == -1) 
  fprintf('ABORT: %s does not exist.\n', nhdrFileName );
  return;
end


fprintf( fid, 'NRRD0004\n' );
fprintf( fid, '# Complete NRRD file format specification at:\n' );
fprintf( fid, '# http://teem.sourceforge.net/nrrd/format.html\n' );
fprintf( fid, 'content: %s\n', nrrdStructure.content);
fprintf( fid, 'type: %s\n', nrrdStructure.type );
fprintf( fid, 'dimension: %d\n', nrrdStructure.dimension );
fprintf( fid, 'space: %s\n', nrrdStructure.space );
fprintf( fid, 'sizes: ' );

for iI=1:length( nrrdStructure.sizes )
  fprintf( fid, '%d', nrrdStructure.sizes(iI) );
  if ( iI~=length( nrrdStructure.sizes ) )
    fprintf( fid, ' ' );
  end
end
fprintf( fid, '\n' );

fprintf( fid, 'space directions: ' );
% if first kind is 3D-masked-symmetric-matrix, assume tensor data.
if strcmp(nrrdStructure.kinds(1), '3D-masked-symmetric-matrix')
    fprintf( fid, 'none ' );
end
sd = nrrdStructure.spacedirections;
fprintf( fid, '(%f,%f,%f) (%f,%f,%f) (%f,%f,%f)\n', ...
 sd(1,1), sd(2,1), sd(3,1), sd(1,2), sd(2,2), sd(3,2), sd(1,3), sd(2,3), sd(3,3) );

fprintf( fid, 'kinds: ' );
for iI=1:length( nrrdStructure.kinds )
  fprintf( fid, '%s', nrrdStructure.kinds{iI} );
  if ( iI~=length( nrrdStructure.kinds ) )
    fprintf( fid, ' ' );
  end
end
fprintf( fid, '\n' );

if ( isfield( nrrdStructure, 'endian' ) )
  fprintf( fid, 'endian: %s\n', nrrdStructure.endian );
end

% for now encoding is raw since nrrdSave does encoding in raw
%fprintf( fid, 'encoding: %s\n', nrrdStructure.encoding );
fprintf( fid, 'encoding: raw\n');

so = nrrdStructure.spaceorigin;
fprintf( fid, 'space origin: (%f,%f,%f)\n', so(1), so(2), so(3) );

% check if there is a measurement frame
if any(strcmp(fieldnames(nrrdStructure),'measurementframe'))
    mf = nrrdStructure.measurementframe;
    fprintf( fid, 'measurement frame: (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)\n', ...
        mf(1,1), mf(2,1), mf(3,1), mf(1,2), mf(2,2), mf(3,2), mf(1,3), mf(2,3), mf(3,3) );
end

% check if field thickness is there
if any(strcmp(fieldnames(nrrdStructure),'thicknesses'))
    fprintf( fid, 'thicknesses: ' );
    for iI=1:length( nrrdStructure.thicknesses )
        fprintf( fid, '%f', nrrdStructure.thicknesses(iI) );
        if ( iI~=length( nrrdStructure.thicknesses ) )
            fprintf( fid, ' ' );
        end
    end
    fprintf( fid, '\n' );
    %thick = nrrdStructure.thicknesses;
    %fprintf( fid, 'thicknesses: %f %f %f\n', thick(1), thick(2), thick(3) );
end

% check if field centerings is there
if any(strcmp(fieldnames(nrrdStructure),'centerings'))    
    fprintf( fid, 'centerings: ' );
    for iI=1:length( nrrdStructure.centerings )
        fprintf( fid, '%s', char(nrrdStructure.centerings(iI)) );
        if ( iI~=length( nrrdStructure.centerings ) )
            fprintf( fid, ' ' );
        end
    end
    fprintf( fid, '\n' );
    
    %ce = nrrdStructure.centerings;
    %fprintf( fid, 'centerings: %s %s %s\n', char(ce(1)), char(ce(2)), char(ce(3)) );
end

% check if field spaceunits is there
if any(strcmp(fieldnames(nrrdStructure),'spaceunits'))
    fprintf( fid, 'space units: ' );
    for iI=1:length( nrrdStructure.spaceunits )
        fprintf( fid, '\"%s\"', char(nrrdStructure.spaceunits(iI)) );
        if ( iI~=length( nrrdStructure.spaceunits ) )
            fprintf( fid, ' ' );
        end
    end
    fprintf( fid, '\n' );
    
    %su = nrrdStructure.spaceunits;
    %fprintf( fid, 'space units: %s %s %s\n', char(su(1)), char(su(2)), char(su(3)) );
end

fprintf( fid, 'data file: %s.raw\n', matFileName);

fclose( fid );