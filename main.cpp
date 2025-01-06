#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

namespace fs = std::filesystem;

void clearScreen() { std::cout << "\033[2J\033[1;1H"; }

void printHeader() {
  std::cout << "\033[1;36m";
  std::cout << "╔════════════════════════════════════════╗\n";
  std::cout << "║               PicTrace-x1              ║\n";
  std::cout << "╚════════════════════════════════════════╝\n";
  std::cout << "\033[0m";
}

void printMenu() {
  std::cout << "\n\033[1;33m";
  std::cout << "Menu Options:\n";
  std::cout << "════════════\n";
  std::cout << "\033[0m";
  std::cout << "1. 🔍 Compare photos\n";
  std::cout << "2. 📋 List all photos\n";
  std::cout << "3. 🚪 Exit\n\n";
  std::cout << "\033[1;32mChoose option (1-3):\033[0m ";
}

void extractZipArchive(const std::string &zipPath) {
  if (!fs::exists(zipPath)) {
    std::cerr << "\033[1;31mError: " << zipPath << " not found\033[0m"
              << std::endl;
    return;
  }

  if (system("mkdir -p ./extracted_photos") != 0) {
    std::cerr << "\033[1;31mError creating directory\033[0m" << std::endl;
    return;
  }

  std::string cmd = "unzip -o " + zipPath + " -d ./extracted_photos";
  if (system(cmd.c_str()) != 0) {
    std::cerr << "\033[1;31mError extracting zip file\033[0m" << std::endl;
    return;
  }

  std::cout << "\033[1;32mPhotos extracted successfully\033[0m" << std::endl;
}

double compareImages(const cv::Mat &img1, const cv::Mat &img2) {
  cv::Mat hist1, hist2;
  int channels[] = {0, 1, 2};
  int histSize[] = {8, 8, 8};
  float range[] = {0, 256};
  const float *ranges[] = {range, range, range};

  cv::calcHist(&img1, 1, channels, cv::Mat(), hist1, 3, histSize, ranges);
  cv::calcHist(&img2, 1, channels, cv::Mat(), hist2, 3, histSize, ranges);

  cv::normalize(hist1, hist1);
  cv::normalize(hist2, hist2);

  return cv::compareHist(hist1, hist2, cv::HISTCMP_CORREL);
}

void findSimilarPhotos(const std::string &targetPhotoPath,
                       const std::vector<std::string> &allPhotos) {
  cv::Mat targetImg = cv::imread(targetPhotoPath);
  if (targetImg.empty()) {
    std::cout << "\033[1;31mError: Cannot open target image\033[0m\n";
    return;
  }

  std::cout << "\n\033[1;34mSearching for similar photos...\033[0m\n\n";
  bool foundSimilar = false;
  int count = 0;

  for (const auto &photo : allPhotos) {
    if (photo == targetPhotoPath)
      continue;

    cv::Mat compareImg = cv::imread(photo);
    if (compareImg.empty())
      continue;

    double similarity = compareImages(targetImg, compareImg);
    if (similarity > 0.85) {
      std::cout << "\033[1;32mSimilar photo " << ++count << ":\033[0m\n";
      std::cout << "📸 " << fs::path(photo).filename() << "\n";
      std::cout << "✨ Similarity: " << std::fixed << std::setprecision(2)
                << similarity * 100 << "%\n\n";
      foundSimilar = true;
    }
  }

  if (!foundSimilar) {
    std::cout << "\033[1;33mNo similar photos found.\033[0m\n";
  }
}

void listAvailablePhotos(const std::vector<std::string> &imageFiles) {
  std::cout << "\n\033[1;36mAvailable photos:\033[0m\n";
  std::cout << "════════════════\n";
  int count = 0;
  for (const auto &file : imageFiles) {
    std::cout << "📸 " << fs::path(file).filename() << "\n";
    if (++count % 5 == 0)
      std::cout << "\n";
  }
  std::cout << "\n";
}

int main() {
  std::string zipPath = "photos.zip";
  extractZipArchive(zipPath);
  std::string folderPath = "./extracted_photos";

  std::vector<std::string> imageFiles;
  for (const auto &entry : fs::directory_iterator(folderPath)) {
    if (entry.path().extension() == ".jpg" ||
        entry.path().extension() == ".png" ||
        entry.path().extension() == ".jpeg") {
      imageFiles.push_back(entry.path().string());
    }
  }

  while (true) {
    clearScreen();
    printHeader();
    printMenu();

    int choice;
    std::cin >> choice;
    std::cin.ignore();

    if (choice == 1) {
      std::cout << "\n\033[1;36mEnter photo name (with extension, e.g. "
                   "photo.jpg):\033[0m ";
      std::string photoName;
      std::getline(std::cin, photoName);
      std::string fullPath = folderPath + "/" + photoName;

      if (fs::exists(fullPath)) {
        findSimilarPhotos(fullPath, imageFiles);
      } else {
        std::cout << "\033[1;31mError: Photo not found\033[0m\n";
      }
    } else if (choice == 2) {
      listAvailablePhotos(imageFiles);
    } else if (choice == 3) {
      std::cout << "\n\033[1;32mThank you for using Photo Similarity "
                   "Finder!\033[0m\n";
      break;
    } else {
      std::cout << "\033[1;31mInvalid option. Please try again.\033[0m\n";
    }

    if (choice != 3) {
      std::cout << "\n\033[1;33mPress Enter to continue...\033[0m";
      std::cin.get();
    }
  }

  return 0;
}
