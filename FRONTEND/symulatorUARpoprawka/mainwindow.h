#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QEvent>

#include "arxdialog.h"
#include "../../BACKEND/BACKEND/SymulatorUAR.h"
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_StartPB_clicked();
    void on_StopPB_clicked();
    void on_ResetPB_clicked();
    void updateChart(double czas, double dummy);

    void on_ARXpushButton_clicked();
    void on_zapiszPushButton_clicked();
    void on_wczytajPushButton_clicked();

private:
    Ui::MainWindow *ui;
    SymulatorUAR    m_symulator;
    ArxDialog       m_arxDialog;

    bool eventFilter(QObject *obj, QEvent *event) override;
    void odswiezKontrolkiPoWczytaniu();

    // obsługa osi czasu (wycinanie pauz)
    double m_timeOffset   = 0.0;   // zsumowana długość pauz
    bool   m_paused       = false; // czy jesteśmy po STOP
    double m_pauseStart   = 0.0;   // czas symulacji w momencie STOP

    // „przyklejenie” pierwszego punktu wykresu do t = 0
    double m_firstTime    = 0.0;   // czas pierwszego punktu (po offsetach)
    bool   m_haveFirstTime = false;

    void initPlots();
    void applyTimeWindowToPlots();
};

#endif // MAINWINDOW_H
