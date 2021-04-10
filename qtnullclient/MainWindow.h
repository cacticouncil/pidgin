//==============================================================================
//
// Filename: MainWindow.h
//
// File purpose: Contains Qt MainWindow class definition.
// 
//==============================================================================

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QLabel>
#include <QWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCommandLinkButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT                // needed for slots/signals

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setConnectionStatusId(int value);
	void connectionStatusIdChanged(int value);
	int getConnectionStatusId();

private slots:
	void on_commandLinkButton_clicked();

signals:
	
private:
    Ui::MainWindow *ui;
    int connectionStatusId;		// 0: disconnected
                                // 1: successfully connected
};

#endif // MAINWINDOW_H
