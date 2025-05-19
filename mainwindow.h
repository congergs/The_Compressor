#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushCompress_clicked();

    void on_pushDecompress_clicked();

    void on_selectFile_clicked();

    void on_selectMethod_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    QString fileSrc;
    bool lossy = 1;
};
#endif // MAINWINDOW_H
