/****************************************************************************

Copyright (C) 2013 QiAn Software
Contact: http://www.0514soft.com

Creator: liyangyao@gmail.com
Date: 2014/10/23

****************************************************************************/

#include "chessboardform.h"
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QMessageBox>
#include <QTimer>

#include "chessboard.h"

#pragma execution_character_set("utf-8")

ChessBoardForm::ChessBoardForm(QWidget *parent)
    : QWidget(parent)
{
    resize(Res::instance().board().size());
    QPalette p;
    p.setColor(QPalette::Background, Qt::black);
    setPalette(p);
    m_isGameOver = false;
    connect(&m_timer, SIGNAL(timeout()), this ,SLOT(onComputerSide()));
}

ChessBoardForm::~ChessBoardForm()
{

}

QPoint ChessBoardForm::point(int x, int y)
{

    int cx = (m_piece_cxDelta + x * m_pieceWidth) +  (m_pieceWidth - 56 ) / 2;
    int cy = (m_piece_cyDelta + y * m_pieceHeight) +  (m_pieceHeight - 56 ) / 2;;

    return QPoint(cx, cy);
}



void ChessBoardForm::paintEvent(QPaintEvent *)
{
    QPixmap pixmap = board.currentPixmap();
    QPainter p(this);

    if (m_scale==1)
    {
        p.drawPixmap(0, 0, pixmap);
    }
    else{
        p.drawPixmap(m_leftMargin, m_topMargin, m_boardWidth, m_boardHeight, pixmap);
    }

}

void ChessBoardForm::onComputerSide()
{
    m_timer.stop();
    if (board.IsMate())
    {
        m_isGameOver = true;
        QMessageBox::information(this, "Hint", "胜利");
        return;
    }
    //qDebug()<<"Checked:"<< board.isChecked();


    board.test1();
    board.updatePixmap();
    repaint();
    if (board.IsMate())
    {
        m_isGameOver = true;
        QMessageBox::information(this, "Hint", "竟然输给电脑了");
        return;
    }
    return;
}

void ChessBoardForm::mousePressEvent(QMouseEvent *e)
{
    if (m_isGameOver) return;
     int x = (e->x() - m_leftMargin) / m_pieceWidth;
     int y = (e->y() - m_topMargin) / m_pieceHeight;
     if (x<0 || x>8 || y<0 || y>9) return;
     //qDebug()<<x<<","<<y;
     int sqDst = board.sq(x, y);
     int sqSelected = board.selected();
     if (sqSelected)
     {
         int mv = board.mv(sqSelected, sqDst);
         int pcCaptured;
         if (board.isLegalMove(mv) && board.makeMove(mv, pcCaptured))
         {                         
             board.updatePixmap();             
             repaint();
             m_timer.start(50);
             return;

         }
     }
     if (board.setSelected(sqDst))
     {
         board.updatePixmap();
         repaint();
     }

}

void ChessBoardForm::resizeEvent(QResizeEvent *)
{
    QPixmap pixmap = board.currentPixmap();
    double XScale = (width() + 0.0) / pixmap.width();
    double YScale = (height() + 0.0) / pixmap.height();
    m_scale = XScale < YScale ? XScale : YScale;
    if (m_scale==1)
    {
        m_boardWidth = pixmap.width();
        m_boardHeight = pixmap.height();
        m_leftMargin = 0;
        m_topMargin = 0;
    }
    else{
        m_boardWidth = pixmap.width() * m_scale;
        m_boardHeight = pixmap.height() * m_scale;
        m_leftMargin = (width() - m_boardWidth) / 2;
        m_topMargin = (height() - m_boardHeight) / 2;
    }
    m_pieceWidth = m_boardWidth / 9;
    m_pieceHeight = m_boardHeight / 10;
}
