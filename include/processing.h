#ifndef PROCESSING_H
#define PROCESSING_H

#include "processing_utils.h"
#include "processing_math.h"
#include "processing_color.h"
#include "processing_java_compatability.h"
#include "processing_pimage.h"
#include "processing_pshape.h"
#include "processing_pgraphics.h"
#include "processing_pfont.h"
#include "processing_time.h"

// This is the global PGraphcs object that forms the top level canvas.
extern PGraphics g;

MAKE_GLOBAL(shader, g.glc);
MAKE_GLOBAL(loadShader, g.glc);
MAKE_GLOBAL(resetShader, g.glc);
MAKE_GLOBAL(hint, g.glc);
MAKE_GLOBAL(get, g);
MAKE_GLOBAL(set, g);
MAKE_GLOBAL(createGraphics, g);
MAKE_GLOBAL(saveFrame, g);
MAKE_GLOBAL(save, g);
MAKE_GLOBAL(background, g);
MAKE_GLOBAL(ellipse, g);
MAKE_GLOBAL(rect, g);
MAKE_GLOBAL(line, g);
MAKE_GLOBAL(point, g);
MAKE_GLOBAL(quad, g);
MAKE_GLOBAL(triangle, g);
MAKE_GLOBAL(arc, g);
MAKE_GLOBAL(shape, g);
MAKE_GLOBAL(bezier, g);
MAKE_GLOBAL(beginShape, g);
MAKE_GLOBAL(vertex, g);
MAKE_GLOBAL(normal, g);
MAKE_GLOBAL(noNormal, g);
MAKE_GLOBAL(bezierVertex, g);
MAKE_GLOBAL(endShape, g);
MAKE_GLOBAL(image, g);
MAKE_GLOBAL(imageMode, g);
MAKE_GLOBAL(tint, g);
MAKE_GLOBAL(noTint, g);
MAKE_GLOBAL(textureMode, g);
MAKE_GLOBAL(texture, g);
MAKE_GLOBAL(noTexture, g);
MAKE_GLOBAL(box, g);
MAKE_GLOBAL(sphere, g);
MAKE_GLOBAL(sphereDetail, g);
MAKE_GLOBAL(createBezier, g);
MAKE_GLOBAL(createRect, g);
MAKE_GLOBAL(createQuad, g);
MAKE_GLOBAL(createLine, g);
MAKE_GLOBAL(createTriangle, g);
MAKE_GLOBAL(createArc, g);
MAKE_GLOBAL(createEllipse, g);
MAKE_GLOBAL(createPoint, g);
MAKE_GLOBAL(createGroup, g);
MAKE_GLOBAL(fill, g);
MAKE_GLOBAL(noFill, g);
MAKE_GLOBAL(noStroke, g);
MAKE_GLOBAL(stroke, g);
MAKE_GLOBAL(strokeWeight, g);
MAKE_GLOBAL(strokeCap, g);
MAKE_GLOBAL(ellipseMode, g);
MAKE_GLOBAL(rectMode, g);
MAKE_GLOBAL(noSmooth,g);
MAKE_GLOBAL(updatePixels, g);
MAKE_GLOBAL(textFont, g);
MAKE_GLOBAL(textAlign, g);
MAKE_GLOBAL(textSize, g);
MAKE_GLOBAL(text, g);
MAKE_GLOBAL(directionalLight, g);
MAKE_GLOBAL(ambientLight, g);
MAKE_GLOBAL(pointLight, g);
MAKE_GLOBAL(lightFalloff, g);
MAKE_GLOBAL(lights, g);
MAKE_GLOBAL(noLights, g);
MAKE_GLOBAL(ortho, g);
MAKE_GLOBAL(perspective, g);
MAKE_GLOBAL(screenX, g);
MAKE_GLOBAL(screenY, g);
MAKE_GLOBAL(camera, g);
MAKE_GLOBAL(pushMatrix, g);
MAKE_GLOBAL(popMatrix, g);
MAKE_GLOBAL(translate, g);
MAKE_GLOBAL(transform, g);
MAKE_GLOBAL(scale, g);
MAKE_GLOBAL(rotate, g);
MAKE_GLOBAL(rotateX, g);
MAKE_GLOBAL(rotateY, g);
MAKE_GLOBAL(rotateZ, g);


extern int setFrameRate;
inline void frameRate(int rate) {
   setFrameRate = rate;
}

extern bool xloop;

inline void noLoop() {
   xloop = false;
}

inline void loop() {
   xloop = true;
}

extern int width;
extern int height;

inline void size(int _width, int _height, int mode = P2D) {
   // Create a window
   width = _width;
   height = _height;
   g = PGraphics(width, height, mode, 2.0);
}


extern unsigned int *pixels; // pointer to the texture's pixel data in the desired format

inline void loadPixels() {
   g.loadPixels();
   pixels = g.pixels.data();
}

extern int frameCount;
extern int mouseX;
extern int mouseY;
extern int pmouseX;
extern int pmouseY;

inline void redraw() {
   frameCount = 0;
}

extern char key;
extern int keyCode;

extern bool mousePressedb;

bool dispatchEvents();
void drawFrame();

void keyPressed();
void keyReleased();
void keyTyped();
void setup();
void draw();
void mousePressed();
void mouseDragged();
void mouseMoved();
void mouseReleased();

#endif
