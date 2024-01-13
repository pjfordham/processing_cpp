#include "glad/glad.h"

PShader cubemapShader;
PShape domeSphere;

GLuint fbo;
GLuint rbo;
GLuint envMapTextureID;

int envMapSize = 1024;

void initCubeMap() {
  sphereDetail(50);
  domeSphere = createSphere(height/2.0f);
  domeSphere.rotateX(HALF_PI);
  domeSphere.setStroke(false);

  glGenTextures(1, &envMapTextureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envMapTextureID);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  for (int i = GL_TEXTURE_CUBE_MAP_POSITIVE_X; i < GL_TEXTURE_CUBE_MAP_POSITIVE_X + 6; i++) {
    glTexImage2D(i, 0, GL_RGBA8, envMapSize, envMapSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  }

  // Init fbo, rbo
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, envMapTextureID, 0);

  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, envMapSize, envMapSize);

  // Attach depth buffer to FBO
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

  // Load cubemap shader.
  cubemapShader = loadShader("cubemapfrag.glsl", "cubemapvert.glsl");
  cubemapShader.set("cubemap", 1);
}

void drawDomeMaster();
void regenerateEnvMap();
void drawScene();

void drawCubeMap() {
  glActiveTexture(GL_TEXTURE1);
  glEnable(GL_TEXTURE_CUBE_MAP);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envMapTextureID);
  regenerateEnvMap();

  drawDomeMaster();

  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void drawDomeMaster() {
  camera();
  ortho();
  g.resetMatrix();
  shader(cubemapShader);
  shape(domeSphere);
  resetShader();
}

// Called to regenerate the envmap
void regenerateEnvMap() {
  // bind fbo
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  // generate 6 views from origin(0, 0, 0)
  glViewport(0, 0, envMapSize, envMapSize);
  perspective(90.0f * DEG_TO_RAD, 1.0f, 1.0f, 1025.0f);
  for (int face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <
                  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; face++) {
    g.resetMatrix();

    if (face == GL_TEXTURE_CUBE_MAP_POSITIVE_X) {
      camera(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f);
    } else if (face == GL_TEXTURE_CUBE_MAP_NEGATIVE_X) {
      camera(0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f);
    } else if (face == GL_TEXTURE_CUBE_MAP_POSITIVE_Y) {
      camera(0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f);
    } else if (face == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y) {
      camera(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    } else if (face == GL_TEXTURE_CUBE_MAP_POSITIVE_Z) {
      camera(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f);
    }

    scale(-1, 1, -1);
    translate(-width * 0.5f, -height * 0.5f, -500);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, face, envMapTextureID, 0);

    drawScene(); // Draw objects in the scene
    g.commit_draw(); // Make sure that the geometry in the scene is pushed to the GPU
    noLights();  // Disabling lights to avoid adding many times
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, face, 0, 0);
  }
}
