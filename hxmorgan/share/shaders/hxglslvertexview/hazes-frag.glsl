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
      float alpha = 0.9* (exp(-sqrtMag * 5.0));

      gl_FragColor.rgb = gl_Color.rgb * (1.0 - sqrtMag);
      gl_FragColor.a = alpha;
    }
}