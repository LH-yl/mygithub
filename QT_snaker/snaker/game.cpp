#include "game.h"
#include "ui_game.h"

game::game(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::game)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::StrongFocus);
    ui->UpButton->setFocusPolicy(Qt::StrongFocus);
    ui->DownButton->setFocusPolicy(Qt::StrongFocus);
    ui->LeftButton->setFocusPolicy(Qt::StrongFocus);
    ui->RightButton->setFocusPolicy(Qt::StrongFocus);
    init();
}

game::~game()
{
    delete ui;
}

void game::clearArgument()
{
    timer = new QTimer;
    m_Direction = 0;
    m_foodCount = 0;
    m_upgradeScore = 100;
    m_currentGrade = 1;
    m_currentSpeed = 200;

}
void game::paintEvent(QPaintEvent *event)
{
    //画游戏盘
   QPainter painter(this);
   //QRect Rect(0*RECTWIDTH,0*RECTHEIGHT,26*RECTWIDTH,22*RECTHEIGHT);
   //painter.drawRect(Rect);
   for(int x = 0;x < COL;++x)
   {
       for(int y = 0;y < ROW;y++)
       {
           QRect Rect(x*RECTWIDTH,y*RECTHEIGHT,RECTWIDTH,RECTHEIGHT);
           painter.drawRect(Rect);
       }
   }
   //画食物
   QRect Foodrect(FoodPosition[0]*RECTWIDTH,FoodPosition[1]*RECTHEIGHT,RECTWIDTH,RECTHEIGHT);
   painter.fillRect(Foodrect,Qt::yellow);
   //画蛇头
   QRect HeadRect(snake[0][0]*RECTWIDTH,snake[0][1]*RECTHEIGHT,RECTWIDTH,RECTHEIGHT);
   painter.fillRect(HeadRect,Qt::green);
   for(int snakebody = 1;snakebody <= m_foodCount;++snakebody)
   {
       QRect BodyRect(snake[snakebody][0]*RECTWIDTH,snake[snakebody][1]*RECTHEIGHT,RECTWIDTH,RECTHEIGHT);
       painter.fillRect(BodyRect,Qt::green);
   }

}
void game::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Left)
    {
        if(m_Direction == emRight)
        {
            return;
        }
        on_LeftButton_clicked();
    }
    else if(event->key() == Qt::Key_Right)
    {
        if(m_Direction == emLeft)
        {
            return;
        }
        on_RightButton_clicked();
    }
    else if(event->key() == Qt::Key_Up)
    {
        if(m_Direction == emDown)
        {
            return;
        }
        on_UpButton_clicked();
    }
    else if(event->key() == Qt::Key_Down)
    {
        if(m_Direction == emUp)
        {
            return;
        }
        on_DownButton_clicked();
    }
}
void game::initBackground()
{
    setAutoFillBackground(true);
    QPalette palette;
    palette.setBrush(QPalette::Background,QBrush(QPixmap("E:/QT/code/day4-code/snaker/image/gameruning.png")));
    this->setPalette(palette);
    /*
    ui->BackButton->setIcon(QIcon("E:/QT/code/day4-code/snaker/quit.png"));
    ui->BackButton->setIconSize(QSize(50,30));
    ui->BackButton->setFlat(true);*/
}
void game::initWindows()
{
    setWindowTitle("贪吃蛇");
    setWindowIcon(QIcon("E:/QT/code/day4-code/snaker/image/gameruning.png"));
}
void game::DrawSnakeHead()
{
    //确定蛇头位置
    snake[0][0] = qrand() % COL;
    snake[0][1] = qrand() % ROW;
    m_Direction = qrand() % 4;
}
void game::CreateFood()
{
    FoodPosition[0] = qrand() %COL;
    FoodPosition[1] = qrand() %ROW;
    //防止食物产生在蛇头上
    for(int bodyId = 0; bodyId <= m_foodCount;++bodyId)
    {
        if(FoodPosition[0] == snake[bodyId][0] && FoodPosition[1] == snake[bodyId][1])
        {
            FoodPosition[0] = qrand() % COL - 1;
            FoodPosition[1] = qrand() % ROW - 1;
        }
    }
}
void game::GameOver()
{
    timer->stop();
    QMessageBox::information(this,"提示","游戏结束");
    ui->LevellineEdit->setText("1");
    ui->ScorellineEdit->setText("0");
}
void game::UpGrade()
{
    if(ui->ScorellineEdit->text().toInt() >= m_upgradeScore*1)
    {
        m_upgradeScore +=100;
        ++m_currentGrade;
        if(m_currentSpeed > 50)
        {
            m_currentSpeed -= 50;
        }
        timer->stop();
        QMessageBox::information(this,"提示","升级成功，进入下一级");
        timer->start(m_currentSpeed);

    }
}
void game::FlashScore()
{
    ui->LevellineEdit->setText(QString::number(m_currentGrade));
    ui->ScorellineEdit->setText(QString::number(m_foodCount *10));
}
void game::snakeHeadRun()
{
    switch (m_Direction) {
    case emUp:
         --snake[0][1];
        break;
    case emDown:
         ++snake[0][1];
        break;
    case emLeft:
        --snake[0][0];
        break;
    case emRight:
        ++snake[0][0];
        break;
    default:
        break;
    }
}
void game::CheckEatFood()
{
    if(snake[0][0] == FoodPosition[0] && snake[0][1] == FoodPosition[1])
    {
        ++m_foodCount;
        UpGrade();
        FlashScore();
        CreateFood();
    }
    //蛇身体的添加
    for(int snId = m_foodCount;snId > 0;--snId)
    {
        snake[snId][0] = snake[snId - 1][0];
        snake[snId][1] = snake[snId - 1][1];
    }
    snakeHeadRun();
}
void game::TimeOutSlots()
{
    if(snake[0][0] > (COL -1) || snake[0][0] < 0 ||snake[0][1] > (ROW -1) || snake[0][1] < 0)
    {
        GameOver();
    }
    for(int i = 1; i < m_foodCount;++i)
    {
        if(snake[0][0] == snake[i][0] && snake[0][1] == snake[i][1])
        {
            GameOver();
        }
    }
    CheckEatFood();
    update();
}
void game::initConncet()
{
    connect(timer,&QTimer::timeout,this,&game::TimeOutSlots);
    timer->start(m_currentSpeed);
}
void game::init()
{
    clearArgument();
    initBackground();
    initWindows();
    DrawSnakeHead();
    CreateFood();
    initConncet();

}

void game::on_UpButton_clicked()
{
    m_Direction = emUp;
}

void game::on_LeftButton_clicked()
{
    m_Direction = emLeft;
}

void game::on_RightButton_clicked()
{
    m_Direction = emRight;
}

void game::on_DownButton_clicked()
{
    m_Direction = emDown;
}

void game::on_BackButton_clicked()
{
    this->close();
}
