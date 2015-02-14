/****************************************************************************

Copyright (C) 2013 QiAn Software
Contact: http://www.0514soft.com

Creator: liyangyao@gmail.com
Date: 2014/10/23

****************************************************************************/

#include "chessboardform.h"
#include <QApplication>
#ifndef ANDROID
#include <QTextCodec>
#endif

int main(int argc, char *argv[])
{
#ifndef ANDROID
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
#endif
    QApplication a(argc, argv);
    ChessBoardForm w;
    w.show();

    return a.exec();
}
