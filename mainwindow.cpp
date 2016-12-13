#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);//stop maximizing the window（禁止最大化窗口）
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
   if (mStandard_M.data)
   {
       QImage image2;
       if (mStandard_M.channels()==3)
       {
           image2 = QImage((uchar*)(mStandard_M.data), mStandard_M.cols, mStandard_M.rows,
                           mStandard_M.cols*mStandard_M.channels(), QImage::Format_RGB888);
       }
       else
       {
           image2 = QImage((const uchar*)(mStandard_M.data),mStandard_M.cols,mStandard_M.rows,
                           mStandard_M.cols*mStandard_M.channels(),QImage::Format_Indexed8);
       }
       ui->mStandard_label->setPixmap(QPixmap::fromImage(image2));
       ui->mStandard_label->resize(ui->mStandard_label->pixmap()->size());
   }
}

//std::string MainWindow::mUTF8ToGBK_s(const char* strUTF8)
//   {
//        int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, NULL, 0);
//        wchar_t* wszGBK = new wchar_t[len + 1];
//        memset(wszGBK, 0, len * 2 + 2);
//        MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, wszGBK, len);
//        len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
//        char* szGBK = new char[len + 1];
//        memset(szGBK, 0, len + 1);
//        WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
//        std::string strTemp(szGBK);
//        if (wszGBK) delete[] wszGBK;
//        if (szGBK) delete[] szGBK;
//        return strTemp;
//    }

void MainWindow::mLoadStandard_v()
{
   mStandard_M=cv::imread("..\\standard.jpg");
   if (mStandard_M.channels()==3)
       cv::cvtColor(mStandard_M,mStandard_M,CV_BGR2RGB);
   mStandard_M.copyTo(mStandardOrigin_M);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (mStandard_M.data)
    {
        ui->mTip_label->setText("Capturing...");
        mStandardRects_v.resize(mStandardRects_v.size()+1);
        mStandard_M.copyTo(mLastStd_M);

        if (event->x()-ui->mStandard_label->x()>=ui->mStandard_label->width())
            mStartPoint_qp.setX(ui->mStandard_label->x()+ui->mStandard_label->width()-15);
        else if(event->x()<ui->mStandard_label->x())
            mStartPoint_qp.setX(ui->mStandard_label->x()-7);
        else
            mStartPoint_qp.setX(event->x()-ui->mStandard_label->x());

        if(event->y()-ui->mStandard_label->y()>=ui->mStandard_label->height())
            mStartPoint_qp.setY(ui->mStandard_label->y()+ui->mStandard_label->height()-15);
        else if(event->y()<ui->mStandard_label->y())
            mStartPoint_qp.setY(ui->mStandard_label->y()-7);
        else
            mStartPoint_qp.setY(event->y()-ui->mStandard_label->y());
        mEndPoint_qp=mStartPoint_qp;
        mStandardRects_v[mStandardRects_v.size()-1].x=mStartPoint_qp.x();
        mStandardRects_v[mStandardRects_v.size()-1].y=mStartPoint_qp.y();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->x()-ui->mStandard_label->x()>=ui->mStandard_label->width())
        mEndPoint_qp.setX(ui->mStandard_label->x()+ui->mStandard_label->width()-15);
    else if(event->x()<ui->mStandard_label->x())
        mEndPoint_qp.setX(ui->mStandard_label->x()-7);
    else
        mEndPoint_qp.setX(event->x()-ui->mStandard_label->x());

    if(event->y()-ui->mStandard_label->y()>=ui->mStandard_label->height())
        mEndPoint_qp.setY(ui->mStandard_label->y()+ui->mStandard_label->height()-15);
    else if(event->y()<ui->mStandard_label->y())
        mEndPoint_qp.setY(ui->mStandard_label->y()-7);
    else
        mEndPoint_qp.setY(event->y()-ui->mStandard_label->y());

    if (mStandard_M.data)
    {
        mLastStd_M.copyTo(mStandard_M);
        mStandardRects_v[mStandardRects_v.size()-1].width=mEndPoint_qp.x()-mStartPoint_qp.x();
        mStandardRects_v[mStandardRects_v.size()-1].height=mEndPoint_qp.y()-mStartPoint_qp.y();
        cv::rectangle(mStandard_M,mStandardRects_v.back(),cv::Scalar(255,0,0),2);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    mStandard_M.copyTo(mLastStd_M);
    ui->mTip_label->setText("OCRing...");
    cv::Mat dst=mStandardOrigin_M(mStandardRects_v.back());

    TBapi.Init(NULL, "normal", tesseract::OEM_DEFAULT);
    TBapi.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);

    cv::imwrite("dst.jpg",dst);

//    TBapi.SetImage((uchar*)dst.data, dst.cols, dst.rows, 1, dst.cols);
//    char* out = TBapi.GetUTF8Text();
    STRING OutputText_dst;
    TBapi.ProcessPages("dst.jpg", NULL, 0, &OutputText_dst);

    qDebug()<<OutputText_dst.string();
    switch(mStandardRects_v.size())
    {
    case 1:
        ui->lineEdit->setText(OutputText_dst.string());
        break;
    case 2:
        ui->lineEdit_2->setText(OutputText_dst.string());
                break;
    case 3:
        ui->lineEdit_3->setText(OutputText_dst.string());
                break;
    case 4:
        ui->lineEdit_4->setText(OutputText_dst.string());
                break;
    case 5:
        ui->lineEdit_5->setText(OutputText_dst.string());
                break;
    case 6:
        ui->lineEdit_6->setText(OutputText_dst.string());
                break;
    case 7:
        ui->lineEdit_7->setText(OutputText_dst.string());
                break;
    case 8:
        ui->lineEdit_8->setText(OutputText_dst.string());
                break;
    case 9:
        ui->lineEdit_9->setText(OutputText_dst.string());
                break;
    case 10:
        ui->lineEdit_10->setText(OutputText_dst.string());
                break;
    case 11:
        ui->lineEdit_11->setText(OutputText_dst.string());
                break;
    case 12:
        ui->lineEdit_12->setText(OutputText_dst.string());
                break;
    case 13:
        ui->lineEdit_14->setText(OutputText_dst.string());
                break;
    }
    ui->mTip_label->setText("OCRing...Success");
}

void MainWindow::on_mLoadStdimg_Button_clicked()
{
    mLoadStandard_v();
}
