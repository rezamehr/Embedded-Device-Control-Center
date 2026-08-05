
#include <QApplication>
#include <QMainWindow>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("Embedded Device Control Center");
    window.resize(1100, 700);
    window.show();

    return app.exec();
}
