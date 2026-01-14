#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QMessageBox>
#include <QKeyEvent>

#include "../../BACKEND/BACKEND/SymulatorUAR.h"

//using namespace QtCharts;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_symulator()
    , m_arxDialog()
    , chartViewY(nullptr), chartViewW(nullptr), chartViewE(nullptr), chartViewU(nullptr)
    , seriesP(nullptr), seriesI(nullptr), seriesD(nullptr), seriesY2(nullptr)
    , seriesW(nullptr), seriesE(nullptr), seriesU(nullptr)
    , chartY(nullptr), chartW(nullptr), chartE(nullptr), chartU(nullptr)

{
    ui->setupUi(this);

    ui->interwalSpinBox->installEventFilter(this);
    ui->czasTrwaniadoubleSpinBox->installEventFilter(this);

    ui->ogrDolneSpinBox->installEventFilter(this);
    ui->ogrGornedoubleSpinBox->installEventFilter(this);
    ui->wzmacniaczSpinBox->installEventFilter(this);
    ui->typComboBox->installEventFilter(this);
    ui->OkresdoubleSpinBox->installEventFilter(this);
    ui->WypelnieniespinBox->installEventFilter(this);
    ui->StalaSkladowadoubleSpinBox_3->installEventFilter(this);


    ui->TiSpinBox->installEventFilter(this);
    ui->TdSpinBox->installEventFilter(this);
    ui->KpSpinBox->installEventFilter(this);


    connect(ui->CzyCalkacheckBox, &QCheckBox::toggled,
            this, [this](bool zaznaczony){
                // zaznaczony -> np. Zew, odznaczony -> Wew
                m_symulator.ustawTrybCalkowania(
                    zaznaczony
                        ? Regulator_PID::LiczCalk::Zew
                        : Regulator_PID::LiczCalk::Wew);
            });
    connect(&m_symulator, &SymulatorUAR::konfiguracjaWczytana,
            this, &MainWindow::odswiezKontrolkiPoWczytaniu);


    // Mozesz ustawic tylko startowy rozmiar, ale BEZ blokowania skalowania
    this->resize(1200, 650);

    // (opcjonalnie) minimalne wysokosci groupBoxow z lewej strony
    ui->symulacjagroupBox->setMinimumHeight(140);
    ui->PIDgroupBox->setMinimumHeight(140);
    ui->regulatorgroupBox->setMinimumHeight(160);
    ui->zapOdczgroupBox->setMinimumHeight(80);

    // ====== KONTENER NA 4 WYKRESY (wewnatrz wykresygroupBox) ======
    QWidget *chartsContainer = new QWidget(this);
    QVBoxLayout *chartsLayout = new QVBoxLayout(chartsContainer);
    chartsLayout->setSpacing(0);
    //chartsLayout->setContentsMargins(0, 0, 0, 0);

    // ------------------------------------------------------------------
    // CH1: PID (P czerwony, I zolty, D niebieski)
    // ------------------------------------------------------------------
    chartY = new QChart();
    //chartY->setTitle("P (czerw), I (zol), D (nieb)");

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
    // CH2: w (zolty, dashed) + y (zielony)
    // ------------------------------------------------------------------
    chartW = new QChart();
    //chartW->setTitle("w (zad zol dashed), y (reg ziel)");

    seriesW = new QLineSeries();
    seriesW->setName("zadana");
    QPen penW(QColor(255, 165, 0), 2);
    //penW.setStyle(Qt::DashLine);
    seriesW->setPen(penW);
    chartW->addSeries(seriesW);

    seriesY2 = new QLineSeries();
    seriesY2->setName("regulowana");
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
    //chartE->setTitle("uchyb");

    seriesE = new QLineSeries();
    seriesE->setName("uchyb");
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
    //chartU->setTitle("sterowanie");

    seriesU = new QLineSeries();
    seriesU->setName("sterowanie");
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

    // ====== WlOzENIE WYKRESoW DO groupBox ======
    // (glowny layout okna jest zrobiony w Designerze)
    QVBoxLayout *wykresyLayout = new QVBoxLayout(ui->groupBox);
    wykresyLayout->setSpacing(2);
    wykresyLayout->setContentsMargins(10, 10, 10, 10);
    wykresyLayout->addWidget(chartsContainer);

    // ui->groupBox->setStyleSheet(
    //     "QGroupBox { border: 2px solid #4A90E2; border-radius: 8px; "
    //     "margin-top: 5px; } "
    //     "QGroupBox::title { subcontrol-origin: margin; left: 10px; "
    //     "padding: 0 5px; }");

    // ====== CONNECTY ======
    connect(ui->StartPB, &QPushButton::clicked,
            this, &MainWindow::on_StartPB_clicked);
    connect(ui->StopPB, &QPushButton::clicked,
            this, &MainWindow::on_StopPB_clicked);
    connect(ui->ResetPB, &QPushButton::clicked,
            this, &MainWindow::on_ResetPB_clicked);

    ui->StopPB->setEnabled(false);
    ui->ResetPB->setEnabled(false);

    connect(&m_symulator, &SymulatorUAR::dataUpdated,
            this, &MainWindow::updateChart);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto *kev = static_cast<QKeyEvent *>(event);
        if (kev->key() == Qt::Key_Return || kev->key() == Qt::Key_Enter) {

            // ===== Symulacja: interwał i czas trwania =====
            if (obj == ui->interwalSpinBox) {
                int interwal = ui->interwalSpinBox->value();
                m_symulator.ustawInterwalSymulacji(interwal);
            }
            else if (obj == ui->czasTrwaniadoubleSpinBox) {
                double czas = ui->czasTrwaniadoubleSpinBox->value();
                m_symulator.ustawOknoObserwacji(czas);
            }

            // ===== Regulator – ograniczenia =====
            else if (obj == ui->ogrDolneSpinBox || obj == ui->ogrGornedoubleSpinBox) {
                double minU = ui->ogrDolneSpinBox->value();
                double maxU = ui->ogrGornedoubleSpinBox->value();
                m_symulator.ustawOgraniczeniaRegulatora(minU, maxU);
            }

            // ===== PID: Ti, Td, Kp =====
            else if (obj == ui->TiSpinBox ||
                     obj == ui->TdSpinBox ||
                     obj == ui->KpSpinBox) {

                double Ti = ui->TiSpinBox->value();
                double Td = ui->TdSpinBox->value();
                double Kp = ui->KpSpinBox->value();

                m_symulator.ustawNastawyPID(Kp, Ti, Td);
            }

            // ===== Generator: amplituda, okres, wypełnienie, składowa stała, typ =====
            else if (obj == ui->wzmacniaczSpinBox ||
                     obj == ui->OkresdoubleSpinBox ||
                     obj == ui->WypelnieniespinBox ||
                     obj == ui->StalaSkladowadoubleSpinBox_3 ||
                     obj == ui->typComboBox)
            {
                double A   = ui->wzmacniaczSpinBox->value();          // amplituda
                double TRZ = ui->OkresdoubleSpinBox->value();         // okres [s]
                double p   = ui->WypelnieniespinBox->value();         // 0..100 %
                double S   = ui->StalaSkladowadoubleSpinBox_3->value();

                // ustaw parametry generatora w warstwie usług
                m_symulator.ustawGenerator(A, TRZ, p, S);

                int idx = ui->typComboBox->currentIndex();
                if (idx == 0) {
                    // Sinusoidalny
                    m_symulator.ustawGeneratorSinus(A, TRZ, S);
                } else {
                    // Prostokątny (wypełnienie 0..1)
                    m_symulator.ustawGeneratorProstokat(A, TRZ, p / 100.0, S);
                }
            }

            // nie blokujemy dalszej obsługi – spinbox/combobox normalnie kończy edycję
            return false;
        }
    }

    // dla innych zdarzeń/obiektów – domyślna obsługa
    return QMainWindow::eventFilter(obj, event);
}


// ===================================================================
// START / STOP / RESET
// ===================================================================

void MainWindow::on_StartPB_clicked()
{
    int interwal = ui->interwalSpinBox->value();
    m_symulator.uruchom(interwal);

    ui->StartPB->setEnabled(false);
    ui->StopPB->setEnabled(true);
    ui->ResetPB->setEnabled(true);
}


void MainWindow::on_StopPB_clicked()
{
    m_symulator.zatrzymaj();

    ui->StartPB->setEnabled(true);
    ui->StopPB->setEnabled(false);
    ui->ResetPB->setEnabled(true);
}

void MainWindow::on_ResetPB_clicked()
{
    m_symulator.resetuj();

    ui->StartPB->setEnabled(!m_symulator.isRunning());
    ui->StopPB->setEnabled(m_symulator.isRunning());
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
// Aktualizacja 4 wykresow: P/I/D, w+y, e, u
// ===================================================================

static void autoskalujOsYzOkna(QChart *chart,
                               const QList<QLineSeries*> &serie,
                               double left, double right)
{
    if (!chart)
        return;

    bool pierwszy = true;
    double minY = 0.0, maxY = 0.0;

    for (QLineSeries *s : serie) {
        if (!s)
            continue;

        const auto &pts = s->pointsVector();
        for (const QPointF &p : pts) {
            if (p.x() < left || p.x() > right)
                continue;   // poza widocznym oknem

            if (pierwszy) {
                minY = maxY = p.y();
                pierwszy = false;
            } else {
                if (p.y() < minY) minY = p.y();
                if (p.y() > maxY) maxY = p.y();
            }
        }
    }

    if (pierwszy)
        return; // brak punktow w oknie

    // margines ±0.5
    double low  = minY - 0.5;
    double high = maxY + 0.5;

    if (auto *axisY = qobject_cast<QValueAxis*>(chart->axisY()))
        axisY->setRange(low, high);
}




void MainWindow::updateChart(double czas, double /*dummy*/)
{
    double w = m_symulator.getWartoscZadana();
    double y = m_symulator.getWartoscRegulowana();
    double e = m_symulator.getUchyb();
    double u = m_symulator.getSterowanie();
    double p = m_symulator.getSkladowaP();
    double i = m_symulator.getSkladowaI();
    double d = m_symulator.getSkladowaD();

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

    autoskalujOsYzOkna(chartY, { seriesP, seriesI, seriesD }, left, czas);
    autoskalujOsYzOkna(chartW, { seriesW, seriesY2 },        left, czas);
    autoskalujOsYzOkna(chartE, { seriesE },                  left, czas);
    autoskalujOsYzOkna(chartU, { seriesU },                  left, czas);


}

void MainWindow::on_ARXpushButton_clicked()
{

    // pobierz aktualna konfiguracje ARX jako teksty
    auto cfg = m_symulator.getKonfiguracjaARX();

    // ustaw kontrolek okienka
    m_arxDialog.ustawZKonfiguracji(cfg.tekstA, cfg.tekstB,
                                    cfg.opoznienie,
                                    cfg.szum,
                                    cfg.minVal,
                                    cfg.maxVal,
                                    cfg.uzywajOgraniczen);

    if (m_arxDialog.exec() == QDialog::Accepted) {

        QString strA      = m_arxDialog.coeffA();
        QString strB      = m_arxDialog.coeffB();
        int    delay      = m_arxDialog.delay();
        double noise      = m_arxDialog.noise();
        double minVal     = m_arxDialog.minVal();
        double maxVal     = m_arxDialog.maxVal();
        bool   useLimits  = m_arxDialog.useLimits();

        auto blad = m_symulator.konfigurujARX(strA, strB,
                                               delay, noise,
                                               minVal, maxVal,
                                               useLimits);
        if (blad != SymulatorUAR::BladARX::BrakBledu) {
            QString msg;
            switch (blad) {
            case SymulatorUAR::BladARX::ZlyFormatA:
                msg = tr("Niepoprawny format wspolczynnikow A.\n"
                         "Uzyj formatu: -0.4, -0.4, 0.6");
                break;
            case SymulatorUAR::BladARX::ZlyFormatB:
                msg = tr("Niepoprawny format wspolczynnikow B.\n"
                         "Uzyj formatu: -0.4, -0.4, 0.6");
                break;
            case SymulatorUAR::BladARX::ZaMaloA:
                msg = tr("Wektor A musi miec co najmniej 3 wspolczynniki.");
                break;
            case SymulatorUAR::BladARX::ZaMaloB:
                msg = tr("Wektor B musi miec co najmniej 3 wspolczynniki.");
                break;
            default:
                break;
            }
            if (!msg.isEmpty())
                m_arxDialog.pokazBlad(msg);

            // opcjonalnie ponownie otworzyc dialog
            on_ARXpushButton_clicked();
        }
    }
}



void MainWindow::on_zapiszPushButton_clicked()
{
    m_symulator.zapiszKonfiguracje();
}

void MainWindow::on_wczytajPushButton_clicked()
{
    m_symulator.odczytajKonfiguracje();
}

void MainWindow::odswiezKontrolkiPoWczytaniu()
{
    const auto& sim = m_symulator.getSymulacja();

    ui->interwalSpinBox->setValue(m_symulator.getInterwalMs());
    ui->czasTrwaniadoubleSpinBox->setValue(m_symulator.getCzasTrwaniaS());


    // --- PID w głównym oknie ---
    const auto& reg = sim.getRegulator();
    ui->KpSpinBox->setValue(reg.getKp());
    ui->TiSpinBox->setValue(reg.getTi());
    ui->TdSpinBox->setValue(reg.getTd());
    ui->ogrGornedoubleSpinBox->setValue(reg.getOgrMax());
    ui->ogrDolneSpinBox->setValue(reg.getOgrMin());
    ui->CzyCalkacheckBox->setChecked(
        reg.getLiczCalk() == Regulator_PID::LiczCalk::Wew);

    // --- Generator w głównym oknie ---
    const auto& gen = sim.getGenerator();
    ui->wzmacniaczSpinBox->setValue(gen.getAmplituda());
    if (gen.getTyp() == TypSygnalu::Sinus) {
        ui->typComboBox->setCurrentText("Sinusoidalny");
    } else {
        ui->typComboBox->setCurrentText("Prostokątny");
    }
    // jeśli masz osobne pola na okres, wypełnienie, składową stałą – też je ustaw

    // --- ARX w ArxDialogu ---
    SymulatorUAR::KonfiguracjaARX cfg = m_symulator.getKonfiguracjaARX();

    m_arxDialog.setTekstA(cfg.tekstA);
    m_arxDialog.setTekstB(cfg.tekstB);
    m_arxDialog.setOpoznienie(cfg.opoznienie);
    m_arxDialog.setSzum(cfg.szum);
    m_arxDialog.setMinMax(cfg.minVal, cfg.maxVal);
    m_arxDialog.setOgraniczeniaAktywne(cfg.uzywajOgraniczen);

}


