#include <QApplication>
#include <QLabel>
#include <QWidget>
#include <purple.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QLabel hello("<center>Hello!</center>");
    hello.setWindowTitle("Hello LibPurple Program");
    hello.resize(400,400);
    hello.show();
    return app.exec();
}
