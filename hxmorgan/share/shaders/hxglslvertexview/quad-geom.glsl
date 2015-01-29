#extension GL_EXT_geometry_shader4 : enable

uniform float sphere_size;

varying out vec3 light_position;
varying out vec4 eye_position;

void main()
{
    float halfsize = sphere_size * 0.5;

    gl_TexCoord[0] = gl_TexCoordIn[0][0];
    gl_FrontColor = gl_FrontColorIn[0];
    
    // Normalize the light position
    light_position = normalize(gl_LightSource[0].position.xyz);
    eye_position = gl_PositionIn[0];
    
    // Vertex 1
    gl_TexCoord[0].st = vec2(-1.0,-1.0);
    gl_Position = gl_PositionIn[0];
    gl_Position.xy += vec2(-halfsize, -halfsize);
    gl_Position = gl_ProjectionMatrix  * gl_Position;
    EmitVertex();

    // Vertex 2
    gl_TexCoord[0].st = vec2(-1.0,1.0);
    gl_Position = gl_PositionIn[0];
    gl_Position.xy += vec2(-halfsize, halfsize);
    gl_Position = gl_ProjectionMatrix  * gl_Position;
    EmitVertex();

    // Vertex 3
    gl_TexCoord[0].st = vec2(1.0,-1.0);
    gl_Position = gl_PositionIn[0];
    gl_Position.xy += vec2(halfsize, -halfsize);
    gl_Position = gl_ProjectionMatrix  * gl_Position;
    EmitVertex();

    // Vertex 4
    gl_TexCoord[0].st = vec2(1.0,1.0);
    gl_Position = gl_PositionIn[0];
    gl_Position.xy += vec2(halfsize, halfsize);
    gl_Position = gl_ProjectionMatrix  * gl_Position;
    EmitVertex();

    EndPrimitive();
}
