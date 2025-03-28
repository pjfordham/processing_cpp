#ifndef PROCESSING_PSURFACE_H
#define PROCESSING_PSURFACE_H

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <string_view>

class PSurface {
   typedef void (*SizeCallbackType)(int, int);
   SizeCallbackType callback;
   GLFWwindow *window = NULL;
   int width = 0;
   int height = 0;
   bool resizable = false;
public:
   void setResizable( bool r );
   void setSize(int _width, int _height);
   void setVisible( bool v );
   void setTitle( std::string_view t );
   void setLocation( int x, int y );

   bool getResizable() const;
   void init(bool test_mode);
   void shutdown();
   void close();
   bool runLoop();
   void pollEvents();
   void swapBuffers();
   void makeContextCurrent();
   void setSizeCallback(SizeCallbackType c);
   void dispatch_size_callback(int _width, int _height);
};

extern PSurface surface;

#endif
