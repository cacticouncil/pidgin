/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCommandLinkButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QListWidget *ProtocolListWidget;
    QCommandLinkButton *commandLinkButton;
    QLineEdit *slackWSLineEdit;
    QLineEdit *userLineEdit;
    QLineEdit *passLineEdit;
    QLabel *slackWSLabel;
    QLabel *WSLabel;
    QLabel *userLabel;
    QLabel *passLabel;
    QLabel *label_3;
    QLabel *label_4;
    QPushButton *pushButton;
    QLabel *label_5;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(391, 602);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        ProtocolListWidget = new QListWidget(centralwidget);
        new QListWidgetItem(ProtocolListWidget);
        new QListWidgetItem(ProtocolListWidget);
        new QListWidgetItem(ProtocolListWidget);
        new QListWidgetItem(ProtocolListWidget);
        new QListWidgetItem(ProtocolListWidget);
        new QListWidgetItem(ProtocolListWidget);
        new QListWidgetItem(ProtocolListWidget);
        new QListWidgetItem(ProtocolListWidget);
        new QListWidgetItem(ProtocolListWidget);
        new QListWidgetItem(ProtocolListWidget);
        new QListWidgetItem(ProtocolListWidget);
        ProtocolListWidget->setObjectName(QString::fromUtf8("ProtocolListWidget"));
        ProtocolListWidget->setGeometry(QRect(40, 50, 311, 231));
        commandLinkButton = new QCommandLinkButton(centralwidget);
        commandLinkButton->setObjectName(QString::fromUtf8("commandLinkButton"));
        commandLinkButton->setGeometry(QRect(260, 420, 101, 41));
        slackWSLineEdit = new QLineEdit(centralwidget);
        slackWSLineEdit->setObjectName(QString::fromUtf8("slackWSLineEdit"));
        slackWSLineEdit->setGeometry(QRect(112, 310, 120, 22));
        userLineEdit = new QLineEdit(centralwidget);
        userLineEdit->setObjectName(QString::fromUtf8("userLineEdit"));
        userLineEdit->setGeometry(QRect(112, 350, 241, 22));
        passLineEdit = new QLineEdit(centralwidget);
        passLineEdit->setObjectName(QString::fromUtf8("passLineEdit"));
        passLineEdit->setGeometry(QRect(112, 390, 241, 22));
        passLineEdit->setEchoMode(QLineEdit::Password);
        slackWSLabel = new QLabel(centralwidget);
        slackWSLabel->setObjectName(QString::fromUtf8("slackWSLabel"));
        slackWSLabel->setGeometry(QRect(40, 310, 71, 16));
        WSLabel = new QLabel(centralwidget);
        WSLabel->setObjectName(QString::fromUtf8("WSLabel"));
        WSLabel->setGeometry(QRect(235, 315, 71, 16));
        userLabel = new QLabel(centralwidget);
        userLabel->setObjectName(QString::fromUtf8("userLabel"));
        userLabel->setGeometry(QRect(40, 350, 71, 16));
        passLabel = new QLabel(centralwidget);
        passLabel->setObjectName(QString::fromUtf8("passLabel"));
        passLabel->setGeometry(QRect(40, 390, 71, 16));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(40, 30, 121, 16));
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(40, 440, 55, 16));
        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(150, 520, 93, 28));
        label_5 = new QLabel(centralwidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(40, 460, 311, 31));
        label_5->setFrameShape(QFrame::Box);
        label_5->setFrameShadow(QFrame::Plain);
        label_5->setTextFormat(Qt::RichText);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 391, 26));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);
        QObject::connect(pushButton, SIGNAL(clicked()), MainWindow, SLOT(close()));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));

        const bool __sortingEnabled = ProtocolListWidget->isSortingEnabled();
        ProtocolListWidget->setSortingEnabled(false);
        QListWidgetItem *___qlistwidgetitem = ProtocolListWidget->item(0);
        ___qlistwidgetitem->setText(QApplication::translate("MainWindow", "0: AIM", nullptr));
        QListWidgetItem *___qlistwidgetitem1 = ProtocolListWidget->item(1);
        ___qlistwidgetitem1->setText(QApplication::translate("MainWindow", "1: Bonjour", nullptr));
        QListWidgetItem *___qlistwidgetitem2 = ProtocolListWidget->item(2);
        ___qlistwidgetitem2->setText(QApplication::translate("MainWindow", "2: Gadu-Gadu", nullptr));
        QListWidgetItem *___qlistwidgetitem3 = ProtocolListWidget->item(3);
        ___qlistwidgetitem3->setText(QApplication::translate("MainWindow", "3: GroupWise", nullptr));
        QListWidgetItem *___qlistwidgetitem4 = ProtocolListWidget->item(4);
        ___qlistwidgetitem4->setText(QApplication::translate("MainWindow", "4: ICQ", nullptr));
        QListWidgetItem *___qlistwidgetitem5 = ProtocolListWidget->item(5);
        ___qlistwidgetitem5->setText(QApplication::translate("MainWindow", "5: IRC", nullptr));
        QListWidgetItem *___qlistwidgetitem6 = ProtocolListWidget->item(6);
        ___qlistwidgetitem6->setText(QApplication::translate("MainWindow", "6: SIMPLE", nullptr));
        QListWidgetItem *___qlistwidgetitem7 = ProtocolListWidget->item(7);
        ___qlistwidgetitem7->setText(QApplication::translate("MainWindow", "7: Sametime", nullptr));
        QListWidgetItem *___qlistwidgetitem8 = ProtocolListWidget->item(8);
        ___qlistwidgetitem8->setText(QApplication::translate("MainWindow", "8: Slack", nullptr));
        QListWidgetItem *___qlistwidgetitem9 = ProtocolListWidget->item(9);
        ___qlistwidgetitem9->setText(QApplication::translate("MainWindow", "9: XMPP", nullptr));
        QListWidgetItem *___qlistwidgetitem10 = ProtocolListWidget->item(10);
        ___qlistwidgetitem10->setText(QApplication::translate("MainWindow", "10: Zephyr", nullptr));
        ProtocolListWidget->setSortingEnabled(__sortingEnabled);

        commandLinkButton->setText(QApplication::translate("MainWindow", "Connect", nullptr));
        slackWSLabel->setText(QApplication::translate("MainWindow", "Workspace:", nullptr));
        WSLabel->setText(QApplication::translate("MainWindow", ".slack.com", nullptr));
        userLabel->setText(QApplication::translate("MainWindow", "Username:", nullptr));
        passLabel->setText(QApplication::translate("MainWindow", "Password:", nullptr));
        label_3->setText(QApplication::translate("MainWindow", "Select Chat Service", nullptr));
        label_4->setText(QApplication::translate("MainWindow", "Status:", nullptr));
        pushButton->setText(QApplication::translate("MainWindow", "EXIT", nullptr));
        label_5->setText(QApplication::translate("MainWindow", "<html><head/><body><p><span style=\" color:#ff0000;\">Not connected</span></p></body></html>", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
