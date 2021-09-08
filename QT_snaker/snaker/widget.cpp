#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    init();
}

Widget::~Widget()
{
    delete ui;
}
void Widget::initBackground()
{
    setAutoFillBackground(true);
    QPalette palette;
    palette.setBrush(QPalette::Background,QBrush(QPixmap("E:/QT/code/day4-code/snaker/image/background.png")));
    setPalette(palette);

    ui->StartButton->setIcon(QIcon("E:/QT/code/day4-code/snaker/image/start.png"));
    ui->StartButton->setIconSize(QSize(120,120));
    ui->StartButton->setFlat(true);

    ui->CloseButton->setIcon(QIcon("E:/QT/code/day4-code/snaker/image/quit.png"));
    ui->CloseButton->setIconSize(QSize(120,120));
    ui->CloseButton->setFlat(true);

}
void Widget::initWindows()
{
    setWindowTitle("贪吃蛇");
   // QIcon con("E:/QT/code/day4-code/snaker/image/ico.png");
    setWindowIcon(QIcon("E:/QT/code/day4-code/snaker/image/ico.png"));
}
void Widget::init()
{
    initWindows();
    initBackground();
}
void Widget::on_StartButton_clicked()
{
    game *widget = new game(this);
    widget->show();
}

void Widget::on_CloseButton_clicked()
{
    this->close();
}
