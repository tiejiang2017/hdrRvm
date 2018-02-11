#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    bsave=false;
    system("mkdir SampleImage");
}
void MainWindow::showOutLabel(Mat inSrc,int labelIndex)
{
    Mat src = inSrc.clone();
    QImage img;
    if(src.type()==CV_8UC3)
    {
        img = QImage(src.data,src.cols,src.rows,src.cols*3,QImage::Format_RGB888);
        if(labelIndex==1)
            ui->labelRGB1->setPixmap(QPixmap::fromImage(img));
        else if(labelIndex==2)
            ui->labelRGB2->setPixmap(QPixmap::fromImage(img));
        else if(labelIndex==3)
            ui->labelRGB3->setPixmap(QPixmap::fromImage(img));
        else if(labelIndex==4)
            ui->labelRGB4->setPixmap(QPixmap::fromImage(img));
        else if(labelIndex==5)
            ui->labelRGB5->setPixmap(QPixmap::fromImage(img));
    }

}
MainWindow::~MainWindow()
{
    timer->stop();
    delete timer;

    rgbStream.stop();
    rgbStream.destroy();


    device.close();
    OpenNI::shutdown();
    delete ui;
}

void MainWindow::on_OpenBtn_clicked()
{
    if(ui->OpenBtn->text()=="open Camera")
    {
        Status rc = STATUS_OK;
        rc = OpenNI::initialize();
        if(rc != STATUS_OK)
        {
            printf("OpenNI::initialize() err...\n");
            return ;
        }
        rc = device.open(openni::ANY_DEVICE);
        if(rc != STATUS_OK)
        {
            printf("device.open() err...\n");
            return ;
        }

        if(device.getSensorInfo(openni::SENSOR_COLOR)!=NULL)
        {
            rc = rgbStream.create(device,openni::SENSOR_COLOR);
            if(rc != STATUS_OK)
            {
                printf("rgbStream.create() err...\n");
                return ;
            }
            VideoMode model;
            model.setFps(30);
            model.setPixelFormat(openni::PIXEL_FORMAT_RGB888);
            model.setResolution(640,480);
            rc = rgbStream.setVideoMode(model);
            if(rc != STATUS_OK)
            {
                printf("rgbStream.setVideoMode() err...\n");
                return ;
            }
        }
        rc = rgbStream.start();
        if(rc != STATUS_OK)
        {
            printf("rgbStream.start() err...\n");
            return ;
        }
        ui->OpenBtn->setText("close Camera");

        timer = new QTimer();
        connect(timer,SIGNAL(timeout()),this,SLOT(onTimeOut()));
        timer->setInterval(30);
        timer->start(30);
        rgbStream.getCameraSettings()->setAutoExposureEnabled(false);
        rgbStream.getCameraSettings()->setAutoWhiteBalanceEnabled(false);
    }
    else
    {
        timer->stop();
        delete timer;
        rgbStream.stop();
        rgbStream.destroy();
        device.close();
        OpenNI::shutdown();

        ui->OpenBtn->setText("open Camera");
    }
}


void MainWindow::getRgb(int index,int exp)
{
    int loop=200;
    Status rc = STATUS_OK;
    if(rgbStream.getCameraSettings()!=NULL)
        rgbStream.getCameraSettings()->setExposure(exp);
    while(loop--)
    {
        QThread::msleep(2);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    rc = rgbStream.readFrame(&rgbFrame);
    if(rc != STATUS_OK)
    {
        printf("rgbStream.readFrame() err..\n");
        return ;
    }
    Mat rgbSrc = Mat(rgbFrame.getHeight(),rgbFrame.getWidth(),CV_8UC3,(void*)rgbFrame.getData()).clone();
    if(rgbSrc.empty())
    {
        printf("rgbSrc.empty()...\n");
        return ;
    }
    if(index==1)rgbMat1 = rgbSrc.clone();
    if(index==2)rgbMat2 = rgbSrc.clone();
    if(index==3)rgbMat3 = rgbSrc.clone();
    if(index==4)rgbMat4 = rgbSrc.clone();
}
void MainWindow::calcHist(Mat srcImage)
{
    double histValue[256]={0};
    Mat gray ;
    cvtColor(srcImage,gray,CV_RGB2GRAY);

    memset(histValue,0,256*sizeof(double));
    for(int i=0;i<gray.rows;i++)
        for(int j=0;j<gray.cols;j++)
        {
            int pix = gray.at<uchar>(i,j);
            histValue[pix]++;
        }


}
void MainWindow::save()
{
    bsave=false;
    QString fileName;
    QFile file;
    for(int i=0;i<9999;i++)
    {
        fileName.sprintf("./SampleImage/%d.png",i);
        file.setFileName(fileName);
        if(!file.exists())
            break;
    }
    imwrite(fileName.toLatin1().data(),rgbMat1);
    file.setFileName("./SampleImage/config.txt");
    if(!file.open(QIODevice::Append))
    {
       return;
    }
    QString ExpInfo="";
    QString sample;
    sample.sprintf(",%d,%d,%d\n",Exp2,Exp3,Exp4);
    ExpInfo +=fileName+sample;
    file.write(ExpInfo.toLatin1().data(),ExpInfo.length());
    file.flush();
    file.close();
}


void MainWindow::onTimeOut()
{
    disconnect(timer,SIGNAL(timeout()),this,SLOT(onTimeOut()));
    rgbStream.getCameraSettings()->setAutoExposureEnabled(false);
    rgbStream.getCameraSettings()->setAutoWhiteBalanceEnabled(false);

    Exp1 = ui->spinBox->text().toInt();
    Exp2 = ui->spinBox_2->text().toInt();
    Exp3 = ui->spinBox_3->text().toInt();
    Exp4 = ui->spinBox_4->text().toInt();

    getRgb(1,Exp1);
    getRgb(2,Exp2);
    getRgb(3,Exp3);
    getRgb(4,Exp4);

    showOutLabel(rgbMat1,1);
    showOutLabel(rgbMat2,2);
    showOutLabel(rgbMat3,3);
    showOutLabel(rgbMat4,4);

    vector<Mat> images;
    images.push_back(rgbMat2);
    images.push_back(rgbMat3);
    images.push_back(rgbMat4);
    Mat fusion;
    Ptr<MergeMertens> merge_mertens = createMergeMertens();
    merge_mertens->process(images, fusion);
    //imwrite("fusion.jpg", fusion * 255);
    //fusion*=255;
    fusion.convertTo(fusion,CV_8UC3,255);
    showOutLabel(fusion,5);

    if(bsave)
    {
        save();
    }
//    imshow("hdr",fusion);
    connect(timer,SIGNAL(timeout()),this,SLOT(onTimeOut()));

}

void MainWindow::on_saveBtn_clicked()
{
    bsave=true;
}
