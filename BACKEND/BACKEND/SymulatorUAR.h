#ifndef SYMULATORUAR_H
#define SYMULATORUAR_H

#include "../BACKEND/SymulacjaUAR.h"
#include "../BACKEND/ZapisOdczytUAR.h"

#include <QObject>
#include <QTimer>
#include <QVector>
#include <vector>
#include <chrono>


class SymulatorUAR : public QObject
{
    Q_OBJECT

public:
    explicit SymulatorUAR(QObject* parent = nullptr);
    ~SymulatorUAR();

    // === ZARZaDZANIE CYKLEM żYCIA ===
    void uruchom(int interwalMs = 200);
    void zatrzymaj();
    void resetuj();

    // === KONFIGURACJA MODELU ARX ===
    bool ustawWspolczynnikiARX(const std::vector<double>& A, const std::vector<double>& B,
                               int opoznienieTransportowe = 1, double odchylenieSzumu = 0.0);
    bool ustawOgraniczeniaModelu(double minU, double maxU, double minY, double maxY);

    // === KONFIGURACJA REGULATORA PID ===
    bool ustawNastawyPID(double kp, double ti, double td);
    bool ustawOgraniczeniaRegulatora(double minU, double maxU);
    void ustawTrybCalkowania(Regulator_PID::LiczCalk tryb);
    void resetujPamiecRegulatora();

    // === KONFIGURACJA GENERATORA ===
    bool ustawGeneratorSinus(double amplituda, double okresTRZ, double skladowaStala);
    bool ustawGeneratorProstokat(double amplituda, double okresTRZ, double wypelnienie, double skladowaStala);
    bool ustawWzmocnienieGeneratora(double wzmocnienie);

    // === KONFIGURACJA SYMULACJI ===
    void ustawInterwalSymulacji(int interwalMs);
    void ustawOknoObserwacji(double sekundy);

    // === DOSTeP DO DANYCH DLA GUI ===
    double getWartoscZadana() const;
    double getWartoscRegulowana() const;
    double getUchyb() const;
    double getSterowanie() const;
    double getSkladowaP() const;
    double getSkladowaI() const;
    double getSkladowaD() const;
    double getCzasSymulacji() const;

    // Historia dla wykresow (ostatnie N probek)
    const QVector<double>& getHistoriaWartoscZadana() const;
    const QVector<double>& getHistoriaWartoscRegulowana() const;
    const QVector<double>& getHistoriaUchyb() const;
    const QVector<double>& getHistoriaSterowanie() const;
    const QVector<double>& getHistoriaSkladowaP() const;
    const QVector<double>& getHistoriaSkladowaI() const;
    const QVector<double>& getHistoriaSkladowaD() const;
    const QVector<double>& getHistoriaCzas() const;

    // Stan symulacji
    bool isRunning() const { return m_czyDziala; }

    const SymulacjaUAR& getSymulacja() const { return m_symulacja; }

    enum class BladARX {
        BrakBledu,
        ZlyFormatA,
        ZlyFormatB,
        ZaMaloA,
        ZaMaloB
    };
    BladARX konfigurujARX(const QString &aText,
                       const QString &bText,
                       int opoznienie,
                       double szum,
                       double minU,
                       double maxU,
                       double minY,
                       double maxY,
                       bool uzywajOgraniczen);

    struct KonfiguracjaARX {
        QString tekstA;
        QString tekstB;
        int opoznienie;
        double szum;
        double minU;
        double maxU;
        double minY;
        double maxY;
        bool uzywajOgraniczen;
    };


    KonfiguracjaARX getKonfiguracjaARX() const;

    void zapiszKonfiguracje();
    void odczytajKonfiguracje();

    int  getInterwalMs() const { return m_interwalMs; }
    double getCzasTrwaniaS() const { return m_czasTrwaniaS; }
    void ustawCzasTrwania(double sekundy) { m_czasTrwaniaS = sekundy; }

    void ustawGenerator(double amplituda,
                        double okresTRZ,
                        double wypelnienieProc,
                        double skladowaStala);
    double getAmplituda() const;
    double getOkresTRZ() const;
    double getWypelnienieProc() const;
    double getSkladowaStala() const;

signals:
    void symulacjaUruchomiona();
    void symulacjaZatrzymana();
    void symulacjaZresetowana();
    void stanSymulacjiZmieniony();  // GUI może odswieżyc wykresy
    void dataUpdated(double czas, double y);  // Emit po każdym kroku

    void konfiguracjaWczytana();

private slots:
    void wykonajKrokSymulacji();

private:
    // === KOMPONENTY WARSTWY DANYCH ===
    SymulacjaUAR m_symulacja;
    ZapisOdczytUAR m_zapisOdczyt;

    // === ZARZaDZANIE CZASEM ===
    QTimer* m_timer;
    int m_interwalMs = 200;              // interwal w ms
    bool m_czyDziala = false;
    double m_czasTrwaniaS = 50.0;

    double m_okresTRZ = 10.0;
    double m_wypelnienie = 0.5;
    double m_skladowaStala = 0.0;

    // === KONWERSJA CZASU RZECZYWISTEGO ↔ DYSKRETNEGO ===
    double m_wspolczynnikTRZdoT = 5.0;   // TRZ[sek]*1000 / TT[ms]

    // === HISTORIA DLA WYKRESoW ===
    QVector<double> m_historiaWartoscZadana;
    QVector<double> m_historiaWartoscRegulowana;
    QVector<double> m_historiaUchyb;
    QVector<double> m_historiaSterowanie;
    QVector<double> m_historiaSkladowaP;
    QVector<double> m_historiaSkladowaI;
    QVector<double> m_historiaSkladowaD;
    QVector<double> m_historiaCzas;

    int m_maxHistoriaPunktow = 50000;    // ~10s przy 200ms
    std::chrono::steady_clock::time_point m_czasStartu;

    // === POMOCNICZE ===
    void wyczyscHistorie();
    void dodajPunktDoHistorii(double w, double y, double e, double u,
                              double sklP, double sklI, double sklD, double czas);
    double obliczSkladowaP() const;
    double obliczSkladowaI() const;
    double obliczSkladowaD() const;
};

#endif // SYMULATORUAR_H
