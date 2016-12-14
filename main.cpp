/*****************************************************************************
* Copyright (c) 2016 GZHU_EENB_LAB629 Corporation
* All Rights Reserved.
*
* Project Name         :   Guangzhou Honda OCR
* File Name            :   main.cpp
*
* Create Date          :   2016/12/10
* Author               :   Zhu Zhihong( Zyuhung )
* Address              :   Guangzhou University(HEMC)

******************************************************************************/
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
