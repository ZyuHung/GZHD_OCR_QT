#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <Windows.h>

#include "strngs.h"
#include "baseapi.h"
#include <string>
#include <locale>
#include <allheaders.h>
#include <direct.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <map>

#include <QDebug>
#include <QMouseEvent>
#include <QPoint>
#include <QPixmap>
#include <QTimer>
#include <QString>
#include <QMainWindow>

using namespace std;
using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    void mLoadStandard_v();
    void ShowCamera_v();
    void PauseCamera_v();
    void CloseCamera_v();
    void UpdateImage_v();
    void LineEdit_standard_ctrler(bool onoff);
    float calScale(vector<Point2f> PatternPts ,vector<Point2f> DetectPts);
    bool getDetectNameplate(const Mat captureImage, Mat& ROI, float &scale);

//    std::string mUTF8ToGBK_s(const char* strUTF8);

    explicit MainWindow(QWidget *parent = 0); 
    ~MainWindow();

private slots:
    void on_mLoadStdimg_Button_clicked();
    void on_mOpenCamera_bt_clicked();

    void on_mCloseCamera_bt_clicked();

private:
    QString s2q(const string &s);
    const int CAM_NO=1;
    Ui::MainWindow *ui;
    bool mIsPress_b=false;
    bool mIsGetDetect=false;
    float mScale_f=0.0;
    std::vector<cv::Rect> mStandardRects_v;
    cv::VideoCapture mCapture_VC;
    cv::Mat mStandardOrigin_M,mStandard_M,mLastStd_M;
    cv::Mat mCamImage_M;
    cv::Mat mDetecteROI;
    QPoint mStartPoint_qp;
    QPoint mEndPoint_qp;
    QTimer mUpdatingTimer_qt;
    tesseract::TessBaseAPI  TBapi;

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

};

#endif // MAINWINDOW_H
