#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "mapbox/pixelmatch.hpp"

struct ImageData {
   unsigned char* data;
   int width;
   int height;
   int channels;

   ImageData() : data(nullptr), width(0), height(0), channels(0) {}

   ~ImageData() {
      if (data) {
         stbi_image_free(data);
      }
   }

   bool load(const std::string& filename) {
      data = stbi_load(filename.c_str(), &width, &height, &channels, 4); // Force RGBA
      if (!data) {
         std::cerr << "Error: Failed to load image " << filename << std::endl;
         std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
         return false;
      }
      channels = 4; // We forced RGBA
      return true;
   }
};

void printUsage(const char* programName) {
   std::cout << "Usage: " << programName << " <image1.png> <image2.png> <output_diff.png> [threshold]\n";
   std::cout << "  threshold: Pixel difference threshold (0.0-1.0), default: 0.1\n";
   std::cout << "Exit codes:\n";
   std::cout << "  0: Images are identical (within threshold)\n";
   std::cout << "  1: Images are different\n";
   std::cout << "  2: Error occurred\n";
}

int main(int argc, char* argv[]) {
   if (argc < 4 || argc > 5) {
      printUsage(argv[0]);
      return 2;
   }

   std::string img1_path = argv[1];
   std::string img2_path = argv[2];
   std::string output_path = argv[3];
   double threshold = 0.1;

   if (argc == 5) {
      try {
         threshold = std::stod(argv[4]);
         if (threshold < 0.0 || threshold > 1.0) {
            std::cerr << "Error: Threshold must be between 0.0 and 1.0" << std::endl;
            return 2;
         }
      } catch (const std::exception& e) {
         std::cerr << "Error: Invalid threshold value" << std::endl;
         return 2;
      }
   }

   // Load images
   ImageData img1, img2;

   if (!img1.load(img1_path)) {
      return 2;
   }

   if (!img2.load(img2_path)) {
      return 2;
   }

   // Check if dimensions match
   if (img1.width != img2.width || img1.height != img2.height) {
      std::cerr << "Error: Images have different dimensions" << std::endl;
      std::cerr << "Image 1: " << img1.width << "x" << img1.height << std::endl;
      std::cerr << "Image 2: " << img2.width << "x" << img2.height << std::endl;
      return 2;
   }

   // Prepare output buffer for difference image
   int pixel_count = img1.width * img1.height;
   std::vector<unsigned char> diff_img(pixel_count * 4); // RGBA


   // Perform pixel comparison
   int diff_pixels = mapbox::pixelmatch(
      img1.data,
      img2.data,
      img1.width,
      img1.height,
      diff_img.data(),
      threshold,
      false);

   // Save the difference image
   if (!stbi_write_png(output_path.c_str(), img1.width, img1.height, 4, diff_img.data(), img1.width * 4)) {
      std::cerr << "Error: Failed to save difference image to " << output_path << std::endl;
      return 2;
   }

   // Calculate difference percentage
   double diff_percentage = (double)diff_pixels / pixel_count * 100.0;

   // Output results
   std::cout << "Comparison complete:" << std::endl;
   std::cout << "Image dimensions: " << img1.width << "x" << img1.height << std::endl;
   std::cout << "Different pixels: " << diff_pixels << " (" << diff_percentage << "%)" << std::endl;
   std::cout << "Threshold: " << threshold << std::endl;
   std::cout << "Difference image saved to: " << output_path << std::endl;

   // Return appropriate exit code
   if (diff_pixels == 0) {
      std::cout << "Images are identical!" << std::endl;
      return 0;
   } else {
      std::cout << "Images are different!" << std::endl;
      return 1;
   }
}
