#version 330 core

layout ( location = 0 ) in vec3 vertex_position;
layout ( location = 1 ) in vec2 texCoords;
layout ( location = 4 ) in vec4 position;

uniform mat4 view;
uniform mat4 projection;
uniform float particleSize;

uniform vec3 movement;
uniform float dt;

out float AliveTime;
out vec2 TexCoords;
out vec4 Position;

void main()
{
   vec4 position_viewspace = view * vec4( position.xyz, 1 ); // ,movement handled by (movement * dt * position.w (AliveTime)

   position_viewspace.xy += particleSize * (vertex_position.xy - vec2(0.5));

   gl_Position = projection * position_viewspace;

   AliveTime = position.w;
   TexCoords = texCoords;
   Position = position;
};