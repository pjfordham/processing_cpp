#include "processing_pshape_svg.h"
#include "processing_pshape.h"
#include "processing_math.h"
#include "processing_enum.h"

#include <fmt/core.h>
#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>


static bool isDigit(std::string::const_iterator x, std::string::const_iterator end) {
   return (x != end) && ( (*x >= '0' && *x <='9')  || *x == '.');
}

static void parseWhiteSpace(std::string::const_iterator &x, std::string::const_iterator end) {
   while( (x != end) && ( *x == ' ' || *x == '\n' || *x == '\t') ) {
      x++;
   }
}

static void parseSign(std::string::const_iterator &x,  std::string::const_iterator end, float &value) {
   if (x == end) {
      abort();
   }
   if (*x == '-') {
      x++;
      value = -1.0;
   } else {
      value = 1.0;
   }
}

static void parseFloat(std::string::const_iterator &x, std::string::const_iterator end, float &value) {
   if (x == end)
      abort();
   parseWhiteSpace( x, end );
   auto begin = x;
   parseSign( x, end, value );
   while( isDigit( x, end ) ) { x++; }
   std::string z(begin,x);
   if (z.length() == 0)
      abort();
   value = std::stof(z);
}

static float cx, cy, lx, ly;

static void parseMove(std::string::const_iterator &i, std::string::const_iterator end, PShape& pshape) {
   parseWhiteSpace( i, end );
   if (*i == 'M') {
      i++;
      float x;
      parseFloat( i, end, x );
      if ( *i == ',') i++;
      float y;
      parseFloat( i, end, y );
      pshape.beginShape();
      pshape.vertex(x,y);
      lx = x;
      ly = y;
   }
}

static void parseLine( std::string::const_iterator &i,  std::string::const_iterator end, PShape &pshape) {
   parseWhiteSpace( i, end );
   if (i == end)
       return;
   if (*i == 'l') {
      i++;
      float x2,y2;
      parseFloat( i, end, x2);
      if ( *i == ',') i++;
      parseFloat( i, end, y2);
      pshape.vertex(lx+x2,ly+y2);
      lx = lx+x2;
      ly = ly+y2;
   }
   else if (*i == 'L') {
      i++;
      float x2,y2;
      parseFloat( i, end, x2);
      if ( *i == ',') i++;
      parseFloat( i, end, y2);
      pshape.vertex(x2,y2);
      lx = x2;
      ly = y2;
   }
   else if (*i == 'h') {
      i++;
      float x2;
      parseFloat( i, end, x2);
      pshape.vertex(lx+x2, ly);
      lx = lx + x2;
      ly = ly;
   }
   else if (*i == 'H') {
      i++;
      float x2;
      parseFloat( i,end,x2);
      pshape.vertex(x2,ly);
      lx = x2;
      ly = ly;
   }
   else if (*i == 'v') {
      i++;
      float y2;
      parseFloat( i,end,y2);
      pshape.vertex(lx,ly+y2);
      lx = lx;
      ly = ly +y2;
   }
   else if (*i == 'V') {
      i++;
      float y2;
      parseFloat( i,end,y2);
      pshape.vertex(lx,y2);
      lx = lx;
      ly = y2;
   }
}

static void parseCurve(std::string::const_iterator &i, std::string::const_iterator end, PShape& pshape) {
   parseWhiteSpace( i, end );
   if (*i == 'c') {
      i++;
      float x2,y2,x3,y3,x4,y4;
      parseFloat( i, end,x2);
      if ( *i == ',') i++;
      parseFloat( i, end,y2);
      if ( *i == ',') i++;
      parseFloat( i, end,x3);
      if ( *i == ',') i++;
      parseFloat( i, end,y3);
      if ( *i == ',') i++;
      parseFloat( i, end,x4);
      if ( *i == ',') i++;
      parseFloat( i, end,y4);

      pshape.bezierVertex(lx+x2,ly+y2,lx+x3,ly+y3,lx+x4,ly+y4);
      cx = lx+x3;
      cy = ly+y3;
      lx = lx+x4;
      ly = ly+y4;
   }
   else if (*i == 'C') {
      i++;
      float x2,y2,x3,y3,x4,y4;
      parseFloat( i, end,x2);
      if ( *i == ',') i++;
      parseFloat( i, end,y2);
      if ( *i == ',') i++;
      parseFloat( i, end,x3);
      if ( *i == ',') i++;
      parseFloat( i, end,y3);
      if ( *i == ',') i++;
      parseFloat( i, end,x4);
      if ( *i == ',') i++;
      parseFloat( i, end,y4);

      pshape.bezierVertex(x2,y2,x3,y3,x4,y4);
      cx = x3;
      cy = y3;
      lx = x4;
      ly = y4;
   }
   else if (*i == 's') {
      i++;
      float x2,y2,x3,y3,x4,y4;
      x2 = (lx - cx);
      y2 = (ly - cy);
      parseFloat( i,end,x3);
      if ( *i == ',') i++;
      parseFloat( i, end,y3);
      if ( *i == ',') i++;
      parseFloat( i, end,x4);
      if ( *i == ',') i++;
      parseFloat( i, end,y4);

      pshape.bezierVertex(lx+x2,ly+y2,lx+x3,ly+y3,lx+x4,ly+y4);
      cx = lx+x3;
      cy = ly+y3;
      lx = lx+x4;
      ly = ly+y4;
   }
   else if (*i == 'S') {
      i++;
      float x2,y2,x3,y3,x4,y4;
      x2 = lx - (cx - lx);
      y2 = ly - (cy - ly);
      parseFloat( i, end,x3);
      if ( *i == ',') i++;
      parseFloat( i, end,y3);
      if ( *i == ',') i++;
      parseFloat( i, end,x4);
      if ( *i == ',') i++;
      parseFloat( i, end,y4);

      pshape.bezierVertex(x2,y2,x3,y3,x4,y4);
      cx = x3;
      cy = y3;
      lx = x4;
      ly = y4;
   }
}

static void parseText(std::string::const_iterator &i, std::string::const_iterator end, std::string_view text) {
   parseWhiteSpace( i, end );
   for( char x : text ) {
      if ( i == end )
         abort();
      if (x != *i++) abort();
   }
}

static void parseSVGTransform(const std::string &data, PShape &pshape,
                              PMatrix &transform) {
   auto i = data.begin();
   auto end = data.end();
   parseText(i ,end, "matrix(");
   float x1,x2,x3,x4,x5,x6;
   parseFloat( i,end,x1);
   if ( *i == ',') i++;
   parseFloat( i,end,x2);
   if ( *i == ',') i++;
   parseFloat( i,end,x3);
   if ( *i == ',') i++;
   parseFloat( i,end,x4);
   if ( *i == ',') i++;
   parseFloat( i,end,x5);
   if ( *i == ',') i++;
   parseFloat( i,end,x6);
   parseText(i ,end, ")");

   transform = PMatrix{ glm::mat4( x1, x2, 0, 0,
                                   x3, x4, 0, 0,
                                    0,  0, 1, 0,
                                   x5, x6, 0, 1 ) };
}

static void parseSVGFillColor(const std::string &data, PShape &pshape, int alpha) {
   if (data != "none") {
      std::string R( data.begin()+1, data.begin()+3 );
      std::string G( data.begin()+3, data.begin()+5 );
      std::string B( data.begin()+5, data.begin()+7 );
      int iR = std::stoi(R,nullptr,16);
      int iG = std::stoi(G,nullptr,16);
      int iB = std::stoi(B,nullptr,16);
      pshape.fill(iR,iG,iB, alpha );
   } else {
      pshape.noFill();
   }
}

static void parseSVGStrokeColor(const std::string &data, PShape &pshape, int alpha) {
   if (data != "none") {
      std::string R( data.begin()+1, data.begin()+3 );
      std::string G( data.begin()+3, data.begin()+5 );
      std::string B( data.begin()+5, data.begin()+7 );
      pshape.stroke( std::stoi(R,nullptr,16), std::stoi(G,nullptr,16), std::stoi(B,nullptr,16), alpha );
   } else {
      pshape.noStroke();
   }
}

static void parseSVGStrokeWeight(const std::string &data, PShape &pshape) {
   pshape.strokeWeight( std::stof( data ) );
}

static void parseSVGOpacity(const std::string &data, int &alpha) {
   alpha = std::stof( data ) * 255;
}

static void parseSVGID(const std::string &data, PShape& pshape) {
   pshape.setID( data );
}

static void parseSVGPath(const std::string &data, PShape& pshape) {
   std::string::const_iterator i = data.begin();
   std::string::const_iterator end = data.end();
   pshape.beginShape();

   parseWhiteSpace(i, end) ;
   bool open = true;
   while ( i != end && *i == 'M') {
      pshape.beginContour();
      i++;
      float x;
      parseFloat( i, end, x );
      if ( *i == ',') i++;
      float y;
      parseFloat( i, end, y );
      pshape.vertex(x,y);
      lx = x;
      ly = y;
      parseWhiteSpace(i, end);
      while ( i != end && (*i == 'c' || *i == 'C' || *i == 's' || *i == 'S'|| *i == 'l' || *i == 'L'
             || *i == 'h' || *i == 'H'|| *i == 'v' || *i == 'V') ) {
         parseCurve(i, end,pshape );
         parseLine( i, end, pshape );
         parseWhiteSpace(i, end);
      }
      if (i != end && ( *i == 'z' || *i == 'Z') )
      {
         open = false;
         pshape.endContour( );
	 i++;
      }
      else
      {
         open = true;
         pshape.endContour();
      }
      parseWhiteSpace(i, end);
   }
   if (open) {
      pshape.endShape( OPEN );
   } else {
      pshape.endShape( CLOSE );
   }
}

static void parseSVGPolygon(const std::string &data, PShape& pshape) {
   std::string::const_iterator i = data.begin();
   std::string::const_iterator end = data.end();
   pshape.beginShape();
   parseWhiteSpace(i, end);
   while ( i != data.end() ) {
      float x;
      parseFloat( i, end, x );
      if ( *i == ',') i++;
      float y;
      parseFloat( i, end, y );
      pshape.vertex(x,y);
      parseWhiteSpace(i, end);
   }
   pshape.endShape(CLOSE);
}

static void parseNode(xmlNode* node, PShape& pshape) {

   std::string type;
   std::string data;

   if (node == nullptr)
      return;

   if (node->type == XML_ELEMENT_NODE) {
      type = (char*)node->name;

      if (type == "svg") {
         xmlChar* xdata = xmlGetProp(node, (xmlChar*)"width");
         float width, height;
         if (xdata) {
            auto x = std::string((char*)xdata);
            auto y = x.cbegin();
            auto end = x.cend();
            parseFloat(y, end, width);
            pshape.width = width;
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"height");
         if (xdata) {
            auto x = std::string((char*)xdata);
            auto y = x.cbegin();
            auto end = x.cend();
            parseFloat(y, end,height);
            pshape.height = height;
         }
         xmlFree(xdata);

         for (xmlNode* child = node->children; child; child = child->next) {
            parseNode(child, pshape);
         }
         return;
      }

      PShape shape = createShape();;
      shape.beginShape();
      shape.fill(BLACK);
      shape.noStroke();

      if (type == "circle") {
         int alpha = 255;
         xmlChar* xdata = xmlGetProp(node, (xmlChar*)"opacity");
         if (xdata) {
            parseSVGOpacity((char*)xdata, alpha);
         }
         xmlFree(xdata);
         float cx, cy, r;
         xdata = xmlGetProp(node, (xmlChar*)"cx");
         if (xdata) {
            auto x = std::string((char*)xdata);
            auto y = x.cbegin();
            auto end = x.cend();
            parseFloat(y, end, cx);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"cy");
         if (xdata) {
            auto x = std::string((char*)xdata);
            auto y = x.cbegin();
            auto end = x.cend();
            parseFloat(y, end, cy);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"r");
         if (xdata) {
            auto x = std::string((char*)xdata);
            auto y = x.cbegin();
            auto end = x.cend();
            parseFloat(y,end, r);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"fill");
         if (xdata) {
            parseSVGFillColor((char*)xdata, shape, alpha);
         }
         xmlFree(xdata);
         shape = drawUntexturedFilledEllipse( cx,cy,2*r,2*r, shape.getFillColor(), PMatrix::Identity() );
      } else if (type == "ellipse") {
         int alpha = 255;
         xmlChar* xdata = xmlGetProp(node, (xmlChar*)"opacity");
         if (xdata) {
            parseSVGOpacity((char*)xdata, alpha);
         }
         xmlFree(xdata);
         float cx, cy, rx,ry;
         xdata = xmlGetProp(node, (xmlChar*)"cx");
         if (xdata) {
            auto x = std::string((char*)xdata);
            auto y = x.cbegin();
            auto end = x.cend();
            parseFloat(y, end,cx);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"cy");
         if (xdata) {
            auto x = std::string((char*)xdata);
            auto y = x.cbegin();
            auto end = x.cend();
            parseFloat(y,end,  cy);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"rx");
         if (xdata) {
            auto x = std::string((char*)xdata);
            auto y = x.cbegin();
            auto end = x.cend();
            parseFloat(y,end, rx);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"ry");
         if (xdata) {
            auto x = std::string((char*)xdata);
            auto y = x.cbegin();
            auto end = x.cend();
            parseFloat(y, end, ry);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"fill");
         if (xdata) {
            parseSVGFillColor((char*)xdata, shape, alpha);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"transform");
         PMatrix transform;
         if (xdata) {
            parseSVGTransform((char*)xdata, shape, transform);
         }
         xmlFree(xdata);
         shape = drawUntexturedFilledEllipse( cx,cy,2*rx,2*ry, shape.getFillColor(), transform );
      } else if (type == "polygon") {
         int alpha = 255;
         xmlChar* xdata = xmlGetProp(node, (xmlChar*)"opacity");
         if (xdata) {
            parseSVGOpacity((char*)xdata, alpha);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"fill");
         if (xdata) {
            parseSVGFillColor((char*)xdata, shape,alpha);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"stroke");
         if (xdata) {
            parseSVGStrokeColor((char*)xdata, shape,alpha);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"stroke-width");
         if (xdata) {
            parseSVGStrokeWeight((char*)xdata, shape);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"points");
         if (xdata) {
            parseSVGPolygon((char*)xdata, shape);
         }
         xmlFree(xdata);
      } else if (type == "path") {
         int alpha = 255;
         xmlChar* xdata = xmlGetProp(node, (xmlChar*)"opacity");
         if (xdata) {
            parseSVGOpacity((char*)xdata, alpha);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"id");
         if (xdata) {
            parseSVGID((char*)xdata, shape);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"fill");
         if (xdata) {
            parseSVGFillColor((char*)xdata, shape,alpha);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"stroke");
         if (xdata) {
            parseSVGStrokeColor((char*)xdata, shape,alpha);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"stroke-width");
         if (xdata) {
            parseSVGStrokeWeight((char*)xdata, shape);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"d");
         if (xdata) {
            parseSVGPath((char*)xdata, shape);
         }
         xmlFree(xdata);
      } else {
         shape.beginShape( GROUP );
         for (xmlNode* child = node->children; child; child = child->next) {
            parseNode(child, shape);
         }
         shape.endShape();
      }
      pshape.addChild( shape );
   }
}

PShape loadShapeSVG(std::string_view filename) {
   LIBXML_TEST_VERSION;

   using namespace std::string_literals;
   xmlDoc* doc = xmlReadFile(("data/"s+std::string(filename)).c_str(), nullptr, 0);
   if (doc == nullptr) {
      std::cerr << "Error loading SVG file." << std::endl;
      return createShape();
   }

   PShape svgShape = createShape();
   svgShape.beginShape(GROUP);
   parseNode(xmlDocGetRootElement(doc), svgShape);
   svgShape.endShape();

   xmlFreeDoc(doc);
   xmlCleanupParser();
   return svgShape;
}
