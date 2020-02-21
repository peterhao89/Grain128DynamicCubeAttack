#include"KreyviumMilpDivisionClass.h"
vector<uint32> ivAll1 = { 0xffffffff,0xffffffff, 0xffffffff, 0xffffffff };
vector<uint32> ivAll0 = { 0,0,0,0 };
string cipherName = "Kreyvium";






//Random Cubes for finding the good one for key recoveries
#if 1
/*
Good Cubes:
Dim115:
{0,1,2,3,4,5,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,39,40,41,42,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,62,63,64,65,67,68,69,70,71,74,75,76,77,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,102,103,104,105,107,108,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127}
*/
int main()
{
	set<int> GoodCube
		= { 0,1,2,3,4,5,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,39,40,41,42,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,62,63,64,65,67,68,69,70,71,74,75,76,77,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,102,103,104,105,107,108,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127 };
	vector<uint32> IVconst = ivAll1;
	int TestRound = 892;
	int Deg;
	ofstream file1;
	string filename;
	cout << "Begin Degree Evaluation:\n";
	filename = cipherName + "Cube" + CubeNameSet(GoodCube, IVLENGTH) + "DegreeEvaluation.txt";
	KreyviumDifferentDegrees degEval = KreyviumDifferentDegrees(GoodCube, IVconst);
	degEval.setOutputRound(TestRound);
	file1.open(filename);
	//degEval.getSPolyDegreeOfZ();//Original Method--Slow
	degEval.getSPolyOverallDegree_Seperately(file1);//Divide-and-Conquer--Fast
	file1.close();
	Deg = degEval.Degree;
	cout << "Degree Evaluation d=" << Deg << endl;
	


	//Computing J:;
	cout << "Computing J:\n";
	KreyviumSuperpolyInvolvedKeysOfCube invv = KreyviumSuperpolyInvolvedKeysOfCube(GoodCube, IVconst);
	invv.setOutputRound(TestRound);
	filename= cipherName + "Cube" + CubeNameSet(GoodCube, IVLENGTH) + "J.txt";
	file1.open(filename);
	invv.getAllInvolvedKeys_Seperately(file1);
	file1.close();

	set<int> InvolvedKeys = invv.InvolvedKeys;
	int countNo;


	cout << "J={";
	countNo = 0;
	for (set<int>::iterator ite = InvolvedKeys.begin(); ite != InvolvedKeys.end(); ++ite)
	{
		countNo++;
		cout << *ite;
		if (countNo != InvolvedKeys.size())
		{
			cout << ",";
		}
	}
	cout << "}\n";

	set<int> notInvolvedKeys;
	for (int i = 0; i < KEYLENGTH; ++i)
	{
		if (InvolvedKeys.find(i) == InvolvedKeys.end())
			notInvolvedKeys.insert(i);
	}

	//Relaxed Term Enumeration
	cout << "Begin relaxed term enumeration for \tilde{J}^2:";
	KreyviumSuperpolyTermEnumerateOfCube termEnum = KreyviumSuperpolyTermEnumerateOfCube(GoodCube, IVconst);
	termEnum.setOutputRound(TestRound);
	/*
	//Origional Method--Slow
	for (int testDeg = Deg; testDeg > 1; --testDeg)
	{
		termEnum.getInvolvedKeysByDegree_FromZ(testDeg);
	}
	*/
	//Divide and Conquer--Fast
	vector<int> TestBits;
	set<int> Deg2Keys; //={ 59, 60, 72, 73, 76, 77, 84, 85, 88, 89 };
	for (int i = 0; i < KreyviumOutputStateBits.size(); ++i)
	{
		KreyviumDifferentDegrees degStateBit = KreyviumDifferentDegrees(GoodCube, IVconst);
		degStateBit.setOutputRound(TestRound);
		degStateBit.getSPolyDegreeOfStateBit(KreyviumOutputStateBits[i]);
		if (degStateBit.Degree > 0)
			TestBits.push_back(KreyviumOutputStateBits[i]);
	}
	filename = cipherName + "Cube" + CubeNameSet(GoodCube, IVLENGTH) + "RelaxedTermEnum.txt";
	file1.open(filename);
	for (int i = 0; i < TestBits.size(); ++i)
	{
		for (int testDeg = Deg; testDeg > 1; --testDeg)
		{

			termEnum.getInvolvedKeysByDegree_FromStateBit(TestBits[i],testDeg,file1);
			if (testDeg == 2)
				Deg2Keys = termEnum.InvolvedKeysByDegree[testDeg - 1];
		}
	}
	file1.close();
	cout << "Get \tilde{J}^2={";
	countNo = 0;
	for (set<int>::iterator ite = Deg2Keys.begin(); ite != Deg2Keys.end(); ++ite)
	{
		countNo++;
		cout << *ite;
		if (countNo != Deg2Keys.size())
		{
			cout << ",";
		}
	}
	cout << "}\n";




	//Get Size Reduced \tilde{J}^1
	cout << "Computing Minimized \tilde{J}^1:" << endl;
	set<int> allKey, LinearKey;
	for (int i = 0; i < KEYLENGTH; ++i)allKey.insert(i);
	filename = cipherName+"Cube"+CubeNameSet(GoodCube,IVLENGTH) + "MinimizeJ1.txt";
	file1.open(filename);
	for (set<int>::iterator ite = Deg2Keys.begin(); ite != Deg2Keys.end(); ++ite)
	{
		set<int> tmpK0 = allKey;
		tmpK0.erase(*ite);
		cout << "Exclude " << *ite << ":\n";
		KreyviumDifferentDegrees degEval = KreyviumDifferentDegrees(GoodCube, ivAll1, tmpK0);
		degEval.setOutputRound(TestRound);
		int tmpDeg = degEval.getSPolyDegreeOfStateBit(KreyviumOutputStateBits[0], file1);
		file1 << *ite << ":" << tmpDeg << endl;
		if (tmpDeg > 0)LinearKey.insert(*ite);
	}
	file1 << "Linear Keys:";
	for (set<int>::iterator ite = LinearKey.begin(); ite != LinearKey.end(); ++ite)
	{
		file1 << dec << *ite << ",";
	}
	file1 << endl;
	file1.close();
	countNo = 0;
	cout << "Get Minimized \tilde{J}^1=" << endl;
	for (set<int>::iterator ite = LinearKey.begin(); ite != LinearKey.end(); ++ite)
	{
		countNo++;
		cout << *ite;
		if (countNo != LinearKey.size())
		{
			cout << ",";
		}
	}
	cout << "}\n";

	getchar();
	return 0;
}
#endif



