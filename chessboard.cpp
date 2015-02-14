/****************************************************************************

Copyright (C) 2013 QiAn Software
Contact: http://www.0514soft.com

Creator: liyangyao@gmail.com
Date: 2014/10/25

****************************************************************************/

#include "chessboard.h"
#include <QPainter>
#include <QDebug>
#include <limits>

#pragma execution_character_set("utf-8")

namespace CC
{
const int PIECE_KING = 0;
const int PIECE_ADVISOR = 1;
const int PIECE_BISHOP = 2;
const int PIECE_KNIGHT = 3;
const int PIECE_ROOK = 4;
const int PIECE_CANNON = 5;
const int PIECE_PAWN = 6;

const int MATE_VALUE = 10000;  // 最高分值，即将死的分值
const int WIN_VALUE = MATE_VALUE - 100; // 搜索出胜负的分值界限，超出此值就说明已经搜索出杀棋了
const int ADVANCED_VALUE = 3;  // 先行权分值

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

    int pieceWidth()
    {
        return m_pieceWidth;
    }

    int pieceHeight()
    {
        return m_pieceHeight;
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
        m_pieceWidth = m_pieces[8].width();
        m_pieceHeight = m_pieces[8].height();
    }

    QPixmap m_board;
    QPixmap m_selected;
    QPixmap m_pieces[23];
    int m_pieceWidth;
    int m_pieceHeight;

};

ChessBoard::ChessBoard()
{
    static quint8 init[256] = {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                               0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                               0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                               0,  0,  0, 20, 19, 18, 17, 16, 17, 18, 19, 20,  0,  0,  0,  0,
                               0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                               0,  0,  0,  0, 21,  0,  0,  0,  0,  0, 21,  0,  0,  0,  0,  0,
                               0,  0,  0, 22,  0, 22,  0, 22,  0, 22,  0, 22,  0,  0,  0,  0,
                               0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                               0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                               0,  0,  0, 14,  0, 14,  0, 14,  0, 14,  0, 14,  0,  0,  0,  0,
                               0,  0,  0,  0, 13,  0,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,
                               0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                               0,  0,  0, 12, 11, 10,  9,  8,  9, 10, 11, 12,  0,  0,  0,  0,
                               0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                               0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                               0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
    memcpy(m_data, init, 256);
    m_pcWidth = Res::instance().board().width() / 9;
    m_pcWidthDelta = (m_pcWidth - Res::instance().pieceWidth()) / 2 + (Res::instance().board().width() - m_pcWidth * 9)/2;
    m_pcHeight = Res::instance().board().height() / 10;
    m_pcHeightDelta = (m_pcHeight - Res::instance().pieceHeight()) / 2 + (Res::instance().board().height() - m_pcHeight * 10)/2;
    m_sdPlayer = 0;
    m_vlBlack = 0;
    m_vlRed = 0;
    m_selected = 0;
    m_distance = 0;
    updatePixmap();
}

bool ChessBoard::isInBoard(int sq)
{
    static const qint8 ccInBoard[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    return ccInBoard[sq];
}

QPixmap& ChessBoard::currentPixmap()
{
    return m_currentPixmap;
}

void ChessBoard::updatePixmap()
{
    m_currentPixmap = Res::instance().board();
    QPainter p(&m_currentPixmap);

    for(int x=0; x<9; x++)
    {
        for(int y=0; y<11; y++)
        {
            int sq = this->sq(x, y);
            int pc = m_data[sq];
            if (pc>0)
            {
                p.drawPixmap(m_pcWidthDelta + m_pcWidth * x, m_pcHeightDelta + m_pcHeight * y, Res::instance().piece(pc));
            }
        }
    }
    if (m_selected != 0)
    {
        int x, y;
        xy(m_selected, x, y);
        p.drawPixmap(m_pcWidthDelta + m_pcWidth * x, m_pcHeightDelta + m_pcHeight * y, Res::instance().selected());
    }
}

int side(int pc)
{
    if (!pc) return -1;
    return pc >= 16 ? 1 : 0;
}

bool ChessBoard::setSelected(int sq)
{
    int pc = m_data[sq];
    if (pc && side(pc) == m_sdPlayer)
    {
        m_selected = sq;
        return true;
    }
    return false;
}


int ChessBoard::selected()
{
    return m_selected;
}

void ChessBoard::changeSide()
{
    m_selected = 0;
    m_sdPlayer = 1 - m_sdPlayer;
}

int ChessBoard::evaluate()
{
    return (m_sdPlayer?m_vlBlack - m_vlRed:m_vlRed - m_vlBlack)+ ADVANCED_VALUE;
}

// 获得走法的起点
inline int SRC(int mv)
{
  return mv & 255;
}

// 获得走法的终点
inline int DST(int mv)
{
  return mv >> 8;
}


int ChessBoard::_movePiece(int mv)
{
    int sqSrc, sqDst, pc, pcCaptured;
    sqSrc = SRC(mv);
    sqDst = DST(mv);
    pcCaptured = m_data[sqDst];
    delPiece(sqDst, pcCaptured);
    pc = m_data[sqSrc];
    delPiece(sqSrc, pc);
    addPiece(sqDst, pc);
    //qDebug()<<"movePiece"<< m_vlBlack<<","<<m_vlRed;
    return pcCaptured;
}

void ChessBoard::_undoMovePiece(int mv, int pcCaptured)
{
    int sqSrc, sqDst, pc;
    sqSrc = SRC(mv);
    sqDst = DST(mv);
    pc = m_data[sqDst];
    delPiece(sqDst, pc);
    addPiece(sqSrc, pc);
    if (pcCaptured != 0)
    {
      addPiece(sqDst, pcCaptured);
    }

    //qDebug()<<"undoMovePiece"<< m_vlBlack<<","<<m_vlRed;
}


bool ChessBoard::makeMove(int mv, int& pcCaptured)
{
    pcCaptured = _movePiece(mv);
    if (isChecked())
    {
         _undoMovePiece(mv, pcCaptured);
        pcCaptured = 0;
        return false;
    }
    m_distance++;
    changeSide();
    return true;
}

void ChessBoard::undoMakeMove(int mv, int pcCaptured)
{
    _undoMovePiece(mv, pcCaptured);
    m_distance--;
    changeSide();
}

bool ChessBoard::isLegalMove(int mv)
{
    int sqSrc, sqDst, pcSrc, pcDst;
    sqSrc = SRC(mv);
    sqDst = DST(mv);
    if (sqSrc==sqDst)
    {
        return false;
    }
    pcSrc = m_data[sqSrc];
    pcDst = m_data[sqDst];
    if (side(pcSrc)==side(pcDst))
    {
        return false;
    }

    Moves mvs;
    generateMoves(sqSrc, mvs);
    for(int i=0; i<mvs.count(); i++)
    {
        if (mvs[i]==mv)
        {
            return true;
        }
    }
    return false;
}

inline int pieceType(int pc)
{
    return pc%8;
}

inline bool IN_FORT(int sq)
{
    // 判断棋子是否在九宫的数组
    static const qint8  ccInFort[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    return ccInFort[sq] != 0;
}

bool SAME_SIDE(int pcSrc, int pcDst)
{
    return side(pcSrc)==side(pcDst);
}

bool ChessBoard::canMove(ChessBoard& board, int sqSrc, int sqDst)
{
    //1.没有子
    //2.对方的子
    int pcSrc = board.m_data[sqSrc];
    int pcDst = board.m_data[sqDst];
    return !pcDst || !SAME_SIDE(pcSrc, pcDst);
}


void ChessBoard::generateKingMoves(ChessBoard& board, int sqSrc, Moves& mvs)
{
    static const int ccKingDelta[4] = {-16, -1, 1, 16};
    for(int i=0; i<4; i++)
    {
        int sqDst = sqSrc + ccKingDelta[i];
        if (IN_FORT(sqDst))
        {
            if (board.canMove(board, sqSrc, sqDst))
            {
                mvs.append(ChessBoard::mv(sqSrc, sqDst));
            }
        }
    }
}

void ChessBoard::generateAdvisorMoves(ChessBoard& board, int sqSrc, Moves& mvs)
{
    static const int ccAdvisorDelta[4] = {-17, -15, 15, 17};
    for(int i=0; i<4; i++)
    {
        int sqDst = sqSrc + ccAdvisorDelta[i];
        if (IN_FORT(sqDst))
        {
            if (board.canMove(board, sqSrc, sqDst))
            {
                mvs.append(ChessBoard::mv(sqSrc, sqDst));
            }
        }
    }
}

// 是否在河的同一边
inline bool SAME_HALF(int sqSrc, int sqDst)
{
    return ((sqSrc ^ sqDst) & 0x80) == 0;
}

inline bool IN_BOARD(int sq)
{
    // 判断棋子是否在棋盘中的数组
    static const qint8 ccInBoard[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    return ccInBoard[sq];
}


void ChessBoard::generateBishopMoves(ChessBoard& board, int sqSrc, Moves& mvs)
{
    static const int ccBishopDelta[4] = {-34, -30, 30, 34};
    static const int ccBishopPin[4] = {-17, -15, 15, 17};
    for(int i=0; i<4; i++)
    {
        int sqDst = sqSrc + ccBishopDelta[i];
        if (!IN_BOARD(sqDst)) continue;
        int sqPin = sqSrc + ccBishopPin[i];
        if (board.m_data[sqPin]) continue;
        if (SAME_HALF(sqSrc, sqDst))
        {
            if (board.isInBoard(sqDst) && board.canMove(board, sqSrc, sqDst))
            {
                mvs.append(ChessBoard::mv(sqSrc, sqDst));
            }
        }

    }
}

void ChessBoard::generateKnightMoves(ChessBoard& board, int sqSrc, Moves& mvs)
{
    static const int ccKingDelta[4] = {-16, -1, 1, 16};
    static const int ccKnightDelta[4][2] = {{-33, -31}, {-18, 14}, {-14, 18}, {31, 33}};
    for (int i=0; i<4; i++)
    {
        int sqPin = sqSrc + ccKingDelta[i];
        if (IN_BOARD(sqPin) && board.m_data[sqPin]==0)
        {
            //马腿没有东西
            for(int j=0; j<2; j++)
            {
                int sqDst = sqSrc + ccKnightDelta[i][j];
                if (board.isInBoard(sqDst) && board.canMove(board, sqSrc, sqDst))
                {
                    mvs.append(ChessBoard::mv(sqSrc, sqDst));
                }
            }
        }
    }
}



void ChessBoard::generateRookMoves(ChessBoard& board, int sqSrc, Moves& mvs)
{
    static const int delta[4] = {-16, -1, 1, 16};
    for(int i=0; i<4; i++)
    {
        int sqDst = sqSrc;
        while (true)
        {
            sqDst += delta[i];
            if (!IN_BOARD(sqDst)) break;
            if (!board.m_data[sqDst])
            {                
                mvs.append(ChessBoard::mv(sqSrc, sqDst));
            }
            else{
                if (board.canMove(board, sqSrc, sqDst))
                {
                    mvs.append(ChessBoard::mv(sqSrc, sqDst));
                }
                break;
            }
        }
    }
}

void ChessBoard::generateCannonMoves(ChessBoard& board, int sqSrc, Moves& mvs)
{
    static const int delta[4] = {-16, -1, 1, 16};
    for(int i=0; i<4; i++)
    {
        int sqDst = sqSrc;
        bool skiped = false;
        while (true)
        {
            sqDst += delta[i];
            if (!IN_BOARD(sqDst)) break;
            if (!board.m_data[sqDst])
            {
                if (!skiped)
                {
                    mvs.append(ChessBoard::mv(sqSrc, sqDst));
                }
            }
            else{
                if (!skiped)
                {
                    skiped = true;
                }
                else{
                    int pc = board.m_data[sqDst];
                    if (pc!=0)
                    {
                        if (board.canMove(board, sqSrc, sqDst))
                        {
                            mvs.append(ChessBoard::mv(sqSrc, sqDst));
                        }
                    }
                    break;
                }
            }
        }
    }
}

int SQ_SIDE(int sq)
{
    const qint8 Side[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    return Side[sq];
}

void ChessBoard::generatePawnMoves(ChessBoard& board, int sqSrc, Moves& mvs)
{
    int sqDst;

    if (board.m_sdPlayer != SQ_SIDE(sqSrc))
    {
        sqDst = sqSrc - 1;
        if (IN_BOARD(sqDst) && board.canMove(board, sqSrc, sqDst))
        {
            mvs.append(ChessBoard::mv(sqSrc, sqDst));
        }
        sqDst = sqSrc + 1;
        if (IN_BOARD(sqDst) && board.canMove(board, sqSrc, sqDst))
        {
            mvs.append(ChessBoard::mv(sqSrc, sqDst));
        }
    }


    if (board.m_sdPlayer==0)
    {
        sqDst = sqSrc - 16;
    }
    else{
        sqDst = sqSrc + 16;
    }
    if (IN_BOARD(sqDst) && board.canMove(board, sqSrc, sqDst))
    {
        mvs.append(ChessBoard::mv(sqSrc, sqDst));
    }
}

void ChessBoard::generateMoves(int sq, Moves& mvs)
{
    int pc = m_data[sq];
    if (!pc) return;
    switch (pieceType(pc)) {
    case PIECE_KING:
        generateKingMoves(*this, sq, mvs);
        break;
    case PIECE_ADVISOR:
        generateAdvisorMoves(*this, sq, mvs);
        break;
    case PIECE_BISHOP:
        generateBishopMoves(*this, sq, mvs);
        break;
    case PIECE_KNIGHT:
        generateKnightMoves(*this, sq, mvs);
        break;
    case PIECE_ROOK:
        generateRookMoves(*this, sq, mvs);
        break;
    case PIECE_CANNON:
        generateCannonMoves(*this, sq, mvs);
        break;
    case PIECE_PAWN:
        generatePawnMoves(*this, sq, mvs);
        break;

    }
}

void ChessBoard::generateMoves(Moves& mvs)
{
    for(int sq=0; sq<256; sq++)
    {
        int pc = m_data[sq];
        if (!pc) continue;
        if (side(pc) == m_sdPlayer)
        {
            generateMoves(sq, mvs);
        }
    }
}

bool ChessBoard::isChecked(int side)
{
    int old = m_sdPlayer;
    m_sdPlayer = side;
    bool result = isChecked();
    m_sdPlayer = old;
    return result;
}


bool ChessBoard::isChecked()
{
    //qDebug()<<"Enter isChecked";
    // 找到棋盘上的帅(将)，再做以下判断：
    int pcKing = PIECE_KING + (m_sdPlayer + 1) * 8;
    //qDebug()<<"pcKing="<<pcKing;


    for (int sqSrc = 0; sqSrc < 256; sqSrc ++)
    {
      if (m_data[sqSrc]!=pcKing)
      {
        continue;
      }


      // 1. 判断是否被对方的兵(卒)将军
      int pcOppPawn = PIECE_PAWN + (2 - m_sdPlayer) * 8;
      static const int delta[2][4] = {{-1, 1, -16},{-1, 1, 16}};
      for(int i=0; i<4; i++)
      {
          int sqDst = sqSrc + delta[m_sdPlayer][i];
          if (m_data[sqDst]==pcOppPawn)
          {
              //qDebug()<<"pawn IsChecked true";
              return true;
          }
      }
      // 2. 判断是否被对方的马将军(以仕(士)的步长当作马腿)
      int pcOppKnight = PIECE_KNIGHT + (2 - m_sdPlayer) * 8;
      static const int ccPinDelta[4] = {-17, -15, 15, 17};
      static const int ccKnightDelta[4][2] = {{-33, -18}, {-31, -14}, {14, 31}, {18, 33}};

      for (int i=0; i<4; i++)
      {
          //qDebug()<<"kinght PinDelta="<<ccPinDelta[i];
          int sqPin = sqSrc + ccPinDelta[i];
          if (m_data[sqPin]) continue;
          for (int j=0; j<2; j++)
          {
              int sqDst = sqSrc + ccKnightDelta[i][j];
              if (m_data[sqDst]==pcOppKnight)
              {
                  //qDebug()<<"knight IsChecked true";
                  return true;
              }
          }

      }

      // 3. 判断是否被对方的车或炮将军(包括将帅对脸)
      int pcOppRook = PIECE_ROOK + (2 - m_sdPlayer) * 8;
      int pcOppCannon = PIECE_CANNON + (2 - m_sdPlayer) * 8;
      int pcOppKing = PIECE_KING + (2 - m_sdPlayer) * 8;

      static const int rookDelta[4] = {-16, -1, 1, 16};
      for(int i=0; i<4; i++)
      {

          int sqDst = sqSrc;
          bool skiped = false;
          while (true)
          {
              sqDst += rookDelta[i];
              if (!IN_BOARD(sqDst)) break;
              int pcDst = m_data[sqDst];
              if (!skiped && (pcDst==pcOppRook || pcDst==pcOppKing))
              {
                  //qDebug()<<"rook IsChecked true";
                  return true;
              }            
              if (pcDst)
              {
                  if (!skiped)
                  {
                    skiped = true;
                  }
                  else{
                      if (pcDst==pcOppCannon)
                      {
                          //qDebug()<<"cannon IsChecked true";
                          return true;
                      }
                      break;
                  }
              }
          }
      }
    }
    return false;
}

bool ChessBoard::IsMate(int side)
{
    int old = m_sdPlayer;
    m_sdPlayer = side;
    bool result = IsMate();
    m_sdPlayer = old;
    return result;
}

bool ChessBoard::IsMate()
{   
    //qDebug()<<"check isMate side="<<m_sdPlayer;
    //int selfSide = m_sdPlayer;
    Moves mvs;
    generateMoves(mvs);
    for(int i=0; i<mvs.count(); i++)
    {
        //qDebug()<<"尝试解招："<< mvString(mvs[i]);
        int pcCaptured = _movePiece(mvs[i]);
        if (!isChecked())
        {
            _undoMovePiece(mvs[i], pcCaptured);
            //qDebug()<<"!!!解招："<< mvString(mvs[i]);
            return false;
        }
        else{
            _undoMovePiece(mvs[i], pcCaptured);
        }
    }
    return true;
}

// 是否在同一行
inline bool SAME_RANK(int sqSrc, int sqDst)
{
    return ((sqSrc ^ sqDst) & 0xf0) == 0;
}

// 是否在同一列
inline bool SAME_FILE(int sqSrc, int sqDst)
{
    return ((sqSrc ^ sqDst) & 0x0f) == 0;
}

QString pcToString(int pc)
{//B. 8~14依次表示红方的帅、仕、相、马、车、炮和兵；
    //C. 16~22依次表示黑方的将、士、象、马、车、炮和卒。
    switch (pc) {
        case 8: return "帅";
        case 9: return "仕";
        case 10: return "相";
        case 11: return "马";
        case 12: return "车";
        case 13: return "炮";
        case 14: return "兵";
        case 16: return "将";
        case 17: return "士";
        case 18: return "象";
        case 19: return "马";
        case 20: return "车";
        case 21: return "炮";
        case 22: return "卒";
    }
    return "";
}

QString ChessBoard::mvString(int mv)
{
    if (mv==0) return "";
    static const qint8 ccPos[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0,
        0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0,
        0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0,
        0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0,
        0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0,
        0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0,
        0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0,
        0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0,
        0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0,
        0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    static const QString RedString[9] = {"九","八","七","六","五","四","三","二","一"};
    static const QString BlackString[9] = {"1","2","3","4","5","6","7","8","9"};

    int sqSrc = SRC(mv);
    int pcSrc = m_data[sqSrc];
    int sqDst = DST(mv);

    QString srcPos;
    QString dstPos;
    int pos = ccPos[sqSrc]-1;
    int p2 = ccPos[sqDst]-1;
    bool isRed =  side(pcSrc)==0;
    if (isRed)
    {
        srcPos = RedString[pos];
        dstPos = RedString[p2];
    }
    else{
        srcPos = BlackString[pos];
        dstPos = BlackString[p2];
    }

    QString act;
    if (SAME_RANK(sqSrc, sqDst))
    {
        act = "平";
    }
    else if (SAME_FILE(sqSrc, sqDst))
    {
        int n = (sqDst - sqSrc)/16;
        if (isRed)
        {
            n = n * -1;
        }
        if (n>0)
        {
            act = "进";
        }
        else{
            act = "退";
            n = n * -1;
        }
        if (isRed)
        {
            dstPos = RedString[9-n];
        }
        else{
            dstPos = BlackString[n-1];
        }
    }
    else{
        int n = sqDst - sqSrc;
        if (isRed)
        {
            n = n * -1;
        }
        if (n>0)
        {
            act = "进";
        }
        else{
            act = "退";
        }
    }
    return pcToString(pcSrc) + srcPos + act + dstPos;

}

// 子力位置价值表
const quint8 cucvlPiecePos[7][256] = {
  { // 帅(将)
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0, 11, 15, 11,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
  }, { // 仕(士)
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0, 20,  0, 20,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0, 23,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0, 20,  0, 20,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
  }, { // 相(象)
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0, 20,  0,  0,  0, 20,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0, 18,  0,  0,  0, 23,  0,  0,  0, 18,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0, 20,  0,  0,  0, 20,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
  }, { // 马
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0, 90, 90, 90, 96, 90, 96, 90, 90, 90,  0,  0,  0,  0,
    0,  0,  0, 90, 96,103, 97, 94, 97,103, 96, 90,  0,  0,  0,  0,
    0,  0,  0, 92, 98, 99,103, 99,103, 99, 98, 92,  0,  0,  0,  0,
    0,  0,  0, 93,108,100,107,100,107,100,108, 93,  0,  0,  0,  0,
    0,  0,  0, 90,100, 99,103,104,103, 99,100, 90,  0,  0,  0,  0,
    0,  0,  0, 90, 98,101,102,103,102,101, 98, 90,  0,  0,  0,  0,
    0,  0,  0, 92, 94, 98, 95, 98, 95, 98, 94, 92,  0,  0,  0,  0,
    0,  0,  0, 93, 92, 94, 95, 92, 95, 94, 92, 93,  0,  0,  0,  0,
    0,  0,  0, 85, 90, 92, 93, 78, 93, 92, 90, 85,  0,  0,  0,  0,
    0,  0,  0, 88, 85, 90, 88, 90, 88, 90, 85, 88,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
  }, { // 车
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,206,208,207,213,214,213,207,208,206,  0,  0,  0,  0,
    0,  0,  0,206,212,209,216,233,216,209,212,206,  0,  0,  0,  0,
    0,  0,  0,206,208,207,214,216,214,207,208,206,  0,  0,  0,  0,
    0,  0,  0,206,213,213,216,216,216,213,213,206,  0,  0,  0,  0,
    0,  0,  0,208,211,211,214,215,214,211,211,208,  0,  0,  0,  0,
    0,  0,  0,208,212,212,214,215,214,212,212,208,  0,  0,  0,  0,
    0,  0,  0,204,209,204,212,214,212,204,209,204,  0,  0,  0,  0,
    0,  0,  0,198,208,204,212,212,212,204,208,198,  0,  0,  0,  0,
    0,  0,  0,200,208,206,212,200,212,206,208,200,  0,  0,  0,  0,
    0,  0,  0,194,206,204,212,200,212,204,206,194,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
  }, { // 炮
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,100,100, 96, 91, 90, 91, 96,100,100,  0,  0,  0,  0,
    0,  0,  0, 98, 98, 96, 92, 89, 92, 96, 98, 98,  0,  0,  0,  0,
    0,  0,  0, 97, 97, 96, 91, 92, 91, 96, 97, 97,  0,  0,  0,  0,
    0,  0,  0, 96, 99, 99, 98,100, 98, 99, 99, 96,  0,  0,  0,  0,
    0,  0,  0, 96, 96, 96, 96,100, 96, 96, 96, 96,  0,  0,  0,  0,
    0,  0,  0, 95, 96, 99, 96,100, 96, 99, 96, 95,  0,  0,  0,  0,
    0,  0,  0, 96, 96, 96, 96, 96, 96, 96, 96, 96,  0,  0,  0,  0,
    0,  0,  0, 97, 96,100, 99,101, 99,100, 96, 97,  0,  0,  0,  0,
    0,  0,  0, 96, 97, 98, 98, 98, 98, 98, 97, 96,  0,  0,  0,  0,
    0,  0,  0, 96, 96, 97, 99, 99, 99, 97, 96, 96,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
  }, { // 兵(卒)
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  9,  9,  9, 11, 13, 11,  9,  9,  9,  0,  0,  0,  0,
    0,  0,  0, 19, 24, 34, 42, 44, 42, 34, 24, 19,  0,  0,  0,  0,
    0,  0,  0, 19, 24, 32, 37, 37, 37, 32, 24, 19,  0,  0,  0,  0,
    0,  0,  0, 19, 23, 27, 29, 30, 29, 27, 23, 19,  0,  0,  0,  0,
    0,  0,  0, 14, 18, 20, 27, 29, 27, 20, 18, 14,  0,  0,  0,  0,
    0,  0,  0,  7,  0, 13,  0, 16,  0, 13,  0,  7,  0,  0,  0,  0,
    0,  0,  0,  7,  0,  7,  0, 15,  0,  7,  0,  7,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
  }
};

void ChessBoard::addPiece(int sq, int pc)
{
    m_data[sq] = pc;
    if (pc==0) return;
    // 红方加分，黑方(注意"cucvlPiecePos"取值要颠倒)减分
    if (pc < 16) {
      m_vlRed += cucvlPiecePos[pc - 8][sq];
      //qDebug()<<"RED sq="<<sq<<" +"<<cucvlPiecePos[pc - 8][sq];
    } else {
      m_vlBlack += cucvlPiecePos[pc - 16][254-sq];
    }
}

void ChessBoard::delPiece(int sq, int pc)
{
     m_data[sq] = 0;
     if (pc==0) return;
     if (pc < 16) {
       m_vlRed -= cucvlPiecePos[pc - 8][sq];
     } else {
       m_vlBlack -= cucvlPiecePos[pc - 16][254-sq];
     }
}

int ChessBoard::test1_max(int depth, int& mv)
{
    if (depth==0)
    {
        mv = 0;
        return m_vlBlack - m_vlRed;
    }
    Moves mvs;
    //int selfSide = m_sdPlayer;
    int best = -999999;
    generateMoves(mvs);
    for(int i=0; i<mvs.count(); i++)
    {
        int oldVal = m_vlBlack - m_vlRed;
        int pcCapture;
        if (makeMove(mvs[i], pcCapture))
        {
            int temp_mv;
            int val = test1_min(depth-1, temp_mv);
            if (val>best)
            {
               best = val;
               mv = mvs[i];
            }
            undoMakeMove(mvs[i], pcCapture);
        }

        int newVal = m_vlBlack - m_vlRed;
        Q_ASSERT(oldVal==newVal);
    }
    return best;
}

int ChessBoard::test1_min(int depth, int& mv)
{
    if (depth==0)
    {
        mv = 0;
        return m_vlBlack - m_vlRed;
    }
    //int selfSide = m_sdPlayer;
    Moves mvs;
    int best = 999999;
    generateMoves(mvs);
    for(int i=0; i<mvs.count(); i++)
    {
        int oldVal = m_vlBlack - m_vlRed;
        int pcCapture;
        if (makeMove(mvs[i], pcCapture))
        {
            int temp_mv;
            int val = test1_max(depth-1, temp_mv);
            if (val<best)
            {
               best = val;
               mv = mvs[i];
            }
            undoMakeMove(mvs[i], pcCapture);
        }

        int newVal = m_vlBlack - m_vlRed;
        if (oldVal!=newVal)
        {
            qDebug()<<"newVal="<<newVal;
        }
        Q_ASSERT(oldVal==newVal);
    }
    return best;
}



int ChessBoard::negaMax(int depth, int& mv)
{
    if (depth==0)
    {
        return m_sdPlayer?m_vlBlack - m_vlRed:m_vlRed - m_vlBlack;
    }
    m_searchCallTimes++;
    int best = INT_MIN;
    Moves mvs;
    generateMoves(mvs);
    for(int i=0; i<mvs.count(); i++)
    {
        int oldVal = m_vlBlack - m_vlRed;
        int pcCapture;
        //qDebug()<<mvString(mvs[i]);
        if (makeMove(mvs[i], pcCapture))
        {
            int temp_mv;
            int val = -1 * negaMax(depth-1, temp_mv);

            if (temp_mv)
            {
                //qDebug()<<mvString(temp_mv)<<" 评分"<<val;
            }
            if (val>best)
            {
               best = val;
               mv = mvs[i];
            }
            undoMakeMove(mvs[i], pcCapture);
            //qDebug()<<"UndoMakeMove";
        }

        int newVal = m_vlBlack - m_vlRed;
        Q_ASSERT(oldVal==newVal);
    }
    return best;
}

int m_historyTable[65536];

bool compareLessTan(int a, int b)
{
    return m_historyTable[a]>m_historyTable[b];
}

void PrintMoves(ChessBoard::Moves& mvs)
{
    QString out;
    for(int i=0; i<mvs.count(); i++)
    {
        out += QString("[%1]").arg(mvs[i]);
    }
    qDebug()<<out;
}



int ChessBoard::alphaBetaSearch(int depth, int alpha, int beta)
{    
    //1.到达水平线返回
    if (depth==0)
    {
        return evaluate();
    }
    m_searchCallTimes++;

    //2.初使化最佳值,最佳走法    
    int vlBest = -MATE_VALUE;
    int mvBest = 0;

    //3.生成走法,根据历史表排序
    Moves mvs;
    generateMoves(mvs);
    //PrintMoves(mvs);
    qSort(mvs.begin(), mvs.end(), compareLessTan);
    //PrintMoves(mvs);
    //qDebug()<<"----------------------";

    // 4. 逐一走这些走法，并进行递归
    for(int i=0; i<mvs.count(); i++)
    {
        //列出走法
        int pcCaptured;
        if (makeMove(mvs[i], pcCaptured))
        {
            int vl = -alphaBetaSearch(depth - 1, -beta, -alpha);
            undoMakeMove(mvs[i], pcCaptured);
            //qDebug()<<mvString(mvs[i])<<" vl="<<vl;

            //进行Alpha-Beta大小判断和截断
            if (vl > vlBest)
            {
                // 找到最佳值(但不能确定是Alpha、PV还是Beta走法)
                vlBest = vl;        // "vlBest"就是目前要返回的最佳值，可能超出Alpha-Beta边界
                if (vl >= beta)
                {
                    // 找到一个Beta走法
                    mvBest = mvs[i];  // Beta走法要保存到历史表
                    break;            // Beta截断
                }
                if (vl > alpha)
                {
                    // 找到一个PV走法
                    mvBest = mvs[i];  // PV走法要保存到历史表
                    alpha = vl;     // 缩小Alpha-Beta边界
                }
            }
        }
    }

    // 5. 所有走法都搜索完了，把最佳走法(不能是Alpha走法)保存到历史表，返回最佳值
    if (vlBest == -MATE_VALUE)
    {
      // 如果是杀棋，就根据杀棋步数给出评价
      return m_distance - MATE_VALUE;
    }
    //qDebug()<<"mvBest="<<mvBest<<" distance="<<m_distance;
    if (mvBest != 0)
    {
      // 如果不是Alpha走法，就将最佳走法保存到历史表
      m_historyTable[mvBest] += depth * depth;
      if (m_distance==0)
      {
        // 搜索根节点时，总是有一个最佳走法(因为全窗口搜索不会超出边界)，将这个走法保存下来
        m_mvComputer = mvBest;
      }
    }
    return vlBest;
}

void ChessBoard::alphaBetaSearchMain()
{
    m_searchCallTimes = 0;
    m_distance = 0;
    m_mvComputer = 0;
    memset(m_historyTable, 0, 65536 * sizeof(int));

    for (int i = 1; i <= 6; i ++)
    {
        int vl = alphaBetaSearch(i, -MATE_VALUE, MATE_VALUE);
        if (vl > WIN_VALUE || vl < -WIN_VALUE)
        {
            break;
        }
    }
}

void ChessBoard::test1()
{
//    m_searchCallTimes = 0;
//    m_distance = 0;
//    m_mvComputer = 0;
//    //negaMax(4, m_mvComputer);

//    memset(m_historyTable, 0, 65536 * sizeof(int));
//    alphaBetaSearch(6, -99999, 99999);
    alphaBetaSearchMain();






    if (m_mvComputer!=0)
    {
        qDebug()<<"电脑出招"<<mvString(m_mvComputer)<<" calltimes:"<<m_searchCallTimes;
    }
    int pcCaptured;
    makeMove(m_mvComputer, pcCaptured);
}




}
