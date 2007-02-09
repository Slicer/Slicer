function volume = getSlicerVolume( id_or_name )

% This is an example script that shows how to establish a reading pipe to a
% running slicer daemon (start Slicer3 with option --daemon).
% These steps have to be done to adapt the script to your environment:
% - matlab extentions "popenr" and "popenw" have to be compiled for your
%   machine: cd into $SLICER_HOME/Modules/SlicerDaemon/matlab/popen , and
%   do "mex popenr.c" and "mex popenw.c" in matlab.
% - make sure you add the path to popen
% - make sure to add the path to the matlab scripts in 
%   $SLICER_HOME/Modules/SlicerDaemon/Tcl
%
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

volume = preadNrrd(p_r);

% close pipe
popenr(p_r,-1)
