#include "SymulatorUAR.h"
#include "Generator.h"
#include <QDebug>
#include <QStringList>
#include <QFileDialog>

SymulatorUAR::SymulatorUAR(QObject* parent)
    : QObject(parent)
    , m_symulacja(
          Model_ARX({-0.4, 0.0, 0.0}, {0.6,0.0,0.0}, 1, 0.0),
          Regulator_PID(0.5, 1.4
                        , 0.0),
          Generator()
          ),m_timer(new QTimer(this))
{
    // Polaczenie timera z slotem symulacji
    connect(m_timer, &QTimer::timeout, this, &SymulatorUAR::wykonajKrokSymulacji);

    // Domyslna konfiguracja wzorcowa do testow
    ustawWspolczynnikiARX({-0.4, 0.0, 0.0}, {0.6,0.0,0.0}, 1, 0.0);
    ustawNastawyPID(0.5, 1.4, 0.0);
    //ustawGeneratorProstokat(1.0, 50.0, 0.5, 0.0);  // TRZ = 50s przy TT=200ms
    ustawGeneratorSinus(2.0,10,0);

    m_czasStartu = std::chrono::steady_clock::now();

    m_historiaWartoscZadana.reserve(m_maxHistoriaPunktow);
    m_historiaWartoscRegulowana.reserve(m_maxHistoriaPunktow);
    m_historiaUchyb.reserve(m_maxHistoriaPunktow);
    m_historiaSterowanie.reserve(m_maxHistoriaPunktow);
    m_historiaSkladowaP.reserve(m_maxHistoriaPunktow);
    m_historiaSkladowaI.reserve(m_maxHistoriaPunktow);
    m_historiaSkladowaD.reserve(m_maxHistoriaPunktow);
    m_historiaCzas.reserve(m_maxHistoriaPunktow);
}

SymulatorUAR::~SymulatorUAR()
{
    if (m_timer) {
        m_timer->stop();
    }
}

// === ZARZaDZANIE CYKLEM zYCIA ===
void SymulatorUAR::uruchom(int interwalMs)
{
    if (m_czyDziala) {
        qDebug() << "Symulacja juz dziala!";
        return;
    }

    m_interwalMs = qMax(10, qMin(1000, interwalMs));  // 10-1000ms
    m_timer->start(m_interwalMs);
    m_czyDziala = true;

    m_wspolczynnikTRZdoT = (1000.0 / m_interwalMs);  // TRZ[sek]*1000 / TT[ms]

    emit symulacjaUruchomiona();
    qDebug() << "Symulacja uruchomiona, interwal:" << m_interwalMs << "ms";
}

void SymulatorUAR::zatrzymaj()
{
    if (!m_czyDziala) {
        qDebug() << "Symulacja nie dziala!";
        return;
    }

    m_timer->stop();
    m_czyDziala = false;

    emit symulacjaZatrzymana();
    qDebug() << "Symulacja zatrzymana";
}

void SymulatorUAR::resetuj()
{
    m_symulacja.reset();
    wyczyscHistorie();
    m_czasStartu = std::chrono::steady_clock::now();

    emit symulacjaZresetowana();
    qDebug() << "Symulacja zresetowana";
}

// === KONFIGURACJA MODELU ARX ===
bool SymulatorUAR::ustawWspolczynnikiARX(const std::vector<double>& A,
                                         const std::vector<double>& B,
                                         int opoznienieTransportowe, double odchylenieSzumu)
{
    if (A.size() < 1 || B.size() < 1) {
        qWarning() << "ARX: minimum 1 wspolczynnik A i B!";
        return false;
    }
    if (opoznienieTransportowe < 1) {
        qWarning() << "ARX: opoznienie transportowe minimum 1!";
        return false;
    }

    m_symulacja.getModelRef().setA(A);
    m_symulacja.getModelRef().setB(B);
    m_symulacja.getModelRef().setopoznienieTransport(opoznienieTransportowe);
    m_symulacja.getModelRef().setOdchylenieZaklocen(odchylenieSzumu);

    qDebug() << "ARX skonfigurowany: A=" << A.size() << ", B=" << B.size();
    return true;
}

bool SymulatorUAR::ustawOgraniczeniaModelu(double minU, double maxU, double minY, double maxY)
{
    m_symulacja.getModelRef().setOgrSterowania(minU, maxU, true);
    m_symulacja.getModelRef().setOgrRegulowania(minY, maxY, true);
    return true;
}

// === KONFIGURACJA REGULATORA PID ===
bool SymulatorUAR::ustawNastawyPID(double kp, double ti, double td)
{
    if (kp < 0) {
        qWarning() << "PID: Kp nie moze byc ujemne!";
        return false;
    }

    m_symulacja.getRegulatorRef().setKp(kp);
    m_symulacja.getRegulatorRef().setTi(ti);
    m_symulacja.getRegulatorRef().setTd(td);

    qDebug() << "PID skonfigurowany: Kp=" << kp << ", Ti=" << ti << ", Td=" << td;
    return true;
}

bool SymulatorUAR::ustawOgraniczeniaRegulatora(double minU, double maxU)
{
    m_symulacja.getRegulatorRef().setOgraniczenia(minU, maxU);
    return true;
}

void SymulatorUAR::ustawTrybCalkowania(Regulator_PID::LiczCalk tryb)
{
    m_symulacja.getRegulatorRef().setLiczCalk(tryb);
}

void SymulatorUAR::resetujPamiecRegulatora()
{
    m_symulacja.getRegulatorRef().resetPamieci();
}

// === KONFIGURACJA GENERATORA ===
bool SymulatorUAR::ustawGeneratorSinus(double amplituda, double okresTRZ, double skladowaStala)
{
    if (okresTRZ <= 0) {
        qWarning() << "Generator: okres musi byc > 0!";
        return false;
    }

    double okresDyskretny = okresTRZ * m_wspolczynnikTRZdoT;
    m_symulacja.getGeneratorRef().setTyp(TypSygnalu::Sinus);
    m_symulacja.getGeneratorRef().setAmplituda(amplituda);
    m_symulacja.getGeneratorRef().setOkres(okresDyskretny);
    m_symulacja.getGeneratorRef().setSkladowaStala(skladowaStala);

    qDebug() << "Generator sinus: A=" << amplituda << ", TRZ=" << okresTRZ << "s";
    return true;
}

bool SymulatorUAR::ustawGeneratorProstokat(double amplituda, double okresTRZ,
                                           double wypelnienie, double skladowaStala)
{
    if (okresTRZ <= 0 || wypelnienie < 0 || wypelnienie > 1) {
        qWarning() << "Generator: nieprawidlowe parametry!";
        return false;
    }

    double okresDyskretny = okresTRZ * m_wspolczynnikTRZdoT;
    m_symulacja.getGeneratorRef().setTyp(TypSygnalu::Prostokat);
    m_symulacja.getGeneratorRef().setAmplituda(amplituda);
    m_symulacja.getGeneratorRef().setOkres(okresDyskretny);
    m_symulacja.getGeneratorRef().setWypelnienie(wypelnienie);
    m_symulacja.getGeneratorRef().setSkladowaStala(skladowaStala);

    qDebug() << "Generator prostokat: A=" << amplituda << ", TRZ=" << okresTRZ << "s";
    return true;
}

bool SymulatorUAR::ustawWzmocnienieGeneratora(double wzmocnienie)
{
    // mozesz dodac prosta walidacje, np. wzmocnienie >= 0
    m_symulacja.getGeneratorRef().setAmplituda(wzmocnienie);
    qDebug() << "Generator: nowe wzmocnienie (amplituda)=" << wzmocnienie;
    return true;
}


void SymulatorUAR::ustawInterwalSymulacji(int interwalMs)
{
    if (m_czyDziala) {
        qDebug() << "Zmiana interwalu podczas symulacji!";
        m_interwalMs = qMax(10, qMin(1000, interwalMs));
        m_timer->setInterval(m_interwalMs);
        m_wspolczynnikTRZdoT = (1000.0 / m_interwalMs);
    }
}
void SymulatorUAR::ustawOknoObserwacji(double sekundy)
{
    // na razie mozesz np. tylko przeliczyc na maks. ilosc punktow historii
    // przy interwale m_interwalMs (ms):
    // czas okna [s] -> liczba probek = sekundy * (1000 / m_interwalMs)

    double iloscProbek = sekundy * (1000.0 / m_interwalMs);
    m_maxHistoriaPunktow = static_cast<int>(iloscProbek);
    m_czasTrwaniaS=sekundy;
    //wyczyscHistorie();

    if (m_maxHistoriaPunktow < 1)
        m_maxHistoriaPunktow = 1;
}


// === DOSTeP DO DANYCH ===
double SymulatorUAR::getWartoscZadana() const { return m_symulacja.getWartoscZadana(); }
double SymulatorUAR::getWartoscRegulowana() const { return m_symulacja.getWartoscRegulowana(); }
double SymulatorUAR::getUchyb() const { return m_symulacja.getUchyb(); }
double SymulatorUAR::getSterowanie() const { return m_symulacja.getSterowanie(); }

// Skladowe PID (obliczone z regulatora)
double SymulatorUAR::getSkladowaP() const { return obliczSkladowaP(); }
double SymulatorUAR::getSkladowaI() const { return obliczSkladowaI(); }
double SymulatorUAR::getSkladowaD() const { return obliczSkladowaD(); }

double SymulatorUAR::getCzasSymulacji() const
{
    auto teraz = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(teraz - m_czasStartu).count();
}

// Historia dla wykresow
const QVector<double>& SymulatorUAR::getHistoriaWartoscZadana() const { return m_historiaWartoscZadana; }
const QVector<double>& SymulatorUAR::getHistoriaWartoscRegulowana() const { return m_historiaWartoscRegulowana; }
const QVector<double>& SymulatorUAR::getHistoriaUchyb() const { return m_historiaUchyb; }
const QVector<double>& SymulatorUAR::getHistoriaSterowanie() const { return m_historiaSterowanie; }
const QVector<double>& SymulatorUAR::getHistoriaSkladowaP() const { return m_historiaSkladowaP; }
const QVector<double>& SymulatorUAR::getHistoriaSkladowaI() const { return m_historiaSkladowaI; }
const QVector<double>& SymulatorUAR::getHistoriaSkladowaD() const { return m_historiaSkladowaD; }
const QVector<double>& SymulatorUAR::getHistoriaCzas() const { return m_historiaCzas; }

// === SLOT TIMERA ===
void SymulatorUAR::wykonajKrokSymulacji()
{
    if (!m_czyDziala) return;

    // Jeden krok symulacji
    m_symulacja.wykonajKrok();

    // Oblicz skladowe PID
    double sklP = obliczSkladowaP();
    double sklI = obliczSkladowaI();
    double sklD = obliczSkladowaD();

    // Dodaj do historii
    double czas = getCzasSymulacji();
    dodajPunktDoHistorii(
        m_symulacja.getWartoscZadana(),
        m_symulacja.getWartoscRegulowana(),
        m_symulacja.getUchyb(),
        m_symulacja.getSterowanie(),
        sklP, sklI, sklD,
        czas
        );

    emit stanSymulacjiZmieniony();
    emit dataUpdated(czas, m_symulacja.getWartoscRegulowana());
}

// === POMOCNICZE ===
void SymulatorUAR::wyczyscHistorie()
{
    m_historiaWartoscZadana.clear();
    m_historiaWartoscRegulowana.clear();
    m_historiaUchyb.clear();
    m_historiaSterowanie.clear();
    m_historiaSkladowaP.clear();
    m_historiaSkladowaI.clear();
    m_historiaSkladowaD.clear();
    m_historiaCzas.clear();
}

void SymulatorUAR::dodajPunktDoHistorii(double w, double y, double e, double u,
                                        double sklP, double sklI, double sklD, double czas)
{
    m_historiaWartoscZadana.append(w);
    m_historiaWartoscRegulowana.append(y);
    m_historiaUchyb.append(e);
    m_historiaSterowanie.append(u);
    m_historiaSkladowaP.append(sklP);
    m_historiaSkladowaI.append(sklI);
    m_historiaSkladowaD.append(sklD);
    m_historiaCzas.append(czas);

    // Okno obserwacji - usun najstarsze punkty
    if (m_historiaWartoscZadana.size() > m_maxHistoriaPunktow) {
        m_historiaWartoscZadana.removeFirst();
        m_historiaWartoscRegulowana.removeFirst();
        m_historiaUchyb.removeFirst();
        m_historiaSterowanie.removeFirst();
        m_historiaSkladowaP.removeFirst();
        m_historiaSkladowaI.removeFirst();
        m_historiaSkladowaD.removeFirst();
        m_historiaCzas.removeFirst();
    }
}

double SymulatorUAR::obliczSkladowaP() const
{
    return m_symulacja.getRegulator().getKp() * m_symulacja.getUchyb();
}


double SymulatorUAR::obliczSkladowaI() const
{
    const auto& reg = m_symulacja.getRegulator();
    return (reg.getTi() > 0) ? reg.getSumaEdivTi() : 0.0;
}

double SymulatorUAR::obliczSkladowaD() const
{
    const auto& reg = m_symulacja.getRegulator();
    return reg.getTd() * reg.getUchybPoprzedni();
}

static std::vector<double> rozdzielWspolczynnik(const QString &text, bool &ok)
{
    ok = true;
    std::vector<double> result;

    // rozdziel po przecinku
    const QStringList parts = text.split(',', Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        ok = false;
        return result;
    }

    for (const QString &raw : parts) {
        QString s = raw.trimmed();      // usun spacje
        if (s.isEmpty())
            continue;

        bool okNum = false;
        double val = s.toDouble(&okNum);  // kropka lub przecinek lokalnie

        if (!okNum) {
            ok = false;
            //result.clear();
            return result;
        }
        result.push_back(val);
    }

    if (result.empty())
        ok = false;

    return result;
}

SymulatorUAR::BladARX SymulatorUAR::konfigurujARX(const QString &tekstA,
                                                  const QString &tekstB,
                                                  int opoznienie,
                                                  double szum,
                                                  double minU,
                                                  double maxU,
                                                  double minY,
                                                  double maxY,
                                                  bool uzywajOgraniczen)
{
    bool okA = false, okB = false;
    std::vector<double> A = rozdzielWspolczynnik(tekstA, okA);
    std::vector<double> B = rozdzielWspolczynnik(tekstB, okB);

    if (!okA)
        return BladARX::ZlyFormatA;
    if (!okB)
        return BladARX::ZlyFormatB;

    if (A.size() < 3)
        return BladARX::ZaMaloA;
    if (B.size() < 3)
        return BladARX::ZaMaloB;

    if (opoznienie < 1)
        opoznienie = 1;

    auto &model = m_symulacja.getModelRef();
    model.setA(A);
    model.setB(B);
    model.setopoznienieTransport(opoznienie);
    model.setOdchylenieZaklocen(szum);
    model.setOgrSterowania(minU, maxU, uzywajOgraniczen);
    model.setOgrRegulowania(minY, maxY, uzywajOgraniczen);

    return BladARX::BrakBledu;
}

static QString wektorNaTekst(const std::vector<double> &v)
{
    QStringList parts;
    for (double x : v)
        parts << QString::number(x, 'g', 6);  // np. "-0.4, 0.6"
    return parts.join(", ");
}

SymulatorUAR::KonfiguracjaARX SymulatorUAR::getKonfiguracjaARX() const
{
    KonfiguracjaARX cfg;
    const auto &model = m_symulacja.getModel();

    cfg.tekstA = wektorNaTekst(model.getA());
    cfg.tekstB = wektorNaTekst(model.getB());
    cfg.opoznienie = model.getOpoznienieTransport();
    cfg.szum = model.getOdchylenieZaklocen();
    cfg.minU = model.getMinU();
    cfg.maxU = model.getMaxU();
    cfg.minY = model.getMinY();
    cfg.maxY = model.getMaxY();
    cfg.uzywajOgraniczen = model.getJestOgrSterowaniaAktywne()
                           && model.getJestgrRegulowaniaAktywne();

    return cfg;
}




void SymulatorUAR::zapiszKonfiguracje()
{
    QString sciezka = QFileDialog::getSaveFileName(
        nullptr,
        "Zapisz konfigurację UAR",
        "",
        "Pliki JSON (*.json)");

    if (sciezka.isEmpty())
        return;

    m_zapisOdczyt.zapiszDoPliku(sciezka, *this);
}

void SymulatorUAR::odczytajKonfiguracje()
{
    QString sciezka = QFileDialog::getOpenFileName(
        nullptr,
        "Wczytaj konfigurację UAR",
        "",
        "Pliki JSON (*.json)");

    if (sciezka.isEmpty())
        return;

    if (!m_zapisOdczyt.odczytajZPliku(sciezka, *this))
        return;

    emit konfiguracjaWczytana();

}

void SymulatorUAR::ustawGenerator(double amplituda,
                                  double okresTRZ,
                                  double wypelnienieProc,
                                  double skladowaStala)
{
    m_okresTRZ      = okresTRZ;
    m_wypelnienie   = wypelnienieProc / 100.0; // GUI: % -> 0..1
    m_skladowaStala = skladowaStala;

    // liczba próbek na okres (T w Generatorze)
    double T = (okresTRZ * 1000.0) / m_interwalMs;

    auto &gen = m_symulacja.getGeneratorRef();
    gen.setAmplituda(amplituda);
    gen.setOkres(T);                    // okres dyskretny
    gen.setWypelnienie(m_wypelnienie);  // 0..1
    gen.setSkladowaStala(m_skladowaStala);
}


double SymulatorUAR::getAmplituda() const      { return m_symulacja.getGenerator().getAmplituda(); }
double SymulatorUAR::getOkresTRZ() const       { return m_okresTRZ; }
double SymulatorUAR::getWypelnienieProc() const { return m_wypelnienie * 100.0; }
double SymulatorUAR::getSkladowaStala() const  { return m_skladowaStala; }



