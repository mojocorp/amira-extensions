uniform float sphere_size;

varying vec3 light_position;
varying vec4 eye_position;

void main()
{
    float x = gl_TexCoord[0].x;
    float y = gl_TexCoord[0].y;
    float zz = 1.0 - x*x - y*y;

    if (zz <= 0.0)
        discard;

    float z = sqrt(zz);

    // The point is on a unit sphere,
    // we don’t need to normalize
    vec3 normal = vec3(x, y, z);

   // Lighting
   float diffuse = max(dot(normal, light_position), 0.0);

   vec4 pos = eye_position;
   pos.z += z*sphere_size;
   pos = gl_ProjectionMatrix * pos;

   gl_FragDepth = (gl_DepthRange.diff * (pos.z / pos.w) + gl_DepthRange.near + gl_DepthRange.far) * 0.5;
   gl_FragColor = gl_Color * diffuse;
}
