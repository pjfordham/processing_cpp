#ifndef PROCESSING_PSURFACE_H
#define PROCESSING_PSURFACE_H

#include <string_view>
#include "processing_pgraphics.h"

struct GLFWwindow;

class MouseEvent {
public:
   enum action_t {
      PRESS,
      RELEASE,
      CLICK,
      MOVE,
      DRAG,
      ENTER,
      EXIT,
      WHEEL,
   };

   action_t action; // Type of event
   int x, y;        // Mouse position
   int button;      // Button clicked
   float count;     // Scroll amount for wheel

   action_t getAction() const { return action; }
   int getX() const { return x; }
   int getY() const { return y; }
   int getButton() const { return button; }
   float getCount() const { return count; }
};

class PSurface {
   typedef void (*SizeCallbackType)(int, int);
   typedef void (*BasicCallbackType)();
   typedef void (*MouseCallbackType)(const MouseEvent&);
   SizeCallbackType callback;
   GLFWwindow *window = NULL;
   bool resizable = false;
public:
   void setResizable( bool r );
   void setSize(int _width, int _height);
   void setVisible( bool v );
   void setTitle( std::string_view t );
   void setLocation( int x, int y );

   bool getResizable() const;
   void construct(PSurface *main = nullptr);
   void shutdown();
   bool runLoop();
   void pollEvents();
   void swapBuffers();
   void makeContextCurrent();
   void setSizeCallback(SizeCallbackType c);
   void dispatch_size_callback(int _width, int _height);

   PSurface();
   ~PSurface();

   int width = 0;
   int height = 0;
   static void init();
   static void close();

   PGraphics g;
   bool xloop = true;

   unsigned int *pixels;

   char key;
   int keyCode;
   int mouseX;
   int mouseY;
   int pmouseX;
   int pmouseY;
   bool mousePressedb;
   bool keyPressedb;

   virtual void keyTyped() {}
   virtual void keyPressed() {}
   virtual void keyReleased() {}
   virtual void setup() {}
   virtual void draw() {}
   virtual void mousePressed() {}
   virtual void mouseDragged() {}
   virtual void mouseMoved() {}
   virtual void mouseReleased() {}
   virtual void mouseWheel(const MouseEvent&) {}

   int drawFrame();
   void size(int _width, int _height);
   void size(int _width, int _height, int mode);
   void setupFrame();
   void loadPixels();
   PGraphics createGraphics(int w, int h, int mode);
   PGraphics createGraphics(int w, int h);
};

#endif
