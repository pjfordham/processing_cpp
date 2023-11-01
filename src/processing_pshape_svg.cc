#include "processing_pshape_svg.h"
#include "processing_pshape.h"
#include "processing_math.h"
#include "processing_enum.h"

#include <fmt/core.h>
#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>


static bool isDigit(std::string::const_iterator x) {
   return (*x >= '0' && *x <='9')  || *x == '.';
}

static void parseWhiteSpace(std::string::const_iterator &x) {
   while( *x == ' ' || *x == '\n' || *x == '\t') {
      x++;
   }
}

static void parseSign(std::string::const_iterator &end, float &value) {
   if (*end == '-') {
      end++;
      value = -1.0;
   } else {
      value = 1.0;
   }
}

static void parseFloat(std::string::const_iterator &end, float &value) {
   parseWhiteSpace( end );
   auto begin = end;
   parseSign( end, value );
   while( isDigit( end ) ) { end++; }
   std::string x(begin,end);
   value = std::stof(x);
}

float cx, cy, lx, ly;

static void parseMove(std::string::const_iterator &i, PShape& pshape) {
   parseWhiteSpace( i );
   if (*i == 'M') {
      i++;
      float x;
      parseFloat( i, x );
      if ( *i == ',') i++;
      float y;
      parseFloat( i, y );
      pshape.beginShape();
      pshape.vertex(x,y);
      fmt::print("vertex {},{}\n", x,y);
      lx = x;
      ly = y;
   }
}

static void parseLine( std::string::const_iterator &i, PShape &pshape) {
   parseWhiteSpace( i );
   if (*i == 'l') {
      i++;
      float x2,y2;
      parseFloat( i,x2);
      if ( *i == ',') i++;
      parseFloat( i,y2);
      pshape.vertex(lx+x2,ly+y2);
      lx = lx+x2;
      ly = ly+y2;
      fmt::print("vertex relative {},{}\n", x2,y2);
   }
   else if (*i == 'L') {
      i++;
      float x2,y2;
      parseFloat( i,x2);
      if ( *i == ',') i++;
      parseFloat( i,y2);
      pshape.vertex(x2,y2);
      lx = x2;
      ly = y2;
      fmt::print("vertex absolute {},{}\n", x2,y2);
   }
   else if (*i == 'h') {
      i++;
      float x2;
      parseFloat( i,x2);
      pshape.vertex(lx+x2,ly);
      lx = lx + x2;
      ly = ly;
      fmt::print("vertex horizontal relative {}\n", x2);
   }
   else if (*i == 'H') {
      i++;
      float x2;
      parseFloat( i,x2);
      pshape.vertex(x2,ly);
      fmt::print("vertex horizontal absolute {}\n", x2);
      lx = x2;
      ly = ly;
   }
   else if (*i == 'v') {
      i++;
      float y2;
      parseFloat( i,y2);
      pshape.vertex(lx,ly+y2);
      lx = lx;
      ly = ly +y2;
      fmt::print("vertex vertical relative {}\n", y2);
   }
   else if (*i == 'V') {
      i++;
      float y2;
      parseFloat( i,y2);
      pshape.vertex(lx,y2);
      lx = lx;
      ly = y2;
      fmt::print("vertex vertical absolute {}\n", y2);
   }
}

static void parseCurve(std::string::const_iterator &i, PShape& pshape) {
   parseWhiteSpace( i );
   if (*i == 'c') {
      i++;
      float x2,y2,x3,y3,x4,y4;
      parseFloat( i,x2);
      if ( *i == ',') i++;
      parseFloat( i,y2);
      if ( *i == ',') i++;
      parseFloat( i,x3);
      if ( *i == ',') i++;
      parseFloat( i,y3);
      if ( *i == ',') i++;
      parseFloat( i,x4);
      if ( *i == ',') i++;
      parseFloat( i,y4);

      pshape.bezierVertex(lx+x2,ly+y2,lx+x3,ly+y3,lx+x4,ly+y4);
      fmt::print("bezierBertex relative  {},{} {},{} {},{}\n", x2,y2,x3,y3,x4,y4);
      cx = lx+x3;
      cy = ly+y3;
      lx = lx+x4;
      ly = ly+y4;
   }
   else if (*i == 'C') {
      i++;
      float x2,y2,x3,y3,x4,y4;
      parseFloat( i,x2);
      if ( *i == ',') i++;
      parseFloat( i,y2);
      if ( *i == ',') i++;
      parseFloat( i,x3);
      if ( *i == ',') i++;
      parseFloat( i,y3);
      if ( *i == ',') i++;
      parseFloat( i,x4);
      if ( *i == ',') i++;
      parseFloat( i,y4);

      pshape.bezierVertex(x2,y2,x3,y3,x4,y4);
      fmt::print("bezierBertex absolute  {},{} {},{} {},{}\n", x2,y2,x3,y3,x4,y4);
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
      parseFloat( i,x3);
      if ( *i == ',') i++;
      parseFloat( i,y3);
      if ( *i == ',') i++;
      parseFloat( i,x4);
      if ( *i == ',') i++;
      parseFloat( i,y4);

      pshape.bezierVertex(lx+x2,ly+y2,lx+x3,ly+y3,lx+x4,ly+y4);
      fmt::print("bezierBertex shortcut relative  {},{} {},{} {},{}\n", x2,y2,x3,y3,x4,y4);
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
      parseFloat( i,x3);
      if ( *i == ',') i++;
      parseFloat( i,y3);
      if ( *i == ',') i++;
      parseFloat( i,x4);
      if ( *i == ',') i++;
      parseFloat( i,y4);

      pshape.bezierVertex(x2,y2,x3,y3,x4,y4);
      fmt::print("bezierBertex shortcut absolute  {},{} {},{} {},{}\n", x2,y2,x3,y3,x4,y4);
      cx = x3;
      cy = y3;
      lx = x4;
      ly = y4;
   }
}

static void parseSVGFillColor(const std::string &data, PShape &pshape, int alpha) {
   fmt::print("Fill Color {}\n", data);
   if (data != "none") {
      std::string R( data.begin()+1, data.begin()+3 );
      std::string G( data.begin()+3, data.begin()+5 );
      std::string B( data.begin()+5, data.begin()+7 );
      int iR = std::stoi(R,nullptr,16);
      int iG = std::stoi(G,nullptr,16);
      int iB = std::stoi(B,nullptr,16);
      fmt::print("Fill Color {} {} {} {} {} \n", data, iR, iG, iB, alpha);
      pshape.fill(iR,iG,iB, alpha );
   } else {
      pshape.noFill();
   }
}

static void parseSVGStrokeColor(const std::string &data, PShape &pshape, int alpha) {
   fmt::print("Stroke Color {}\n", data);
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
   fmt::print("Stroke Weight {}\n", data);
   pshape.strokeWeight( std::stof( data ) );
}

static void parseSVGOpacity(const std::string &data, int &alpha) {
   fmt::print("Opacity {}\n", data);
   alpha = std::stof( data ) * 255;
}

static void parseSVGPath(const std::string &data, PShape& pshape) {
   std::string::const_iterator i = data.begin();
   if (*i == 'M') {
      i++;
      float x;
      parseFloat( i, x );
      if ( *i == ',') i++;
      float y;
      parseFloat( i, y );
      pshape.beginShape();
      fmt::print("beginshape\n");
      pshape.vertex(x,y);
      lx = x;
      ly = y;
      fmt::print("vertex {},{}\n", x,y);
      parseWhiteSpace(i);
      while (*i == 'c' || *i == 'C' || *i == 's' || *i == 'S'|| *i == 'l' || *i == 'L') {
         parseCurve(i, pshape );
         parseLine( i, pshape );
         parseWhiteSpace(i);
      }
      if (*i == 'z')
      {
         fmt::print("end shape close\n");
         pshape.endShape(CLOSE);
      }
      else
      {
         fmt::print("end shape open {}\n", *i);
         pshape.endShape();
      }

   }
   return;
}

static void parseSVGPolygon(const std::string &data, PShape& pshape) {
   std::string::const_iterator i = data.begin();
   fmt::print("beginshape POLYGON\n");
   pshape.beginShape();
   parseWhiteSpace(i);
   while ( i != data.end() ) {
      float x;
      parseFloat( i, x );
      if ( *i == ',') i++;
      float y;
      parseFloat( i, y );
      pshape.vertex(x,y);
      fmt::print("vertex {},{}\n", x,y);
      parseWhiteSpace(i);
   }
   fmt::print("endshape\n");
   pshape.endShape(CLOSE);
   return;
}

static void parseNode(xmlNode* node, PShape& pshape) {

   std::string type;
   std::string data;

   if (node == nullptr)
      return;

   if (node->type == XML_ELEMENT_NODE) {
      type = (char*)node->name;

      PShape shape;
      fmt::print("PShape start {} {}\n",type,(void*)&shape);
      shape.fill(BLACK);
      shape.noStroke();

      if (type == "svg") {
         xmlChar* xdata = xmlGetProp(node, (xmlChar*)"width");
         float width, height;
         if (xdata) {
            auto x = std::string((char*)xdata);
            auto y = x.cbegin();
            parseFloat(y, width);
            pshape.width = width;
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"height");
         if (xdata) {
            auto x = std::string((char*)xdata);
            auto y = x.cbegin();
            parseFloat(y, height);
            pshape.height = height;
         }
         xmlFree(xdata);
         fmt::print("W{},H{}\n", width,height);
      }

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
            parseFloat(y, cx);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"cy");
         if (xdata) {
            auto x = std::string((char*)xdata);
            auto y = x.cbegin();
            parseFloat(y, cy);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"r");
         if (xdata) {
            auto x = std::string((char*)xdata);
            auto y = x.cbegin();
            parseFloat(y, r);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"fill");
         if (xdata) {
            parseSVGFillColor((char*)xdata, shape, alpha);
         }
         xmlFree(xdata);
         fmt::print("Circle {} {} {}\n",cx,cy,r);
         shape = drawUntexturedFilledEllipse( cx,cy,2*r,2*r, shape.fill_color, PMatrix::Identity() );
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
            parseFloat(y, cx);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"cy");
         if (xdata) {
            auto x = std::string((char*)xdata);
            auto y = x.cbegin();
            parseFloat(y, cy);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"rx");
         if (xdata) {
            auto x = std::string((char*)xdata);
            auto y = x.cbegin();
            parseFloat(y, rx);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"ry");
         if (xdata) {
            auto x = std::string((char*)xdata);
            auto y = x.cbegin();
            parseFloat(y, ry);
         }
         xmlFree(xdata);
         xdata = xmlGetProp(node, (xmlChar*)"fill");
         if (xdata) {
            parseSVGFillColor((char*)xdata, shape, alpha);
         }
         xmlFree(xdata);
         fmt::print("Ellipse {} {} {} {}\n",cx,cy,rx,ry);
         shape = drawUntexturedFilledEllipse( cx,cy,2*rx,2*ry, shape.fill_color, PMatrix::Identity() );
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
         shape.style = GROUP;
         for (xmlNode* child = node->children; child; child = child->next) {
            parseNode(child, shape);
         }
      }
      pshape.addChild( shape );
      if (shape.style == GROUP)
         fmt::print("PShape end GROUP {} {} #{}#\n",(void*)&shape,shape.getChildCount(),type);
      else
         fmt::print("PShape end {} {} \n",type, (void*)&shape);
   }
}

PShape loadShape(const std::string &filename) {
   LIBXML_TEST_VERSION;

   using namespace std::string_literals;
   xmlDoc* doc = xmlReadFile(("data/"s+filename).c_str(), nullptr, 0);
   if (doc == nullptr) {
      std::cerr << "Error loading SVG file." << std::endl;
      return {};
   }

   PShape svgShape;
   svgShape.beginShape(GROUP);
   parseNode(xmlDocGetRootElement(doc), svgShape);
   svgShape.endShape();

   fmt::print("PShape end GROUP {} {} \n",(void*)&svgShape,svgShape.getChildCount());

   xmlFreeDoc(doc);
   xmlCleanupParser();
   return svgShape;
}
