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

#include <QDebug>
#include <QMouseEvent>
#include <QPoint>
#include <QPixmap>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    void mLoadStandard_v();
    std::string mUTF8ToGBK_s(const char* strUTF8);

    explicit MainWindow(QWidget *parent = 0); 
    ~MainWindow();

private slots:
    void on_mLoadStdimg_Button_clicked();

private:
    Ui::MainWindow *ui;
    std::vector<cv::Rect> mStandardRects_v;
    cv::Mat mStandardOrigin_M,mStandard_M,mLastStd_M;
    QPoint mStartPoint_qp;
    QPoint mEndPoint_qp;
    tesseract::TessBaseAPI  TBapi;

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

};

#endif // MAINWINDOW_H
