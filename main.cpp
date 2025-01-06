#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <vector>
#include <cstdlib>

namespace fs = std::filesystem;

void extractZipArchive(const std::string& zipPath) {
    if (system("mkdir -p ./extracted_photos") != 0) {
        std::cerr << "Error creating directory" << std::endl;
        return;
    }
    std::string cmd = "unzip -o " + zipPath + " -d ./extracted_photos";
    if (system(cmd.c_str()) != 0) {
        std::cerr << "Error extracting zip file" << std::endl;
        return;
    }
}

double compareImages(const cv::Mat& img1, const cv::Mat& img2) {
    cv::Mat hist1, hist2;
    int channels[] = {0, 1, 2};
    int histSize[] = {8, 8, 8};
    float range[] = {0, 256};
    const float* ranges[] = {range, range, range};
    
    cv::calcHist(&img1, 1, channels, cv::Mat(), hist1, 3, histSize, ranges);
    cv::calcHist(&img2, 1, channels, cv::Mat(), hist2, 3, histSize, ranges);
    
    cv::normalize(hist1, hist1);
    cv::normalize(hist2, hist2);
    
    return cv::compareHist(hist1, hist2, cv::HISTCMP_CORREL);
}

void findSimilarPhotos(const std::string& targetPhotoPath, const std::vector<std::string>& allPhotos) {
    cv::Mat targetImg = cv::imread(targetPhotoPath);
    if(targetImg.empty()) {
        std::cout << "Error: Cannot open target image\n";
        return;
    }

    std::cout << "\nSearching for similar photos...\n\n";
    bool foundSimilar = false;

    for(const auto& photo : allPhotos) {
        if(photo == targetPhotoPath) continue;

        cv::Mat compareImg = cv::imread(photo);
        if(compareImg.empty()) continue;

        double similarity = compareImages(targetImg, compareImg);
        if(similarity > 0.85) {
            std::cout << "Similar photo found:\n";
            std::cout << photo << "\n";
            std::cout << "Similarity: " << similarity * 100 << "%\n\n";
            foundSimilar = true;
        }
    }

    if(!foundSimilar) {
        std::cout << "No similar photos found.\n";
    }
}

int main() {
    std::string zipPath = "photos.zip";
    extractZipArchive(zipPath);
    std::string folderPath = "./extracted_photos";
    
    std::vector<std::string> imageFiles;
    
    // Collect image files
    for(const auto& entry : fs::directory_iterator(folderPath)) {
        if(entry.path().extension() == ".jpg" || 
           entry.path().extension() == ".png" ||
           entry.path().extension() == ".jpeg") {
            imageFiles.push_back(entry.path().string());
        }
    }

    while(true) {
        std::cout << "\n=== Photo Similarity Finder ===\n";
        std::cout << "1. Enter photo name to compare\n";
        std::cout << "2. List all photos\n";
        std::cout << "3. Exit\n";
        std::cout << "Choose option (1-3): ";

        int choice;
        std::cin >> choice;
        std::cin.ignore();

        if(choice == 1) {
            std::cout << "Enter photo name (with extension, e.g. photo.jpg): ";
            std::string photoName;
            std::getline(std::cin, photoName);
            std::string fullPath = folderPath + "/" + photoName;

            if(fs::exists(fullPath)) {
                findSimilarPhotos(fullPath, imageFiles);
            } else {
                std::cout << "Error: Photo not found\n";
            }
        }
        else if(choice == 2) {
            std::cout << "\nAvailable photos:\n";
            for(const auto& file : imageFiles) {
                std::cout << fs::path(file).filename() << "\n";
            }
        }
        else if(choice == 3) {
            break;
        }
        else {
            std::cout << "Invalid option. Please try again.\n";
        }
    }
    
    return 0;
}
