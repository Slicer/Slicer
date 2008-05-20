function volume = getSlicerVolume( id_or_name, transformDT_flag, original_measurementframe)
%function volume = getSlicerVolume( id_or_name, transformDT_flag)
% transformDT_flag: 'transformDT'
% This is an example script that shows how to establish a reading pipe to a
% running slicer daemon (start Slicer3 with option --daemon).
% These steps have to be done to adapt the script to your environment:
% - matlab extentions "popenr" and "popenw" have to be compiled for your
%   machine: cd into $Slicer3_HOME/lib/Slicer3/Modules/SlicerDaemon/Matlab/popen , and
%   do "mex popenr.c" and "mex popenw.c" in matlab.
% - make sure you add the path to popen
% - make sure to add the path to the matlab scripts in 
%   $Slicer3_HOME/lib/Slice3/Modules/SlicerDaemon/Tcl
%
% Input: Name or ID of volume loaded in Slicer. For Tensor Data, you can
%        optionally input the string 'transformDT'. If 'transformDT' is 
%        given, the diffusion tensors 
%        from slicer get transformed back into their orginal gradient space 
%        according to measurement frame and space directions.   
% returns a struct with image data from slicer volume with id "id" or
% name "name" and image information according to the nrrd format.

% add path for popen
cpath = pwd;
cd('popen');
pName = pwd;
addpath (pName);

% find slicerget.tcl script
cd( cpath );
cd('../Tcl');
pScript = pwd;
cd( cpath );

if (isa(id_or_name,'numeric')) 
   % fprintf('The id is: %d.\n',id_or_name);
    cmd_r = sprintf('%s/slicerget.tcl %d',pScript, id_or_name);
elseif (isa(id_or_name,'char'))
   % fprintf('The name is: %s.\n',id_or_name);
    cmd_r = sprintf('%s/slicerget.tcl %s',pScript, id_or_name);
else
    fprintf('Usage: getSlicerVolume(id) or getSlicerVolume(name).\n');
    exit;
end

p_r = popenr(cmd_r);
if p_r < 0
    error(['Error running popenr(',cmd_r,')']);
end

if (nargin==2) 
    error(['Original measurement frame needed to transform tensor back into original measurement frame.']);
elseif (nargin==3 & strcmp(transformDT_flag,'transformDT'))
    volume = preadNrrd(p_r,1,original_measurementframe);
else
    volume = preadNrrd(p_r,0); 
end 
    

% close pipe
popenr(p_r,-1)
