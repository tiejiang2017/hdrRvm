#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QApplication>
#include <QMainWindow>
#include <opencv2/opencv.hpp>
#include <OpenNI.h>
#include <QTimer>
#include<qthread.h>
#include <QImage>
#include<QFile>


using namespace std;
using namespace cv;
using namespace openni;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int Exp1,Exp2,Exp3,Exp4;
    void calcHist(Mat srcImage);
    void showOutLabel(Mat inSrc,int labelIndex);
    void getRgb(int index,int exp);
    void save();

public slots:
    void onTimeOut();
private slots:
    void on_OpenBtn_clicked();


    void on_saveBtn_clicked();

private:
    Ui::MainWindow *ui;
    openni::Device device;
    VideoStream rgbStream;
    VideoFrameRef rgbFrame;
    Mat rgbMat1,rgbMat2,rgbMat3,rgbMat4;
    bool bsave;
    QTimer *timer;
};

#endif // MAINWINDOW_H
