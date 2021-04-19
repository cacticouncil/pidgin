/*
* Filename: hello.cpp
*
*
* File purpose: simple QT application that contains calls to libpurple library
*
*/

#include <QApplication>
#include <QLabel>
#include <QWidget>
#include <purple.h>
#include <string>
using namespace std;

int main(int argc, char *argv[])
{
	//create a QApplication
    QApplication app(argc, argv);
	//get libpurple version
    string helloMsg = "Hello from LibPurple Version " + string(purple_core_get_version());
    QString qstr = QString::fromStdString(helloMsg);
	//output to the widget using QLabel
    QLabel hello("<center>" + qstr + "</center>");
    hello.setWindowTitle("Hello LibPurple Program");
    hello.resize(400,400);
    hello.show();
    return app.exec();
}
