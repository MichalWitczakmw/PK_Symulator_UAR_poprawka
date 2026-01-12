#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class SymulatorUAR;
class Generator;
class Model_ARX;
class Regulator_PID;

QT_FORWARD_DECLARE_CLASS(QChartView)
QT_FORWARD_DECLARE_CLASS(QLineSeries)
QT_FORWARD_DECLARE_CLASS(QChart)

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
    void updateChart(double czas, double dummy);   // ZOSTAWIAMY TYLKO TO

private:
    Ui::MainWindow *ui;
    SymulatorUAR *m_symulator;
    QChartView *chartView;
    QLineSeries *series;
    QChart *chart;

    QChartView *chartViewY, *chartViewW, *chartViewE, *chartViewU;
    QLineSeries *seriesY, *seriesW, *seriesE, *seriesU;
    QChart *chartY, *chartW, *chartE, *chartU;
    QLineSeries *seriesP, *seriesI, *seriesD, *seriesY2;
};

#endif // MAINWINDOW_H
