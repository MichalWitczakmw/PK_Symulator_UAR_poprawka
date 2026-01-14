#include "ZapisOdczytUAR.h"
#include "SymulatorUAR.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

bool ZapisOdczytUAR::zapiszDoPliku(const QString& sciezka,
                                   const SymulatorUAR& sym) const
{
    // === ARX z SymulatorUAR ===
    SymulatorUAR::KonfiguracjaARX arxCfg = sym.getKonfiguracjaARX();

    // === PID & Generator bezposrednio z SymulacjaUAR ===
    const auto& sim = sym.getSymulacja();
    const auto& reg = sim.getRegulator();
    const auto& gen = sim.getGenerator();

    QJsonObject root;

    // ===== SYMULACJA =====
    QJsonObject jSim;
    jSim["interwalMs"]   = sym.getInterwalMs();   // dodasz getter
    jSim["czasTrwaniaS"] = sym.getCzasTrwaniaS(); // dodasz getter
    root["symulacja"]    = jSim;

    // ------- MODEL (ARX) -------
    QJsonObject jModel;
    jModel["A"]          = arxCfg.tekstA;
    jModel["B"]          = arxCfg.tekstB;
    jModel["opoznienie"] = arxCfg.opoznienie;
    jModel["szum"]       = arxCfg.szum;
    jModel["minVal"]     = arxCfg.minVal;
    jModel["maxVal"]     = arxCfg.maxVal;
    jModel["ogr"]        = arxCfg.uzywajOgraniczen;
    root["model"]        = jModel;

    // ------- REGULATOR PID -------
    QJsonObject jReg;
    jReg["kp"]      = reg.getKp();
    jReg["ti"]      = reg.getTi();
    jReg["td"]      = reg.getTd();
    jReg["ogrMin"]  = reg.getOgrMin();
    jReg["ogrMax"]  = reg.getOgrMax();
    jReg["trybCalk"]= static_cast<int>(reg.getLiczCalk());
    root["regulator"] = jReg;

    // ------- GENERATOR -------
    QJsonObject jGen;
    jGen["typ"]          = static_cast<int>(gen.getTyp());
    jGen["amplituda"]    = gen.getAmplituda();
    jGen["okres"]        = gen.getOkres();
    jGen["wypelnienie"]  = gen.getWypelnienie();
    jGen["skladowaStala"]= gen.getSkladowaStala();
    root["generator"]    = jGen;

    // ------- zapis do pliku -------
    QJsonDocument doc(root);

    QFile file(sciezka);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Nie mozna otworzyc pliku do zapisu:" << sciezka;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "Konfiguracja UAR zapisana do" << sciezka;
    return true;
}

bool ZapisOdczytUAR::odczytajZPliku(const QString& sciezka,
                                    SymulatorUAR& sym) const
{
    QFile file(sciezka);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Nie mozna otworzyc pliku do odczytu:" << sciezka;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "Blad parsowania JSON:" << err.errorString();
        return false;
    }

    QJsonObject root = doc.object();

    if (root.contains("symulacja") && root["symulacja"].isObject()) {
        QJsonObject jSim = root["symulacja"].toObject();
        int interwalMs   = jSim["interwalMs"].toInt(200);
        double czasS     = jSim["czasTrwaniaS"].toDouble(50.0);

        sym.ustawInterwalSymulacji(interwalMs);
        sym.ustawCzasTrwania(czasS);
    }

    // ===== ARX =====
    if (root.contains("model") && root["model"].isObject()) {
        QJsonObject jm = root["model"].toObject();

        QString tekstA    = jm["A"].toString();
        QString tekstB    = jm["B"].toString();
        int     opoznienie= jm["opoznienie"].toInt(1);
        double  szum      = jm["szum"].toDouble(0.0);
        double  minVal    = jm["minVal"].toDouble(-10.0);
        double  maxVal    = jm["maxVal"].toDouble(10.0);
        bool    ogr       = jm["ogr"].toBool(true);

        sym.konfigurujARX(tekstA, tekstB,
                          opoznienie, szum,
                          minVal, maxVal, ogr);
    }

    // ===== PID =====
    if (root.contains("regulator") && root["regulator"].isObject()) {
        QJsonObject jr = root["regulator"].toObject();

        double kp   = jr["kp"].toDouble(1.0);
        double ti   = jr["ti"].toDouble(0.0);
        double td   = jr["td"].toDouble(0.0);
        double omin = jr["ogrMin"].toDouble(-1e10);
        double omax = jr["ogrMax"].toDouble(1e10);
        int    tryb = jr["trybCalk"].toInt(0);

        sym.ustawNastawyPID(kp, ti, td);
        sym.ustawOgraniczeniaRegulatora(omin, omax);
        sym.ustawTrybCalkowania(
            static_cast<Regulator_PID::LiczCalk>(tryb));
    }

    // ===== GENERATOR =====
    if (root.contains("generator") && root["generator"].isObject()) {
        QJsonObject jg = root["generator"].toObject();

        int    typ      = jg["typ"].toInt(0);
        double A        = jg["amplituda"].toDouble(1.0);
        double okres    = jg["okres"].toDouble(10.0);
        double wypeln   = jg["wypelnienie"].toDouble(0.5);
        double S        = jg["skladowaStala"].toDouble(0.0);

        if (typ == static_cast<int>(TypSygnalu::Sinus)) {
            sym.ustawGeneratorSinus(A, okres, S);
        } else {
            sym.ustawGeneratorProstokat(A, okres, wypeln, S);
        }
    }

    qDebug() << "Konfiguracja UAR wczytana z" << sciezka;
    return true;
}
