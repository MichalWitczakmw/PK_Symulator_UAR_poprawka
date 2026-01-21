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
    // przyciski START / STOP / RESET
    void on_StartPB_clicked();
    void on_StopPB_clicked();
    void on_ResetPB_clicked();

    // slot aktualizacji wykresów wywoływany przez SymulatorUAR::dataUpdated
    void aktualizujWykresy(double czas, double dummy);

    // ARX, zapis, odczyt, reset pamięci regulatora
    void on_ARXpushButton_clicked();
    void on_zapiszPushButton_clicked();
    void on_wczytajPushButton_clicked();
    void on_resetTiTdPIDpushButton_clicked();

private:
    Ui::MainWindow *ui;
    SymulatorUAR    m_symulator;
    ArxDialog       m_arxDialog;

    // filtr zdarzeń (Enter na spinboxach i innych kontrolkach)
    bool eventFilter(QObject *obj, QEvent *event) override;

    // odświeżenie kontrolek po wczytaniu konfiguracji
    void odswiezKontrolkiPoWczytaniu();

    // obsługa osi czasu (wycinanie pauz)
    double m_timeOffset    = 0.0;  // zsumowana długość pauz
    bool   m_paused        = false; // czy jesteśmy po STOP
    double m_pauseStart    = 0.0;   // czas symulacji w momencie STOP

    // „przyklejenie” pierwszego punktu wykresu do t = 0
    double m_firstTime     = 0.0;   // czas pierwszego punktu (po offsetach)
    bool   m_haveFirstTime = false;

    // inicjalizacja i konfiguracja wykresów
    void inicjalizujWykresy();
    void zastosujOknoCzasuDoWykresow();
};

#endif // MAINWINDOW_H
