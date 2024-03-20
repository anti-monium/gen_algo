#include "GameOfLife.h"
#include <omp.h>

int pop_size;

class Mutation
{
public:
	virtual void mutate(GameField* pop) = 0;
	virtual ~Mutation() = default;
};

class Crossover
{
public:
	virtual GameField* crossing(const GameField* pop) = 0;
	virtual ~Crossover() = default;

};

class Selection
{
public:
	virtual GameField* select(const GameField* pop, const int* func) = 0;
	virtual ~Selection() = default;

};

class SurvivalFunc
{
public:
	virtual int Survival(const GameField* field) = 0;
	virtual ~SurvivalFunc() = default;

};

//-------------------------------------------


class FuncCellAutomat : public SurvivalFunc
{
	int maxSteps = 100;
public:
	FuncCellAutomat(){}
	int Survival(const GameField* field)
	{
		GameOfLife game(field->Field, maxSteps);
		game.Play(-1);
		int val = 0;
		for (int i = 0; i < GameField::Size; i++)
		{
            for (int j = 0; j < GameField::Size; j++) val += game.f.Field[i][j];
		}
		if (game.staticConf == true) val += 50 * 50; //штраф
		return val;
	}
};

class ProportionalSelection : public Selection
{
public:
	GameField* select(const GameField* pop, const int* func)
	{
		// http://qai.narod.ru/GA/strategies.html
		GameField* new_pop = new GameField[pop_size];
		int f_sum = 0;
		for (int i = 0; i < pop_size; i++) f_sum += func[i];
		double avg_f = (1.0 * f_sum) / pop_size;
		int j = 0;
		for (int i = 0; i < pop_size; i++)
		{
			if (j >= pop_size) break;
			for (int k = 0; k < func[i] / avg_f; k++ ) 
				if (j < pop_size) new_pop[j++].DeepCopy(pop[i].Field);
			if (j >= pop_size) break;
			if (rand() % 100 > ((1.0 * func[i]) / avg_f) * 100) new_pop[j++].DeepCopy(pop[i].Field);
		}
		for (; j < pop_size; j++) {
			new_pop[j].DeepCopy(pop[j].Field);
		}
		return new_pop;
	}
};

class TwoPointCB : public Crossover
{
	int point1, point2;
	double p_crossing = 0.8;
public:
	GameField* crossing(const GameField* pop)
	{
		GameField* new_pop = new GameField[pop_size];
		int length = GameField::Size;
		srand(time(0));
		point1 = rand() % length;
		point2 = point1 + rand() % (length - point1);
		for (int i = 0; i < pop_size; i += 2)
		{
			int par1_index = rand() % pop_size;
			int par2_index = rand() % pop_size;

			double p = (1.0*rand()) / RAND_MAX;
			if (p > p_crossing) point1 = length;
			for (int j = 0; j < length; j++)
			{
				for (int k = 0; k < length; k++) {
					if (j >= point1 && j < point2 && k >= point1 && k < point2) {
                		new_pop[i].Field[j][k] = pop[par2_index].Field[j][k];
                		if (i + 1 < pop_size) new_pop[i + 1].Field[j][k] = pop[par1_index].Field[j][k];
					} else {
						new_pop[i].Field[j][k] = pop[par1_index].Field[j][k];
                		if (i + 1 < pop_size) new_pop[i + 1].Field[j][k] = pop[par2_index].Field[j][k];
					}
				}
			}
		}
		return new_pop;
	}
};

class BitMutation : public Mutation
{
	double Pmut;
public:
	BitMutation(double p)
	{
		Pmut = p;
	}

	void mutate(GameField* pop)
	{
		for (int i = 0; i < pop_size; i++)
		{
			for (int j = 0; j < GameField::Size; j++)
			{
                for (int k = 0; k < GameField::Size; k++)
                {
				    double p = (1.0*rand()) / RAND_MAX;
				    if (p < Pmut) pop[i].Field[j][k] = (pop[i].Field[j][k] == 0) ? 1 : 0;
                }
			}
		}
	}
};

class GeneticGoL
{
	Mutation* mut;
	Selection* sel;
	Crossover* cov;
	SurvivalFunc* sf;

	GameField* population;
	int* func;

	GameField best;
	int best_value = 0;

	int msi;
	int steps_count;

	void getGenerationResult(int *stable_steps_count)
	{
		int f_sum = 0;
#pragma omp parallel
			{
#pragma omp for
				for (int i = 0; i < pop_size; i++) func[i] = sf->Survival(&(population[i]));
			}
			for (int i = 0; i < pop_size; i++)
			{
				if (func[i] < best_value)
				{
					best.DeepCopy(population[i].Field);
					best_value = func[i];
					if (stable_steps_count != nullptr) *stable_steps_count = -1;
					std::cout << func[i] << std::endl;
				}
				f_sum += func[i];
			}
			//std::cout << "\tAverage f(i) = " << f_sum << ", best result: " << best_value << std::endl;
			std::cout << "\tAverage f(i) = " << (1.0 * f_sum) / pop_size << ", best result: " << best_value << std::endl;

	}

public:

	GeneticGoL(Mutation* m, Selection* s, Crossover* c, SurvivalFunc* f, int max_stable_iterations)
	{
		mut = m;
		sel = s;
		cov = c;
		sf = f;
		msi = max_stable_iterations;
		population = new GameField[pop_size];
		func = new int[pop_size];
		best_value = GameField::Size * GameField::Size + 1;
#pragma omp parallel
		{
#pragma omp for
			for (int i = 0; i < pop_size; i++)
			{
				population[i].RandomField();
            }
        }
		std::cout << "Generation 0.";
		getGenerationResult(nullptr);
	}

	void Start()
	{
		int stable_steps_count = 0;
		steps_count = 0;
		GameField* new_pop;
		while (stable_steps_count < msi)
		{
			steps_count++;
			std::cout << "Generation " << steps_count << ".";
			new_pop = sel->select(population, func);
			delete[] population;
			population = new_pop;
			new_pop = cov->crossing(population);
			delete[] population;
			population = new_pop;
			mut->mutate(population);
			getGenerationResult(&stable_steps_count);
			stable_steps_count++;
		}
	}

	GameField* GetBestField()
	{
		return &best;
	}

	int GetBestValue()
	{
		return best_value;
	}

	int GetStepsCount()
	{
		return steps_count;
	}

	~GeneticGoL()
	{
		delete[] population;
		delete[] func;
		delete mut;
		delete sel;
		delete cov;
		delete sf;
	}

};