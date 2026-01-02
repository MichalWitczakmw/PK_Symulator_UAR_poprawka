#include "SymulatorUAR.h"
#include "Generator.h"
#include <QDebug>

SymulatorUAR::SymulatorUAR(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this)),m_symulacja(
          Model_ARX({-0.4}, {0.6}, 1, 0.0),
          Regulator_PID(1.0, 0.0, 0.0),
          Generator()
          )
{
    // Połączenie timera z slotem symulacji
    connect(m_timer, &QTimer::timeout, this, &SymulatorUAR::wykonajKrokSymulacji);

    // Domyślna konfiguracja wzorcowa do testów
    ustawWspolczynnikiARX({-0.4}, {0.6}, 1, 0.0);
    ustawNastawyPID(1.0, 0.0, 0.0);
    ustawGeneratorProstokąt(1.0, 50.0, 0.5, 0.0);  // TRZ = 50s przy TT=200ms

    m_czasStartu = std::chrono::steady_clock::now();
}

SymulatorUAR::~SymulatorUAR()
{
    if (m_timer) {
        m_timer->stop();
    }
}

// === ZARZĄDZANIE CYKLEM ŻYCIA ===
void SymulatorUAR::uruchom(int interwałMs)
{
    if (m_czyDziała) {
        qDebug() << "Symulacja już działa!";
        return;
    }

    m_interwałMs = qMax(10, qMin(1000, interwałMs));  // 10-1000ms
    m_timer->start(m_interwałMs);
    m_czyDziała = true;

    m_współczynnikTRZdoT = (1000.0 / m_interwałMs);  // TRZ[sek]*1000 / TT[ms]

    emit symulacjaUruchomiona();
    qDebug() << "Symulacja uruchomiona, interwał:" << m_interwałMs << "ms";
}

void SymulatorUAR::zatrzymaj()
{
    if (!m_czyDziała) {
        qDebug() << "Symulacja nie działa!";
        return;
    }

    m_timer->stop();
    m_czyDziała = false;

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
                                         int opóźnienieTransportowe, double odchylenieSzumu)
{
    if (A.size() < 1 || B.size() < 1) {
        qWarning() << "ARX: minimum 1 współczynnik A i B!";
        return false;
    }
    if (opóźnienieTransportowe < 1) {
        qWarning() << "ARX: opóźnienie transportowe minimum 1!";
        return false;
    }

    m_symulacja.getModelRef().setA(A);
    m_symulacja.getModelRef().setB(B);
    m_symulacja.getModelRef().setopoznienieTransport(opóźnienieTransportowe);
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
        qWarning() << "PID: Kp nie może być ujemne!";
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
bool SymulatorUAR::ustawGeneratorSinus(double amplituda, double okresTRZ, double składowaStała)
{
    if (okresTRZ <= 0) {
        qWarning() << "Generator: okres musi być > 0!";
        return false;
    }

    double okresDyskretny = okresTRZ * m_współczynnikTRZdoT;
    m_symulacja.getGeneratorRef().setTyp(TypSygnalu::Sinus);
    m_symulacja.getGeneratorRef().setAmplituda(amplituda);
    m_symulacja.getGeneratorRef().setOkres(okresDyskretny);
    m_symulacja.getGeneratorRef().setSkladowaStala(składowaStała);

    qDebug() << "Generator sinus: A=" << amplituda << ", TRZ=" << okresTRZ << "s";
    return true;
}

bool SymulatorUAR::ustawGeneratorProstokąt(double amplituda, double okresTRZ,
                                           double wypełnienie, double składowaStała)
{
    if (okresTRZ <= 0 || wypełnienie < 0 || wypełnienie > 1) {
        qWarning() << "Generator: nieprawidłowe parametry!";
        return false;
    }

    double okresDyskretny = okresTRZ * m_współczynnikTRZdoT;
    m_symulacja.getGeneratorRef().setTyp(TypSygnalu::Sinus);
    m_symulacja.getGeneratorRef().setAmplituda(amplituda);
    m_symulacja.getGeneratorRef().setOkres(okresDyskretny);
    m_symulacja.getGeneratorRef().setWypelnienie(wypełnienie);
    m_symulacja.getGeneratorRef().setSkladowaStala(składowaStała);

    qDebug() << "Generator prostokąt: A=" << amplituda << ", TRZ=" << okresTRZ << "s";
    return true;
}

void SymulatorUAR::ustawInterwałSymulacji(int interwałMs)
{
    if (m_czyDziała) {
        qDebug() << "Zmiana interwału podczas symulacji!";
        m_interwałMs = qMax(10, qMin(1000, interwałMs));
        m_timer->setInterval(m_interwałMs);
        m_współczynnikTRZdoT = (1000.0 / m_interwałMs);
    }
}

// === DOSTĘP DO DANYCH ===
double SymulatorUAR::getWartoscZadana() const { return m_symulacja.getWartoscZadana(); }
double SymulatorUAR::getWartoscRegulowana() const { return m_symulacja.getWartoscRegulowana(); }
double SymulatorUAR::getUchyb() const { return m_symulacja.getUchyb(); }
double SymulatorUAR::getSterowanie() const { return m_symulacja.getSterowanie(); }

// Składowe PID (obliczone z regulatora)
double SymulatorUAR::getSkladowaP() const { return obliczSkladowaP(); }
double SymulatorUAR::getSkladowaI() const { return obliczSkladowaI(); }
double SymulatorUAR::getSkladowaD() const { return obliczSkladowaD(); }

double SymulatorUAR::getCzasSymulacji() const
{
    auto teraz = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(teraz - m_czasStartu).count();
}

// Historia dla wykresów
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
    if (!m_czyDziała) return;

    // Jeden krok symulacji
    m_symulacja.wykonajKrok();

    // Oblicz składowe PID
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

    // Okno obserwacji - usuń najstarsze punkty
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
