#include "include/QVideoPlayer.h"
#include <QtWidgets/QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QVideoPlayer w;
   
    w.show();
    return a.exec();
}
