class Record {
public:
  std::string name;
  float mpg;
  int cylinders;
  float displacement;
  float horsepower;
  float weight;
  float acceleration;
  int year;
  float origin;

  float stof( const std::string &s ) {
     try {
        return std::stof(s);
     } catch( ... ) {
        return {};
     }
  }

  int stoi( const std::string &s ) {
     try {
        return std::stoi(s);
     } catch( ... ) {
        return {};
     }
  }

  Record() {}

  Record(const std::vector<std::string> &pieces) :
     name( pieces[0] ),
     mpg( stof(pieces[1]) ),
     cylinders( stoi(pieces[2]) ),
     displacement( stof(pieces[3]) ),
     horsepower( stof(pieces[4]) ),
     weight( stof(pieces[5]) ),
     acceleration( stof(pieces[6]) ),
     year( stoi(pieces[7]) ),
     origin( stof(pieces[8]) ) {
  }
};
