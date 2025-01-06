
#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <vector>
#include <cstdlib>

namespace fs = std::filesystem;

void extractZipArchive(const std::string& zipPath) {
    std::string cmd = "unzip -o " + zipPath + " -d ./extracted_photos";
    if (system(cmd.c_str()) != 0) {
        std::cerr << "Error extracting zip file" << std::endl;
        return;
    }
}

// Calculate image similarity using histogram comparison
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

int main() {
    std::string zipPath = "photos.zip";
    extractZipArchive(zipPath);
    std::string folderPath = "./extracted_photos";
    
    std::vector<std::string> imageFiles;
    
    // Collect image files from directory
    for(const auto& entry : fs::directory_iterator(folderPath)) {
        if(entry.path().extension() == ".jpg" || 
           entry.path().extension() == ".png" ||
           entry.path().extension() == ".jpeg") {
            imageFiles.push_back(entry.path().string());
        }
    }
    
    // Compare all images
    for(size_t i = 0; i < imageFiles.size(); i++) {
        cv::Mat img1 = cv::imread(imageFiles[i]);
        if(img1.empty()) continue;
        
        for(size_t j = i + 1; j < imageFiles.size(); j++) {
            cv::Mat img2 = cv::imread(imageFiles[j]);
            if(img2.empty()) continue;
            
            double similarity = compareImages(img1, img2);
            if(similarity > 0.85) { // Threshold for similarity
                std::cout << "Similar images found:\n";
                std::cout << imageFiles[i] << "\n";
                std::cout << imageFiles[j] << "\n";
                std::cout << "Similarity: " << similarity * 100 << "%\n\n";
            }
        }
    }
    
    return 0;
}
