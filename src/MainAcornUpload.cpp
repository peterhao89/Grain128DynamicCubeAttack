#include"AcornMilpDivisionClass.h"
vector<uint32> ivAll1 = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
vector<uint32> ivAll0 = { 0,0,0,0 };

string cipherName = "Acorn";







//Test Good Cubes
#if 1




int main()
{
	srand(time(NULL));
	set<int> CubeAll, KeysAll;
	for (int i = 0; i < IVLENGTH; ++i)CubeAll.insert(i);
	for (int i = 0; i < KEYLENGTH; ++i)KeysAll.insert(i);
	vector<uint32> IVconst = ivAll1;
	string filename;
	ofstream file1;
	int HighestDeg = 2;
	set<int> goodCube = {
		0,1,2,4,5,6,7,8,9,10,11,15,16,17,18,19,20,21,22,23,24,26,27,28,29,30,31,32,33,34,35,36,37,38,39,
		40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
		80,81,83,84,85,86,87,89,90,91,92,93,94,95,96,97,99,100,102,103,104,105,106,107,108,109,110,112,113,114,115,116,117,
		118,119,120,121,122,123,125,126,127
	};
	int testRound = 763;


	filename = cipherName + "Cube" + CubeNameSet(goodCube, IVLENGTH) + "DegEval.txt";
	//Get Degree Evaluation d=2
	cout << "Degree Evaluation begin:\n";
	AcornDifferentDegrees degEval1 = AcornDifferentDegrees(goodCube, IVconst);
	degEval1.setOutputRound(testRound);
	file1.open(filename);
	degEval1.getSPolyDegree(file1);
	file1.close();
	HighestDeg = degEval1.Degree;
	cout << "Degree Evaluate d=" << HighestDeg << endl;
	AcornSuperpolyInvolvedKeysOfCube invv = AcornSuperpolyInvolvedKeysOfCube(goodCube, IVconst);
	invv.setOutputRound(testRound);



	//Get J (equivalent to the original \tilde{J}^1
	cout << "Computing J<-TermEnum(I,IV,R,1)\n";
	filename = cipherName+"Cube" + CubeNameSet(goodCube, IVLENGTH) + "J.txt";
	file1.open(filename);
	invv.getInvolvedKeys(file1);
	file1.close();
	set<int> involvedKeys;//{ 0,1,2,5,6,7,9,10,11,12,13,15,16,17,19,20,21,22,23,24,25,26,27,28,29,34,35,39,44,46,49,50,54,56,59,60,83,93 };
	involvedKeys = invv.InvolvedKeys;


	cout << "Get J containing all involved key bits. J={";
	int countNo = 0;
	for (set<int>::iterator ite = involvedKeys.begin(); ite != involvedKeys.end(); ++ite)
	{
		++countNo;
		cout << *ite;
		if (countNo != involvedKeys.size())
			cout << ", ";
	}
	cout << "}\n";
	


	set<int> notInvolvedKeys;
	for (int i = 0; i < KEYLENGTH; ++i)
	{
		if (involvedKeys.find(i) == involvedKeys.end())
			notInvolvedKeys.insert(i);
	}




	cout << "Computing \tilde{J}^2<-TermEnum(I,IV,R,2)\n";
	set<int> involvedKeysHighestDeg;
	for (int evalDeg = HighestDeg; evalDeg > 1; --evalDeg)
	{
		AcornSuperpolyTermEnumerateOfCube termEnum(goodCube, IVconst, notInvolvedKeys);
		termEnum.setOutputRound(testRound);
		termEnum.Degree = HighestDeg;
		filename = cipherName + "Cube" + CubeNameSet(goodCube, IVLENGTH) + "RelaxedDeg" + to_string(evalDeg) + ".txt";
		file1.open(filename, ios::app);
		termEnum.getInvolvedKeysByDegree(evalDeg, file1);
		file1.close();
		if (evalDeg == HighestDeg)
			involvedKeysHighestDeg = termEnum.InvolvedKeysByDegree[HighestDeg - 1];
	}
	cout << "Used relaxed term enumeration to get \tilde{J}^2={";
	countNo = 0;
	for (set<int>::iterator ite = involvedKeysHighestDeg.begin(); ite != involvedKeysHighestDeg.end(); ++ite)
	{
		++countNo;
		cout << *ite;
		if (countNo != involvedKeysHighestDeg.size())
			cout << ", ";
	}
	cout << "}\n";



	//Minimize \tiled{J}^1
	cout << "Minimizing \tilde{J}^1\n";
	filename = cipherName+"Cube" +CubeNameSet(goodCube,IVLENGTH)+ "MinimizeJ1.txt";
	file1.open(filename, ios::app);
	set<int> MinimizeJ1 = involvedKeys;
	for (set<int>::iterator ite = involvedKeysHighestDeg.begin(); ite != involvedKeysHighestDeg.end(); ++ite)
	{
		set<int> tmpK0 = KeysAll;
		tmpK0.erase(*ite);
		AcornDifferentDegrees degEval(goodCube, IVconst, tmpK0);
		degEval.setOutputRound(testRound);
		degEval.getSPolyDegree(file1);
		file1 << "Exclude " << *ite << ": " << degEval.Degree << endl;
		if (degEval.Degree == 0)
		{
			MinimizeJ1.erase(*ite);
		}
	}
	file1.close();
	cout << "Get Minimized \tilde{J}^1={";
	countNo = 0;
	for (set<int>::iterator ite = MinimizeJ1.begin(); ite != MinimizeJ1.end(); ++ite)
	{
		++countNo;
		cout << *ite;
		if (countNo != MinimizeJ1.size())
			cout << ", ";
	}
	cout << "}\n";



	return 0;
}

#endif



#if 0
int main()
{
	set<int> CubeAll, KeysAll;
	for (int i = 0; i < IVLENGTH; ++i)CubeAll.insert(i);
	for (int i = 0; i < KEYLENGTH; ++i)KeysAll.insert(i);
	int testRound = 751;
	int Dim = 102;

	set<int> TestCube;
	string filename;
	while (1)
	{
		Dim = 102 + (rand_32() % 4);
		TestCube = RandSet(Dim, CubeAll);
		AcornDifferentDegrees mmy(TestCube, ivAll0);
		mmy.setOutputRound(testRound);
		int iDegree = mmy.getSPolyDegree();
		if (iDegree <= 5)
		{
			filename = "LowDegCube" + CubeNameSet(TestCube, IVLENGTH) + ".txt";
			ofstream fileGet(filename);
			fileGet << "Cube:";
			for (set<int>::iterator ite = TestCube.begin(); ite != TestCube.end(); ++ite)
			{
				fileGet << dec << *ite << ",";
			}
			fileGet << endl;
			fileGet << "Dim:" << Dim << endl;
			fileGet.close();
			break;
		}
	}
	ofstream file1;
	file1.open(filename, ios::app);
	AcornSuperpolyInvolvedKeysOfCube invv(TestCube, ivAll0);
	invv.setOutputRound(testRound);
	invv.getInvolvedKeys(file1);
	file1.close();
	return 0;
}
#endif




#if 0
int main()
{
	set<int> Cube = { 0,1,2,3,7,9,10,11,12,13,14,15,16,17,19,21,22,23,24,26,27,28,29,30,32,33,36,38,39,41,44,45,46,47,48,49,51,52,54,55,57,58,59,60,62,63,64,65,66,67,69,70,72,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,96,97,98,99,100,101,102,103,104,105,107,108,109,110,111,112,113,114,116,117,118,119,120,121,122,125,126 };
	int TestRound = 750;

	AcornDifferentDegrees mmy = AcornDifferentDegrees(Cube, ivAll0);
	mmy.setOutputRound(TestRound);
	mmy.getSPolyDegree();
	cout << "Round " << TestRound << ":" << mmy.Degree << endl;


	AcornSuperpolyInvolvedKeysOfCube invv = AcornSuperpolyInvolvedKeysOfCube(Cube, ivAll0);
	invv.setOutputRound(TestRound);
	invv.getInvolvedKeys();



	getchar();
	return 0;
}
#endif


//Draw Upper Bound for Acorn
#if 0
int main()
{
	set<int> CubeAll, KeysAll;
	for (int i = 0; i < IVLENGTH; ++i)CubeAll.insert(i);
	for (int i = 0; i < KEYLENGTH; ++i)KeysAll.insert(i);
	int testRound = 750;

	cout << "Test Round Start From ";
	cin >> testRound;

	ofstream file1;
	string filename = cipherName + "AllActiveK0.txt";

	while (1)
	{
		cout << "At Round " << testRound << endl;
		bool isUpperBound = true;
		for (int i = 0; i < KEYLENGTH; ++i)
		{
			set<int> testK0 = KeysAll;
			testK0.erase(i);
			AcornDifferentDegrees mmy(CubeAll, ivAll0);
			mmy.setK0ForModel(testK0);
			mmy.setOutputRound(testRound);
			int iDegree = mmy.getSPolyDegree();
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
	getchar();
	return 0;
}

#endif
