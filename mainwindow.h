#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QPixmap>
#include <opencv2/opencv.hpp>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    void mLoadStandard_v();

    explicit MainWindow(QWidget *parent = 0); 
    ~MainWindow();

private slots:
    void on_mLoadStdimg_Button_clicked();

private:
    Ui::MainWindow *ui;

protected:
//    void paintEvent(QPaintEvent *event);

};

#endif // MAINWINDOW_H
