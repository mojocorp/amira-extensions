void main()
{
    // Discard fragment where the squared magnitude 
    // is greater than the squared circle radius
    float sqrtMag = dot(gl_TexCoord[0].xy, gl_TexCoord[0].xy);

    if(sqrtMag > 1.0)
    {
      discard;
    }
    else
    {
      gl_FragColor = gl_Color * (1.0 - sqrtMag);
    }
}