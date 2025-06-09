#version 150

layout(lines) in;
layout(line_strip, max_vertices=2) out;

uniform ivec2 ViewportSize;
noperspective out float stippleCoord;

void main()
{
  int i=0;
  vec4 pos0 = gl_in[0].gl_Position;
  vec2 p0 = pos0.xy / pos0.w * ViewportSize;
  gl_Position = pos0;
  stippleCoord = 0.0;
  EmitVertex();

  i=1;
  vec4 pos1 = gl_in[i].gl_Position;
  vec2 p1 = pos1.xy / pos1.w * ViewportSize;
  float len = length(p0.xy - p1.xy);
  stippleCoord = len / 32;
  gl_Position = pos1;
  EmitVertex();
}
