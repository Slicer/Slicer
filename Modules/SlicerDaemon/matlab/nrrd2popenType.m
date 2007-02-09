% convert nrrd type of an exported Slicer volume to format string passed to popen
% handles nrrd types returned by slicerget.tcl 

function pType = nrrd2popenType( nrrdType )
   
  switch nrrdType
    case {'short'} 
      pType = 'int16';
    case {'double'}
      pType = 'double';
    case {'float'}
      pType = 'float';
    case {'ushort'}
      pType = 'uint16';
    case {'char'}
      pType = 'char';
    case {'unsigned char'}
      pType = 'uint8';
    case {'int'}
      pType = 'int32';
    case {'uint'}
      pType = 'uint32';
    case {'longlong'}
      pType = 'int64';
    case {'ulonglong'}
      pType = 'uint64';
    otherwise
     pType = 'unknown type'
  end
