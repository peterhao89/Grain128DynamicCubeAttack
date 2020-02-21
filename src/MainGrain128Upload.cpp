#include"Grain128MilpDivisionClass.h"
#include"Grain128CubeRunner.h"
#include<omp.h>

vector<uint32> ivAll1 = { 0xffffffff, 0xffffffff, 0xffffffff };
vector<uint32> ivAll0 = { 0,0,0 };
string cipherName = "Grain128";
vector<nullifyOneCrucialBit> strategies = { { 90, 158, true },{ 30,158,true } };



void practicalEvaluationWithRandomKeys(int testTime = 1000)
{
	int testRound = 179;
	vector<CubeAndStrategy> ontest =
	{
		{ { 15,31,33,34,44,65,79,86,88 }, strategies[0], ivAll0 },
		{ { 5,6,9,37,40,41,43,64,67 }, strategies[0], ivAll0 },
	};
	cout << "Practical Computation of the biases" << endl;
	for (int i = 0; i < ontest.size(); ++i)
	{
		cout << "Cube No." << i + 1 << endl;
		string filename = "PracticalEvaluatedBiasOfCubeNo" + to_string(i + 1) + ".txt";
		ofstream file1(filename);
		DynamicCubeAttack mmy = DynamicCubeAttack(ontest[i], testRound);
		mmy.count0SumForOneGuess(0, testTime, file1);
		file1 << "epsilon_0: " << double(mmy.count0Sum) / double(testTime) - 0.5 << endl;
		cout << "epsilon_0: " << double(mmy.count0Sum) / double(testTime) - 0.5 << endl;
		mmy.count0SumForOneGuess(1, testTime, file1);
		file1 << "epsilon_1: " << double(mmy.count0Sum) / double(testTime) - 0.5 << endl;
		cout << "epsilon_1: " << double(mmy.count0Sum) / double(testTime) - 0.5 << endl;
		file1.close();
	}

}

void divisionPropertyVerificationOfDegreeAndLambda(int testRound)
{
	
	vector<CubeAndStrategy> ontest;
	vector<set<int>> Lambda;

	if (testRound == 179)
	{
		ontest =
		{
			{ { 15,31,33,34,44,65,79,86,88 }, strategies[0], ivAll0 },
			{ { 5,6,9,37,40,41,43,64,67 }, strategies[0], ivAll0 },
		};
		Lambda =
		{
			{ 37,125 },
			{ 75 }
		};
	}
	else
	{
		testRound = 256;
		vector<set<int>> ExcludeIndices=
		{
			/*90-b158*/
			{ 39,40,41,43,50,90 },
			{ 39,40,41,45,50,90 },
			{ 21,39,40,41,43,90 },
			{ 25,39,40,41,43,90 },
			{ 30,39,40,41,43,90 },
			{ 39,40,41,43,44,90 },
			{ 39,40,41,43,49,90 },
			{ 39,40,41,43,50,90 },
			{ 39,40,41,43,54,90 },
			{ 39,40,41,43,61,90 },
			{ 39,40,41,43,72,90 },
			{ 39,40,41,43,87,90 },
			{ 39,40,41,43,89,90 },
			{ 39,40,41,43,90,94 },
			{ 40,41,43,44,50,90 },
			/*30-b158*/
			{ 30,39,40,41,45,50 },
			{ 21,30,39,40,41,43 },
			{ 25,30,39,40,41,43 },
			{ 30,39,40,41,43,44 },
			{ 30,39,40,41,43,49 },
			{ 30,39,40,41,43,50 },
			{ 30,39,40,41,43,50 },
			{ 30,40,41,43,44,50 },
			{ 30,39,40,41,43,54 },
			{ 30,39,40,41,43,61 },
			{ 30,39,40,41,43,72 },
			{ 30,39,40,41,43,87 },
			{ 30,39,40,41,43,89 },
			{ 30,39,40,41,43,94 }
		};

		for (int i = 0; i < 15; ++i)
		{
			set<int> Cube;
			for (int j = 0; j < IVLENGTH; ++j)
			{
				if (ExcludeIndices[i].find(j) == ExcludeIndices[i].end())
				{
					Cube.insert(j);
				}
			}
			ontest.push_back({ Cube,strategies[0],ivAll0 });
		}
		for (int i = 15; i < 29; ++i)
		{
			set<int> Cube;
			for (int j = 0; j < IVLENGTH; ++j)
			{
				if (ExcludeIndices[i].find(j) == ExcludeIndices[i].end())
				{
					Cube.insert(j);
				}
			}
			ontest.push_back({ Cube,strategies[1],ivAll0 });
		}
		Lambda=
		{
			/*1-15*/
			{36,81,85},
			{65,81,125 },
			{38,81,125 },
			{81,91,125 },
			{57,81,125 },
			{38,81,125 },
			{77,81,125 },
			{58,81,125 },
			{57,81,125 },
			{54,81,125 },
			{81,125 },
			{81,125 },
			{81,91,125 },
			{81,125 },
			{69,81,125 },
			/*16-29*/
			{ 38,81,125 },
			{81,89,125 },
			{52,81,125 },
			{81,89,125 },
			{77,81,125 },
			{69,81,125 },
			{58,81,125 },
			{69,81,125 },
			{81,106,125 },
			{50,81,125 },
			{81,125 },
			{81,125 },
			{81,91,125 },
			{81,125 }
		};



	}



	cout << "MILP model aided division property based degree evaluations:\n";
	for (int i = 0; i < ontest.size(); ++i)
	{
		cout << "Cube No." << i + 1 << endl;
		string filename = "divisionPropertyEvaluateDegreesOfCubeNo" + to_string(i + 1) + ".txt";
		ofstream file1(filename);
		int countNo = 0;
		file1 << "Cube Index:";
		for (set<int>::iterator ite = ontest[i].Cube.begin(); ite != ontest[i].Cube.end(); ++ite)
		{
			++countNo;
			file1 << *ite;
			if (countNo < ontest[i].Cube.size())
				file1 << ",";
		}
		file1 << endl;
		file1 << "Round " << testRound << endl;
		file1 << "Non-Cube IV:" << ontest[i].nonCubeIVs[0] << ","
			<< ontest[i].nonCubeIVs[1] << ","
			<< ontest[i].nonCubeIVs[2] << endl;
		file1 << "Experiments:\n";

		Grain128DifferentDegrees degEval = Grain128DifferentDegrees(ontest[i].Cube, ontest[i].nonCubeIVs);
		degEval.setNullificationStrategy({ ontest[i].oneStrategy });
		degEval.setOutputRound(testRound);
		degEval.getSPolyDegreeOfZ();
		cout << "d0=" << degEval.Degree << endl;
		file1 << "d0=" << degEval.Degree << endl;


		degEval.getSPolyDegreeOfZ_DynamicWrong();
		cout << "d1=" << degEval.Degree << endl;
		file1 << "d1=" << degEval.Degree << endl;
		degEval.setK0ForModel(Lambda[i]);
		degEval.getSPolyDegreeOfZ_DynamicWrong();
		cout << "With key bits in Lambda={";
		file1 << "With key bits in Lambda={";
		countNo = 0;
		for (set<int>::iterator ite = Lambda[i].begin(); ite != Lambda[i].end(); ++ite)
		{
			++countNo;
			file1 << *ite;
			cout << *ite;
			if (countNo < Lambda[i].size())
			{
				file1 << ",";
				cout << ",";
			}
		}
		cout << "} set to 0, the Degree is " << degEval.Degree << endl;
		file1 << "} set to 0, the Degree is " << degEval.Degree << endl;
		file1.close();
	}

}



//Practically compute the probability of 0-summations
#if 1
int main()
{
	srand(time(NULL));
	
	cout << "Experiments:\n";
	int testNo;
	cout << "1: Practically evaluate epsilon_0, epsilon_1 for the 2 cubes on 179-round Grain-128;\n";
	cout << "2: Evaluations of different superpoly degrees for the 2 cubes on 179-round Grain-128;\n";
	cout << "3: Evaluations of different superpoly degrees for the 29 cubes on 256-round Grain-128 (take long time);\n";
	cin >> testNo;
	switch (testNo)
	{
		case 1:
		{
			cout << "How many random keys do you want to test?(better over 1000)\n";
			int testTime;
			cin >> testTime;
			practicalEvaluationWithRandomKeys(testTime);
			break;
		}
		case 2:
		{
			divisionPropertyVerificationOfDegreeAndLambda(179);
			break;
		}
		case 3:
		{
			divisionPropertyVerificationOfDegreeAndLambda(256);
			break;
		}
		default:
		{	
			cout << "Illegal input!" << endl;
			break;
		}
	}





	getchar();

	return 0;
}

#endif
