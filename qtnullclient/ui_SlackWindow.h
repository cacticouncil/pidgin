/********************************************************************************
** Form generated from reading UI file 'SlackWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.2.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SLACKWINDOW_H
#define UI_SLACKWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SlackWindow
{
public:
    QWidget *centralwidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *SlackWindow)
    {
        if (SlackWindow->objectName().isEmpty())
            SlackWindow->setObjectName(QString::fromUtf8("SlackWindow"));
        SlackWindow->resize(1947, 1001);
        centralwidget = new QWidget(SlackWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        SlackWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(SlackWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1947, 20));
        SlackWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(SlackWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        SlackWindow->setStatusBar(statusbar);

        retranslateUi(SlackWindow);

        QMetaObject::connectSlotsByName(SlackWindow);
    } // setupUi

    void retranslateUi(QMainWindow *SlackWindow)
    {
        SlackWindow->setWindowTitle(QCoreApplication::translate("SlackWindow", "SlackWindow", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SlackWindow: public Ui_SlackWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SLACKWINDOW_H
