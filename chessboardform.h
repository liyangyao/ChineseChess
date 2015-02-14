/****************************************************************************

Copyright (C) 2013 QiAn Software
Contact: http://www.0514soft.com

Creator: liyangyao@gmail.com
Date: 2014/10/23

****************************************************************************/

#ifndef CHESSBOARDFORM_H
#define CHESSBOARDFORM_H

#include <QWidget>
#include <memory>
#include <QTimer>
#include "chessboard.h"

class Res
{
public:
    static Res& instance()
    {
        static Res ins;
        return ins;
    }

    QPixmap& board()
    {
        return m_board;
    }

    QPixmap& selected()
    {
        return m_selected;
    }

    QPixmap& piece(int index)
    {
        return m_pieces[index];
    }

private:
    explicit Res():
        m_board(":/board.bmp", "bmp"),
        m_selected(":/selected.png", "png")
    {
        for(int i=8; i<=14; i++)
        {
            m_pieces[i] = QPixmap(QString(":/%1.png").arg(i), "png");
        }
        for(int i=16; i<=22; i++)
        {
            m_pieces[i] = QPixmap(QString(":/%1.png").arg(i), "png");
        }
    }

    QPixmap m_board;
    QPixmap m_selected;
    QPixmap m_pieces[23];

};

class ChessBoardForm : public QWidget
{
    Q_OBJECT

public:
    ChessBoardForm(QWidget *parent = 0);
    ~ChessBoardForm();

private:
    CC::ChessBoard board;
    double m_scale;
    int m_leftMargin;
    int m_topMargin;
    int m_boardWidth;
    int m_boardHeight;
    int m_pieceWidth;
    int m_pieceHeight;

private:
    bool m_isGameOver;
    QTimer m_timer;

//    int m_boardWidth;
//    int m_boardHeight;
    int m_piece_cxDelta;
    int m_piece_cyDelta;
//    int m_pieceWidth;
//    int m_piectHeight;
    QPoint point(int x, int y);
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *);

private slots:
    void onComputerSide();
};

#endif // CHESSBOARDFORM_H
