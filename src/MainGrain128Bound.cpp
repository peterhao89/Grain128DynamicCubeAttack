#include"Grain128MilpDivisionClass.h"
vector<uint32> ivAll1 = { 0xffffffff, 0xffffffff, 0xffffffff };
vector<uint32> ivAll0 = { 0,0,0 };
string cipherName = "Grain128";

//Draw Upper Bound for bias tester
void drawBiasBound()
{
	set<int> CubeAll, KeysAll;
	for (int i = 0; i < IVLENGTH; ++i)CubeAll.insert(i);
	for (int i = 0; i < KEYLENGTH; ++i)KeysAll.insert(i);
	int testRound = 240;

	cout << "Start to draw the bias cube tester bound: \n";
	cout << "Test Start From Round " << testRound << endl;

	ofstream file1;
	string filename = cipherName+"BiasTesterBound.txt";

	while (1)
	{
		cout << "At Round " << testRound << endl;
		bool isUpperBound = true;
		for (int i = 0; i < KEYLENGTH; ++i)
		{
			set<int> testK0 = KeysAll;
			testK0.erase(i);
			Grain128DifferentDegrees mmy = Grain128DifferentDegrees(CubeAll, ivAll0);
			mmy.setK0ForModel(testK0);
			mmy.setOutputRound(testRound);
			int iDegree = mmy.getSPolyDegreeOfZ();
			file1.open(filename, ios::app);
			file1 << "Round " << testRound << endl;
			file1 << "Exclude Key:" << dec << i << endl;
			file1 << "Degree is " << dec << iDegree << endl;
			file1.close();
			if (iDegree == 0)
			{
				isUpperBound = false;
				break;
			}
		}
		if (isUpperBound == false)
		{
			++testRound;
		}
		else
		{
			break;
		}
	}
	file1.open(filename, ios::app);
	file1 << dec << "Upper Bound is " << testRound << endl;
	file1.close();
}

//Draw Upper Bound for Zero-Sum tester
void drawZeroSumBound()
{
	set<int> CubeAll, KeysAll;
	for (int i = 0; i < IVLENGTH; ++i)CubeAll.insert(i);
	for (int i = 0; i < KEYLENGTH; ++i)KeysAll.insert(i);
	int testRound = 240;

	cout << "Start to draw the zero-sum cube tester bound: \n";
	cout << "Test Start From Round " << testRound << endl;

	ofstream file1;
	string filename = cipherName + "ZeroSumBound.txt";

	while (1)
	{
		cout << "At Round " << testRound << endl;
		bool isUpperBound = true;
		Grain128DifferentDegrees mmy(CubeAll, ivAll0);
		mmy.setOutputRound(testRound);
		int iDegree = mmy.getSPolyDegreeOfZ();
		file1.open(filename, ios::app);
		file1 << "Round " << testRound << endl;
		file1 << "Degree is " << dec << iDegree << endl;
		file1.close();
		if (iDegree == 0)
		{
			isUpperBound = false;
		}
		if (isUpperBound == false)
		{
			++testRound;
		}
		else
		{
			break;
		}
	}
	file1.open(filename, ios::app);
	file1 << dec << "Upper Bound is " << testRound << endl;
	file1.close();
}

int main()
{
	cout << "Draw Bounds for "+cipherName+" against different cube testers:\n";
	int testNo;
	cout << "1: Zero-Sum;\n";
	cout << "2: Bias;\n";
	cin >> testNo;
	switch (testNo)
	{
		case 1:
		{
			drawZeroSumBound();
			break;
		}
		case 2:
		{
			drawBiasBound();
			break;
		}
		default:
		{	
			cout << "Illegal input!\n";
			break;
		}
	}
	return 0;
}

