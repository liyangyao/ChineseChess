/****************************************************************************

Copyright (C) 2013 QiAn Software
Contact: http://www.0514soft.com

Creator: liyangyao@gmail.com
Date: 2014/10/25

****************************************************************************/

#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QObject>
#include <QPixmap>

namespace CC
{

class ChessBoard
{
public:
    typedef QVector<int> Moves;
    ChessBoard();
    static bool isInBoard(int sq);
    inline static int sq(int x, int y)
    {
        return (y + 3) * 16 + x + 3;
    }

    inline static void xy(int sq, int& x, int& y)
    {
        x = sq % 16 - 3;
        y = sq / 16 - 3;
    }

    // 根据起点和终点获得走法
    inline static int mv(int sqSrc, int sqDst)
    {
        return sqSrc + sqDst * 256;
    }

    void updatePixmap();

    QPixmap& currentPixmap();
    bool setSelected(int sq);
    int selected();
    int _movePiece(int mv);
    void _undoMovePiece(int mv, int pcCaptured);
    bool makeMove(int mv, int& pcCaptured);
    void undoMakeMove(int mv, int pcCaptured);


    bool isLegalMove(int mv);
    void generateMoves(int sq, Moves& mvs);
    void generateMoves(Moves& mvs);
    bool isChecked();
    bool isChecked(int side);
    bool IsMate(int side);
    bool IsMate();

    QString mvString(int mv);
    int test1_max(int depth, int& mv);
    int test1_min(int depth, int& mv);
    void test1();
    int negaMax(int depth, int& mv);
    int alphaBetaSearch(int depth, int alpha, int beta);
    void alphaBetaSearchMain();

private:
    quint8 m_data[256];
    quint8 m_selected;
    int m_sdPlayer;// 轮到谁走，0=红方，1=黑方
    QPixmap m_currentPixmap;
    int m_pcWidth;
    int m_pcWidthDelta;
    int m_pcHeight;
    int m_pcHeightDelta;
    int m_vlRed;
    int m_vlBlack;
    int m_distance;
    int m_mvComputer;
    int m_searchCallTimes;
    void addPiece(int sq, int pc);
    void delPiece(int sq, int pc);
    void changeSide();
    int evaluate();




private:
    static bool canMove(ChessBoard& board, int sqSrc, int sqDst);
    static void generateKingMoves(ChessBoard& board, int sqSrc, Moves& mvs);

    static void generateAdvisorMoves(ChessBoard& board, int sqSrc, Moves& mvs);
    static void generateBishopMoves(ChessBoard& board, int sqSrc, Moves& mvs);
    static void generateKnightMoves(ChessBoard& board, int sqSrc, Moves& mvs);
    static void generateRookMoves(ChessBoard& board, int sqSrc, Moves& mvs);
    static void generateCannonMoves(ChessBoard& board, int sqSrc, Moves& mvs);
    static void generatePawnMoves(ChessBoard& board, int sqSrc, Moves& mvs);



};
}

#endif // CHESSBOARD_H
