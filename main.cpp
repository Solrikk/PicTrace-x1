#include <QApplication>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMimeData> // Add this line
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>
#include <filesystem>
#include <iomanip>
#include <opencv2/opencv.hpp>
#include <vector>

namespace fs = std::filesystem;

class DropArea : public QLabel {
  Q_OBJECT
public:
  DropArea(QWidget *parent = nullptr) : QLabel(parent) {
    setAlignment(Qt::AlignCenter);
    setText("Перетащите фото сюда\nили нажмите для выбора");
    setMinimumSize(300, 200);
    setFrameStyle(QFrame::Box);
    setAcceptDrops(true);
  }

protected:
  void dragEnterEvent(QDragEnterEvent *event) override {
    if (event->mimeData()->hasUrls())
      event->acceptProposedAction();
  }

  void dropEvent(QDropEvent *event) override {
    QString file = event->mimeData()->urls().first().toLocalFile();
    emit imageDropped(file);
  }

  void mousePressEvent(QMouseEvent *event) override {
    QString file = QFileDialog::getOpenFileName(
        this, "Выберите фото", QString(), "Images (*.jpg *.jpeg *.png)");
    if (!file.isEmpty())
      emit imageDropped(file);
  }

signals:
  void imageDropped(const QString &path);
};

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow() {
    QWidget *centralWidget = new QWidget;
    QGridLayout *layout = new QGridLayout(centralWidget);

    dropArea = new DropArea;
    connect(dropArea, &DropArea::imageDropped, this, &MainWindow::processImage);

    resultsArea = new QWidget;
    resultsLayout = new QGridLayout(resultsArea);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidget(resultsArea);
    scrollArea->setWidgetResizable(true);

    layout->addWidget(dropArea, 0, 0);
    layout->addWidget(scrollArea, 1, 0);

    setCentralWidget(centralWidget);
    resize(800, 600);
    setWindowTitle("PicTrace-x1");

    extractPhotos();
  }

private:
  void extractPhotos() {
    (void)system("mkdir -p ./extracted_photos");               // Change here
    (void)system("unzip -o photos.zip -d ./extracted_photos"); // Change here

    for (const auto &entry : fs::directory_iterator("./extracted_photos")) {
      if (entry.path().extension() == ".jpg" ||
          entry.path().extension() == ".jpeg" ||
          entry.path().extension() == ".png") {
        imageFiles.push_back(entry.path().string());
      }
    }
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

  void processImage(const QString &path) {
    clearResults();

    cv::Mat targetImg = cv::imread(path.toStdString());
    if (targetImg.empty()) {
      dropArea->setText("Ошибка загрузки изображения");
      return;
    }

    QPixmap preview = QPixmap(path).scaled(300, 200, Qt::KeepAspectRatio);
    dropArea->setPixmap(preview);

    int row = 0;
    for (const auto &photo : imageFiles) {
      cv::Mat compareImg = cv::imread(photo);
      if (compareImg.empty())
        continue;

      double similarity = compareImages(targetImg, compareImg);
      if (similarity > 0.50) {
        QLabel *imageLabel = new QLabel;
        QPixmap pixmap(QString::fromStdString(photo));
        imageLabel->setPixmap(pixmap.scaled(200, 150, Qt::KeepAspectRatio));

        QLabel *similarityLabel = new QLabel(
            QString("Схожесть: %1%").arg(similarity * 100, 0, 'f', 2));

        resultsLayout->addWidget(imageLabel, row, 0);
        resultsLayout->addWidget(similarityLabel, row, 1);
        row++;
      }
    }
  }

  void clearResults() {
    QLayoutItem *child;
    while ((child = resultsLayout->takeAt(0)) != nullptr) {
      delete child->widget();
      delete child;
    }
  }

private:
  DropArea *dropArea;
  QWidget *resultsArea;
  QGridLayout *resultsLayout;
  std::vector<std::string> imageFiles;
};

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  MainWindow window;
  window.show();
  return app.exec();
}

#include "main.moc"
