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

    if (ui->verticalLayout_8) {
        // Kolejność w verticalLayout_8:
        // 0 - QWidget z wykresem zadana/regulowana
        // 1 - QWidget z wykresem PID
        // 2 - QWidget z wykresem sterowania
        // 3 - QWidget z wykresem uchybu
        //ui->verticalLayout_8->setStretch(0, 3);  // główny wykres – większy
        //ui->verticalLayout_8->setStretch(1, 2);  // PID
        //ui->verticalLayout_8->setStretch(2, 2);  // sterowanie
        //ui->verticalLayout_8->setStretch(3, 2);  // uchyb
    }

    this->setStyleSheet("QMainWindow { background-color: #d0d0d0; }"
                        "QWidget { background-color: #d0d0d0; }"
                        "QGroupBox { border: 2px solid #2f4f4f; border-radius: "
                        "5px; margin-top: 1ex; padding-top: 10px; } "
                        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 3px; "
                        "background-color: #d0d0d0; }");

    // filtrowanie Entera na spinboxach / comboboxach
    ui->interwalSpinBox->installEventFilter(this);
    ui->czasTrwaniadoubleSpinBox->installEventFilter(this);

    ui->ogrDolneSpinBox->installEventFilter(this);
    ui->ogrGornedoubleSpinBox->installEventFilter(this);
    ui->wzmacniaczSpinBox->installEventFilter(this);
    ui->OkresdoubleSpinBox->installEventFilter(this);
    ui->WypelnieniespinBox->installEventFilter(this);
    ui->StalaSkladowadoubleSpinBox_3->installEventFilter(this);

    ui->TiSpinBox->installEventFilter(this);
    ui->TdSpinBox->installEventFilter(this);
    ui->KpSpinBox->installEventFilter(this);

    connect(ui->typComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx){
                double A   = ui->wzmacniaczSpinBox->value();
                double TRZ = ui->OkresdoubleSpinBox->value();
                double p   = ui->WypelnieniespinBox->value();
                double S   = ui->StalaSkladowadoubleSpinBox_3->value();

                m_symulator.ustawGenerator(A, TRZ, p, S);

                if (idx == 0) {
                    m_symulator.ustawGeneratorSinus(A, TRZ, S);
                } else {
                    m_symulator.ustawGeneratorProstokat(A, TRZ, p / 100.0, S);
                }
            });

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

    inicjalizujWykresy();
    zastosujOknoCzasuDoWykresow();

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
            this, &MainWindow::aktualizujWykresy);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ===================================================================
// Inicjalizacja wykresów QCustomPlot
// ===================================================================

void MainWindow::inicjalizujWykresy()
{
    auto skonfigurujPojedynczyWykres = [](QCustomPlot *p, const QString &yLabel)
    {
        p->clearGraphs();
        p->clearItems();
        p->xAxis->setLabel("t [s]");
        p->yAxis->setLabel(yLabel);
        p->yAxis->setRange(-2, 2);
        p->axisRect()->setupFullAxesBox();

        // całkowite wyłączenie interakcji
        p->setInteractions(QCP::Interactions());

        p->legend->setVisible(false);

        QColor bg("#d0d0d0");
        p->setBackground(bg);
        p->axisRect()->setBackground(bg);
    };

    // ===== PID: P/I/D =====
    skonfigurujPojedynczyWykres(ui->WykresPID, "");
    ui->WykresPID->addGraph(); // P
    ui->WykresPID->graph(0)->setName("Skladowa P");
    ui->WykresPID->graph(0)->setPen(QPen(QColor(255, 0, 0), 2));
    ui->WykresPID->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->WykresPID->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);

    ui->WykresPID->addGraph(); // I
    ui->WykresPID->graph(1)->setName("Skladowa I");
    ui->WykresPID->graph(1)->setPen(QPen(QColor(0, 128, 0), 2));
    ui->WykresPID->graph(1)->setLineStyle(QCPGraph::lsLine);
    ui->WykresPID->graph(1)->setScatterStyle(QCPScatterStyle::ssNone);

    ui->WykresPID->addGraph(); // D
    ui->WykresPID->graph(2)->setName("Skladowa D");
    ui->WykresPID->graph(2)->setPen(QPen(QColor(0, 0, 255), 2));
    ui->WykresPID->graph(2)->setLineStyle(QCPGraph::lsLine);
    ui->WykresPID->graph(2)->setScatterStyle(QCPScatterStyle::ssNone);

    // ===== Zadana + regulowana (w + y) =====
    skonfigurujPojedynczyWykres(ui->WykresZadanaRegulowana, "");
    ui->WykresZadanaRegulowana->addGraph(); // w
    ui->WykresZadanaRegulowana->graph(0)->setName("Wartosc zadana w");
    ui->WykresZadanaRegulowana->graph(0)->setPen(QPen(QColor(255, 165, 0), 2));
    ui->WykresZadanaRegulowana->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->WykresZadanaRegulowana->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);

    ui->WykresZadanaRegulowana->addGraph(); // y
    ui->WykresZadanaRegulowana->graph(1)->setName("Wartosc regulowana y");
    ui->WykresZadanaRegulowana->graph(1)->setPen(QPen(QColor(0, 160, 0), 2));
    ui->WykresZadanaRegulowana->graph(1)->setLineStyle(QCPGraph::lsLine);
    ui->WykresZadanaRegulowana->graph(1)->setScatterStyle(QCPScatterStyle::ssNone);

    // ===== Uchyb e =====
    skonfigurujPojedynczyWykres(ui->WykresUchyb, "");
    ui->WykresUchyb->addGraph();
    ui->WykresUchyb->graph(0)->setName("Uchyb e");
    ui->WykresUchyb->graph(0)->setPen(QPen(QColor(0, 0, 255), 2));
    ui->WykresUchyb->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->WykresUchyb->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->WykresUchyb->yAxis->setLabelColor(QColor(0, 0, 255));

    // ===== Sterowanie u =====
    skonfigurujPojedynczyWykres(ui->WykresSterowanie, "");
    ui->WykresSterowanie->addGraph();
    ui->WykresSterowanie->graph(0)->setName("Sterowanie u");
    ui->WykresSterowanie->graph(0)->setPen(QPen(QColor(255, 0, 0), 2));
    ui->WykresSterowanie->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->WykresSterowanie->graph(0)->setScatterStyle(QCPScatterStyle::ssNone);
    ui->WykresSterowanie->yAxis->setLabelColor(QColor(255, 0, 0));

    ui->WykresPID->replot();
    ui->WykresZadanaRegulowana->replot();
    ui->WykresUchyb->replot();
    ui->WykresSterowanie->replot();


}

// Stałe okno czasu na osi X (0..czasTrwania)
void MainWindow::zastosujOknoCzasuDoWykresow()
{
    double okno = m_symulator.getCzasTrwaniaS();
    if (okno <= 0) okno = 1.0;

    ui->WykresPID->xAxis->setRange(0, okno);
    ui->WykresZadanaRegulowana->xAxis->setRange(0, okno);
    ui->WykresUchyb->xAxis->setRange(0, okno);
    ui->WykresSterowanie->xAxis->setRange(0, okno);

    ui->WykresPID->replot();
    ui->WykresZadanaRegulowana->replot();
    ui->WykresUchyb->replot();
    ui->WykresSterowanie->replot();

    QString etykieta = QStringLiteral("t [s]  (okno = %1 s)")
                           .arg(okno, 0, 'f', 0);

    ui->WykresPID->xAxis->setLabel(etykieta);
    ui->WykresZadanaRegulowana->xAxis->setLabel(etykieta);
    ui->WykresUchyb->xAxis->setLabel(etykieta);
    ui->WykresSterowanie->xAxis->setLabel(etykieta);
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
                zastosujOknoCzasuDoWykresow();
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
                     obj == ui->StalaSkladowadoubleSpinBox_3)
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

    if (m_czyWstrzymanie) {
        double teraz = m_symulator.getCzasSymulacji();
        m_przesuniecieCzasu += (teraz - m_czasStartPauzy);
        m_czyWstrzymanie = false;
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

    m_czyWstrzymanie = true;
    m_czasStartPauzy = m_symulator.getCzasSymulacji();
}

void MainWindow::on_ResetPB_clicked()
{
    m_symulator.resetuj();

    ui->StartPB->setEnabled(!m_symulator.isRunning());
    ui->StopPB->setEnabled(m_symulator.isRunning());
    ui->ResetPB->setEnabled(true);

    // reset czasu
    m_przesuniecieCzasu    = 0.0;
    m_czyWstrzymanie       = false;
    m_czasStartPauzy       = 0.0;
    m_czasPierwszy         = 0.0;
    m_mamyCzasPierwszy     = false;

    auto wyczyscWykres = [](QCustomPlot *p){
        if (!p) return;
        const int n = p->graphCount();
        for (int i = 0; i < n; ++i)
            p->graph(i)->data()->clear();
        p->replot();
    };

    wyczyscWykres(ui->WykresPID);
    wyczyscWykres(ui->WykresZadanaRegulowana);
    wyczyscWykres(ui->WykresUchyb);
    wyczyscWykres(ui->WykresSterowanie);

    zastosujOknoCzasuDoWykresow();
}

// ===================================================================
// Pomocnicze: wygładzanie zakresu Y
// ===================================================================

static void plynnieUstawZakres(QCPAxis *oś, double docDol, double docGora, double alpha = 0.2)
{
    QCPRange r = oś->range();
    double nowDol  = r.lower + alpha * (docDol  - r.lower);
    double nowGora = r.upper + alpha * (docGora - r.upper);
    oś->setRange(nowDol, nowGora);
}

// Zwraca true, jeśli znaleziono chociaż jeden punkt w oknie [left, right]
static bool policzZakresWidoczny(QCPGraph *g, double lewaKrawedz, double prawaKrawedz, double &minY, double &maxY)
{
    if (!g)
        return false;

    auto *dane = g->data().data();
    if (!dane || dane->isEmpty())
        return false;

    bool znalezionoPunkt = false;
    double lokalneMinY = 0.0;
    double lokalneMaxY = 0.0;

    for (auto i = dane->constBegin(); i != dane->constEnd(); ++i)
    {
        double x = i->key;
        if (x < lewaKrawedz || x > prawaKrawedz)
            continue;

        double y = i->value;

        if (!znalezionoPunkt) {
            lokalneMinY = y;
            lokalneMaxY = y;
            znalezionoPunkt = true;
        } else {
            if (y < lokalneMinY) lokalneMinY = y;
            if (y > lokalneMaxY) lokalneMaxY = y;
        }
    }
    if (!znalezionoPunkt)
        return false;

    minY = lokalneMinY;
    maxY = lokalneMaxY;
    return true;
}

// ===================================================================
// Aktualizacja 4 wykresów QCustomPlot
// ===================================================================

void MainWindow::aktualizujWykresy(double czas, double)
{
    if (!m_symulator.isRunning())
        return;

    // czas symulacji po uwzględnieniu pauz
    double czasBazowy = czas - m_przesuniecieCzasu;
    if (!m_mamyCzasPierwszy) {
        m_czasPierwszy = czasBazowy;
        m_mamyCzasPierwszy = true;
    }
    double t = czasBazowy - m_czasPierwszy;

    double w = m_symulator.getWartoscZadana();
    double y = m_symulator.getWartoscRegulowana();
    double e = m_symulator.getUchyb();
    double u = m_symulator.getSterowanie();
    double p = m_symulator.getSkladowaP();
    double i = m_symulator.getSkladowaI();
    double d = m_symulator.getSkladowaD();

    const int MAKS_PUNKTOW = 5000;
    double okno = m_symulator.getCzasTrwaniaS();
    if (okno <= 0) okno = 1.0;

    double left  = (t < okno) ? 0.0 : (t - okno);
    double right = left + okno;

    // przycinanie TYLKO po liczbie punktów (bez cięcia po czasie)
    auto przytnijDane = [MAKS_PUNKTOW](QCPGraph *g)
    {
        if (!g) return;
        auto *container = g->data().data();
        if (!container) return;

        while (container->size() > MAKS_PUNKTOW)
            container->remove(container->constBegin()->key);
    };

    // ===== PID =====
    ui->WykresPID->graph(0)->addData(t, p);
    ui->WykresPID->graph(1)->addData(t, i);
    ui->WykresPID->graph(2)->addData(t, d);
    przytnijDane(ui->WykresPID->graph(0));
    przytnijDane(ui->WykresPID->graph(1));
    przytnijDane(ui->WykresPID->graph(2));

    double minYpid = 0.0, maxYpid = 0.0;
    bool mamyZakresPID = false;
    double mn, mx;

    if (policzZakresWidoczny(ui->WykresPID->graph(0), left, right, mn, mx)) {
        minYpid = mn; maxYpid = mx; mamyZakresPID = true;
    }
    if (policzZakresWidoczny(ui->WykresPID->graph(1), left, right, mn, mx)) {
        if (!mamyZakresPID) { minYpid = mn; maxYpid = mx; mamyZakresPID = true; }
        else { if (mn < minYpid) minYpid = mn; if (mx > maxYpid) maxYpid = mx; }
    }
    if (policzZakresWidoczny(ui->WykresPID->graph(2), left, right, mn, mx)) {
        if (!mamyZakresPID) { minYpid = mn; maxYpid = mx; mamyZakresPID = true; }
        else { if (mn < minYpid) minYpid = mn; if (mx > maxYpid) maxYpid = mx; }
    }

    if (mamyZakresPID) {
        double dol = qMin(-2.0, minYpid);
        double gor = qMax( 2.0, maxYpid);
        plynnieUstawZakres(ui->WykresPID->yAxis, dol, gor);
    }

    ui->WykresPID->xAxis->setRange(left, right);
    ui->WykresPID->replot();

    // ===== w + y – niezależny zakres Y =====
    ui->WykresZadanaRegulowana->graph(0)->addData(t, w);
    ui->WykresZadanaRegulowana->graph(1)->addData(t, y);
    przytnijDane(ui->WykresZadanaRegulowana->graph(0));
    przytnijDane(ui->WykresZadanaRegulowana->graph(1));

    double minWY = 0.0, maxWY = 0.0;
    bool mamyZakresWY = false;

    if (policzZakresWidoczny(ui->WykresZadanaRegulowana->graph(0), left, right, mn, mx)) {
        minWY = mn; maxWY = mx; mamyZakresWY = true;
    }
    if (policzZakresWidoczny(ui->WykresZadanaRegulowana->graph(1), left, right, mn, mx)) {
        if (!mamyZakresWY) { minWY = mn; maxWY = mx; mamyZakresWY = true; }
        else { if (mn < minWY) minWY = mn; if (mx > maxWY) maxWY = mx; }
    }

    if (mamyZakresWY) {
        double dol = qMin(-2.0, minWY);
        double gor = qMax( 2.0, maxWY);
        plynnieUstawZakres(ui->WykresZadanaRegulowana->yAxis, dol, gor);
    }

    ui->WykresZadanaRegulowana->xAxis->setRange(left, right);
    ui->WykresZadanaRegulowana->replot();

    // ===== e – własny zakres =====
    ui->WykresUchyb->graph(0)->addData(t, e);
    przytnijDane(ui->WykresUchyb->graph(0));

    double minE = 0.0, maxE = 0.0;
    if (policzZakresWidoczny(ui->WykresUchyb->graph(0), left, right, minE, maxE)) {
        double dol = qMin(-2.0, minE);
        double gor = qMax( 2.0, maxE);
        plynnieUstawZakres(ui->WykresUchyb->yAxis, dol, gor);
    }

    ui->WykresUchyb->xAxis->setRange(left, right);
    ui->WykresUchyb->replot();

    // ===== u – własny zakres =====
    ui->WykresSterowanie->graph(0)->addData(t, u);
    przytnijDane(ui->WykresSterowanie->graph(0));

    double minU = 0.0, maxU = 0.0;
    if (policzZakresWidoczny(ui->WykresSterowanie->graph(0), left, right, minU, maxU)) {
        double dol = qMin(-2.0, minU);
        double gor = qMax( 2.0, maxU);
        plynnieUstawZakres(ui->WykresSterowanie->yAxis, dol, gor);
    }

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
                                   cfg.minU,
                                   cfg.maxU,
                                   cfg.minY,
                                   cfg.maxY,
                                   cfg.uzywajOgraniczen);

    if (m_arxDialog.exec() == QDialog::Accepted) {

        QString strA      = m_arxDialog.coeffA();
        QString strB      = m_arxDialog.coeffB();
        int    delay      = m_arxDialog.delay();
        double noise      = m_arxDialog.noise();
        double minU       = m_arxDialog.minU();
        double maxU       = m_arxDialog.maxU();
        double minY       = m_arxDialog.minY();
        double maxY       = m_arxDialog.maxY();
        bool   useLimits  = m_arxDialog.useLimits();

        auto blad = m_symulator.konfigurujARX(strA, strB,
                                              delay, noise,
                                              minU, maxU,
                                              minY, maxY,
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

    ui->wzmacniaczSpinBox->setValue(m_symulator.getAmplituda());
    ui->OkresdoubleSpinBox->setValue(m_symulator.getOkresTRZ());
    ui->WypelnieniespinBox->setValue(m_symulator.getWypelnienieProc());
    ui->StalaSkladowadoubleSpinBox_3->setValue(m_symulator.getSkladowaStala());

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
    m_arxDialog.setMinMax(cfg.minU, cfg.maxU, cfg.minY, cfg.maxY);
    m_arxDialog.setOgraniczeniaAktywne(cfg.uzywajOgraniczen);

    zastosujOknoCzasuDoWykresow();
}

void MainWindow::on_resetTiTdPIDpushButton_clicked()
{
    m_symulator.resetujPamiecRegulatora();
}
