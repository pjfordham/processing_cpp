#ifndef PROCESSING_H
#define PROCESSING_H

#include "processing_utils.h"
#include "processing_math.h"
#include "processing_color.h"
#include "processing_java_compatability.h"
#include "processing_pimage.h"
#include "processing_pshape.h"
#include "processing_pshape_svg.h"
#include "processing_pgraphics.h"
#include "processing_pfont.h"
#include "processing_time.h"
#include "processing_pshader.h"
#include "processing_xml.h"
#include "processing_json.h"
#include "processing_psurface.h"

extern "C" void keyPressed();
extern "C" void keyReleased();
extern "C" void keyTyped();
extern "C" void setup();
extern "C" void draw();
extern "C" void mousePressed();
extern "C" void mouseDragged();
extern "C" void mouseMoved();
extern "C" void mouseReleased();
extern "C" void mouseWheel(const MouseEvent &event);
#ifdef _MSC_VER
#pragma comment(linker, "/alternatename:keyTyped=keyTyped_weak")
#pragma comment(linker, "/alternatename:keyPressed=keyPressed_weak")
#pragma comment(linker, "/alternatename:keyReleased=keyReleased_weak")
#pragma comment(linker, "/alternatename:setup=setup_weak")
#pragma comment(linker, "/alternatename:draw=draw_weak")
#pragma comment(linker, "/alternatename:mousePressed=mousePressed_weak")
#pragma comment(linker, "/alternatename:mouseDragged=mouseDragged_weak")
#pragma comment(linker, "/alternatename:mouseMoved=mouseMoved_weak")
#pragma comment(linker, "/alternatename:mouseReleased=mouseReleased_weak")
#pragma comment(linker, "/alternatename:mouseWheel=mouseWheel_weak")
#endif

// This is the global PSurface object that forms the top level canvas.
// PSurface for the main application window. Multiple windows/surfaces
// are supported.
class PSurfaceMain : public PSurface {
   void keyTyped() { ::keyTyped(); }
   void keyPressed() { ::keyPressed(); }
   void keyReleased() { ::keyReleased(); }
   void setup() { ::setup(); }
   void draw() { ::draw(); }
   void mousePressed() { ::mousePressed(); }
   void mouseDragged() { ::mouseDragged(); }
   void mouseMoved() { ::mouseMoved(); }
   void mouseReleased() { ::mouseReleased(); }
   void mouseWheel(const MouseEvent&q) {::mouseWheel(q); };
};
extern PSurfaceMain surface;

MAKE_GLOBAL(shader, surface.g);
MAKE_GLOBAL(resetShader, surface.g);
MAKE_GLOBAL(hint, surface.g);
MAKE_GLOBAL(get, surface.g);
MAKE_GLOBAL(set, surface.g);
MAKE_GLOBAL(saveFrame, surface.g);
MAKE_GLOBAL(save, surface.g);
MAKE_GLOBAL(background, surface.g);
MAKE_GLOBAL(ellipse, surface.g);
MAKE_GLOBAL(rect, surface.g);
MAKE_GLOBAL(line, surface.g);
MAKE_GLOBAL(point, surface.g);
MAKE_GLOBAL(quad, surface.g);
MAKE_GLOBAL(triangle, surface.g);
MAKE_GLOBAL(arc, surface.g);
MAKE_GLOBAL(shape, surface.g);
MAKE_GLOBAL(bezier, surface.g);
MAKE_GLOBAL(beginShape, surface.g);
MAKE_GLOBAL(beginContour, surface.g);
MAKE_GLOBAL(endContour, surface.g);
MAKE_GLOBAL(vertex, surface.g);
MAKE_GLOBAL(normal, surface.g);
MAKE_GLOBAL(noNormal, surface.g);
MAKE_GLOBAL(bezierVertex, surface.g);
MAKE_GLOBAL(curveVertex, surface.g);
MAKE_GLOBAL(curveTightness, surface.g);
MAKE_GLOBAL(endShape, surface.g);
MAKE_GLOBAL(image, surface.g);
MAKE_GLOBAL(imageMode, surface.g);
MAKE_GLOBAL(tint, surface.g);
MAKE_GLOBAL(noTint, surface.g);
MAKE_GLOBAL(textureMode, surface.g);
MAKE_GLOBAL(texture, surface.g);
MAKE_GLOBAL(noTexture, surface.g);
MAKE_GLOBAL(box, surface.g);
MAKE_GLOBAL(sphere, surface.g);
MAKE_GLOBAL(sphereDetail, surface.g);
MAKE_GLOBAL(createBezier, surface.g);
MAKE_GLOBAL(createRect, surface.g);
MAKE_GLOBAL(createQuad, surface.g);
MAKE_GLOBAL(createLine, surface.g);
MAKE_GLOBAL(createTriangle, surface.g);
MAKE_GLOBAL(createArc, surface.g);
MAKE_GLOBAL(createEllipse, surface.g);
MAKE_GLOBAL(createSphere, surface.g);
MAKE_GLOBAL(createBox, surface.g);
MAKE_GLOBAL(createPoint, surface.g);
MAKE_GLOBAL(createGroup, surface.g);
MAKE_GLOBAL(ambient, surface.g);
MAKE_GLOBAL(emissive, surface.g);
MAKE_GLOBAL(specular, surface.g);
MAKE_GLOBAL(shininess, surface.g);
MAKE_GLOBAL(fill, surface.g);
MAKE_GLOBAL(noFill, surface.g);
MAKE_GLOBAL(noStroke, surface.g);
MAKE_GLOBAL(stroke, surface.g);
MAKE_GLOBAL(strokeWeight, surface.g);
MAKE_GLOBAL(strokeCap, surface.g);
MAKE_GLOBAL(ellipseMode, surface.g);
MAKE_GLOBAL(rectMode, surface.g);
MAKE_GLOBAL(smooth, surface.g);
MAKE_GLOBAL(noSmooth, surface.g);
MAKE_GLOBAL(updatePixels, surface.g);
MAKE_GLOBAL(textAlign, surface.g);
MAKE_GLOBAL(text, surface.g);
MAKE_GLOBAL(directionalLight, surface.g);
MAKE_GLOBAL(ambientLight, surface.g);
MAKE_GLOBAL(pointLight, surface.g);
MAKE_GLOBAL(spotLight, surface.g);
MAKE_GLOBAL(lightFalloff, surface.g);
MAKE_GLOBAL(lightSpecular, surface.g);
MAKE_GLOBAL(lights, surface.g);
MAKE_GLOBAL(noLights, surface.g);
MAKE_GLOBAL(ortho, surface.g);
MAKE_GLOBAL(perspective, surface.g);
MAKE_GLOBAL(screenX, surface.g);
MAKE_GLOBAL(screenY, surface.g);
MAKE_GLOBAL(camera, surface.g);
MAKE_GLOBAL(pushMatrix, surface.g);
MAKE_GLOBAL(resetMatrix, surface.g);
MAKE_GLOBAL(popMatrix, surface.g);
MAKE_GLOBAL(translate, surface.g);
MAKE_GLOBAL(transform, surface.g);
MAKE_GLOBAL(scale, surface.g);
MAKE_GLOBAL(rotate, surface.g);
MAKE_GLOBAL(rotateX, surface.g);
MAKE_GLOBAL(rotateY, surface.g);
MAKE_GLOBAL(rotateZ, surface.g);
MAKE_GLOBAL(blendMode, surface.g);
MAKE_GLOBAL(filter, surface.g);
MAKE_GLOBAL(size, surface);
MAKE_GLOBAL(loadPixels, surface);
MAKE_GLOBAL(createGraphics, surface);

extern int frameRateb;

extern int textureWrapMode;

inline void textureWrap(int wrap) {
   textureWrapMode = wrap;
}

extern int setFrameRate;
inline void frameRate(int rate) {
   setFrameRate = rate;
}

inline void orientation(int o) {}

extern bool xloop;

inline void noLoop() {
   xloop = false;
}

inline void loop() {
   xloop = true;
}

void exit();

extern int &width;
extern int &height;


extern unsigned int *&pixels; // pointer to the texture's pixel data in the desired format

extern int frameCount;
extern int &mouseX;
extern int &mouseY;
extern int &pmouseX;
extern int &pmouseY;

inline void redraw() {
   frameCount = 0;
}

extern char &key;
extern int &keyCode;

extern bool &mousePressedb;
extern bool &keyPressedb;

bool dispatchEvents();


#endif
