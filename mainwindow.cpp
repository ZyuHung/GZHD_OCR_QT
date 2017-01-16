/*****************************************************************************
* Copyright (c) 2016 GZHU_EENB_LAB629 Corporation
* All Rights Reserved.
*
* Project Name         :   Guangzhou Honda OCR
* File Name            :   mainwindow.cpp
* Abstract Description :   OCR for nameplate
*
* Create Date          :   2016/12/10
* Author               :   Zhu Zhihong( Zyuhung ), Yao Xuwen( Uwen )
* Address              :   Guangzhou University(HEMC)

******************************************************************************/

/*
 * 此程序在detect nameplate的过程中偶尔出现bug
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);//stop maximizing the window（禁止最大化窗口）
    ui->mAlarm_label->setVisible(false);
    connect(&mUpdatingTimer_qt,&QTimer::timeout,this,&MainWindow::UpdateImage_v);//set the timer for updating camera images（定时更新摄像头画面）
}

MainWindow::~MainWindow()
{
    delete ui;
}

float calScale(vector<Point2f> PatternPts, vector<Point2f> DetectPts)
{
    float PatternDistance = 0.f;
    float DetectDistance = 0.f;

    int PatSize = PatternPts.size();
    int DetSize = DetectPts.size();

    int PatNum = ((PatSize - 1) * PatSize) / 2;
    int DetNum = ((DetSize - 1) * DetSize) / 2;

    for (int i = 0; i<PatternPts.size(); i++)
    {
        for (int j = i + 1; j<PatternPts.size(); j++)
        {
            PatternDistance += norm(PatternPts[i] - PatternPts[j]);
        }
    }
    PatternDistance /= PatNum;
    for (int i = 0; i<DetectPts.size(); i++)
    {
        for (int j = i + 1; j<DetectPts.size(); j++)
        {
            DetectDistance += norm(DetectPts[i] - DetectPts[j]);
        }
    }
    DetectDistance /= DetNum;
    return DetectDistance / PatternDistance;
}

//captureImage输入图像，ROI输出目标，size输出目标尺寸
bool MainWindow::GetDetectNameplate_b(const Mat captureImage, Mat& ROI, float& scale)
{
    if (captureImage.empty())//检查输入图像是否为空
            return false;

        //【1】载入图像、显示并转化为灰度图
        Mat trainImage = imread("D:\\Guangzhou Honda OCR_With UI\\GZHD_OCR\\standard.jpg");
        Mat trainImage_gray;
        //imshow("原始图", trainImage);
        //cvWaitKey(0);
        cvtColor(trainImage, trainImage_gray, CV_BGR2GRAY);
        int dd = trainImage_gray.cols;
        //【2】检测SIFT关键点、提取训练图像描述符
        vector<KeyPoint> train_keyPoint;
        Mat trainDescription;
        SiftFeatureDetector featureDetector;
        featureDetector.detect(trainImage_gray, train_keyPoint);
        SiftDescriptorExtractor featureExtractor;
        featureExtractor.compute(trainImage_gray, train_keyPoint, trainDescription);

        // 【3】进行基于描述符的暴力匹配
        BFMatcher matcher;
        vector<Mat> train_desc_collection(1, trainDescription);
        matcher.add(train_desc_collection);
        matcher.train();

        //【4】创建视频对象、定义帧率
        //VideoCapture cap(0);
        //cap.set(CV_CAP_PROP_FRAME_WIDTH, 1980);
        //cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);

        //unsigned int frameCount = 0;//帧数

        Mat captureImage_gray;
        //<2>转化图像到灰度
        cvtColor(captureImage, captureImage_gray, CV_BGR2GRAY);

        //<3>检测SURF关键点、提取测试图像描述符
        vector<KeyPoint> test_keyPoint;
        Mat testDescriptor;
        featureDetector.detect(captureImage_gray, test_keyPoint);
        featureExtractor.compute(captureImage_gray, test_keyPoint, testDescriptor);

        vector<Point2f> patpoint;

        //<4>匹配训练和测试描述符
        vector<vector<DMatch> > matches;
        matcher.knnMatch(testDescriptor, matches, 2);


        // <5>根据劳氏算法（Lowe's algorithm），得到优秀的匹配点
        vector<DMatch> goodMatches;
        for (unsigned int i = 0; i < matches.size(); i++)
        {
            if (matches[i][0].distance < 0.6 * matches[i][1].distance)
            {
                goodMatches.push_back(matches[i][0]);
            }

        }



        if (goodMatches.size() > 30)
        {
            Point tempPoint;


            vector<Point2f> detpoint;

            for (int index = 0; index < goodMatches.size(); index++)
            {
                tempPoint = test_keyPoint[goodMatches[index].queryIdx].pt;
                //righty = max(righty, tempPoint.y);
                //lefty = min(lefty, tempPoint.y);
                detpoint.push_back(tempPoint);
                patpoint.push_back(train_keyPoint[goodMatches[index].trainIdx].pt);
            }

            Mat dstImage;
            drawMatches(captureImage, test_keyPoint, trainImage, train_keyPoint, matches, dstImage);
//            imshow("匹配窗口", dstImage);

            scale = calScale(patpoint, detpoint);

            Mat H = findHomography(patpoint, detpoint, CV_RANSAC);
            std::vector<Point2f> obj_corners(4);
            obj_corners[0] = Point(0, 0);
            obj_corners[1] = Point(trainImage.cols, 0);
            obj_corners[2] = Point(trainImage.cols, trainImage.rows);
            obj_corners[3] = Point(0, trainImage.rows);
            std::vector<Point2f> scene_corners(4);
            perspectiveTransform(obj_corners, scene_corners, H);

            float x = abs(scene_corners[1].x - scene_corners[0].x);
            float y = abs(scene_corners[3].y - scene_corners[0].y);
            Size size(x, y);

            int  location = (scene_corners[1].x + scene_corners[0].x) / 2;
            if (location > captureImage.cols*0.35&&location < captureImage.cols*0.65)
            {
                Mat im_dst = Mat::zeros(size, CV_8UC3);

                vector<Point2f> pts_dst;

                pts_dst.push_back(Point2f(0, 0));
                pts_dst.push_back(Point2f(size.width - 1, 0));
                pts_dst.push_back(Point2f(size.width - 1, size.height - 1));
                pts_dst.push_back(Point2f(0, size.height - 1));

                /*for (int i = 0; i < 3; i++)
                {
                line(captureImage, scene_corners[i], scene_corners[i + 1], Scalar(255, 255, 255));
                }
                line(captureImage, scene_corners[0], scene_corners[3], Scalar(255, 255, 255));*/

                Mat h = findHomography(scene_corners, pts_dst);
                warpPerspective(captureImage, im_dst, h, size);

                ROI = im_dst;
                return true;
            }


        }
        return false;
    }

void MainWindow::paintEvent(QPaintEvent *event)
{
    //Update the standard image.
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
   //Update the camera image.
   if (mCamImage_M.data)
   {
       QImage cam;
       if (mCamImage_M.channels()==3)
       {
           cam = QImage((uchar*)(mCamImage_M.data), mCamImage_M.cols, mCamImage_M.rows,
                           mCamImage_M.cols*mCamImage_M.channels(), QImage::Format_RGB888);
       }
       else
       {
           cam = QImage((const uchar*)(mCamImage_M.data),mCamImage_M.cols,mCamImage_M.rows,
                           mCamImage_M.cols*mCamImage_M.channels(),QImage::Format_Indexed8);
       }
       ui->mCam_label->setScaledContents(true);
       qDebug()<<"232";
       if (mCamImage_M.cols>mStandardOrigin_M.cols)
           ui->mCam_label->resize(mStandardOrigin_M.cols,mCamImage_M.rows/((float)mCamImage_M.cols/mStandardOrigin_M.cols));
       else
           ui->mCam_label->resize(mCamImage_M.cols,mCamImage_M.rows);
       ui->mCam_label->setPixmap(QPixmap::fromImage(cam));

   }
}

void MainWindow::UpdateImage_v()
{
    //如果已检测到nameplate则close camera并显示nameplate
    if (mIsGetDetect_b)
    {
        mCamImage_M=mDetecteROI_M;
        mUpdatingTimer_qt.stop();
        mIsGetDetect_b=false;
    }
    else
    {
//        mCapture_VC.set(CV_CAP_PROP_FRAME_WIDTH, 800);
//        mCapture_VC.set(CV_CAP_PROP_FRAME_HEIGHT, 600);
        mCapture_VC>>mCamImage_M;//get Image from camera（从摄像头视频流获取视频画面）
        ui->mTip_label->setText("Capturing...");
    }
    //Update camera image
    if (mCamImage_M.data )
    {
        if (!mIsGetDetect_b)
            mIsGetDetect_b=GetDetectNameplate_b(mCamImage_M,mDetecteROI_M,mScale_f);
        if (mCamImage_M.channels()==3);
            cv::cvtColor(mCamImage_M,mCamImage_M,CV_BGR2RGB);
    }
    else
    {
        //camera 丢失
        CloseCamera_v();
        ui->mTip_label->setText("Camera Lost");
    }
    //如果检测到nameplate则进行尺寸变换并OCR
    if (mIsGetDetect_b)
    {
        cv::cvtColor(mDetecteROI_M,mDetecteROI_M,CV_BGR2RGB);
        ui->mTip_label->setText("Captured success, OCRing...");
        cv::Mat ocrimg;
        mDetecteROI_M.copyTo(ocrimg);
        for (int i=0;i<mStandardRects_v_R.size();i++)
        {
            Rect ROI(mStandardRects_v_R[i].x*mScale_f,mStandardRects_v_R[i].y*mScale_f,
                     mStandardRects_v_R[i].width*mScale_f,mStandardRects_v_R[i].height*mScale_f);
            rectangle(mDetecteROI_M, ROI,Scalar(0,0,255),2);
            cv::imwrite("dst.jpg",ocrimg(ROI));
            STRING OutputText_dst;
            TBapi.ProcessPages("dst.jpg", NULL, 0, &OutputText_dst);
            string out=OutputText_dst.string();
            for (auto j=out.begin();j!=out.end();)
            {
                //去空格、换行、制表符
                switch (*j)
                {
                case '\n':
                case '\r':
                case ' ':
                case '\t':
                    out.erase(j);
                    break;
                default:
                    j++;
                    break;
                }
            }
            //格子对应显示OCR结果（后期考虑如何联合所有linetext统一管理）
            switch (i)
            {
            case 0:
                ui->lineEdit_14->setText(out.c_str());
                if (out.c_str()!=ui->lineEdit->text())
                {
                    ui->mAlarm_label->setVisible(true);
                    ui->lineEdit_14->setEnabled(true);
                }
                break;
            case 1:
                ui->lineEdit_15->setText(out.c_str());
                if (out.c_str()!=ui->lineEdit_2->text())
                {
                    ui->mAlarm_label->setVisible(true);
                    ui->lineEdit_15->setEnabled(true);
                }
                break;
            case 2:
                ui->lineEdit_16->setText(out.c_str());
                if (out.c_str()!=ui->lineEdit_3->text())
                {
                    ui->mAlarm_label->setVisible(true);
                    ui->lineEdit_16->setEnabled(true);
                }
                break;
            case 3:
                ui->lineEdit_17->setText(out.c_str());
                if (out.c_str()!=ui->lineEdit_4->text())
                {
                    ui->mAlarm_label->setVisible(true);
                    ui->lineEdit_17->setEnabled(true);
                }
                break;
            case 4:
                ui->lineEdit_18->setText(out.c_str());
                if (out.c_str()!=ui->lineEdit_5->text())
                {
                    ui->mAlarm_label->setVisible(true);
                    ui->lineEdit_18->setEnabled(true);
                }
                break;
            case 5:
                ui->lineEdit_19->setText(out.c_str());
                if (out.c_str()!=ui->lineEdit_6->text())
                {
                    ui->mAlarm_label->setVisible(true);
                    ui->lineEdit_19->setEnabled(true);
                }
                break;
            case 6:
                ui->lineEdit_20->setText(out.c_str());
                if (out.c_str()!=ui->lineEdit_7->text())
                {
                    ui->mAlarm_label->setVisible(true);
                    ui->lineEdit_20->setEnabled(true);
                }
                break;
            case 7:
                ui->lineEdit_21->setText(out.c_str());
                if (out.c_str()!=ui->lineEdit_8->text())
                {
                    ui->mAlarm_label->setVisible(true);
                    ui->lineEdit_21->setEnabled(true);
                }
                break;
            case 8:
                ui->lineEdit_22->setText(out.c_str());
                if (out.c_str()!=ui->lineEdit_9->text())
                {
                    ui->mAlarm_label->setVisible(true);
                    ui->lineEdit_22->setEnabled(true);
                }
                break;
            case 9:
                ui->lineEdit_23->setText(out.c_str());
                if (out.c_str()!=ui->lineEdit_10->text())
                {
                    ui->mAlarm_label->setVisible(true);
                    ui->lineEdit_23->setEnabled(true);
                }
                break;
            case 10:
                ui->lineEdit_24->setText(out.c_str());
                if (out.c_str()!=ui->lineEdit_11->text())
                {
                    ui->mAlarm_label->setVisible(true);
                    ui->lineEdit_24->setEnabled(true);
                }
                break;
            case 11:
                ui->lineEdit_25->setText(out.c_str());
                if (out.c_str()!=ui->lineEdit_12->text())
                {
                    ui->mAlarm_label->setVisible(true);
                    ui->lineEdit_25->setEnabled(true);
                }
                break;
            case 12:
                ui->lineEdit_26->setText(out.c_str());
                if (out.c_str()!=ui->lineEdit_13->text())
                {
                    ui->mAlarm_label->setVisible(true);
                    ui->lineEdit_26->setEnabled(true);
                }
                break;
            default:
                break;
            }
            ui->mOpenCamera_bt->setEnabled(true);      //set "Start" button disabled.
            ui->mCloseCamera_bt->setEnabled(false);     //set "STOP" button abled.
            ui->mTip_label->setText("Camera Close");

        }
    }

}

void MainWindow::CloseCamera_v()
{
    mIsGetDetect_b=false;
//    mCamImage_M.release();
    mDetecteROI_M.release();
    mUpdatingTimer_qt.stop();
    ui->mTip_label->setText("Camera Close");
    ui->mOpenCamera_bt->setEnabled(true);      //set "Start" button disabled.
    ui->mCloseCamera_bt->setEnabled(false);     //set "STOP" button abled.
}

void MainWindow::mLoadStandard_v()
{
   mStandard_M=cv::imread("D:\\Guangzhou Honda OCR_With UI\\GZHD_OCR\\standard.jpg");
   if (mStandard_M.data)
   {
       if (mStandard_M.channels()==3)
           cv::cvtColor(mStandard_M,mStandard_M,CV_BGR2RGB);
       mStandardRects_v_R.clear();
       //保存一副原始图像待OCR
       mStandard_M.copyTo(mStandardOrigin_M);
       LineEdit_standard_ctrler(true);
       ui->mOpenCamera_bt->setEnabled(true);
        ui->mTip_label->setText("Loaded standard image Successfully.");
   }
   else
   {
       ui->mTip_label->setText("Loaded standard image fail.");
   }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (mStandard_M.data && event->x()-ui->mStandard_label->x()>0 &&  event->x()<ui->mStandard_label->width())
    {
        mIsPress_b=true;
        ui->mTip_label->setText("Capturing...");
        mStandardRects_v_R.resize(mStandardRects_v_R.size()+1);
        mStandard_M.copyTo(mLastStd_M);

        mStartPoint_qp.setX(event->x()-ui->mStandard_label->x());

        mStartPoint_qp.setY(event->y()-ui->mStandard_label->y());
        mEndPoint_qp=mStartPoint_qp;

        mStandardRects_v_R.back().x=mStartPoint_qp.x();
        mStandardRects_v_R.back().y=mStartPoint_qp.y();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (mStandard_M.data && mIsPress_b)
    {
        if (event->x()-ui->mStandard_label->x()>=ui->mStandard_label->width())
            mEndPoint_qp.setX(/*ui->mStandard_label->x()+*/ui->mStandard_label->width());
        else if(event->x()<ui->mStandard_label->x())
            mEndPoint_qp.setX(ui->mStandard_label->x()-16);
        else
            mEndPoint_qp.setX(event->x()-ui->mStandard_label->x());

        if(event->y()-ui->mStandard_label->y()>=ui->mStandard_label->height())
            mEndPoint_qp.setY(/*ui->mStandard_label->y()+*/ui->mStandard_label->height());
        else if(event->y()<ui->mStandard_label->y())
            mEndPoint_qp.setY(ui->mStandard_label->y()-16);
        else
            mEndPoint_qp.setY(event->y()-ui->mStandard_label->y());

        mLastStd_M.copyTo(mStandard_M);

        mStandardRects_v_R.back().width=mEndPoint_qp.x()-mStartPoint_qp.x();
        mStandardRects_v_R.back().height=mEndPoint_qp.y()-mStartPoint_qp.y();

        cv::rectangle(mStandard_M,cv::Point(mStartPoint_qp.x(),mStartPoint_qp.y()),
                      cv::Point(mEndPoint_qp.x(),mEndPoint_qp.y()),cv::Scalar(255,0,0),2);

    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (mStandard_M.data && mIsPress_b)
    {
        if (mStandardRects_v_R.back().width<0)
        {
            mStandardRects_v_R.back().x=mEndPoint_qp.x();
            mStandardRects_v_R.back().width=abs(mEndPoint_qp.x()-mStartPoint_qp.x());
        }
        if (mStandardRects_v_R.back().height<0)
        {
            mStandardRects_v_R.back().y=mEndPoint_qp.y();
            mStandardRects_v_R.back().height=abs(mEndPoint_qp.y()-mStartPoint_qp.y());
        }
        mStandard_M.copyTo(mLastStd_M);
        ui->mTip_label->setText("OCRing...");
        cv::Mat dst=mStandardOrigin_M(mStandardRects_v_R.back());

        //放手则自动OCR
    TBapi.Init(NULL, "normal", tesseract::OEM_DEFAULT);
    TBapi.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);

    cv::imwrite("dst.jpg",dst);

    STRING OutputText_dst;
    TBapi.ProcessPages("dst.jpg", NULL, 0, &OutputText_dst);
    string out=OutputText_dst.string();
    for (auto j=out.begin();j!=out.end();)
    {
        //同理，去除回车、空格及制表符
        switch (*j)
        {
        case '\n':
        case '\r':
        case ' ':
        case '\t':
            out.erase(j);
            break;
        default:
            j++;
            break;
        }
    }
    //后期请考虑如何改进统一管理lineEdit
    switch(mStandardRects_v_R.size())
    {
    case 1:
        ui->lineEdit->setText(out.c_str());
        break;
    case 2:
        ui->lineEdit_2->setText(out.c_str());
                break;
    case 3:
        ui->lineEdit_3->setText(out.c_str());
                break;
    case 4:
        ui->lineEdit_4->setText(out.c_str());
                break;
    case 5:
        ui->lineEdit_5->setText(out.c_str());
                break;
    case 6:
        ui->lineEdit_6->setText(out.c_str());
                break;
    case 7:
        ui->lineEdit_7->setText(out.c_str());
                break;
    case 8:
        ui->lineEdit_8->setText(out.c_str());
                break;
    case 9:
        ui->lineEdit_9->setText(out.c_str());
                break;
    case 10:
        ui->lineEdit_10->setText(out.c_str());
                break;
    case 11:
        ui->lineEdit_11->setText(out.c_str());
                break;
    case 12:
        ui->lineEdit_12->setText(out.c_str());
                break;
    case 13:
        ui->lineEdit_13->setText(out.c_str());
                break;
    }
    ui->mTip_label->setText("OCRing...Success");
    mIsPress_b=false;
    }
}

void MainWindow::ShowCamera_v()
{
    if (mCapture_VC.open(CAM_NO))
    {
        mCapture_VC>>mCamImage_M;
        //because the color sequence of Mat in opencv is BGR but RGB in Qt, must conver BGR to RGB.
        //opencv的颜色序列为BGR，而Qt的颜色序列为RGB
        cv::cvtColor(mCamImage_M,mCamImage_M,CV_BGR2RGB);
        ui->mTip_label->setText(tr("Camera open."));

        mUpdatingTimer_qt.start(30);//FPS = 30;

        ui->mOpenCamera_bt->setEnabled(false);      //set "OpenCamera" button disabled.
        ui->mCloseCamera_bt->setEnabled(true);     //set "CloseCamera" button abled.

    }
    else
    {
        ui->mTip_label->setText(tr("Camera open FAILED."));
    }
}

void MainWindow::LineEdit_standard_ctrler(bool onoff)
{
    //统一管理lineEdit的暂行办法（粗暴）
    if (onoff)
    {
        ui->lineEdit->setEnabled(true);
        ui->lineEdit_2->setEnabled(true);
        ui->lineEdit_3->setEnabled(true);
        ui->lineEdit_4->setEnabled(true);
        ui->lineEdit_5->setEnabled(true);
        ui->lineEdit_6->setEnabled(true);
        ui->lineEdit_7->setEnabled(true);
        ui->lineEdit_8->setEnabled(true);
        ui->lineEdit_9->setEnabled(true);
        ui->lineEdit_10->setEnabled(true);
        ui->lineEdit_11->setEnabled(true);
        ui->lineEdit_12->setEnabled(true);
        ui->lineEdit_13->setEnabled(true);
    }
    else
    {
        ui->lineEdit->setEnabled(false);
        ui->lineEdit_2->setEnabled(false);
        ui->lineEdit_3->setEnabled(false);
        ui->lineEdit_4->setEnabled(false);
        ui->lineEdit_5->setEnabled(false);
        ui->lineEdit_6->setEnabled(false);
        ui->lineEdit_7->setEnabled(false);
        ui->lineEdit_8->setEnabled(false);
        ui->lineEdit_9->setEnabled(false);
        ui->lineEdit_10->setEnabled(false);
        ui->lineEdit_11->setEnabled(false);
        ui->lineEdit_12->setEnabled(false);
        ui->lineEdit_13->setEnabled(false);
    }
}


void MainWindow::on_mLoadStdimg_Button_clicked()
{
    mLoadStandard_v();
}

void MainWindow::on_mOpenCamera_bt_clicked()
{
    ShowCamera_v();
}

void MainWindow::on_mCloseCamera_bt_clicked()
{
    CloseCamera_v();
}
