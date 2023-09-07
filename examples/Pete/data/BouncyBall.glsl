/* This animation is the material of my first youtube tutorial about creative
   coding, which is a video in which I try to introduce programmers to GLSL
   and to the wonderful world of shaders, while also trying to share my recent
   passion for this community.
                                       Video URL: https://youtu.be/f4s1h2YETNY
*/

uniform float iTime;
uniform vec2 resolution;

//https://iquilezles.org/articles/palettes/
vec3 palette( float t ) {
    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(1.0, 1.0, 1.0);
    vec3 d = vec3(0.263,0.416,0.557);

    return a + b*cos( 6.28318*(c*t+d) );
}

    //if ( abs(uv.x) < 0.5 && abs(uv.y) < 0.5 ) finalColor = vec3(0.0,1.0,0.0);

/*uv = fract(uv) - 0.5 ;
    if (0.2 < length(uv)  && length(uv) <  0.3  ) finalColor = vec3(0.0,1.0,0.0);
    else finalColor = vec3(0.0,0.0,0.0);

    uv = fract(2.0 * uv) - 0.5 ;
    if (0.2 < length(uv)  && length(uv) <  0.3  ) finalColor = finalColor + vec3(0.0,1.0,0.0);
    else finalColor = finalColor + vec3(0.0,0.0,0.0);

    uv = fract(2.0 * uv) - 0.5 ;
    if (0.2 < length(uv)  && length(uv) <  0.3  ) finalColor = finalColor + vec3(0.0,1.0,0.0);
    else finalColor = finalColor + vec3(0.0,0.0,0.0);*/

float bounce( float w, float t ) {
  t = mod(t, 2.0 * w );
  if (t < w) return t;
  else return (2.0 * w ) - t;
}

void main( void ) {
    vec2 xy = gl_FragCoord.xy;
    vec3 finalColor = vec3(0.0);
    float fTime = iTime / 5.0;

    vec2 pos = vec2( bounce( resolution.x, fTime ), bounce( resolution.y,fTime) );
    if ( distance(xy, pos) < 10.0 ) finalColor=vec3(1.0,1.0,1.0);
    gl_FragColor = vec4(finalColor, 1.0);
}