#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>

#include "../../BACKEND/BACKEND/SymulatorUAR.h"

//using namespace QtCharts;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_symulator(new SymulatorUAR(this))
    , chartViewY(nullptr), chartViewW(nullptr), chartViewE(nullptr), chartViewU(nullptr)
    , seriesP(nullptr), seriesI(nullptr), seriesD(nullptr), seriesY2(nullptr)
    , seriesW(nullptr), seriesE(nullptr), seriesU(nullptr)
    , chartY(nullptr), chartW(nullptr), chartE(nullptr), chartU(nullptr)
    , m_arxDialog(nullptr)
{
    ui->setupUi(this);

    // Możesz ustawić tylko startowy rozmiar, ale BEZ blokowania skalowania
    this->resize(1200, 650);

    // (opcjonalnie) minimalne wysokości groupBoxów z lewej strony
    ui->symulacjagroupBox->setMinimumHeight(140);
    ui->PIDgroupBox->setMinimumHeight(140);
    ui->regulatorgroupBox->setMinimumHeight(160);
    ui->zapOdczgroupBox->setMinimumHeight(80);

    // ====== KONTENER NA 4 WYKRESY (wewnątrz wykresygroupBox) ======
    QWidget *chartsContainer = new QWidget(this);
    QVBoxLayout *chartsLayout = new QVBoxLayout(chartsContainer);
    chartsLayout->setSpacing(2);
    chartsLayout->setContentsMargins(0, 0, 0, 0);

    // ------------------------------------------------------------------
    // CH1: PID (P czerwony, I żółty, D niebieski)
    // ------------------------------------------------------------------
    chartY = new QChart();
    chartY->setTitle("P (czerw), I (żół), D (nieb)");

    seriesP = new QLineSeries();
    seriesP->setName("P");
    QPen penP(QColor(255, 0, 0), 2);
    seriesP->setPen(penP);
    chartY->addSeries(seriesP);

    seriesI = new QLineSeries();
    seriesI->setName("I");
    QPen penI(QColor(255, 255, 0), 2);
    seriesI->setPen(penI);
    chartY->addSeries(seriesI);

    seriesD = new QLineSeries();
    seriesD->setName("D");
    QPen penD(QColor(0, 0, 255), 2);
    seriesD->setPen(penD);
    chartY->addSeries(seriesD);

    chartY->createDefaultAxes();
    chartY->axisX()->setTitleText("Czas [s]");
    chartY->legend()->setVisible(true);
    chartY->axisX()->setMinorGridLineVisible(true);

    if (auto *ay = qobject_cast<QValueAxis *>(chartY->axisY()))
        ay->setRange(-2.0, 2.0);

    chartViewY = new QChartView(chartY, this);
    chartViewY->setRenderHint(QPainter::Antialiasing);
    chartViewY->setMinimumHeight(150);
    chartsLayout->addWidget(chartViewY);

    // ------------------------------------------------------------------
    // CH2: w (żółty, dashed) + y (zielony)
    // ------------------------------------------------------------------
    chartW = new QChart();
    chartW->setTitle("w (zad żół dashed), y (reg ziel)");

    seriesW = new QLineSeries();
    seriesW->setName("w");
    QPen penW(QColor(255, 255, 0), 2);
    penW.setStyle(Qt::DashLine);
    seriesW->setPen(penW);
    chartW->addSeries(seriesW);

    seriesY2 = new QLineSeries();
    seriesY2->setName("y");
    QPen penY2(QColor(0, 255, 0), 2);
    seriesY2->setPen(penY2);
    chartW->addSeries(seriesY2);

    chartW->createDefaultAxes();
    chartW->axisX()->setTitleText("Czas [s]");
    chartW->legend()->setVisible(true);
    chartW->axisX()->setMinorGridLineVisible(true);

    if (auto *ay = qobject_cast<QValueAxis *>(chartW->axisY()))
        ay->setRange(-2.0, 2.0);

    chartViewW = new QChartView(chartW, this);
    chartViewW->setRenderHint(QPainter::Antialiasing);
    chartViewW->setMinimumHeight(150);
    chartsLayout->addWidget(chartViewW);

    // ------------------------------------------------------------------
    // CH3: e (uchyb)
    // ------------------------------------------------------------------
    chartE = new QChart();
    chartE->setTitle("uchyb");

    seriesE = new QLineSeries();
    seriesE->setName("e");
    QPen penE(QColor(0, 0, 255), 2);
    seriesE->setPen(penE);
    chartE->addSeries(seriesE);

    chartE->createDefaultAxes();
    chartE->axisX()->setTitleText("Czas [s]");
    chartE->legend()->setVisible(true);
    chartE->axisX()->setMinorGridLineVisible(true);

    if (auto *ay = qobject_cast<QValueAxis *>(chartE->axisY()))
        ay->setRange(-2.0, 2.0);

    chartViewE = new QChartView(chartE, this);
    chartViewE->setRenderHint(QPainter::Antialiasing);
    chartViewE->setMinimumHeight(150);
    chartsLayout->addWidget(chartViewE);

    // ------------------------------------------------------------------
    // CH4: u (sterowanie)
    // ------------------------------------------------------------------
    chartU = new QChart();
    chartU->setTitle("sterowanie");

    seriesU = new QLineSeries();
    seriesU->setName("u");
    QPen redPen(QColor(255, 0, 0), 2);
    seriesU->setPen(redPen);
    chartU->addSeries(seriesU);

    chartU->createDefaultAxes();
    chartU->axisX()->setTitleText("Czas [s]");
    chartU->legend()->setVisible(true);
    chartU->axisX()->setMinorGridLineVisible(true);

    if (auto *ay = qobject_cast<QValueAxis *>(chartU->axisY()))
        ay->setRange(-2.0, 2.0);

    chartViewU = new QChartView(chartU, this);
    chartViewU->setRenderHint(QPainter::Antialiasing);
    chartViewU->setMinimumHeight(150);
    chartsLayout->addWidget(chartViewU);

    // ====== WŁOŻENIE WYKRESÓW DO groupBox ======
    // (główny layout okna jest zrobiony w Designerze)
    QVBoxLayout *wykresyLayout = new QVBoxLayout(ui->groupBox);
    wykresyLayout->setSpacing(2);
    wykresyLayout->setContentsMargins(10, 10, 10, 10);
    wykresyLayout->addWidget(chartsContainer);

    ui->groupBox->setStyleSheet(
        "QGroupBox { border: 2px solid #4A90E2; border-radius: 8px; "
        "margin-top: 5px; } "
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; "
        "padding: 0 5px; }");

    // ====== CONNECTY ======
    connect(ui->StartPB, &QPushButton::clicked,
            this, &MainWindow::on_StartPB_clicked);
    connect(ui->StopPB, &QPushButton::clicked,
            this, &MainWindow::on_StopPB_clicked);
    connect(ui->ResetPB, &QPushButton::clicked,
            this, &MainWindow::on_ResetPB_clicked);

    ui->StopPB->setEnabled(false);
    ui->ResetPB->setEnabled(false);

    connect(m_symulator, &SymulatorUAR::dataUpdated,
            this, &MainWindow::updateChart);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ===================================================================
// START / STOP / RESET
// ===================================================================

void MainWindow::on_StartPB_clicked()
{
    m_symulator->uruchom(200);

    ui->StartPB->setEnabled(false);
    ui->StopPB->setEnabled(true);
    ui->ResetPB->setEnabled(true);

    if (seriesP)  seriesP->clear();
    if (seriesI)  seriesI->clear();
    if (seriesD)  seriesD->clear();
    if (seriesW)  seriesW->clear();
    if (seriesY2) seriesY2->clear();
    if (seriesE)  seriesE->clear();
    if (seriesU)  seriesU->clear();
}

void MainWindow::on_StopPB_clicked()
{
    m_symulator->zatrzymaj();

    ui->StartPB->setEnabled(true);
    ui->StopPB->setEnabled(false);
    ui->ResetPB->setEnabled(true);
}

void MainWindow::on_ResetPB_clicked()
{
    m_symulator->resetuj();

    ui->StartPB->setEnabled(!m_symulator->isRunning());
    ui->StopPB->setEnabled(m_symulator->isRunning());
    ui->ResetPB->setEnabled(true);

    if (seriesP)  seriesP->clear();
    if (seriesI)  seriesI->clear();
    if (seriesD)  seriesD->clear();
    if (seriesW)  seriesW->clear();
    if (seriesY2) seriesY2->clear();
    if (seriesE)  seriesE->clear();
    if (seriesU)  seriesU->clear();
}

// ===================================================================
// Aktualizacja 4 wykresów: P/I/D, w+y, e, u
// ===================================================================

void MainWindow::updateChart(double czas, double /*dummy*/)
{
    double w = m_symulator->getWartoscZadana();
    double y = m_symulator->getWartoscRegulowana();
    double e = m_symulator->getUchyb();
    double u = m_symulator->getSterowanie();
    double p = m_symulator->getSkladowaP();
    double i = m_symulator->getSkladowaI();
    double d = m_symulator->getSkladowaD();

    static const int MAX_POINTS = 800;
    double left = qMax(0.0, czas - 15.0);

    if (seriesP)  seriesP->append(czas, p);
    if (seriesI)  seriesI->append(czas, i);
    if (seriesD)  seriesD->append(czas, d);
    if (seriesW)  seriesW->append(czas, w);
    if (seriesY2) seriesY2->append(czas, y);
    if (seriesE)  seriesE->append(czas, e);
    if (seriesU)  seriesU->append(czas, u);

    if (seriesP  && seriesP->count()  > MAX_POINTS) seriesP->removePoints(0, 1);
    if (seriesI  && seriesI->count()  > MAX_POINTS) seriesI->removePoints(0, 1);
    if (seriesD  && seriesD->count()  > MAX_POINTS) seriesD->removePoints(0, 1);
    if (seriesW  && seriesW->count()  > MAX_POINTS) seriesW->removePoints(0, 1);
    if (seriesY2 && seriesY2->count() > MAX_POINTS) seriesY2->removePoints(0, 1);
    if (seriesE  && seriesE->count()  > MAX_POINTS) seriesE->removePoints(0, 1);
    if (seriesU  && seriesU->count()  > MAX_POINTS) seriesU->removePoints(0, 1);

    if (chartY) chartY->axisX()->setRange(left, czas);
    if (chartW) chartW->axisX()->setRange(left, czas);
    if (chartE) chartE->axisX()->setRange(left, czas);
    if (chartU) chartU->axisX()->setRange(left, czas);
}

void MainWindow::on_ARXpushButton_clicked()
{
    if (!m_arxDialog) {
        m_arxDialog = new ArxDialog(this);
        m_arxDialog->setModal(true);   // jeśli ma blokować główne okno
    }
    m_arxDialog->show();
    m_arxDialog->raise();
    m_arxDialog->activateWindow();
}
