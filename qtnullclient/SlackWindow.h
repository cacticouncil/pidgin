#ifndef SLACKWINDOW_H
#define SLACKWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class SlackWindow; }
QT_END_NAMESPACE

class SlackWindow : public QMainWindow
{
    Q_OBJECT

public:
    SlackWindow(QWidget *parent = nullptr);
    ~SlackWindow();

private:
    Ui::SlackWindow *ui;
};
#endif // SLACKWINDOW_H
