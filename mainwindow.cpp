#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//    setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);//stop maximizing the window（禁止最大化窗口）
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mLoadStandard_v()
{
    cv::Mat standard=cv::imread("..\\standard.jpg",3);
    cv::cvtColor(standard,standard,CV_BGR2RGB);
    QImage* img=new QImage();
    img->load("..\\standard.jpg");
    QImage image2 = QImage((uchar*)(standard.data), standard.cols, standard.rows, QImage::Format_RGB888);
    ui->mStandard_label->setPixmap(QPixmap::fromImage(*img));
    ui->mStandard_label->resize(ui->mStandard_label->pixmap()->size());
//    setFixedSize(this->width(), this->height());
}


void MainWindow::on_mLoadStdimg_Button_clicked()
{
    mLoadStandard_v();
}
