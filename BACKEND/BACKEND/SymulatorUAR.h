#ifndef SYMULATORUAR_H
#define SYMULATORUAR_H

#include "../BACKEND/SymulacjaUAR.h"
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

    // === ZARZĄDZANIE CYKLEM ŻYCIA ===
    void uruchom(int interwałMs = 200);
    void zatrzymaj();
    void resetuj();

    // === KONFIGURACJA MODELU ARX ===
    bool ustawWspolczynnikiARX(const std::vector<double>& A, const std::vector<double>& B,
                               int opóźnienieTransportowe = 1, double odchylenieSzumu = 0.0);
    bool ustawOgraniczeniaModelu(double minU, double maxU, double minY, double maxY);

    // === KONFIGURACJA REGULATORA PID ===
    bool ustawNastawyPID(double kp, double ti, double td);
    bool ustawOgraniczeniaRegulatora(double minU, double maxU);
    void ustawTrybCalkowania(Regulator_PID::LiczCalk tryb);
    void resetujPamiecRegulatora();

    // === KONFIGURACJA GENERATORA ===
    bool ustawGeneratorSinus(double amplituda, double okresTRZ, double składowaStała);
    bool ustawGeneratorProstokąt(double amplituda, double okresTRZ, double wypełnienie, double składowaStała);
    bool ustawWzmocnienieGeneratora(double wzmocnienie);

    // === KONFIGURACJA SYMULACJI ===
    void ustawInterwalSymulacji(int interwałMs);
    void ustawOknoObserwacji(double sekundy);

    // === DOSTĘP DO DANYCH DLA GUI ===
    double getWartoscZadana() const;
    double getWartoscRegulowana() const;
    double getUchyb() const;
    double getSterowanie() const;
    double getSkladowaP() const;
    double getSkladowaI() const;
    double getSkladowaD() const;
    double getCzasSymulacji() const;

    // Historia dla wykresów (ostatnie N próbek)
    const QVector<double>& getHistoriaWartoscZadana() const;
    const QVector<double>& getHistoriaWartoscRegulowana() const;
    const QVector<double>& getHistoriaUchyb() const;
    const QVector<double>& getHistoriaSterowanie() const;
    const QVector<double>& getHistoriaSkladowaP() const;
    const QVector<double>& getHistoriaSkladowaI() const;
    const QVector<double>& getHistoriaSkladowaD() const;
    const QVector<double>& getHistoriaCzas() const;

    // Stan symulacji
    bool isRunning() const { return m_czyDziała; }

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
                       double minVal,
                       double maxVal,
                       bool uzywajOgraniczen);

    struct KonfiguracjaARX {
        QString tekstA;
        QString tekstB;
        int opoznienie;
        double szum;
        double minVal;
        double maxVal;
        bool uzywajOgraniczen;
    };

    KonfiguracjaARX getKonfiguracjaARX() const;


signals:
    void symulacjaUruchomiona();
    void symulacjaZatrzymana();
    void symulacjaZresetowana();
    void stanSymulacjiZmieniony();  // GUI może odświeżyć wykresy
     void dataUpdated(double czas, double y);  // Emit po każdym kroku

private slots:
    void wykonajKrokSymulacji();

private:
    // === KOMPONENTY WARSTWY DANYCH ===
    SymulacjaUAR m_symulacja;

    // === ZARZĄDZANIE CZASEM ===
    QTimer* m_timer;
    int m_interwałMs = 200;              // interwał w ms
    bool m_czyDziała = false;

    // === KONWERSJA CZASU RZECZYWISTEGO ↔ DYSKRETNEGO ===
    double m_współczynnikTRZdoT = 5.0;   // TRZ[sek]*1000 / TT[ms]

    // === HISTORIA DLA WYKRESÓW ===
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
