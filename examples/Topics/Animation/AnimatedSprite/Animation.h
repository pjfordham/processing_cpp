// Class for animating a sequence of GIFs

class Animation {
public:
  std::vector<PImage> images;
  int imageCount;
  int frame;

  Animation() {}

  Animation(std::string imagePrefix, int count) {
    imageCount = count;
    frame = 0;
    for (int i = 0; i < imageCount; i++) {
      // Use nf() to number format 'i' into four digits
      std::string filename = imagePrefix + nf(i, 4) + ".gif";
      fmt::print("{}\n", filename);
      images.emplace_back( loadImage(filename) );
    }
  }

  void display(float xpos, float ypos) {
    frame = (frame+1) % imageCount;
    image(images[frame], xpos, ypos);
  }

  int getWidth() {
    return images[0].width;
  }
};
