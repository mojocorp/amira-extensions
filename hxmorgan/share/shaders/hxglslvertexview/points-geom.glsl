#extension GL_EXT_geometry_shader4 : enable

void main()
{
    // copy color
    gl_FrontColor = gl_FrontColorIn[0];
 
    // copy position
    gl_Position = gl_PositionIn[0];
 
    // done with the vertex
    EmitVertex();
}
