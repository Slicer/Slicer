
function pline = pgetl( p )

% read a line from a pipe opened by popen

%------read a line

pline = '';

while(1)
    Y = popenr(p,1,'char');
    if (Y==10)
      break;
    end
    pline = [pline Y];
end

pline = char(pline);
