// GeneticAlg.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <chrono>
#include <string>
#include <fstream>
#include <cmath>

#include "GeneticAlgo.h"

using namespace std;
int GameField::Size;

void research()
{
	GameField::Size = 50;
	pop_size = 100;
	int max_stable = 50;	//задаем максимальное количество поколений без улучшения результата
	double pmut_init = 1.0 / (GameField::Size * GameField::Size);
	GameOfLife best_game(100);

	ofstream log;
	log.open("./res/GAresearch.log");
	ofstream resfile;
	string fname;

	for (int i = 0; i <= 9; i++)
	{
		double pmut = pmut_init * pow(1.5, i);
		for (int j = 0; j <= 9; j++)
		{
			cout << "\n\n----------\nSeries " << i << " run " << j << " with Pmut = " << pmut << " starting.\n";

			auto t_start = chrono::system_clock::now();
			GeneticGoL ga(new BitMutation(pmut), new ProportionalSelection(), new TwoPointCB(), new FuncCellAutomat(), max_stable);
			ga.Start();
			auto t_end = chrono::system_clock::now();
			chrono::duration<double> duration = t_end - t_start;
			cout << "\nRun " << i << ":" << j << " successfully finished.\n";
			cout << "Duration " << duration.count() << " seconds\n";
			log << "Series " << i << " run " << j << ":\n\tPmut = " << pmut << "; time = " << duration.count() << " seconds; ";
			log << "Generations total " << ga.GetStepsCount() << "; Best value = " << ga.GetBestValue() << endl;
			GameField* bf = ga.GetBestField();
			best_game.StartNew(bf, -1);
			resfile.open("./res/series_" + to_string(i) + "_run_" + to_string(j) + "_sol.txt");
			for (int m = 0; m < 50; m++)
			{
				for (int n = 0; n < 50; n++)
					resfile << (((*bf).Field[m][n]>0) ? "X" : "-");
				resfile << endl;
			}
			resfile.close();
			resfile.open("./res/series_" + to_string(i) + "_run_" + to_string(j) + "_sol_after100.txt");
            //эксперименты с визуализацией
			for (int m = 0; m < 50; m++)
			{
				for (int n = 0; n < 50; n++)
					resfile << ((best_game.f.Field[m][n]>0) ? "X" : "-");
				resfile << endl;
			}
			resfile.close();
		}
		cout << "\nSeries " << i << " finished\n";
	}
}


int main()
{		
	string choice = "";
	cout << "Enter the required action:\n" <<
		"\tPlaner\n\tFile\n\tGA\n\tResearch\n: ";
	cin >> choice;
	if (choice == "Planer")
	{
		GameField::Size = 20;
		GameField Planer(0);	//"планер"
		Planer.Field[5][3] = Planer.Field[5][4] = Planer.Field[5][5] = Planer.Field[4][5] = Planer.Field[3][4] = 1;
		GameOfLife gameplaner(Planer.Field, 60);
		gameplaner.Play(1);
		return 0;
	}
	else if (choice == "File")
	{
		GameField::Size = 50;
		GameField ff;
		string fname = "";
		cout << "Enter filename: ";
		cin >> fname;
		ifstream f;
		f.open(fname);
		if (f.bad())
		{
			cout << "Cannot open this file.\n";
			return 0;
		}
		char c;
        for (int i = 0; i < GameField::Size; i++) {
            for (int j = 0; j < GameField::Size; j++) {
                c = f.get();
                if (c == EOF) {
                    cout << "Bad file.\n";
                    return 0;
                } else if (c == 'X') ff.Field[i][j] = 1;
			    else if (c == '-') ff.Field[i][j] = 0;
            }
        }
		GameOfLife game(ff.Field, 100);
		game.Play(1);
		cout << "result: \n\n\n";
		for (int i = 0; i < GameField::Size; i++)
		{
			for (int j = 0; j < GameField::Size; j++)
				cout << ((game.f.Field[i][j]>0) ? "X" : "-");
			cout << endl;
		}
		FuncCellAutomat fc;
		cout << "\nValue = " << fc.Survival(&ff) << endl;
		return 0;
	}
	else if (choice == "Research")
	{
		research();
		return 0;
	}


	srand(time(0));

	GameField::Size = 50;
	pop_size = 100;
	double Pmut = 1.0 / (GameField::Size * GameField::Size);
	int max_stable_steps = 60;

	//запуск генетического алгоритма
	auto t_start = chrono::system_clock::now();
	GeneticGoL ga(new BitMutation(Pmut), new ProportionalSelection(), new TwoPointCB(), new FuncCellAutomat(), max_stable_steps);
	ga.Start();
	auto t_end = chrono::system_clock::now();
	chrono::duration<double> duration = t_end - t_start;
	cout << "\nSuccessfully finished.\n";
	cout << "Duration " << duration.count() << " seconds\n";

	//получение результатов
	int bv = ga.GetBestValue();
	GameField* bf = ga.GetBestField();
	GameOfLife best_game(bf->Field, 100);
	cout << "Best value = " << bv << "\n";
    cout << "Enter latency in seconds: ";
	int l;
	cin >> l;
	best_game.Play(l);
	return 0;
}