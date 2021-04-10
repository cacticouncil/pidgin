//==============================================================================
//
// Filename: MainWindow.cpp
//
// File purpose: Contains Qt MainWindow class member function definitions.
// 
//==============================================================================

#include "MainWindow.h"
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
    }   
}

int MainWindow::getConnectionStatusId()
{
    return connectionStatusId;
}

// Important functionality that is triggered upon user clicking "Connect"
// button
void MainWindow::on_commandLinkButton_clicked()
{
    // TODO: Fix this. libpurple is not initialized here.
    ui->label_5->setText("libpurple initialized.");

    // Collect plugin option number from listview
    int purplePluginNum = static_cast<int>(ui->listWidget->currentRow());
    
    // Collect login credentials from lineEdit text boxes
    QString q_username = ui->lineEdit->text();
    QString q_password = ui->lineEdit_2->text();

    std::string purpleUsername = q_username.toStdString();
    std::string purplePassword = q_password.toStdString();

    ui->label_5->setText("Attempting to connect ...");
    
    connectPurplePluginProtocol(purplePluginNum, purpleUsername,
        purplePassword);

    return;
}