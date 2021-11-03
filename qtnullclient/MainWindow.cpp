//==============================================================================
//
// Filename: MainWindow.cpp
//
// File purpose: Contains Qt MainWindow class member function definitions.
// 
//==============================================================================

#include "MainWindow.h"
#include "SlackWindow.h"

#include "qtnullclient.h"

#include <iostream>
#include <string>
#include <QString>
#include <QLocale>

#include <ui_mainwindow.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connectionStatusId = 0;

    connect(ui->ProtocolListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(currentItemChanged(QListWidgetItem*,QListWidgetItem*)));
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::connectionStatusIdChanged(int value)
{
    if (value == 1)
        ui->label_5->setText(
            "<font color='green'>Account succesfully connected!</font>");
}


void MainWindow::setConnectionStatusId(int value)
{
    if (value != connectionStatusId)
    {
        connectionStatusId = value;
        connectionStatusIdChanged(value);
        
        SlackWindow *s = new SlackWindow;
        s->show();
    }   
}

int MainWindow::getConnectionStatusId()
{
    return connectionStatusId;
}

void MainWindow::currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (ui->ProtocolListWidget->currentRow() == 8)
    {
        ui->slackWSLineEdit->setVisible(true);
        ui->slackWSLabel->setVisible(true);
        ui->WSLabel->setVisible(true);
        ui->userLabel->setText("E-Mail:");
    }
    else
    {
        ui->slackWSLineEdit->setVisible(false);
        ui->slackWSLabel->setVisible(false);
        ui->WSLabel->setVisible(false);
        ui->userLabel->setText("Username:");
    }
}

// Important functionality that is triggered upon user clicking "Connect"
// button
void MainWindow::on_commandLinkButton_clicked()
{
    // TODO: Fix this. libpurple is not initialized here.
    ui->label_5->setText("libpurple initialized.");

    // Collect plugin option number from listview
    int purplePluginNum = static_cast<int>(ui->ProtocolListWidget->currentRow());
    
    // Collect login credentials from lineEdit text boxes
    QString q_username = ui->userLineEdit->text();
    QString q_password = ui->passLineEdit->text();

    if (purplePluginNum == 8)
    {
        q_username = ui->userLineEdit->text() + '%' + ui->slackWSLineEdit->text() + ".slack.com";
    }

    std::string purpleUsername = q_username.toStdString();
    std::string purplePassword = q_password.toStdString();

    ui->label_5->setText("Attempting to connect ...");
    connectPurplePluginProtocol(purplePluginNum, purpleUsername, purplePassword);

    return;
}