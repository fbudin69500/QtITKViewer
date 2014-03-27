#include "TestQt4.h"
#include <QApplication>


int main( int argc , char** argv )
{
  QApplication app(argc, argv);
  
  TestQt4 window;
  window.show();
  
  return app.exec();
  
}
