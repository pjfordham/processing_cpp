/**
 * Listing files in directories and subdirectories
 * by Daniel Shiffman.
 *
 * This example has three functions:<br />
 * 1) List the names of files in a directory<br />
 * 2) List the names along with metadata (size, lastModified)<br />
 *    of files in a directory<br />
 * 3) List the names along with metadata (size, lastModified)<br />
 *    of files in a directory and all subdirectories (using recursion)
 */

std::vector<std::string> listFileNames(std::string path);
std::vector<File> listFilesRecursive(std::string path);
std::vector<File> listFiles(std::string path);

void setup() {

  // Using just the path of this sketch to demonstrate,
  // but you can list any directory you like.
  std::string path = sketchPath();

  fmt::print("Listing all filenames in a directory: \n");
  std::vector<std::string> filenames = listFileNames(path);
  printArray(filenames);

  fmt::print("\nListing info about all files in a directory: \n");
  std::vector<File> files = listFiles(path);
  for (int i = 0; i < files.size(); i++) {
    File f = files[i];
    fmt::print("Name: {}\n", f.getName());
    fmt::print("Is directory: {}\n", f.isDirectory());
    fmt::print("Size: {}\n", f.length());
    auto time = f.lastModified();
    fmt::print("Last Modified: {}\n", std::asctime(std::localtime(&time)));
    fmt::print("-----------------------\n");
  }

  fmt::print("\nListing info about all files in a directory and all subdirectories: \n");
  std::vector<File> allFiles = listFilesRecursive(path);

  for (File f : allFiles) {
    fmt::print("Name: {}\n", f.getName());
    fmt::print("Full path: {}\n", f.getAbsolutePath());
    fmt::print("Is directory: {}\n", f.isDirectory());
    fmt::print("Size: {}\n", f.length());
    auto time = f.lastModified();
    fmt::print("Last Modified: {}\n", std::asctime(std::localtime(&time)));
    fmt::print("-----------------------\n");
  }

  noLoop();
}

// Nothing is drawn in this program and the draw() doesn't loop because
// of the noLoop() in setup()
void draw() {
}

// This function returns all the files in a directory as an array of Strings
std::vector<std::string> listFileNames(std::string dir) {
  File file(dir);
  if (file.isDirectory()) {
     std::vector<std::string> names = file.list();
    return names;
  } else {
    // If it's not a directory
    return {};;
  }
}

// This function returns all the files in a directory as an array of File objects
// This is useful if you want more info about the file
std::vector<File> listFiles(std::string dir) {
  File file(dir);
  if (file.isDirectory()) {
     std::vector<File> files = file.listFiles();
    return files;
  } else {
    // If it's not a directory
     return {};
  }
}

void recurseDir(std::vector<File> &a, std::string dir);

// Function to get a list of all files in a directory and all subdirectories
std::vector<File> listFilesRecursive(std::string dir) {
  std::vector<File> fileList;
  recurseDir(fileList, dir);
  return fileList;
}

// Recursive function to traverse subdirectories
void recurseDir(std::vector<File> &a, std::string dir) {
  File file(dir);
  if (file.isDirectory()) {
    // If you want to include directories in the list
    a.push_back(file);
    std::vector<File> subfiles = file.listFiles();
    for (int i = 0; i < subfiles.size(); i++) {
      // Call this function on all files in this directory
      recurseDir(a, subfiles[i].getAbsolutePath());
    }
  } else {
    a.push_back(file);
  }
}
