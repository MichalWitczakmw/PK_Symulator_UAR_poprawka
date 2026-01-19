#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QKeyEvent>

// ===================================================================
// Konstruktor
// ===================================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_symulator()
    , m_arxDialog()
{
    ui->setupUi(this);

    // filtrowanie Entera na spinboxach / comboboxach
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
                m_symulator.ustawTrybCalkowania(
                    zaznaczony
                        ? Regulator_PID::LiczCalk::Zew
                        : Regulator_PID::LiczCalk::Wew);
            });

    connect(&m_symulator, &SymulatorUAR::konfiguracjaWczytana,
            this, &MainWindow::odswiezKontrolkiPoWczytaniu);

    this->resize(1200, 650);

    ui->symulacjagroupBox->setMinimumHeight(140);
    ui->PIDgroupBox->setMinimumHeight(140);
    ui->regulatorgroupBox->setMinimumHeight(160);
    ui->zapOdczgroupBox->setMinimumHeight(80);

    initPlots();
    applyTimeWindowToPlots();


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

// ===================================================================
// Inicjalizacja wykresów QCustomPlot
// ===================================================================

void MainWindow::initPlots()
{
    auto setupPlot = [](QCustomPlot *p, const QString &yLabel)
    {
        p->clearGraphs();
        p->xAxis->setLabel("t [s]");
        p->yAxis->setLabel(yLabel);
        p->yAxis->setRange(-3, 3);
        p->axisRect()->setupFullAxesBox();
        p->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
        p->legend->setVisible(false);
    };

    // ===== PID: P/I/D =====
    setupPlot(ui->WykresPID, "Skladowe PID");
    ui->WykresPID->addGraph(); // P
    ui->WykresPID->graph(0)->setName("Skladowa P");
    ui->WykresPID->graph(0)->setPen(QPen(QColor(255, 0, 0), 2));
    ui->WykresPID->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->WykresPID->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);

    ui->WykresPID->addGraph(); // I
    ui->WykresPID->graph(1)->setName("Skladowa I");
    ui->WykresPID->graph(1)->setPen(QPen(QColor(255, 255, 0), 2));
    ui->WykresPID->graph(1)->setLineStyle(QCPGraph::lsLine);
    ui->WykresPID->graph(1)->setScatterStyle(QCPScatterStyle::ssNone);

    ui->WykresPID->addGraph(); // D
    ui->WykresPID->graph(2)->setName("Skladowa D");
    ui->WykresPID->graph(2)->setPen(QPen(QColor(0, 0, 255), 2));
    ui->WykresPID->graph(2)->setLineStyle(QCPGraph::lsLine);
    ui->WykresPID->graph(2)->setScatterStyle(QCPScatterStyle::ssNone);

    // wiersz nad wykresem z trzema kolorowymi napisami
    ui->WykresPID->plotLayout()->insertRow(0);
    {
        QCPLayoutGrid *row = new QCPLayoutGrid;
        ui->WykresPID->plotLayout()->addElement(0, 0, row);

        QFont f("Sans Serif", 9);

        auto mk = [&](const QString &txt, const QColor &col)->QCPTextElement* {
            QCPTextElement *e = new QCPTextElement(ui->WykresPID, txt, f);
            e->setTextColor(col);
            e->setTextFlags(Qt::AlignLeft | Qt::AlignVCenter);
            return e;
        };

        row->addElement(0, 0, mk("Skladowa P", QColor(255, 0, 0)));
        row->addElement(0, 1, mk("   Skladowa I", QColor(255, 255, 0)));
        row->addElement(0, 2, mk("   Skladowa D", QColor(0, 0, 255)));
    }

    // ===== Zadana + regulowana (w + y) =====
    setupPlot(ui->WykresZadanaRegulowana, "Wartosc zadana / regulowana");
    ui->WykresZadanaRegulowana->addGraph(); // w
    ui->WykresZadanaRegulowana->graph(0)->setName("Wartosc zadana w");
    ui->WykresZadanaRegulowana->graph(0)->setPen(QPen(QColor(255, 165, 0), 2));
    ui->WykresZadanaRegulowana->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->WykresZadanaRegulowana->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);

    ui->WykresZadanaRegulowana->addGraph(); // y
    ui->WykresZadanaRegulowana->graph(1)->setName("Wartosc regulowana y");
    ui->WykresZadanaRegulowana->graph(1)->setPen(QPen(QColor(0, 255, 0), 2));
    ui->WykresZadanaRegulowana->graph(1)->setLineStyle(QCPGraph::lsLine);
    ui->WykresZadanaRegulowana->graph(1)->setScatterStyle(QCPScatterStyle::ssNone);

    ui->WykresZadanaRegulowana->plotLayout()->insertRow(0);
    {
        QCPLayoutGrid *row = new QCPLayoutGrid;
        ui->WykresZadanaRegulowana->plotLayout()->addElement(0, 0, row);

        QFont f("Sans Serif", 9);

        QCPTextElement *wTxt = new QCPTextElement(
            ui->WykresZadanaRegulowana, "Wartosc zadana w", f);
        wTxt->setTextColor(QColor(255, 165, 0));
        wTxt->setTextFlags(Qt::AlignLeft | Qt::AlignVCenter);

        QCPTextElement *yTxt = new QCPTextElement(
            ui->WykresZadanaRegulowana, "   Wartosc regulowana y", f);
        yTxt->setTextColor(QColor(0, 255, 0));
        yTxt->setTextFlags(Qt::AlignLeft | Qt::AlignVCenter);

        row->addElement(0, 0, wTxt);
        row->addElement(0, 1, yTxt);
    }

    // ===== Uchyb e =====
    setupPlot(ui->WykresUchyb, "Uchyb e");
    ui->WykresUchyb->addGraph();
    ui->WykresUchyb->graph(0)->setName("Uchyb e");
    ui->WykresUchyb->graph(0)->setPen(QPen(QColor(0, 0, 255), 2));
    ui->WykresUchyb->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->WykresUchyb->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->WykresUchyb->yAxis->setLabelColor(QColor(0, 0, 255));

    ui->WykresUchyb->plotLayout()->insertRow(0);
    {
        QFont f("Sans Serif", 9);
        QCPTextElement *eTxt = new QCPTextElement(
            ui->WykresUchyb, "Uchyb e", f);
        eTxt->setTextColor(QColor(0, 0, 255));
        eTxt->setTextFlags(Qt::AlignLeft | Qt::AlignVCenter);
        ui->WykresUchyb->plotLayout()->addElement(0, 0, eTxt);
    }

    // ===== Sterowanie u =====
    setupPlot(ui->WykresSterowanie, "Sterowanie u");
    ui->WykresSterowanie->addGraph();
    ui->WykresSterowanie->graph(0)->setName("Sterowanie u");
    ui->WykresSterowanie->graph(0)->setPen(QPen(QColor(255, 0, 0), 2));
    ui->WykresSterowanie->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->WykresSterowanie->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->WykresSterowanie->yAxis->setLabelColor(QColor(255, 0, 0));

    ui->WykresSterowanie->plotLayout()->insertRow(0);
    {
        QFont f("Sans Serif", 9);
        QCPTextElement *uTxt = new QCPTextElement(
            ui->WykresSterowanie, "Sterowanie u", f);
        uTxt->setTextColor(QColor(255, 0, 0));
        uTxt->setTextFlags(Qt::AlignLeft | Qt::AlignVCenter);
        ui->WykresSterowanie->plotLayout()->addElement(0, 0, uTxt);
    }

    ui->WykresPID->replot();
    ui->WykresZadanaRegulowana->replot();
    ui->WykresUchyb->replot();
    ui->WykresSterowanie->replot();
}



// Stała „skala czasu trwania” na osi X (0..window)
void MainWindow::applyTimeWindowToPlots()
{
    double window = ui->czasTrwaniadoubleSpinBox->value();
    if (window <= 0) window = 1.0;

    ui->WykresPID->xAxis->setRange(0, window);
    ui->WykresZadanaRegulowana->xAxis->setRange(0, window);
    ui->WykresUchyb->xAxis->setRange(0, window);
    ui->WykresSterowanie->xAxis->setRange(0, window);

    ui->WykresPID->replot();
    ui->WykresZadanaRegulowana->replot();
    ui->WykresUchyb->replot();
    ui->WykresSterowanie->replot();

    QString label = QStringLiteral("t [s]  (okno = %1 s)")
                        .arg(window, 0, 'f', 0); // bez miejsc po przecinku

    ui->WykresPID->xAxis->setLabel(label);
    ui->WykresZadanaRegulowana->xAxis->setLabel(label);
    ui->WykresUchyb->xAxis->setLabel(label);
    ui->WykresSterowanie->xAxis->setLabel(label);
}

// ===================================================================
// Event filter – Enter na polach edycyjnych
// ===================================================================

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto *kev = static_cast<QKeyEvent *>(event);
        if (kev->key() == Qt::Key_Return || kev->key() == Qt::Key_Enter) {

            if (obj == ui->interwalSpinBox) {
                int interwal = ui->interwalSpinBox->value();
                m_symulator.ustawInterwalSymulacji(interwal);
            }
            else if (obj == ui->czasTrwaniadoubleSpinBox) {
                double czas = ui->czasTrwaniadoubleSpinBox->value();
                m_symulator.ustawOknoObserwacji(czas);
                applyTimeWindowToPlots();
            }
            else if (obj == ui->ogrDolneSpinBox || obj == ui->ogrGornedoubleSpinBox) {
                double minU = ui->ogrDolneSpinBox->value();
                double maxU = ui->ogrGornedoubleSpinBox->value();
                m_symulator.ustawOgraniczeniaRegulatora(minU, maxU);
            }
            else if (obj == ui->TiSpinBox ||
                     obj == ui->TdSpinBox ||
                     obj == ui->KpSpinBox) {

                double Ti = ui->TiSpinBox->value();
                double Td = ui->TdSpinBox->value();
                double Kp = ui->KpSpinBox->value();
                m_symulator.ustawNastawyPID(Kp, Ti, Td);
            }
            else if (obj == ui->wzmacniaczSpinBox ||
                     obj == ui->OkresdoubleSpinBox ||
                     obj == ui->WypelnieniespinBox ||
                     obj == ui->StalaSkladowadoubleSpinBox_3 ||
                     obj == ui->typComboBox)
            {
                double A   = ui->wzmacniaczSpinBox->value();
                double TRZ = ui->OkresdoubleSpinBox->value();
                double p   = ui->WypelnieniespinBox->value();
                double S   = ui->StalaSkladowadoubleSpinBox_3->value();

                m_symulator.ustawGenerator(A, TRZ, p, S);

                int idx = ui->typComboBox->currentIndex();
                if (idx == 0) {
                    m_symulator.ustawGeneratorSinus(A, TRZ, S);
                } else {
                    m_symulator.ustawGeneratorProstokat(A, TRZ, p / 100.0, S);
                }
            }
            return false;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

// ===================================================================
// START / STOP / RESET
// ===================================================================

void MainWindow::on_StartPB_clicked()
{
    int interwal = ui->interwalSpinBox->value();

    if (m_paused) { // wracamy ze STOP
        double now = m_symulator.getCzasSymulacji();
        m_timeOffset += (now - m_pauseStart);
        m_paused = false;
    }

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

    m_paused = true;
    m_pauseStart = m_symulator.getCzasSymulacji();
}

void MainWindow::on_ResetPB_clicked()
{
    m_symulator.resetuj();

    ui->StartPB->setEnabled(!m_symulator.isRunning());
    ui->StopPB->setEnabled(m_symulator.isRunning());
    ui->ResetPB->setEnabled(true);

    // reset czasu
    m_timeOffset   = 0.0;
    m_paused       = false;
    m_pauseStart   = 0.0;
    m_firstTime    = 0.0;
    m_haveFirstTime = false;

    auto clearPlot = [](QCustomPlot *p){
        if (!p) return;
        const int n = p->graphCount();
        for (int i = 0; i < n; ++i)
            p->graph(i)->data()->clear();
        p->replot();
    };

    clearPlot(ui->WykresPID);
    clearPlot(ui->WykresZadanaRegulowana);
    clearPlot(ui->WykresUchyb);
    clearPlot(ui->WykresSterowanie);

    applyTimeWindowToPlots(); // 0..czasTrwania
}

// ===================================================================
// Aktualizacja 4 wykresów QCustomPlot
// ===================================================================

void MainWindow::updateChart(double czas, double)
{
    if (!m_symulator.isRunning())
        return;

    // czas symulacji po uwzględnieniu pauz
    double baseTime = czas - m_timeOffset;

    // zapamiętaj pierwszy czas i „przesuń” go do zera
    if (!m_haveFirstTime) {
        m_firstTime = baseTime;
        m_haveFirstTime = true;
    }
    double t = baseTime - m_firstTime;

    double w = m_symulator.getWartoscZadana();
    double y = m_symulator.getWartoscRegulowana();
    double e = m_symulator.getUchyb();
    double u = m_symulator.getSterowanie();
    double p = m_symulator.getSkladowaP();
    double i = m_symulator.getSkladowaI();
    double d = m_symulator.getSkladowaD();

    const int MAX_POINTS = 800;
    double window = ui->czasTrwaniadoubleSpinBox->value();
    if (window <= 0) window = 1.0;

    double left  = (t < window) ? 0.0 : (t - window);
    double right = left + window;

    auto trimData = [MAX_POINTS](QCPGraph *g)
    {
        if (!g) return;
        QCPDataContainer<QCPGraphData> *container = g->data().data();
        while (container->size() > MAX_POINTS)
        {
            double firstKey = container->constBegin()->key;
            container->remove(firstKey);
        }
    };

    // PID
    ui->WykresPID->graph(0)->addData(t, p);
    ui->WykresPID->graph(1)->addData(t, i);
    ui->WykresPID->graph(2)->addData(t, d);
    trimData(ui->WykresPID->graph(0));
    trimData(ui->WykresPID->graph(1));
    trimData(ui->WykresPID->graph(2));
    ui->WykresPID->xAxis->setRange(left, right);
    ui->WykresPID->replot();

    // w + y
    ui->WykresZadanaRegulowana->graph(0)->addData(t, w);
    ui->WykresZadanaRegulowana->graph(1)->addData(t, y);
    trimData(ui->WykresZadanaRegulowana->graph(0));
    trimData(ui->WykresZadanaRegulowana->graph(1));
    ui->WykresZadanaRegulowana->xAxis->setRange(left, right);
    ui->WykresZadanaRegulowana->replot();

    // e
    ui->WykresUchyb->graph(0)->addData(t, e);
    trimData(ui->WykresUchyb->graph(0));
    ui->WykresUchyb->xAxis->setRange(left, right);
    ui->WykresUchyb->replot();

    // u
    ui->WykresSterowanie->graph(0)->addData(t, u);
    trimData(ui->WykresSterowanie->graph(0));
    ui->WykresSterowanie->xAxis->setRange(left, right);
    ui->WykresSterowanie->replot();
}

// ===================================================================
// ARX dialog, zapis/odczyt, odświeżenie kontrolek
// ===================================================================

void MainWindow::on_ARXpushButton_clicked()
{
    auto cfg = m_symulator.getKonfiguracjaARX();

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

    const auto& reg = sim.getRegulator();
    ui->KpSpinBox->setValue(reg.getKp());
    ui->TiSpinBox->setValue(reg.getTi());
    ui->TdSpinBox->setValue(reg.getTd());
    ui->ogrGornedoubleSpinBox->setValue(reg.getOgrMax());
    ui->ogrDolneSpinBox->setValue(reg.getOgrMin());
    ui->CzyCalkacheckBox->setChecked(
        reg.getLiczCalk() == Regulator_PID::LiczCalk::Wew);

    const auto& gen = sim.getGenerator();
    ui->wzmacniaczSpinBox->setValue(gen.getAmplituda());
    if (gen.getTyp() == TypSygnalu::Sinus) {
        ui->typComboBox->setCurrentText("Sinusoidalny");
    } else {
        ui->typComboBox->setCurrentText("Prostokątny");
    }

    SymulatorUAR::KonfiguracjaARX cfg = m_symulator.getKonfiguracjaARX();
    m_arxDialog.setTekstA(cfg.tekstA);
    m_arxDialog.setTekstB(cfg.tekstB);
    m_arxDialog.setOpoznienie(cfg.opoznienie);
    m_arxDialog.setSzum(cfg.szum);
    m_arxDialog.setMinMax(cfg.minVal, cfg.maxVal);
    m_arxDialog.setOgraniczeniaAktywne(cfg.uzywajOgraniczen);

    applyTimeWindowToPlots();

}
