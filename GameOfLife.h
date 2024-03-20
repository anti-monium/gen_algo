#include <iostream>
#include <chrono>
#include <thread>

struct GameField
{
    int8_t **Field = nullptr;
    static int Size;

    GameField()
    {
        Field = new int8_t *[Size];
        for (int i = 0; i < Size; i++) Field[i] = new int8_t [Size];
    }

    GameField(int8_t a)
	{
		if (Field == nullptr)
        {
            Field = new int8_t *[Size];
            for (int i = 0; i < Size; i++) Field[i] = new int8_t [Size];
        }
		for (int i = 0; i < Size; i++)
		{
            for (int j = 0; j < Size; j++) Field[i][j] = a;
		}
	}

    void DeepCopy(int8_t **f)
    {
        if (Field == nullptr)
        {
            Field = new int8_t *[Size];
            for (int i = 0; i < Size; i++) Field[i] = new int8_t [Size];
        }
		for (int i = 0; i < Size; i++)
		{
            for (int j = 0; j < Size; j++) Field[i][j] = f[i][j];
		}
    }

    void RandomField()
    {
        if (Field == nullptr)
        {
            Field = new int8_t *[Size];
            for (int i = 0; i < Size; i++) Field[i] = new int8_t [Size];
        }
		for (int i = 0; i < Size; i++)
		{
            for (int j = 0; j < Size; j++) Field[i][j] = rand() % 2;
		}
    }

    int8_t* BitSolution()
    {
        int8_t *solution = new int8_t[Size*Size];
        for (int i = 0; i < Size; i++) {
            for (int j = 0; j < Size; j++) solution[i * Size + j] = Field[i][j];
        }
		return solution;
    }

    ~GameField()
	{
		if (Field != nullptr)
			for (int i = 0; i < Size; i++) delete[] Field[i];
            delete[] Field;
		Field = nullptr;
	}
};

class GameOfLife
{
  public:
    GameField f;
    int maxSteps;

    bool staticConf = false;
    bool winning = false;
    int steps = 0;

    GameOfLife(int def_maxSteps) {
        f.RandomField();
		maxSteps = def_maxSteps;
    }

    GameOfLife(int8_t **conf, int def_maxSteps)	//передаем уже существующую конфигурацию
	{
		f.DeepCopy(conf);
		maxSteps = def_maxSteps;
	}

    int Play(int visualisation)	//запускаем работу клеточного автомата на заданное количество шагов,
                                    // либо пока автомат не станет стационарным
	{	                            //значения visualisation: -1 без демонстрации, >=0 - задержка в секундах
		if (visualisation > -1)
			PrintState(visualisation);
		GameField old_f = GameField();
		while (!staticConf && steps <= maxSteps)
		{
			steps++;
			staticConf = true;
			old_f.DeepCopy(f.Field);
			for (int i = 0; i < GameField::Size; i++)
			{
				for (int j = 0; j < GameField::Size; j++)
				{
					int neighbours = 0;
					if (i - 1 >= 0) neighbours += old_f.Field[i - 1][j];
					if (j - 1 >= 0) neighbours += old_f.Field[i][j - 1];
					if (i - 1 >= 0 && j - 1 >= 0) neighbours += old_f.Field[i - 1][j - 1];
					if (i + 1 < GameField::Size) neighbours += old_f.Field[i + 1][j];
					if (j + 1 < GameField::Size) neighbours += old_f.Field[i][j + 1];
					if (i + 1 < GameField::Size && j + 1 < GameField::Size) neighbours += old_f.Field[i + 1][j + 1];
					if (i - 1 >= 0 && j + 1 < GameField::Size) neighbours += old_f.Field[i - 1][j + 1];
					if (i + 1 < GameField::Size && j - 1 >= 0) neighbours += old_f.Field[i + 1][j - 1];
					if (neighbours < 2 || neighbours > 3)	//клетка умирает
					{
						f.Field[i][j] = 0;
						staticConf = false;
					}
					else if (neighbours == 3)	//рождается новая клетка
					{
						f.Field[i][j] = 1; 
						staticConf = false;
					}
				}
			}
			if (visualisation > -1)
				PrintState(visualisation);
		}
		if (!staticConf && steps >= maxSteps)	//автомат прожил нужное количество шагов и не стал вырожденным(стационарным)
			winning = true;
		return steps;
	}

	int StartNew(const GameField* base_conf, int visualisation)	//запуск игры с новым начальным полем
	{
		steps = 0;
		winning = false;
		staticConf = false;
		f.DeepCopy(base_conf->Field);
		return Play(visualisation);
	}

    void PrintState(int visualisation) 
	{
		system("clear");
		std::cout << "\n\nstep " << (int)steps << "\n\n";
		for (int i = 0; i < GameField::Size; i++)
		{
			for (int j = 0; j < GameField::Size; j++)
			{
				std::cout << ((f.Field[i][j]>0) ? "X" : "-");
			}
			std::cout << '\n';
		}
		std::this_thread::sleep_for(std::chrono::nanoseconds(visualisation * 100000000));
		if (staticConf)
			std::cout << "degenerated\n";
	}
};