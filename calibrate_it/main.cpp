#include "calibrate_defines.h"
#include "maindialog.h"

#include <QApplication>
#include <QMessageBox>
#include <exception>

////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Globals::Init();

    try {
        MainDialog maindlg(&app);
        maindlg.show();
        app.exec();
    }
    catch(const std::exception& ex) { 
        QMessageBox::critical(NULL, "Error", QString("Exception was thrown\n\"") + ex.what() + "\"", "Stop");
        return -1;
    }
    catch(...) { 
        QMessageBox::critical(NULL, "Error", "Unhandled exception was thrown!", "Stop");
        return -1;
    }
    return 0;
}
