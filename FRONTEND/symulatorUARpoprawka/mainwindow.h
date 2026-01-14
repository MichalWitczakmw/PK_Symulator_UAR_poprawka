#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "arxdialog.h"
#include "../../BACKEND/BACKEND/SymulatorUAR.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

QT_FORWARD_DECLARE_CLASS(QChartView)
QT_FORWARD_DECLARE_CLASS(QLineSeries)
QT_FORWARD_DECLARE_CLASS(QChart)

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

    void on_ARXpushButton_clicked();   // klik ARX – otwarcie dialogu

    bool eventFilter(QObject *obj, QEvent *event);

    void on_zapiszPushButton_clicked();

    void on_wczytajPushButton_clicked();

private:
    Ui::MainWindow *ui;
    SymulatorUAR m_symulator;
    // dialog ARX
    ArxDialog m_arxDialog;

    // stare jednoseryjne rzeczy – mozesz zostawic, ale NIE beda uzywane
    QChartView *chartView;
    QLineSeries *series;
    QChart *chart;

    // 4 wykresy: PID, w+y, e, u
    QChartView *chartViewY;
    QChartView *chartViewW;
    QChartView *chartViewE;
    QChartView *chartViewU;

    // serie
    QLineSeries *seriesP;    // P
    QLineSeries *seriesI;    // I
    QLineSeries *seriesD;    // D
    QLineSeries *seriesY2;   // y (regulowana)

    QLineSeries *seriesW;    // w (zadana)
    QLineSeries *seriesE;    // e (uchyb)
    QLineSeries *seriesU;    // u (sterowanie)

    // obiekty wykresow
    QChart *chartY;
    QChart *chartW;
    QChart *chartE;
    QChart *chartU;

    void odswiezKontrolkiPoWczytaniu();
};

#endif // MAINWINDOW_H
