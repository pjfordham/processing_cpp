PShader shaderArt;

void setup() {
  size(640, 360, P2D);
  noStroke();

  shaderArt = loadShader("ShaderArt.glsl");
  shaderArt.set("resolution", float(width), float(height));
}

void draw() {
  shaderArt.set("iTime", millis());
  shader(shaderArt);
  rect(0, 0, width, height);
}

