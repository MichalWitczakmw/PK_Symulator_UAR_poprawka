#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "../../BACKEND/BACKEND/SymulatorUAR.h"

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
    void on_StartPB_clicked();

    void on_StopPB_clicked();

    void on_ResetPB_clicked();

    void aktualizujWyswietlacz();

private:
    Ui::MainWindow *ui;

    SymulatorUAR m_symulator;
};
#endif // MAINWINDOW_H
