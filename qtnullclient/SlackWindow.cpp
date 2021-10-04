#include "SlackWindow.h"
#include "MainWindow.h"

#include "qtnullclient.h"

#include "ui_SlackWindow.h"

SlackWindow::SlackWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SlackWindow)
{
    ui->setupUi(this);
}

SlackWindow::~SlackWindow()
{
    delete ui;
}

