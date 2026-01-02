#include "ZapisOdczytUAR.h"
#include "SymulacjaUAR.h"
//#include <nlohmann/json.hpp>

//using json = nlohmann::json;

bool ZapisOdczytUAR::zapiszDoPliku(const string& sciezka, const SymulacjaUAR& symulacja)  const
{
    /*json j;

	//SymulacjaUAR
	j["symulacja"]["uchyb"] = symulacja.getUchyb();
	j["symulacja"]["sterowanie"] = symulacja.getSterowanie();
	j["symulacja"]["wartoscZadana"] = symulacja.getWartoscZadana();
	j["symulacja"]["wartoscRegulowana"] = symulacja.getWartoscRegulowana();
	//j["symulacja"]["poprzedniaWartoscRegulowana"] = symulacja.getPoprzedniaWartoscRegulowana();

	// Model_ARX
	j["model"]["wspolczynnikA"] = symulacja.model().getA();
	j["model"]["wspolczynnikB"] = symulacja.model().getB();
	j["model"]["opoznienieTransportowe"] = symulacja.model().getOpoznienieTransport();
	j["model"]["odchylenieZaklocen"] = symulacja.model().getOdchylenieZaklocen();
	j["model"]["minU"] = symulacja.model().getMinU();
	j["model"]["maxU"] = symulacja.model().getMaxU();
	j["model"]["minY"] = symulacja.model().getMinY();
	j["model"]["maxY"] = symulacja.model().getMaxY();
	j["model"]["ogrSterowania"] = symulacja.model().getJestOgrSterowaniaAktywne();
	j["model"]["ogrRegulowania"] = symulacja.model().getJestgrRegulowaniaAktywne();

	// Regulator_PID
	j["regulator"]["kp"] = symulacja.regulator().getKp();
	j["regulator"]["ti"] = symulacja.regulator().getTi();
	j["regulator"]["td"] = symulacja.regulator().getTd();
	j["regulator"]["ogrMin"] = symulacja.regulator().getOgrMin();
	j["regulator"]["ogrMax"] = symulacja.regulator().getOgrMax();
	j["regulator"]["trybCalk"] = static_cast<int>(symulacja.regulator().getLiczCalk());

	// Generator
	j["generator"]["typ"] = static_cast<int>(symulacja.generator().getTyp());
	j["generator"]["amplituda"] = symulacja.generator().getAmplituda();
	j["generator"]["okres"] = symulacja.generator().getOkres();
	j["generator"]["wypelnienie"] = symulacja.generator().getWypelnienie();
	j["generator"]["skladowaStala"] = symulacja.generator().getSkladowaStala();

	std::ofstream plik(sciezka);
	if (!plik.is_open())
		return false;
	plik << j.dump(4);
    plik.close();*/
    return true;
}

bool ZapisOdczytUAR::odczytajZPliku(const string& sciezka, SymulacjaUAR& symulacja) const
{
   /* ifstream plik(sciezka);
    if (!plik.is_open())
        return false;
    json j;
    plik >> j;

    // SymulacjaUAR
    symulacja.setUchyb(j["symulacja"]["uchyb"].get<double>());
    symulacja.setSterowanie(j["symulacja"]["sterowanie"].get<double>());
    symulacja.setWartoscZadana(j["symulacja"]["wartoscZadana"].get<double>());
    symulacja.setWartoscRegulowana(j["symulacja"]["wartoscRegulowana"].get<double>());
    //symulacja.setPoprzedniaWartoscRegulowana(j["symulacja"]["poprzedniaWartoscRegulowana"].get<double>());


    // Model ARX
    symulacja.model().setA(j["model"]["wspolczynnikA"].get<std::vector<double>>());
    symulacja.model().setB(j["model"]["wspolczynnikB"].get<std::vector<double>>());
    symulacja.model().setopoznienieTransport(j["model"]["opoznienieTransportowe"].get<int>());
    symulacja.model().setOdchylenieZaklocen(j["model"]["odchylenieZaklocen"].get<double>());
    symulacja.model().setOgrSterowania(
        j["model"]["minU"].get<double>(),
        j["model"]["maxU"].get<double>(),
        j["model"]["ogrSterowania"].get<bool>());
    symulacja.model().setOgrRegulowania(
        j["model"]["minY"].get<double>(),
        j["model"]["maxY"].get<double>(),
        j["model"]["ogrRegulowania"].get<bool>());

    // Regulator PID
    symulacja.regulator().setKp(j["regulator"]["kp"].get<double>());
    symulacja.regulator().setTi(j["regulator"]["ti"].get<double>());
    symulacja.regulator().setTd(j["regulator"]["td"].get<double>());
    symulacja.regulator().setOgraniczenia(
        j["regulator"]["ogrMin"].get<double>(),
        j["regulator"]["ogrMax"].get<double>());
    symulacja.regulator().setLiczCalk(
        static_cast<Regulator_PID::LiczCalk>(j["regulator"]["trybCalk"].get<int>()));

    // Generator
    symulacja.generator().setTyp(
        static_cast<TypSygna³u>(j["generator"]["typ"].get<int>()));
    symulacja.generator().setAmplituda(j["generator"]["amplituda"].get<double>());
    symulacja.generator().setOkres(j["generator"]["okres"].get<double>());
    symulacja.generator().setWypelnienie(j["generator"]["wypelnienie"].get<double>());
    symulacja.generator().setSkladowaStala(j["generator"]["skladowaStala"].get<double>());
*/
    return true;
}
