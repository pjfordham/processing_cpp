#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

#define PROCESSING_TEXTURE_SHADER

uniform sampler2D texture[16];
uniform vec2 texOffset[16];

varying vec4 vertColor;
varying vec4 vertTexCoord;

void main(void) {
  // Grouping texcoord variables in order to make it work in the GMA 950. See post #13
  // in this thread:
  // http://www.idevgames.com/forums/thread-3467.html
  vec2 tc0 = vertTexCoord.st + vec2(-texOffset[0].s, -texOffset[0].t);
  vec2 tc1 = vertTexCoord.st + vec2(         0.0, -texOffset[0].t);
  vec2 tc2 = vertTexCoord.st + vec2(+texOffset[0].s, -texOffset[0].t);
  vec2 tc3 = vertTexCoord.st + vec2(-texOffset[0].s,          0.0);
  vec2 tc4 = vertTexCoord.st + vec2(         0.0,          0.0);
  vec2 tc5 = vertTexCoord.st + vec2(+texOffset[0].s,          0.0);
  vec2 tc6 = vertTexCoord.st + vec2(-texOffset[0].s, +texOffset[0].t);
  vec2 tc7 = vertTexCoord.st + vec2(         0.0, +texOffset[0].t);
  vec2 tc8 = vertTexCoord.st + vec2(+texOffset[0].s, +texOffset[0].t);

  vec4 col0 = texture2D(texture[0], tc0);
  vec4 col1 = texture2D(texture[0], tc1);
  vec4 col2 = texture2D(texture[0], tc2);
  vec4 col3 = texture2D(texture[0], tc3);
  vec4 col4 = texture2D(texture[0], tc4);
  vec4 col5 = texture2D(texture[0], tc5);
  vec4 col6 = texture2D(texture[0], tc6);
  vec4 col7 = texture2D(texture[0], tc7);
  vec4 col8 = texture2D(texture[0], tc8);

  vec4 sum = (1.0 * col0 + 2.0 * col1 + 1.0 * col2 +  
              2.0 * col3 + 4.0 * col4 + 2.0 * col5 +
              1.0 * col6 + 2.0 * col7 + 1.0 * col8) / 16.0;            
  gl_FragColor = vec4(sum.rgb, 1.0) * vertColor;  
}
