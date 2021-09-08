/********************************************************************************
** Form generated from reading UI file 'game.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GAME_H
#define UI_GAME_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_game
{
public:
    QPushButton *DownButton;
    QPushButton *UpButton;
    QPushButton *RightButton;
    QPushButton *LeftButton;
    QLineEdit *ScorellineEdit;
    QLineEdit *LevellineEdit;
    QLabel *Scorelabel;
    QLabel *Levellabel;
    QPushButton *BackButton;

    void setupUi(QWidget *game)
    {
        if (game->objectName().isEmpty())
            game->setObjectName(QString::fromUtf8("game"));
        game->resize(800, 460);
        game->setMinimumSize(QSize(800, 460));
        game->setMaximumSize(QSize(800, 460));
        DownButton = new QPushButton(game);
        DownButton->setObjectName(QString::fromUtf8("DownButton"));
        DownButton->setGeometry(QRect(630, 380, 80, 80));
        DownButton->setMinimumSize(QSize(80, 80));
        DownButton->setMaximumSize(QSize(80, 80));
        QFont font;
        font.setFamily(QString::fromUtf8("\351\273\221\344\275\223"));
        font.setPointSize(20);
        DownButton->setFont(font);
        UpButton = new QPushButton(game);
        UpButton->setObjectName(QString::fromUtf8("UpButton"));
        UpButton->setGeometry(QRect(630, 220, 80, 80));
        UpButton->setMinimumSize(QSize(80, 80));
        UpButton->setMaximumSize(QSize(80, 80));
        UpButton->setFont(font);
        RightButton = new QPushButton(game);
        RightButton->setObjectName(QString::fromUtf8("RightButton"));
        RightButton->setGeometry(QRect(720, 300, 80, 80));
        RightButton->setMinimumSize(QSize(80, 80));
        RightButton->setMaximumSize(QSize(80, 80));
        RightButton->setFont(font);
        LeftButton = new QPushButton(game);
        LeftButton->setObjectName(QString::fromUtf8("LeftButton"));
        LeftButton->setGeometry(QRect(540, 300, 80, 80));
        LeftButton->setMinimumSize(QSize(80, 80));
        LeftButton->setMaximumSize(QSize(80, 80));
        LeftButton->setFont(font);
        ScorellineEdit = new QLineEdit(game);
        ScorellineEdit->setObjectName(QString::fromUtf8("ScorellineEdit"));
        ScorellineEdit->setGeometry(QRect(640, 10, 151, 41));
        ScorellineEdit->setFont(font);
        LevellineEdit = new QLineEdit(game);
        LevellineEdit->setObjectName(QString::fromUtf8("LevellineEdit"));
        LevellineEdit->setGeometry(QRect(640, 90, 151, 41));
        LevellineEdit->setFont(font);
        Scorelabel = new QLabel(game);
        Scorelabel->setObjectName(QString::fromUtf8("Scorelabel"));
        Scorelabel->setGeometry(QRect(530, 10, 111, 41));
        Levellabel = new QLabel(game);
        Levellabel->setObjectName(QString::fromUtf8("Levellabel"));
        Levellabel->setGeometry(QRect(530, 90, 101, 41));
        BackButton = new QPushButton(game);
        BackButton->setObjectName(QString::fromUtf8("BackButton"));
        BackButton->setGeometry(QRect(730, 420, 71, 41));

        retranslateUi(game);

        QMetaObject::connectSlotsByName(game);
    } // setupUi

    void retranslateUi(QWidget *game)
    {
        game->setWindowTitle(QCoreApplication::translate("game", "Form", nullptr));
        DownButton->setText(QCoreApplication::translate("game", "\342\206\223", nullptr));
        UpButton->setText(QCoreApplication::translate("game", "\342\206\221", nullptr));
        RightButton->setText(QCoreApplication::translate("game", "\342\206\222", nullptr));
        LeftButton->setText(QCoreApplication::translate("game", "\342\206\220", nullptr));
        ScorellineEdit->setText(QCoreApplication::translate("game", "    0", nullptr));
        LevellineEdit->setText(QCoreApplication::translate("game", "    1", nullptr));
        Scorelabel->setText(QCoreApplication::translate("game", "<html><head/><body><p align=\"center\"><span style=\" font-size:20pt;\">Score\357\274\232</span></p></body></html>", nullptr));
        Levellabel->setText(QCoreApplication::translate("game", "<html><head/><body><p align=\"center\"><span style=\" font-size:20pt;\">Level\357\274\232</span></p></body></html>", nullptr));
        BackButton->setText(QCoreApplication::translate("game", "Back", nullptr));
    } // retranslateUi

};

namespace Ui {
    class game: public Ui_game {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GAME_H
