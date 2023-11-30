#ifndef PROCESSING_PSHADER_BUILDER_H
#define PROCESSING_PSHADER_BUILDER_H

#include <memory>
#include <vector>
#include <set>
#include <fmt/core.h>

struct GLSL_Type {
   enum type_ {
      MAT4,
      MAT3,
      FLOAT,
      INT,
      VEC3,
      VEC2,
      VEC4,
      SAMPLER2D
   };
   type_ type;

   std::string_view toString() const {
      switch(type)
      {
      case MAT4:
         return "mat4";
      case MAT3:
         return "mat3";
      case FLOAT:
         return "float";
      case INT:
         return "int";
      case VEC3:
         return "vec3";
      case VEC2:
         return "vec2";
      case VEC4:
         return "vec4";
      case SAMPLER2D:
         return "sampler2D";
      default:
         abort();
      }
   }
};

struct GLSL_Interpolation {
   enum type_ {
      NONE,
      SMOOTH,
      FLAT,
      PERSPECTIVE,
   };
   type_ type;

   std::string_view toString() const {
      switch(type)
      {
      case NONE:
         return "";
      case FLAT:
         return "flat";
      case SMOOTH:
         return "smooth";
      case PERSPECTIVE:
         return "perspective";
      default:
         abort();
      }
   }
};

struct GLSL_Input {
   bool operator<( const GLSL_Input &other ) const {
      return name < other.name;
   }
   GLSL_Type type;
   std::string name;
   GLSL_Interpolation interp;
   std::string declarationLine() const {
      if (interp.type == GLSL_Interpolation::NONE)
         return fmt::format("in {} {};", type.toString(), name);
      else
         return fmt::format("{} in {} {};", interp.toString(), type.toString(), name);
   }
};



struct GLSL_Expr;
struct GLSL_Statement;

struct GLSL_Output {
   bool operator<( const GLSL_Output &other ) const {
      return name < other.name;
   }
   GLSL_Type type;
   std::string name;
   GLSL_Interpolation interp;
   std::string declarationLine() const {
      if (interp.type == GLSL_Interpolation::NONE)
         return fmt::format("out {} {};", type.toString(), name);
      else
         return fmt::format("{} out {} {};", interp.toString(), type.toString(), name);
   }
   GLSL_Expr operator=(const GLSL_Expr &input);
};


struct GLSL_Uniform {
   bool operator<( const GLSL_Uniform &other ) const {
      return name < other.name;
   }
   GLSL_Type type;
   std::string name;
   int dimension; // zero mean not an array, or maybe 1 does
   std::string declarationLine() const {
      if (dimension > 1)
         return fmt::format("uniform {} {}[{}];", type.toString(), name, dimension);
      else
         return fmt::format("uniform {} {};", type.toString(), name);
   }


};

template <typename T>
std::set<T> operator+(const std::set<T>& set1, const std::set<T>& set2) {
    std::set<T> result = set1; // Copy set1

    // Insert elements from set2 into the result set
    result.insert(set2.begin(), set2.end());

    return result;
}

struct GLSL_Function {
   std::string name;
   GLSL_Expr operator()(const GLSL_Expr &a);
   GLSL_Expr operator()(const GLSL_Expr &a, const GLSL_Expr &b);
   GLSL_Expr operator()(const GLSL_Expr &a, const GLSL_Expr &b, const GLSL_Expr &c);
   GLSL_Expr operator()(const GLSL_Expr &a, const GLSL_Expr &b, const GLSL_Expr &c, const GLSL_Expr &d);
};

struct GLSL_Expr {
   std::set<GLSL_Input> uses;
   std::set<GLSL_Output> produces;
   std::set<GLSL_Uniform> uniforms;
   std::string print_;

   GLSL_Expr(   std::set<GLSL_Input> uses_,
                std::set<GLSL_Output> produces_,
                std::set<GLSL_Uniform> uniforms_,
                std::string print ) {
      uses = uses_;
      produces = produces_;
      uniforms = uniforms_;
      print_= print;
   };
   GLSL_Expr() {}

   std::string print() const {
      return print_;
   }

   template<typename... Args>
   static GLSL_Expr Function(const GLSL_Function &f, Args&&... args) {
      GLSL_Expr r;
      r.print_ = fmt::format("{}(", f.name);

      // Expand the argument pack using a fold expression
      ((r.uses = r.uses + args.uses,
        r.uniforms = r.uniforms + args.uniforms,
        r.print_ = r.print_ + args.print() + ","), ...);

      r.print_.pop_back();
      r.print_ = r.print_ + fmt::format(")");
      return r;
   }
   static GLSL_Expr Value(float a) {
      return { {},{},{}, fmt::format("{}",a)};
   }
   static GLSL_Expr vec3tovec4(const GLSL_Expr &a) {
      return { {a.uses}, {}, {}, fmt::format("vec4({},1.0)",a.print())};
   }
   static GLSL_Expr Equals(const GLSL_Output &a, const GLSL_Expr &b) {
      return { {b.uses},{a},{b.uniforms}, fmt::format("{} = {};", a.name, b.print()) };
   }
   static GLSL_Expr Multiply(const GLSL_Expr &a, const GLSL_Expr &b) {
      return { {a.uses + b.uses}, {}, {a.uniforms + b.uniforms}, fmt::format("( {} * {} )",a.print(), b.print()) };
   }
   static GLSL_Expr Add(const GLSL_Expr &a, const GLSL_Expr &b) {
      return { {a.uses + b.uses}, {}, {a.uniforms + b.uniforms}, fmt::format("( {} + {} )",a.print(), b.print()) };
   }
   static GLSL_Expr Input(const GLSL_Input &a) {
      return { {a},{},{}, fmt::format("{}",a.name) };
   }
   static GLSL_Expr Uniform(const GLSL_Uniform &a) {
      return { {},{},{a}, fmt::format("{}",a.name) };
   }
   GLSL_Expr(const GLSL_Input &i) {
      *this = Input(i);
   }
   GLSL_Expr(const GLSL_Uniform &i) {
      *this = Uniform(i);
   }
   GLSL_Expr(float a) {
      *this = Value(a);
   }
};

inline GLSL_Expr GLSL_Function::operator()(const GLSL_Expr &a) {
   return GLSL_Expr::Function(*this, a);
}
inline GLSL_Expr GLSL_Function::operator()(const GLSL_Expr &a, const GLSL_Expr &b) {
   return GLSL_Expr::Function(*this, a,b);
}
inline GLSL_Expr GLSL_Function::operator()(const GLSL_Expr &a, const GLSL_Expr &b, const GLSL_Expr &c) {
   return GLSL_Expr::Function(*this, a,b,c);
}
inline GLSL_Expr GLSL_Function::operator()(const GLSL_Expr &a, const GLSL_Expr &b, const GLSL_Expr &c, const GLSL_Expr &d) {
   return GLSL_Expr::Function(*this, a,b,c,d);
}


inline GLSL_Expr operator*(const GLSL_Expr &a, const GLSL_Expr &b) {
   return GLSL_Expr::Multiply(a,b);
}

inline GLSL_Expr operator+(const GLSL_Expr &a, const GLSL_Expr &b) {
   return GLSL_Expr::Add(a,b);
}

inline GLSL_Expr GLSL_Output::operator=(const GLSL_Expr &input) {
   return GLSL_Expr::Equals(*this,input);
}

struct Shader {
   static constexpr const char *version = "#version 400\n";
   static constexpr const char *main_preamble = "void main() {\n";
   static constexpr const char *main_postamble = "}\n";
   std::vector<GLSL_Expr> code;
   std::string print() {
      std::set<GLSL_Input> inputs;
      std::set<GLSL_Output> outputs;
      std::set<GLSL_Uniform> uniforms;

      std::string shader = version;

      for (auto &c : code ) {
         inputs.merge(c.uses);
         outputs.merge(c.produces);
         uniforms.merge(c.uniforms);
      }

      for (auto &u : inputs) {
         shader += fmt::format("{}\n", u.declarationLine() );
      }
      for (auto &u : uniforms) {
         shader += fmt::format("{}\n", u.declarationLine());
      }
      for (auto &u : outputs) {
         if (u.name != "gl_Position")
            shader += fmt::format("{}\n", u.declarationLine());
      }

      shader += fmt::format("{}", main_preamble);
      for (auto &c : code ) {
         shader += fmt::format("    {}\n", c.print());
      }
      shader += fmt::format("{}", main_postamble);
      return shader;
   }
};

#endif
