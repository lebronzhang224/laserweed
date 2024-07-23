#include <QApplication>
#include "login.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   login x;
   x.show();
//   int flag=0;
//   if(x.close()){
//       flag=1;
//   }
//   if(flag==1)
//   {
//           MainWindow w;
//           w.show();
//   }

   return a.exec();
}
