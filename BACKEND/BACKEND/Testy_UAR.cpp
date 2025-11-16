// TU includy:

#include <iostream>
#include <iomanip>
#include <vector>
#include "Model_ARX.h"
#include "Regulator_PID.h"
#include "SymulacjaUAR.h"

#define DEBUG  // ustaw na MAIN aby skompilować program docelowy / ustaw na DEBUG aby skompilować program testujacy 

#ifdef DEBUG

//Funkcje pomocnicze dla testów:
using namespace std;

void raportBleduSekwencji(std::vector<double>& spodz, std::vector<double>& fakt)
{
	constexpr size_t PREC = 3;
	std::cerr << std::fixed << std::setprecision(PREC);
	std::cerr << "  Spodziewany:\t";
	for (auto& el : spodz)
		std::cerr << el << ", ";
	std::cerr << "\n  Faktyczny:\t";
	for (auto& el : fakt)
		std::cerr << el << ", ";
	std::cerr << std::endl << std::endl;
}

bool porownanieSekwencji(std::vector<double>& spodz, std::vector<double>& fakt)
{
	constexpr double TOL = 1e-3;	// tolerancja dla porównań zmiennoprzecinkowych
	bool result = fakt.size() == spodz.size();
	for (int i = 0; result && i < fakt.size(); i++)
		result = fabs(fakt[i] - spodz[i]) < TOL;
	return result;
}

void myAssert(std::vector<double>& spodz, std::vector<double>& fakt)
{
	if (porownanieSekwencji(spodz, fakt))
		std::cerr << "OK!\n";
	else
	{
		std::cerr << "FAIL!\n";
		raportBleduSekwencji(spodz, fakt);
	}
}

// testy dla samego ARX:

namespace TESTY_Model_ARX
{
	void wykonaj_testy();
	void test_brakPobudzenia();
	void test_skokJednostkowy_1();
	void test_skokJednostkowy_2();
	void test_skokJednostkowy_3();
}

void TESTY_Model_ARX::wykonaj_testy()
{
	test_brakPobudzenia();
	test_skokJednostkowy_1();
	test_skokJednostkowy_2();
	test_skokJednostkowy_3();
}

void TESTY_Model_ARX::test_brakPobudzenia()
{
	//Sygnatura testu:
	std::cerr << "Model_ARX (-0.4 | 0.6 | 1 | 0 ) -> test zerowego pobudzenia: ";
	try
	{
		// Przygotowanie danych:
		Model_ARX instancjaTestowa({ -0.4 }, { 0.6 }, 1, 0);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu (tu same 0)
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy (tu same 0)
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja modelu:

		for (int i = 0; i < LICZ_ITER; i++)
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);

		// Walidacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}

void TESTY_Model_ARX::test_skokJednostkowy_1()
{
	//Sygnatura testu:
	std::cerr << "Model_ARX (-0.4 | 0.6 | 1 | 0 ) -> test skoku jednostkowego nr 1: ";

	try
	{
		// Przygotowanie danych:
		Model_ARX instancjaTestowa({ -0.4 }, { 0.6 }, 1, 0);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu 
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy 
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja skoku jednostkowego w chwili 1. (!!i - daje 1 dla i != 0);
		for (int i = 0; i < LICZ_ITER; i++)
			sygWe[i] = !!i;
		spodzSygWy = { 0, 0, 0.6, 0.84, 0.936, 0.9744, 0.98976, 0.995904, 0.998362,
			           0.999345, 0.999738, 0.999895, 0.999958, 0.999983, 0.999993, 0.999997, 0.999999,
			           1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

		// Symulacja modelu:
		for (int i = 0; i < LICZ_ITER; i++)
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);

		// Walidacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}

void TESTY_Model_ARX::test_skokJednostkowy_2()
{
	//Sygnatura testu:
	std::cerr << "Model_ARX (-0.4 | 0.6 | 2 | 0 ) -> test skoku jednostkowego nr 2: ";

	try
	{
		// Przygotowanie danych:
		Model_ARX instancjaTestowa({ -0.4 }, { 0.6 }, 2, 0);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu, 
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja skoku jednostkowego w chwili 1. (!!i - daje 1 dla i != 0);
		for (int i = 0; i < LICZ_ITER; i++)
			sygWe[i] = !!i;
		spodzSygWy = { 0, 0, 0, 0.6, 0.84, 0.936, 0.9744, 0.98976, 0.995904, 0.998362, 0.999345, 0.999738, 0.999895, 0.999958, 0.999983, 0.999993, 0.999997, 0.999999, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

		// Symulacja modelu:
		for (int i = 0; i < LICZ_ITER; i++)
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);

		// Walidacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}

void TESTY_Model_ARX::test_skokJednostkowy_3()
{
	//Sygnatura testu:
	std::cerr << "Model_ARX (-0.4, 0.2 | 0.6, 0.3 | 2 | 0 ) -> test skoku jednostkowego nr 3: ";
	try
	{
		// Przygotowanie danych:
		Model_ARX instancjaTestowa({ -0.4,0.2 }, { 0.6, 0.3 }, 2, 0);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu, 
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja skoku jednostkowego w chwili 1. (!!i - daje 1 dla i != 0);
		for (int i = 0; i < LICZ_ITER; i++)
			sygWe[i] = !!i;
		spodzSygWy = { 0, 0, 0, 0.6, 1.14, 1.236, 1.1664, 1.11936, 1.11446, 1.12191, 1.12587, 1.12597, 1.12521, 1.12489, 1.12491, 1.12499, 1.12501, 1.12501, 1.125, 1.125, 1.125, 1.125, 1.125, 1.125, 1.125, 1.125, 1.125, 1.125, 1.125, 1.125 };

		// Symulacja modelu:
		for (int i = 0; i < LICZ_ITER; i++)
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);

		// Weryfikacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}


// testy dla samego Regulatora PID:

namespace TESTY_RegulatorPID
{
	void wykonaj_testy();
	void test_P_brakPobudzenia();
	void test_P_skokJednostkowy();
	void test_PI_skokJednostkowy_1();
	void test_PI_skokJednostkowy_2();
	void test_PID_skokJednostkowy();
	void test_PI_skokJednostkowy_3();
}

void TESTY_RegulatorPID::wykonaj_testy()
{
	test_P_brakPobudzenia();
	test_P_skokJednostkowy();
	test_PI_skokJednostkowy_1();
	test_PI_skokJednostkowy_2();
	test_PID_skokJednostkowy();
	test_PI_skokJednostkowy_3();
}

void TESTY_RegulatorPID::test_P_brakPobudzenia()
{
	//Sygnatura testu:
	std::cerr << "RegP (k = 0.5) -> test zerowego pobudzenia: ";
	try
	{
		// Przygotowanie danych:
		Regulator_PID instancjaTestowa(0.5);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu (tu same 0)
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy (tu same 0)
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja modelu:

		for (int i = 0; i < LICZ_ITER; i++)
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);

		// Walidacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}

void TESTY_RegulatorPID::test_P_skokJednostkowy()
{
	//Sygnatura testu:
	std::cerr << "RegP (k = 0.5) -> test skoku jednostkowego: ";

	try
	{
		// Przygotowanie danych:
		Regulator_PID instancjaTestowa(0.5);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu 
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy 
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja skoku jednostkowego w chwili 1. (!!i - daje 1 dla i != 0);
		for (int i = 0; i < LICZ_ITER; i++)
			sygWe[i] = !!i;
		spodzSygWy = { 0.0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5,
			           0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5 };

		// Symulacja modelu:
		for (int i = 0; i < LICZ_ITER; i++)
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);

		// Walidacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}

void TESTY_RegulatorPID::test_PI_skokJednostkowy_1()
{
	//Sygnatura testu:
	std::cerr << "RegPI (k = 0.5, TI = 1.0) -> test skoku jednostkowego nr 1: ";

	try
	{
		// Przygotowanie danych:
		Regulator_PID instancjaTestowa(0.5, 1.0);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu 
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy 
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja skoku jednostkowego w chwili 1. (!!i - daje 1 dla i != 0);
		for (int i = 0; i < LICZ_ITER; i++)
			sygWe[i] = !!i;
		spodzSygWy = { 0, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5,
			           13.5, 14.5, 15.5, 16.5, 17.5, 18.5, 19.5, 20.5, 21.5, 22.5, 23.5,
			           24.5, 25.5, 26.5, 27.5, 28.5, 29.5 };

		// Symulacja modelu:
		for (int i = 0; i < LICZ_ITER; i++)
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);

		// Walidacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}

void TESTY_RegulatorPID::test_PI_skokJednostkowy_2()
{
	//Sygnatura testu:
	std::cerr << "RegPI (k = 0.5, TI = 10.0) -> test skoku jednostkowego nr 2: ";

	try
	{
		// Przygotowanie danych:
		Regulator_PID instancjaTestowa(0.5, 10.0);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu 
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy 
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja skoku jednostkowego w chwili 1. (!!i - daje 1 dla i != 0);
		for (int i = 0; i < LICZ_ITER; i++)
			sygWe[i] = !!i;
		spodzSygWy = { 0, 0.6, 0.7, 0.8, 0.9, 1, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9,
			           2, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9, 3, 3.1, 3.2, 3.3, 3.4 };

		// Symulacja modelu:
		for (int i = 0; i < LICZ_ITER; i++)
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);

		// Walidacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}

void TESTY_RegulatorPID::test_PID_skokJednostkowy()
{
	//Sygnatura testu:
	std::cerr << "RegPID (k = 0.5, TI = 10.0, TD = 0.2) -> test skoku jednostkowego: ";

	try
	{
		// Przygotowanie danych:
		Regulator_PID instancjaTestowa(0.5, 10.0, 0.2);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu 
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy 
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja skoku jednostkowego w chwili 1. (!!i - daje 1 dla i != 0);
		for (int i = 0; i < LICZ_ITER; i++)
			sygWe[i] = !!i;
		spodzSygWy = { 0, 0.8, 0.7, 0.8, 0.9, 1, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2, 2.1,
			           2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9, 3, 3.1, 3.2, 3.3, 3.4 };

		// Symulacja modelu:
		for (int i = 0; i < LICZ_ITER; i++)
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);

		// Walidacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}

void TESTY_RegulatorPID::test_PI_skokJednostkowy_3() 
{
	//Sygnatura testu:
	std::cerr << "RegPI (k = 0.5, TI = 10.0 -> 5.0 -> 10.0) -> test skoku jednostkowego nr 3: ";

	try
	{
		// Przygotowanie danych:
		Regulator_PID instancjaTestowa(0.5, 10.0);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu 
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy 
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja skoku jednostkowego w chwili 1. (!!i - daje 1 dla i != 0);
		for (int i = 0; i < LICZ_ITER; i++)
			sygWe[i] = !!i;
		spodzSygWy = { 0, 0.6, 0.7, 0.8, 0.9, 1, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.8, 2, 2.2, 2.4,
			           2.6, 2.8, 3, 3.2, 3.4, 3.6, 3.8, 4, 2.35, 2.45, 2.55, 2.65, 2.75, 2.85 };

		// Symulacja modelu:
		for (int i = 0; i < LICZ_ITER; i++)
		{
			if (i == LICZ_ITER * 1 / 5) // przelaczenie na wew. liczenie calki - nie powinno być zauważalane
				instancjaTestowa.setLiczCalk(Regulator_PID::LiczCalk::Wew);
			if (i == LICZ_ITER * 2 / 5) // zmiana stalej calkowania - powinna byc tylko zmiana nachylenia 
				instancjaTestowa.setStalaCalk(5.0);
			if (i == LICZ_ITER * 3 / 5) // przelaczenie na zew. liczenie calki - nie powinno być zauważalane
				instancjaTestowa.setLiczCalk(Regulator_PID::LiczCalk::Zew);
			if (i == LICZ_ITER * 4 / 5) // zmiana stalej calkowania - powinien wsytapic skok wartosci i zmiana nachylenia 
				instancjaTestowa.setStalaCalk(10.0);
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);
		}
		// Uwaga przy poprawnej implementacji zmiany sposobu liczenia całki, nie powinno dość do sytuacji, gdy
		// zmiana sposobu liczenia powoduje skokową zmianę wartości sterowania. dla liczenia całki zwenetrznie
		// zmiana stalej calkowania powinna powodować skok. dla liczenia wewnętrznego, powinno sie tylko zmienic
		// nachylenie.

		// Walidacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}
/*
// testy dla samego Regulatora ON/OFF:

namespace TESTY_RegulatorOnOff
{
	void wykonaj_testy();
	void test_brakPobudzenia();
	void test_skokPowyzHist();
	void test_skokPonizHist();
}

void TESTY_RegulatorOnOff::wykonaj_testy()
{
	test_brakPobudzenia();
	test_skokPowyzHist();
	test_skokPonizHist();
}

void TESTY_RegulatorOnOff::test_brakPobudzenia()
{
	//Sygnatura testu:
	std::cerr << "RegOnOff (1.0,0.1) -> test zerowego pobudzenia: ";
	try
	{
		// Przygotowanie danych:
		RegulatorOnOff instancjaTestowa(1.0,0.1);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu (tu same 0)
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy (tu same 0)
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja modelu:

		for (int i = 0; i < LICZ_ITER; i++)
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);

		// Walidacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}

void TESTY_RegulatorOnOff::test_skokPowyzHist()
{
	//Sygnatura testu:
	std::cerr << "RegOnOff (1.0,0.1) -> test powyzej Hist: ";
	try
	{
		// Przygotowanie danych:
		RegulatorOnOff instancjaTestowa(1.0, 0.1);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu (tu same 0)
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy (tu same 0)
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja skoku jednostkowego w chwili 1. (!!i - daje 1 dla i != 0);
		for (int i = 0; i < LICZ_ITER; i++)
			sygWe[i] = !!i;
		spodzSygWy = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

		// Symulacja modelu:
		for (int i = 0; i < LICZ_ITER; i++)
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);

		// Walidacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}

void TESTY_RegulatorOnOff::test_skokPonizHist()
{
	//Sygnatura testu:
	std::cerr << "RegOnOff (1.0,1.1) -> test ponizej Hist: ";
	try
	{
		// Przygotowanie danych:
		RegulatorOnOff instancjaTestowa(1.0, 1.1);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu (tu same 0)
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy (tu same 0)
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja skoku jednostkowego w chwili 1. (!!i - daje 1 dla i != 0);
		for (int i = 0; i < LICZ_ITER; i++)
			sygWe[i] = !!i;
		spodzSygWy = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

		// Symulacja modelu:
		for (int i = 0; i < LICZ_ITER; i++)
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);

		// Walidacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}


// testy dla pelnego UAR:

namespace TESTY_ProstyUAR
{
	void wykonaj_testy();
	void test_UAR_1_brakPobudzenia();
	void test_UAR_1_skokJednostkowyPID();
	void test_UAR_2_skokJednostkowyPID();
	void test_UAR_3_skokJednostkowyPID();
	void test_UAR_4_skokJednostkowyONOFF();
	void test_UAR_5_skokJednostkowyONOFF();
}

void TESTY_ProstyUAR::wykonaj_testy()
{
	test_UAR_1_brakPobudzenia();
	test_UAR_1_skokJednostkowyPID();
	test_UAR_2_skokJednostkowyPID();
	test_UAR_3_skokJednostkowyPID();
	test_UAR_4_skokJednostkowyONOFF();
	test_UAR_5_skokJednostkowyONOFF();
}

void TESTY_ProstyUAR::test_UAR_1_brakPobudzenia()
{
	//Sygnatura testu:
	std::cerr << "UAR_1 -> test zerowego pobudzenia: ";
	try
	{
		// Przygotowanie danych:
		RegulatorPID testPID(0.5, 5.0, 0.2);
		Model_ARX testARX({ -0.4 }, { 0.6 });
		ProstyUAR instancjaTestowa(testARX, testPID);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu (tu same 0)
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy (tu same 0)
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja UAR:

		for (int i = 0; i < LICZ_ITER; i++)
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);

		// Walidacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}

void TESTY_ProstyUAR::test_UAR_1_skokJednostkowyPID()
{
	//Sygnatura testu:
	std::cerr << "UAR_1 PID -> test skoku jednostkowego: ";
	try
	{
		// Przygotowanie danych:
		RegulatorPID testPID(0.5, 5.0, 0.2);
		Model_ARX testARX({ -0.4 }, { 0.6 });
		ProstyUAR instancjaTestowa(testARX, testPID);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu (tu same 0)
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy (tu same 0)
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja skoku jednostkowego w chwili 1. (!!i - daje 1 dla i != 0);
		for (int i = 0; i < LICZ_ITER; i++)
			sygWe[i] = !!i;
		spodzSygWy = { 0.0, 0.0, 0.54, 0.756, 0.6708, 0.64008, 0.729, 0.810437, 0.834499,
			           0.843338, 0.8664, 0.8936, 0.911886, 0.923312, 0.93404, 0.944929,
			           0.954065, 0.961042, 0.966815, 0.971965, 0.97642, 0.980096, 0.983143,
			           0.985741, 0.987964, 0.989839, 0.991411, 0.992739, 0.993865, 0.994818 
		             };

		// Symulacja UAR:

		for (int i = 0; i < LICZ_ITER; i++)
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);

		// Walidacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}

void TESTY_ProstyUAR::test_UAR_2_skokJednostkowyPID()
{
	//Sygnatura testu:
	std::cerr << "UAR_2 PID (k = 2) -> test skoku jednostkowego: ";
	try
	{
		// Przygotowanie danych:
		RegulatorPID testPID(0.5, 5.0, 0.2);
		Model_ARX testARX({ -0.4 }, { 0.6 }, 2);
		ProstyUAR instancjaTestowa(testARX, testPID);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu (tu same 0)
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy (tu same 0)
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja skoku jednostkowego w chwili 1. (!!i - daje 1 dla i != 0);
		for (int i = 0; i < LICZ_ITER; i++)
			sygWe[i] = !!i;
		spodzSygWy = { 0.0, 0.0, 0.0, 0.54, 0.756, 0.9624, 0.87336, 0.841104, 0.771946, 0.821644,
			           0.863453, 0.93272, 0.952656, 0.965421, 0.954525, 0.955787, 0.957472,
			           0.969711, 0.978075, 0.985968, 0.987821, 0.989149, 0.989053, 0.990645,
			           0.992248, 0.994403, 0.995707, 0.996677, 0.997024, 0.997388
		             };
		// Symulacja UAR:

		for (int i = 0; i < LICZ_ITER; i++)
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);

		// Walidacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}

void TESTY_ProstyUAR::test_UAR_3_skokJednostkowyPID()
{
	//Sygnatura testu:
	std::cerr << "UAR_3 PID (kP=1.0,Ti=2.0) -> test skoku jednostkowego: ";
	try
	{
		// Przygotowanie danych:
		RegulatorPID testPID(1.0, 2.0, 0.2);
		Model_ARX testARX({ -0.4 }, { 0.6 }, 1);
		ProstyUAR instancjaTestowa(testARX, testPID);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu (tu same 0)
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy (tu same 0)
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja skoku jednostkowego w chwili 1. (!!i - daje 1 dla i != 0);
		for (int i = 0; i < LICZ_ITER; i++)
			sygWe[i] = !!i;
		spodzSygWy = { 0.0, 0.0, 1.02, 1.608, 1.1028, 0.41736, 0.546648, 1.20605, 1.43047,
			           0.999176, 0.615056, 0.799121, 1.21304, 1.26025, 0.939289, 0.748507,
			           0.927166, 1.17292, 1.14155, 0.921616, 0.843258, 0.990018, 1.12577,
			           1.068, 0.927024, 0.908125, 1.01702, 1.08484, 1.02618, 0.941508
		             };
		// Symulacja UAR:

		for (int i = 0; i < LICZ_ITER; i++)
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);

		// Walidacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}

void TESTY_ProstyUAR::test_UAR_4_skokJednostkowyONOFF()
{
	//Sygnatura testu:
	std::cerr << "UAR_4 ONOFF (uON=2.0,Hist=0.1) -> test skoku jednostkowego: ";
	try
	{
		// Przygotowanie danych:
		RegulatorOnOff testOnOff(2.0,0.1);
		// dynamika obiektu musi być bardzo wolna aby regulator OnOff miał szanse działać poprawnie.
		Model_ARX testARX({ -0.95 }, { 0.05 }, 1); 
		ProstyUAR instancjaTestowa(testARX, testOnOff);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu (tu same 0)
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy (tu same 0)
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja skoku jednostkowego w chwili 1. (!!i - daje 1 dla i != 0);
		for (int i = 0; i < LICZ_ITER; i++)
			sygWe[i] = !!i;
		spodzSygWy = { 0, 0, 0.1, 0.195, 0.28525, 0.370988, 0.452438, 0.529816,
			           0.603325, 0.673159, 0.739501, 0.802526, 0.8624, 0.91928,
			           0.973316, 1.02465, 1.07342, 1.11975, 1.16376, 1.10557,
			           1.05029, 0.997778, 0.947889, 0.900495, 0.85547, 0.812697,
			           0.872062, 0.928459, 0.982036, 1.03293
		};
		// Symulacja UAR:

		for (int i = 0; i < LICZ_ITER; i++)
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);

		// Walidacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}

void TESTY_ProstyUAR::test_UAR_5_skokJednostkowyONOFF()
{
	//Sygnatura testu:
	std::cerr << "UAR_5 ONOFF (uON=4.0,Hist=0.2) -> test skoku jednostkowego: ";
	try
	{
		// Przygotowanie danych:
		RegulatorOnOff testOnOff(4.0, 0.2);
		// dynamika obiektu musi być bardzo wolna aby regulator OnOff miał szanse działać poprawnie.
		Model_ARX testARX({ -0.95 }, { 0.05 }, 1);
		ProstyUAR instancjaTestowa(testARX, testOnOff);
		constexpr size_t LICZ_ITER = 30;
		std::vector<double> sygWe(LICZ_ITER);      // pobudzenie modelu (tu same 0)
		std::vector<double> spodzSygWy(LICZ_ITER); // spodziewana sekwencja wy (tu same 0)
		std::vector<double> faktSygWy(LICZ_ITER);  // faktyczna sekwencja wy

		// Symulacja skoku jednostkowego w chwili 1. (!!i - daje 1 dla i != 0);
		for (int i = 0; i < LICZ_ITER; i++)
			sygWe[i] = !!i;
		spodzSygWy = { 0, 0, 0.2, 0.39, 0.5705, 0.741975, 0.904876, 1.05963, 1.20665, 1.34632, 1.279,
			           1.21505, 1.1543, 1.09658, 1.04176, 0.989668, 0.940184, 0.893175, 0.848516, 0.80609,
			           0.765786, 0.727497, 0.891122, 1.04657, 1.19424, 1.33453, 1.4678, 1.39441, 1.32469, 1.25845
		};
		// Symulacja UAR:

		for (int i = 0; i < LICZ_ITER; i++)
			faktSygWy[i] = instancjaTestowa.symuluj(sygWe[i]);

		// Walidacja poprawności i raport:
		myAssert(spodzSygWy, faktSygWy);
	}
	catch (...)
	{
		std::cerr << "INTERUPTED! (niespodziwany wyjatek)\n";
	}
}
*/

int main()
{
	
	//TESTY_Model_ARX::wykonaj_testy();
	TESTY_RegulatorPID::wykonaj_testy();
	//TESTY_RegulatorOnOff::wykonaj_testy();
	//TESTY_ProstyUAR::wykonaj_testy();
}

#endif


#ifdef MAIN


int main()
{
	//Twój program
}

#endif

