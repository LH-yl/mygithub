#ifndef GAME_H
#define GAME_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QMessageBox>
#define COL 26 //列
#define ROW 22  //行
#define RECTWIDTH 20
#define RECTHEIGHT 20

namespace Ui {
class game;
}

class game : public QWidget
{
    Q_OBJECT

public:
    explicit game(QWidget *parent = nullptr);
    ~game();
    enum EmDirection
    {
        emUp = 0,
        emDown,
        emLeft,
        emRight
    };
protected:
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
private:
    void clearArgument();
    void init();
    void initBackground();
    void initWindows();
    void DrawSnakeHead();
    void CreateFood();
    void initConncet();
    void GameOver();
    void CheckEatFood();
    void UpGrade();
    void FlashScore();
    void snakeHeadRun();
private slots:
    void TimeOutSlots();

    void on_UpButton_clicked();

    void on_LeftButton_clicked();

    void on_RightButton_clicked();

    void on_DownButton_clicked();

    void on_BackButton_clicked();

private:
    Ui::game *ui;
    int FoodPosition[2];
    int snake[100][2];
    QTimer *timer;
    int m_Direction;
    int m_foodCount;
    int m_upgradeScore;
    int m_currentGrade;
    int m_currentSpeed;
};

#endif // GAMMINGWIDGET_H

