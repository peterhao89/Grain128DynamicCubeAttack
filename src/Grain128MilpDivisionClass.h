#pragma once
#include"DivisionPropertyBasic.h"
#define CORENUM 6
#define KEYLENGTH 128
#define IVLENGTH 96

#define PRINTRESULT 0

struct nullifyOneCrucialBit
{
	int IVIndex;
	int stateIndex;
	bool isNFSR;
};

class Grain128MilpDivisionModel
{
public:
	GRBModel * milpModel;
	vector<DP_Structure> Sreg,Breg;
	vector<DP_Structure> IVdivision;
	vector<DP_Structure> Keydivision;

	set<int> S_Nullified;
	set<int> B_Nullified;

	int startRound;
	int endRound;
	set<int> K0;//key bits with 0 value
	vector<nullifyOneCrucialBit> NullificationStrategy;


	Grain128MilpDivisionModel(GRBEnv * milpEnv)
	{
		milpModel = new GRBModel(*milpEnv);
		Sreg.clear();
		Breg.clear();
		IVdivision.clear();
		Keydivision.clear();
		S_Nullified.clear();
		B_Nullified.clear();
		K0.clear();
		startRound = 0;
		endRound = 0;
		NullificationStrategy.clear();
	}


	~Grain128MilpDivisionModel()
	{
		delete milpModel;
	}


	void setStartRound(int strd)
	{
		startRound = strd;
	}

	void setEndRound(int endrd)
	{
		endRound = endrd;
	}

	void initIVwithCubeAndIVconst(set<int> Cube, vector<uint32> IVconst = {})
	{
		if (IVconst.empty() == true)//IVconst==NULL
		{
			for (int i = 0; i < IVLENGTH; ++i)
			{
				IVdivision.push_back(DP_Structure(*milpModel, _delta_Flag));
				if (Cube.find(i) != Cube.end())
				{
					milpModel->addConstr(IVdivision[i].val == 1);
				}
				else
				{
					milpModel->addConstr(IVdivision[i].val == 0);
				}

			}
		}
		else
		{
			for (int i = 0; i < IVLENGTH; ++i)
			{
				if (Cube.find(i) != Cube.end())
				{
					IVdivision.push_back(DP_Structure(*milpModel, _delta_Flag));
					milpModel->addConstr(IVdivision[i].val == 1);
				}
				else
				{
					if (bitW32(IVconst, i) == 0)
					{
						IVdivision.push_back(DP_Structure(*milpModel, _0c_Flag));
					}
					else
					{
						IVdivision.push_back(DP_Structure(*milpModel, _1c_Flag));
					}
				}
			}
		}

	}

	void initIVwithFreeIVsAndIVconst(set<int> FreeIVs, vector<uint32> IVconst = {})
	{
		GRBLinExpr VariableSUM = 0;
		if (IVconst.empty() == true)//IVconst==NULL
		{
			for (int i = 0; i < IVLENGTH; ++i)
			{
				IVdivision.push_back(DP_Structure(*milpModel, _delta_Flag));
				if (FreeIVs.find(i) != FreeIVs.end())
				{
					VariableSUM += IVdivision[i].val;
				}
				else
				{
					milpModel->addConstr(IVdivision[i].val == 0);
				}

			}
		}
		else
		{
			for (int i = 0; i < IVLENGTH; ++i)
			{
				if (FreeIVs.find(i) != FreeIVs.end())
				{
					IVdivision.push_back(DP_Structure(*milpModel, _delta_Flag));
					VariableSUM += IVdivision[i].val;
				}
				else
				{
					if (bitW32(IVconst, i) == 0)
					{
						IVdivision.push_back(DP_Structure(*milpModel, _0c_Flag));
					}
					else
					{
						IVdivision.push_back(DP_Structure(*milpModel, _1c_Flag));
					}
				}
			}
		}
		//return VariableSUM;
	}

	void initKeyWithK0()
	{
		if (K0.empty() == true)
		{
			for (int i = 0; i < KEYLENGTH; ++i)
			{
				Keydivision.push_back(DP_Structure(*milpModel, _delta_Flag));
			}
		}
		else
		{
			for (int i = 0; i < KEYLENGTH; ++i)
			{
				if (K0.find(i) == K0.end())
				{
					Keydivision.push_back(DP_Structure(*milpModel, _delta_Flag));
				}
				else
				{
					Keydivision.push_back(DP_Structure(*milpModel, _0c_Flag));
				}
			}
		}
	}

	void setK0(set<int> inK0)
	{
		K0 = inK0;
	}

	void setInitStateWithKeyIV()
	{
		Breg.clear();
		Sreg.clear();
		for (int i = 0; i < 128; ++i)
		{
			Breg.push_back(Keydivision[i]);
			if (i < IVLENGTH)Sreg.push_back(IVdivision[i]);
			else
			{
				Sreg.push_back(DP_Structure(*milpModel, _1c_Flag));
			}
		}
	}

	void setKeyIV_Dynamic()
	{
		/*
		#v3->b131
		#v9->b137
		#v14->b142
		#v30,v90->b158
		*/
		for (int i = 0; i < NullificationStrategy.size(); ++i)
			setOneDynamicIV(NullificationStrategy[i]);
	/*	vector<DP_Structure> toBeSum;
		if (B_Nullified.find(131) != B_Nullified.end() && IVdivision[3].F==_0c_Flag)
		{
			vector<vector<int>> IV3 = { { 3 },{ 5 },{ 6, 70 },{ 14, 16 },{ 15, 98 },{ 15, 139 },{ 18 },{ 20, 21 },{ 29 },
			{ 30, 62 },{ 39 },{ 43, 51 },{ 48 },{ 59 },{ 64, 68 },{ 67 },{ 71, 87 },{ 76 },{ 92 },{ 94 },{ 98, 173 },{ 99 },
			{ 144, 151 },{ 191, 210 },{ -1 } };
			for (int i = 0; i < IV3.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV3[i]));
			IVdivision[3] = Sum(*milpModel, toBeSum);
		}
		else if (B_Nullified.find(137) != B_Nullified.end() && IVdivision[9].F==_0c_Flag)
		{
			vector<vector<int>> IV9 = { { 9 },{ 11 },{ 12, 76 },{ 20, 22 },{ 21, 104 },{ 21, 145 },{ 24 },{ 26, 27 },{ 35 },
			{ 36, 68 },{ 45 },{ 49, 57 },{ 54 },{ 65 },{ 70, 74 },{ 73 },{ 77, 93 },{ 82 },{ 98 },{ 100 },{ 104, 179 },{ 105 },
			{ 150, 157 },{ 197, 216 },{ -1 } };
			for (int i = 0; i < IV9.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV9[i]));
			IVdivision[9] = Sum(*milpModel, toBeSum);
		}
		else if (B_Nullified.find(142) != B_Nullified.end() && IVdivision[14].F==_0c_Flag)
		{
			vector<vector<int>> IV14 = { { 14 },{ 16 },{ 17, 81 },{ 25, 27 },{ 26, 109 },{ 26, 150 },{ 29 },{ 31, 32 },
			{ 40 },{ 41, 73 },{ 50 },{ 54, 62 },{ 59 },{ 70 },{ 75, 79 },{ 78 },{ 82, 98 },{ 87 },{ 103 },{ 105 },{ 109, 184 },
			{ 110 },{ 155, 162 },{ 202, 221 },{ -1 } };

			for (int i = 0; i < IV14.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV14[i]));
			IVdivision[14] = Sum(*milpModel, toBeSum);
		}
		else if (B_Nullified.find(158) != B_Nullified.end() && IVdivision[30].F==_0c_Flag)
		{
			vector<vector<int>> IV30 = { { 30 },{ 32 },{ 33, 97 },{ 41, 43 },{ 42, 125 },{ 42, 166 },{ 45 },{ 47, 48 },{ 56 },
			{ 57, 89 },{ 66 },{ 70, 78 },{ 75 },{ 86 },{ 91, 95 },{ 94 },{ 98, 114 },{ 103 },{ 119 },{ 121 },{ 125, 200 },
			{ 126 },{ 171, 178 },{ 218 },{ -1 } };

			for (int i = 0; i < IV30.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV30[i]));
			IVdivision[30] = Sum(*milpModel, toBeSum);
		}
		else if (B_Nullified.find(158) != B_Nullified.end() && IVdivision[90].F == _0c_Flag)
		{
			vector<vector<int>> IV90 = { { 30 },{ 32 },{ 33, 97 },{ 41, 43 },{ 42, 125 },{ 42, 166 },{ 45 },{ 47, 48 },{ 56 },{ 57, 89 },{ 66 },{ 70, 78 },{ 75 },{ 86 },{ 91, 95 },{ 94 },{ 98, 114 },{ 103 },{ 119 },{ 121 },{ 125, 200 },{ 126 },{ 158 },{ 171, 178 },{ -1 } };
			for (int i = 0; i < IV90.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV90[i]));
			IVdivision[90] = Sum(*milpModel, toBeSum);
		}
		//[[30], [32], [33, 97], [41, 43], [42, 125], [42, 166], [45], [47, 48], [56], [57, 89], [66], [70, 78], [75], [86], [91, 95], [94], [98, 114], [103], [119], [121], [125, 200], [126], [171, 178], [218], [-1]]
		*/
	}

	void setKeyIV_DynamicWrong()
	{
		/*
		#v3->b131
		#v9->b137
		#v14->b142
		#v30,v90->b158
		*/
		for (int i = 0; i < NullificationStrategy.size(); ++i)
			setOneDynamicIV_Wrong(NullificationStrategy[i]);

		/*
		vector<DP_Structure> toBeSum;
		if (B_Nullified.find(131) != B_Nullified.end() && IVdivision[3].F != _delta_Flag)
		{
			vector<vector<int>> IV3 = { { 3 },{ 5 },{ 6, 70 },{ 14, 16 },{ 15, 98 },{ 15, 139 },{ 18 },{ 20, 21 },{ 29 },
			{ 30, 62 },{ 39 },{ 43, 51 },{ 48 },{ 59 },{ 64, 68 },{ 67 },{ 71, 87 },{ 76 },{ 92 },{ 94 },{ 98, 173 },{ 99 },
			{ 144, 151 },{ 191, 210 },{ -1 } };
			for (int i = 0; i < IV3.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV3[i]));
			toBeSum.push_back(DP_Structure(*milpModel, _1c_Flag));
			IVdivision[3] = Sum(*milpModel, toBeSum);
		}
		else if (B_Nullified.find(137) != B_Nullified.end() && IVdivision[9].F != _delta_Flag)
		{
			vector<vector<int>> IV9 = { { 9 },{ 11 },{ 12, 76 },{ 20, 22 },{ 21, 104 },{ 21, 145 },{ 24 },{ 26, 27 },{ 35 },
			{ 36, 68 },{ 45 },{ 49, 57 },{ 54 },{ 65 },{ 70, 74 },{ 73 },{ 77, 93 },{ 82 },{ 98 },{ 100 },{ 104, 179 },{ 105 },
			{ 150, 157 },{ 197, 216 },{ -1 } };
			for (int i = 0; i < IV9.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV9[i]));
			toBeSum.push_back(DP_Structure(*milpModel, _0c_Flag));
			IVdivision[9] = Sum(*milpModel, toBeSum);
		}
		else if (B_Nullified.find(142) != B_Nullified.end() && IVdivision[14].F != _delta_Flag)
		{
			vector<vector<int>> IV14 = { { 14 },{ 16 },{ 17, 81 },{ 25, 27 },{ 26, 109 },{ 26, 150 },{ 29 },{ 31, 32 },
			{ 40 },{ 41, 73 },{ 50 },{ 54, 62 },{ 59 },{ 70 },{ 75, 79 },{ 78 },{ 82, 98 },{ 87 },{ 103 },{ 105 },{ 109, 184 },
			{ 110 },{ 155, 162 },{ 202, 221 },{ -1 } };

			for (int i = 0; i < IV14.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV14[i]));
			toBeSum.push_back(DP_Structure(*milpModel, _1c_Flag));
			IVdivision[14] = Sum(*milpModel, toBeSum);
		}
		else if (B_Nullified.find(158) != B_Nullified.end() && IVdivision[30].F != _delta_Flag)
		{
			vector<vector<int>> IV30 = { { 30 },{ 32 },{ 33, 97 },{ 41, 43 },{ 42, 125 },{ 42, 166 },{ 45 },{ 47, 48 },{ 56 },
			{ 57, 89 },{ 66 },{ 70, 78 },{ 75 },{ 86 },{ 91, 95 },{ 94 },{ 98, 114 },{ 103 },{ 119 },{ 121 },{ 125, 200 },
			{ 126 },{ 171, 178 },{ 218 },{ -1 } };

			for (int i = 0; i < IV30.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV30[i]));
			toBeSum.push_back(DP_Structure(*milpModel, _1c_Flag));
			IVdivision[30] = Sum(*milpModel, toBeSum);
		}
		else if (B_Nullified.find(158) != B_Nullified.end() && IVdivision[90].F !=_delta_Flag)
		{
			vector<vector<int>> IV90 = { { 30 },{ 32 },{ 33, 97 },{ 41, 43 },{ 42, 125 },{ 42, 166 },{ 45 },{ 47, 48 },{ 56 },
			{ 57, 89 },{ 66 },{ 70, 78 },{ 75 },{ 86 },{ 91, 95 },{ 94 },{ 98, 114 },{ 103 },{ 119 },{ 121 },{ 125, 200 },
			{ 126 },{ 158 },{ 171, 178 },{ -1 } };
			for (int i = 0; i < IV90.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV90[i]));
			toBeSum.push_back(DP_Structure(*milpModel, _1c_Flag));
			IVdivision[90] = Sum(*milpModel, toBeSum);
		}
		*/
	}

	void setOneDynamicIV(nullifyOneCrucialBit NullOne)
	{
		vector<DP_Structure> toBeSum;
		if (NullOne.IVIndex==90 && NullOne.stateIndex==158 && NullOne.isNFSR==true)
		{
			vector<vector<int>> IV90 = { { 30 },{ 32 },{ 33, 97 },{ 41, 43 },{ 42, 125 },{ 42, 166 },{ 45 },{ 47, 48 },{ 56 },
			{ 57, 89 },{ 66 },{ 70, 78 },{ 75 },{ 86 },{ 91, 95 },{ 94 },{ 98, 114 },{ 103 },{ 119 },{ 121 },{ 125, 200 },
			{ 126 },{ 158 },{ 171, 178 },{ -1 } };
			for (int i = 0; i < IV90.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV90[i]));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		if (NullOne.IVIndex == 30 && NullOne.stateIndex==158 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV30 = { { 30 },{ 32 },{ 33, 97 },{ 41, 43 },{ 42, 125 },{ 42, 166 },{ 45 },{ 47, 48 },{ 56 },
			{ 57, 89 },{ 66 },{ 70, 78 },{ 75 },{ 86 },{ 91, 95 },{ 94 },{ 98, 114 },{ 103 },{ 119 },{ 121 },{ 125, 200 },
			{ 126 },{ 171, 178 },{ 218 },{ -1 } };

			for (int i = 0; i < IV30.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV30[i]));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		/*
		if (NullOne.IVIndex == 8 && NullOne.stateIndex == 136 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV8 = { { 8 },{ 10 },{ 11, 75 },{ 19, 21 },{ 20, 103 },{ 20, 144 },{ 23 },{ 25, 26 },{ 34 },{ 35, 67 },
			{ 44 },{ 48, 56 },{ 53 },{ 64 },{ 69, 73 },{ 72 },{ 76, 92 },{ 81 },{ 97 },{ 99 },{ 103, 178 },{ 104 },{ 149, 156 },
			{ 196, 215 },{ -1 } };
			for (int i = 0; i < IV8.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV8[i]));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		if (NullOne.IVIndex == 10 && NullOne.stateIndex == 138 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV10 = { { 10 },{ 12 },{ 13, 77 },{ 21, 23 },{ 22, 105 },{ 22, 146 },{ 25 },{ 27, 28 },{ 36 },{ 37, 69 },{ 46 },
			{ 50, 58 },{ 55 },{ 66 },{ 71, 75 },{ 74 },{ 78, 94 },{ 83 },{ 99 },{ 101 },{ 105, 180 },{ 106 },{ 151, 158 },{ 198, 217 },
			{ -1 } };
			for (int i = 0; i < IV10.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV10[i]));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		if (NullOne.IVIndex == 18 && NullOne.stateIndex == 146 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV18 = { { 10 },{ 12 },{ 13, 77 },{ 21, 23 },{ 22, 105 },{ 22, 146 },{ 25 },{ 27, 28 },{ 36 },{ 37, 69 },{ 46 },
			{ 50, 58 },{ 55 },{ 66 },{ 71, 75 },{ 74 },{ 78, 94 },{ 83 },{ 99 },{ 101 },{ 105, 180 },{ 106 },{ 151, 158 },{ 198, 217 },
			{ -1 } };
			for (int i = 0; i < IV18.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV18[i]));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		if (NullOne.IVIndex == 78 && NullOne.stateIndex == 146 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV78 = { { 18 },{ 20 },{ 21, 85 },{ 29, 31 },{ 30, 113 },{ 30, 154 },{ 33 },{ 35, 36 },{ 44 },{ 45, 77 },{ 54 },
			{ 58, 66 },{ 63 },{ 74 },{ 79, 83 },{ 82 },{ 86, 102 },{ 91 },{ 107 },{ 109 },{ 113, 188 },{ 114 },{ 146 },{ 159, 166 },
			{ -1 } };
			for (int i = 0; i < IV78.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV78[i]));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		if (NullOne.IVIndex == 25 && NullOne.stateIndex == 153 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV25 = { { 25 },{ 27 },{ 28, 92 },{ 36, 38 },{ 37, 120 },{ 37, 161 },{ 40 },{ 42, 43 },{ 51 },
			{ 52, 84 },{ 61 },{ 65, 73 },{ 70 },{ 81 },{ 86, 90 },{ 89 },{ 93, 109 },{ 98 },{ 114 },{ 116 },{ 120, 195 },{ 121 },
			{ 166, 173 },{ 213 },{ -1 } };
			for (int i = 0; i < IV25.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV25[i]));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		if (NullOne.IVIndex == 85 && NullOne.stateIndex == 153 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV85 = { { 25 },{ 27 },{ 28, 92 },{ 36, 38 },{ 37, 120 },{ 37, 161 },{ 40 },{ 42, 43 },{ 51 },
			{ 52, 84 },{ 61 },{ 65, 73 },{ 70 },{ 81 },{ 86, 90 },{ 89 },{ 93, 109 },{ 98 },{ 114 },{ 116 },{ 120, 195 },{ 121 },{ 153 },
			{ 166, 173 },{ -1 } };
			for (int i = 0; i < IV85.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV85[i]));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		if (NullOne.IVIndex == 26 && NullOne.stateIndex == 154 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV26 = { { 26 },{ 28 },{ 29, 93 },{ 37, 39 },{ 38, 121 },{ 38, 162 },{ 41 },{ 43, 44 },{ 52 },
			{ 53, 85 },{ 62 },{ 66, 74 },{ 71 },{ 82 },{ 87, 91 },{ 90 },{ 94, 110 },{ 99 },{ 115 },{ 117 },{ 121, 196 },{ 122 },
			{ 167, 174 },{ 214 },{ -1 } };
			for (int i = 0; i < IV26.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV26[i]));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		if (NullOne.IVIndex == 86 && NullOne.stateIndex == 154 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV86={ { 26 },{ 28 },{ 29, 93 },{ 37, 39 },{ 38, 121 },{ 38, 162 },{ 41 },{ 43, 44 },{ 52 },{ 53, 85 },
			{ 62 },{ 66, 74 },{ 71 },{ 82 },{ 87, 91 },{ 90 },{ 94, 110 },{ 99 },{ 115 },{ 117 },{ 121, 196 },{ 122 },{ 154 },{ 167, 174 },
			{ -1 } };
			for (int i = 0; i < IV86.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV86[i]));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		*/
	}

	void setOneDynamicIV_Wrong(nullifyOneCrucialBit NullOne)
	{
		vector<DP_Structure> toBeSum;
		if (NullOne.IVIndex == 90 && NullOne.stateIndex==158 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV90 = { { 30 },{ 32 },{ 33, 97 },{ 41, 43 },{ 42, 125 },{ 42, 166 },{ 45 },{ 47, 48 },{ 56 },
			{ 57, 89 },{ 66 },{ 70, 78 },{ 75 },{ 86 },{ 91, 95 },{ 94 },{ 98, 114 },{ 103 },{ 119 },{ 121 },{ 125, 200 },
			{ 126 },{ 158 },{ 171, 178 },{ -1 } };
			for (int i = 0; i < IV90.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV90[i]));
			toBeSum.push_back(DP_Structure(*milpModel, _1c_Flag));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		if (NullOne.IVIndex == 30 && NullOne.stateIndex==158 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV30 = { { 30 },{ 32 },{ 33, 97 },{ 41, 43 },{ 42, 125 },{ 42, 166 },{ 45 },{ 47, 48 },{ 56 },
			{ 57, 89 },{ 66 },{ 70, 78 },{ 75 },{ 86 },{ 91, 95 },{ 94 },{ 98, 114 },{ 103 },{ 119 },{ 121 },{ 125, 200 },
			{ 126 },{ 171, 178 },{ 218 },{ -1 } };

			for (int i = 0; i < IV30.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV30[i]));
			toBeSum.push_back(DP_Structure(*milpModel, _1c_Flag));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		/*
		if (NullOne.IVIndex == 8 && NullOne.stateIndex == 136 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV8 = { { 8 },{ 10 },{ 11, 75 },{ 19, 21 },{ 20, 103 },{ 20, 144 },{ 23 },{ 25, 26 },{ 34 },{ 35, 67 },
			{ 44 },{ 48, 56 },{ 53 },{ 64 },{ 69, 73 },{ 72 },{ 76, 92 },{ 81 },{ 97 },{ 99 },{ 103, 178 },{ 104 },{ 149, 156 },
			{ 196, 215 },{ -1 } };
			for (int i = 0; i < IV8.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV8[i]));
			toBeSum.push_back(DP_Structure(*milpModel, _1c_Flag));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		if (NullOne.IVIndex == 10 && NullOne.stateIndex == 138 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV10 = { { 10 },{ 12 },{ 13, 77 },{ 21, 23 },{ 22, 105 },{ 22, 146 },{ 25 },{ 27, 28 },{ 36 },{ 37, 69 },{ 46 },
			{ 50, 58 },{ 55 },{ 66 },{ 71, 75 },{ 74 },{ 78, 94 },{ 83 },{ 99 },{ 101 },{ 105, 180 },{ 106 },{ 151, 158 },{ 198, 217 },
			{ -1 } };
			for (int i = 0; i < IV10.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV10[i]));
			toBeSum.push_back(DP_Structure(*milpModel, _1c_Flag));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		if (NullOne.IVIndex == 18 && NullOne.stateIndex == 146 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV18 = { { 10 },{ 12 },{ 13, 77 },{ 21, 23 },{ 22, 105 },{ 22, 146 },{ 25 },{ 27, 28 },{ 36 },{ 37, 69 },{ 46 },
			{ 50, 58 },{ 55 },{ 66 },{ 71, 75 },{ 74 },{ 78, 94 },{ 83 },{ 99 },{ 101 },{ 105, 180 },{ 106 },{ 151, 158 },{ 198, 217 },
			{ -1 } };
			for (int i = 0; i < IV18.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV18[i]));
			toBeSum.push_back(DP_Structure(*milpModel, _1c_Flag));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		if (NullOne.IVIndex == 78 && NullOne.stateIndex == 146 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV78 = { { 18 },{ 20 },{ 21, 85 },{ 29, 31 },{ 30, 113 },{ 30, 154 },{ 33 },{ 35, 36 },{ 44 },{ 45, 77 },{ 54 },
			{ 58, 66 },{ 63 },{ 74 },{ 79, 83 },{ 82 },{ 86, 102 },{ 91 },{ 107 },{ 109 },{ 113, 188 },{ 114 },{ 146 },{ 159, 166 },
			{ -1 } };
			for (int i = 0; i < IV78.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV78[i]));
			toBeSum.push_back(DP_Structure(*milpModel, _1c_Flag));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		if (NullOne.IVIndex == 25 && NullOne.stateIndex == 153 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV25 = { { 25 },{ 27 },{ 28, 92 },{ 36, 38 },{ 37, 120 },{ 37, 161 },{ 40 },{ 42, 43 },{ 51 },
			{ 52, 84 },{ 61 },{ 65, 73 },{ 70 },{ 81 },{ 86, 90 },{ 89 },{ 93, 109 },{ 98 },{ 114 },{ 116 },{ 120, 195 },{ 121 },
			{ 166, 173 },{ 213 },{ -1 } };
			for (int i = 0; i < IV25.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV25[i]));
			toBeSum.push_back(DP_Structure(*milpModel, _1c_Flag));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		if (NullOne.IVIndex == 85 && NullOne.stateIndex == 153 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV85 = { { 25 },{ 27 },{ 28, 92 },{ 36, 38 },{ 37, 120 },{ 37, 161 },{ 40 },{ 42, 43 },{ 51 },
			{ 52, 84 },{ 61 },{ 65, 73 },{ 70 },{ 81 },{ 86, 90 },{ 89 },{ 93, 109 },{ 98 },{ 114 },{ 116 },{ 120, 195 },{ 121 },{ 153 },
			{ 166, 173 },{ -1 } };
			for (int i = 0; i < IV85.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV85[i]));
			toBeSum.push_back(DP_Structure(*milpModel, _1c_Flag));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		if (NullOne.IVIndex == 26 && NullOne.stateIndex == 154 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV26 = { { 26 },{ 28 },{ 29, 93 },{ 37, 39 },{ 38, 121 },{ 38, 162 },{ 41 },{ 43, 44 },{ 52 },
			{ 53, 85 },{ 62 },{ 66, 74 },{ 71 },{ 82 },{ 87, 91 },{ 90 },{ 94, 110 },{ 99 },{ 115 },{ 117 },{ 121, 196 },{ 122 },
			{ 167, 174 },{ 214 },{ -1 } };
			for (int i = 0; i < IV26.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV26[i]));
			toBeSum.push_back(DP_Structure(*milpModel, _1c_Flag));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		if (NullOne.IVIndex == 86 && NullOne.stateIndex == 154 && NullOne.isNFSR == true)
		{
			vector<vector<int>> IV86 = { { 26 },{ 28 },{ 29, 93 },{ 37, 39 },{ 38, 121 },{ 38, 162 },{ 41 },{ 43, 44 },{ 52 },{ 53, 85 },
			{ 62 },{ 66, 74 },{ 71 },{ 82 },{ 87, 91 },{ 90 },{ 94, 110 },{ 99 },{ 115 },{ 117 },{ 121, 196 },{ 122 },{ 154 },{ 167, 174 },
			{ -1 } };
			for (int i = 0; i < IV86.size(); ++i)
				toBeSum.push_back(COPY_AND_Index(IV86[i]));
			toBeSum.push_back(DP_Structure(*milpModel, _1c_Flag));
			IVdivision[NullOne.IVIndex] = Sum(*milpModel, toBeSum);
		}
		*/
	}



	void setInitStateWith256Variables()
	{
		for (int i = 0; i < 128; ++i)
		{
			Sreg.push_back(DP_Structure(*milpModel, _delta_Flag));
			Breg.push_back(DP_Structure(*milpModel, _delta_Flag));
		}
	}

	void addOneDynamicIV(nullifyOneCrucialBit nullOne)
	{
		NullificationStrategy.push_back(nullOne);
		if (nullOne.isNFSR == true)B_Nullified.insert(nullOne.stateIndex);
		else S_Nullified.insert(nullOne.stateIndex);
	}


	void UpdateModelByRounds()
	{
		int iniRound = endRound - startRound;
		for (int r = 0; r < endRound-startRound; ++r)
		{
			UpdateInternalState(r);
		}

	}

	void UpdateModelByRounds_DynamicWrong()
	{
		int iniRound = endRound - startRound;
		for (int r = 0; r < endRound - startRound; ++r)
		{
			UpdateInternalState_DynamicWrong(r);
		}

	}

	DP_Structure Zoutput(int Round)
	{
		FlagValue FinalWheCon(0, 0);
		vector<int> Bindex = { 2, 15, 36, 45, 64, 73, 89 };
		vector<int> Sindex = { 93 };
		vector<DP_Structure> ToBeSum;
		for (int i = 0; i < Bindex.size(); ++i)
		{
			FinalWheCon += Breg[Round + Bindex[i]].F;
			if (Breg[Round + Bindex[i]].F.isDelta == 1)
			{
				vector<DP_Structure> CanSum = COPY(*milpModel, Breg[Round + Bindex[i]], 2);
				Breg[Round + Bindex[i]] = CanSum[0];
				ToBeSum.push_back(CanSum[1]);
			}
		}

		for (int i = 0; i < Sindex.size(); ++i)
		{
			FinalWheCon += Sreg[Round + Sindex[i]].F;
			if (Sreg[Round + Sindex[i]].F.isDelta == 1)
			{
				vector<DP_Structure> CanSum = COPY(*milpModel, Sreg[Round + Sindex[i]], 2);
				Sreg[Round + Sindex[i]] = CanSum[0];
				ToBeSum.push_back(CanSum[1]);
			}
		}

		DP_Structure b12s8 = BS_Multi({ 12 }, { 8 }, Round);
		if (b12s8.F.isDelta == 1)ToBeSum.push_back(b12s8);
		FinalWheCon += b12s8.F;



		DP_Structure s13s20 = BS_Multi({}, { 13, 20 }, Round);
		if (s13s20.F.isDelta == 1)ToBeSum.push_back(s13s20);
		FinalWheCon += s13s20.F;



		DP_Structure b95s42 = BS_Multi({ 95 }, { 42 }, Round);
		if (b95s42.F.isDelta == 1)ToBeSum.push_back(b95s42);
		FinalWheCon += b95s42.F;



		DP_Structure s60s79 = BS_Multi({}, { 60, 79 }, Round);
		if (s60s79.F.isDelta == 1)ToBeSum.push_back(s60s79);
		FinalWheCon += s60s79.F;

		DP_Structure b12b95s95 = BS_Multi({ 12, 95 }, { 95 }, Round);
		if (b12b95s95.F.isDelta == 1)ToBeSum.push_back(b12b95s95);
		FinalWheCon += b12b95s95.F;

		if (FinalWheCon.isDelta == 1)
		{
			DP_Structure res = Sum(*milpModel, ToBeSum);
			return res;
		}
		else
		{
			DP_Structure res(*milpModel, FinalWheCon);
			return res;
		}


	}


private:
	void UpdateInternalState(int r)
	{
		vector<DP_Structure> sToBeSum, bToBeSum;
		if (B_Nullified.find(r + 128) == B_Nullified.end() && S_Nullified.find(r + 128) == S_Nullified.end())
		{
			//zr
			DP_Structure zr = Zoutput(r);
			vector<DP_Structure> zrVec = COPY(*milpModel, zr, 2);
			bToBeSum.push_back(zrVec[0]);
			sToBeSum.push_back(zrVec[1]);

			//s0
			vector<DP_Structure> S0vec = COPY(*milpModel, Sreg[r], 2);
			sToBeSum.push_back(S0vec[0]);
			bToBeSum.push_back(S0vec[1]);

			//b', s'
			DP_Structure bsub = NFSR128(r);
			DP_Structure ssub = LFSR128(r);
			bToBeSum.push_back(bsub);
			sToBeSum.push_back(ssub);

			//Summation
			Breg.push_back(Sum(*milpModel, bToBeSum));
			Sreg.push_back(Sum(*milpModel, sToBeSum));
		}
		else if (B_Nullified.find(r + 128) == B_Nullified.end() && S_Nullified.find(r + 128) != S_Nullified.end())
		{
			//zr
			DP_Structure zr = Zoutput(r);

			//b'
			DP_Structure bsub = NFSR128(r);
			//s[r+128]=0
			Sreg.push_back(DP_Structure(*milpModel, _0c_Flag));
			//b[r+128]=b'+s[r]+z[r]
			bToBeSum = { zr,bsub,Sreg[r] };
			Breg.push_back(Sum(*milpModel, bToBeSum));
		}
		else if (B_Nullified.find(r + 128) != B_Nullified.end() && S_Nullified.find(r + 128) == S_Nullified.end())
		{
			//zr
			DP_Structure zr = Zoutput(r);
			//s'
			DP_Structure ssub = LFSR128(r);
			//s[r+128]=s'+s[r]+z[r]
			sToBeSum = { zr,ssub,Sreg[r] };
			Sreg.push_back(Sum(*milpModel, sToBeSum));
			//b[r+128]=0
			Breg.push_back(DP_Structure(*milpModel, _0c_Flag));
		}
		else
		{
			Sreg.push_back(DP_Structure(*milpModel, _0c_Flag));
			Breg.push_back(DP_Structure(*milpModel, _0c_Flag));
			if (Sreg[r].F.isDelta == 1)milpModel->addConstr(Sreg[r].val == 0);
			if (Breg[r].F.isDelta == 1)milpModel->addConstr(Breg[r].val == 0);
		}
	}


	void UpdateInternalState_DynamicWrong(int r)
	{
		vector<DP_Structure> sToBeSum, bToBeSum;
		if (B_Nullified.find(r + 128) == B_Nullified.end() && S_Nullified.find(r + 128) == S_Nullified.end())
		{
			//zr
			DP_Structure zr = Zoutput(r);
			vector<DP_Structure> zrVec = COPY(*milpModel, zr, 2);
			bToBeSum.push_back(zrVec[0]);
			sToBeSum.push_back(zrVec[1]);

			//s0
			vector<DP_Structure> S0vec = COPY(*milpModel, Sreg[r], 2);
			sToBeSum.push_back(S0vec[0]);
			bToBeSum.push_back(S0vec[1]);

			//b', s'
			DP_Structure bsub = NFSR128(r);
			DP_Structure ssub = LFSR128(r);
			bToBeSum.push_back(bsub);
			sToBeSum.push_back(ssub);

			//Summation
			Breg.push_back(Sum(*milpModel, bToBeSum));
			Sreg.push_back(Sum(*milpModel, sToBeSum));
		}
		else if (B_Nullified.find(r + 128) == B_Nullified.end() && S_Nullified.find(r + 128) != S_Nullified.end())
		{
			//zr
			DP_Structure zr = Zoutput(r);

			//b'
			DP_Structure bsub = NFSR128(r);
			//s[r+128]=0
			Sreg.push_back(DP_Structure(*milpModel, _1c_Flag));
			//b[r+128]=b'+s[r]+z[r]
			bToBeSum = { zr,bsub,Sreg[r] };
			Breg.push_back(Sum(*milpModel, bToBeSum));
		}
		else if (B_Nullified.find(r + 128) != B_Nullified.end() && S_Nullified.find(r + 128) == S_Nullified.end())
		{
			//zr
			DP_Structure zr = Zoutput(r);
			//s'
			DP_Structure ssub = LFSR128(r);
			//s[r+128]=s'+s[r]+z[r]
			sToBeSum = { zr,ssub,Sreg[r] };
			Sreg.push_back(Sum(*milpModel, sToBeSum));
			//b[r+128]=0
			Breg.push_back(DP_Structure(*milpModel, _1c_Flag));
		}
		else
		{
			Sreg.push_back(DP_Structure(*milpModel, _1c_Flag));
			Breg.push_back(DP_Structure(*milpModel, _1c_Flag));
			if (Sreg[r].F.isDelta == 1)milpModel->addConstr(Sreg[r].val == 0);
			if (Breg[r].F.isDelta == 1)milpModel->addConstr(Breg[r].val == 0);
		}
	}

	DP_Structure B_Multi(vector<int> Bindex, int Round)
	{
		FlagValue BMultiWheCon(0, 1);
		FlagValue MultiWheCon(0, 1);
		vector<DP_Structure> ToBeMulti;
		for (int i = 0; i < Bindex.size(); ++i)
		{
			MultiWheCon *= Breg[Bindex[i] + Round].F;
		}

		if (MultiWheCon.isDelta == 0)
		{
			DP_Structure res = DP_Structure(*milpModel, MultiWheCon);
			return res;
		}

		for (int i = 0; i < Bindex.size(); ++i)
		{
			if (Breg[Bindex[i] + Round].F.isDelta == 1)
			{
				vector<DP_Structure> bcopy = COPY(*milpModel, Breg[Bindex[i] + Round], 2);
				Breg[Round + Bindex[i]] = bcopy[0];
				ToBeMulti.push_back(bcopy[1]);
			}
		}

		DP_Structure res = Multi(*milpModel, ToBeMulti);
		return res;


	}


	DP_Structure BS_Multi(vector<int> Bindex, vector<int> Sindex, int Round)
	{
		FlagValue SMultiWheCon(0, 1);
		FlagValue BMultiWheCon(0, 1);
		FlagValue MultiWheCon(0, 1);
		vector<DP_Structure> ToBeMulti;
		for (int i = 0; i < Sindex.size(); ++i)
		{
			MultiWheCon *= Sreg[Sindex[i] + Round].F;
		}
		for (int i = 0; i < Bindex.size(); ++i)
		{
			MultiWheCon *= Breg[Bindex[i] + Round].F;
		}
		if (MultiWheCon.isDelta == 0)
		{
			DP_Structure res = DP_Structure(*milpModel, MultiWheCon);
			return res;
		}

		for (int i = 0; i < Sindex.size(); ++i)
		{
			if (Sreg[Sindex[i] + Round].F.isDelta == 1)
			{
				vector<DP_Structure> scopy = COPY(*milpModel, Sreg[Sindex[i] + Round], 2);
				Sreg[Round + Sindex[i]] = scopy[0];
				ToBeMulti.push_back(scopy[1]);
			}
		}

		for (int i = 0; i < Bindex.size(); ++i)
		{
			if (Breg[Bindex[i] + Round].F.isDelta == 1)
			{
				vector<DP_Structure> bcopy = COPY(*milpModel, Breg[Bindex[i] + Round], 2);
				Breg[Round + Bindex[i]] = bcopy[0];
				ToBeMulti.push_back(bcopy[1]);
			}
		}
		DP_Structure res = Multi(*milpModel, ToBeMulti);
		return res;

	}


	// LFSR update. Svec will expand by 1 variable
	//\sum    S_reg{96, 81, 70, 38, 7}
	//S0 has not been added
	DP_Structure LFSR128(int Round)
	{
		vector<int> LFSRindx = { 96, 81, 70, 38, 7 };
		vector<DP_Structure> ToBeSum;
		FlagValue FinalWheCon = FlagValue(0, 0);
		//DP_Structure Lsub = DP_Structure(model, FinalWheCon);
		for (int i = 0; i < LFSRindx.size(); i++)
		{
			FinalWheCon += Sreg[Round + LFSRindx[i]].F;
			if (Sreg[Round + LFSRindx[i]].F.isDelta == 1)
			{
				vector<DP_Structure> CanSum = COPY(*milpModel, Sreg[Round + LFSRindx[i]], 2);
				Sreg[Round + LFSRindx[i]] = CanSum[0];
				ToBeSum.push_back(CanSum[1]);
			}
		}
		/*
		if (Svec[Round].F.isDelta == 1)
		{
		ToBeSum.push_back(Svec[Round]);
		}
		*/



		if (FinalWheCon.isDelta == 0)
		{
			DP_Structure Lsub = DP_Structure(*milpModel, FinalWheCon);
			return Lsub;
		}
		else
		{
			DP_Structure Lsub = Sum(*milpModel, ToBeSum);
			return Lsub;
		}
	}


	//NFSR update. Bvec will expand by 1 variable--B[r+128]
	//S0 has not been added
	DP_Structure NFSR128(int Round)
	{
		FlagValue FinalWheCon(0, 0);
		vector<DP_Structure> ToBeSum;


		//b0 added without COPY
		if (Breg[Round].F.isDelta == 1)ToBeSum.push_back(Breg[Round]);


		//Other b bits are added with COPY
		vector<int> Bindex = { 26, 56, 91, 96 };
		for (int i = 0; i < Bindex.size(); ++i)
		{
			FinalWheCon += Breg[Round + Bindex[i]].F;
			if (Breg[Round + Bindex[i]].F.isDelta == 1)
			{
				vector<DP_Structure> CanSum = COPY(*milpModel, Breg[Round + Bindex[i]], 2);
				Breg[Round + Bindex[i]] = CanSum[0];
				ToBeSum.push_back(CanSum[1]);
			}
		}


		vector<vector<int>> BMultiIndex = {
			{ 3,67 },
			{ 11,13 },
			{ 17,18 },
			{ 27,59 },
			{ 40,48 },
			{ 61,65 },
			{ 68,84 }
		};
		for (int term = 0; term < BMultiIndex.size(); term++)
		{
			DP_Structure Multires = B_Multi(BMultiIndex[term], Round);
			FinalWheCon += Multires.F;
			//DP_Structure MultiRes(model, MultiWheCon);
			if (Multires.F.isDelta == 1)
			{
				ToBeSum.push_back(Multires);
			}
		}

		if (FinalWheCon.isDelta == 1)
		{
			DP_Structure bsub = Sum(*milpModel, ToBeSum);
			return bsub;
		}
		else
		{
			DP_Structure bsub = DP_Structure(*milpModel, FinalWheCon);
			return bsub;
		}
	}



	DP_Structure COPY_AND_Index(vector<int> index)
	{
		if (index.empty() == true)return DP_Structure(*milpModel, _0c_Flag);
		if (index[0] == -1)return DP_Structure(*milpModel, _1c_Flag);
		if (index[0] == -2)return DP_Structure(*milpModel, _0c_Flag);


		FlagValue wres(0, 1);
		for (int i = 0; i < index.size(); ++i)
		{
			if (index[i] < 128)wres *= Keydivision[index[i]].F;
			else wres *= IVdivision[index[i] - 128].F;
		}
		if (wres.isDelta == 0)return DP_Structure(*milpModel, wres);


		vector<DP_Structure> ToBeMulti;
		for (int i = 0; i < index.size(); ++i)
		{
			if (index[i]<128 && Keydivision[index[i]].F.isDelta == 1)
			{
				vector<DP_Structure> cp = COPY(*milpModel, Keydivision[index[i]], 2);
				Keydivision[index[i]] = cp[0];
				ToBeMulti.push_back(cp[1]);
			}
			else if (index[i] >= 128 && IVdivision[index[i] - 128].F.isDelta == 1)
			{
				vector<DP_Structure> cp = COPY(*milpModel, IVdivision[index[i] - 128], 2);
				IVdivision[index[i] - 128] = cp[0];
				ToBeMulti.push_back(cp[1]);
			}
		}
		if (ToBeMulti.size() == 1)return ToBeMulti[0];

		DP_Structure resultt(*milpModel, wres);
		for (int i = 0; i < ToBeMulti.size(); ++i)
		{
			milpModel->addConstr(resultt.val >= ToBeMulti[i].val);
		}
		return resultt;
	}



};






class Grain128DifferentDegrees
{
public:
	GRBEnv * milpEnvironment;
	Grain128MilpDivisionModel * Model;
	set<int> Cube;
	set<int> K0;
	vector<uint32> IVconst;
	int Degree;
	int outputRound;
	vector<nullifyOneCrucialBit> NullificationStrategy;

	Grain128DifferentDegrees(set<int> setCube, vector<uint32> setIVconst = {}, set<int> inK0 = {})
	{
		// Create the environment
		milpEnvironment = new GRBEnv();
		// close standard output
		milpEnvironment->set(GRB_IntParam_OutputFlag, 0);
		milpEnvironment->set(GRB_IntParam_MIPFocus, GRB_MIPFOCUS_BESTBOUND);
		milpEnvironment->set(GRB_IntParam_VarBranch, GRB_VARBRANCH_PSEUDO_SHADOW);
		milpEnvironment->set(GRB_IntParam_LazyConstraints, 1);
		// number of cores
		milpEnvironment->set(GRB_IntParam_Threads, CORENUM);
		Cube = setCube;
		IVconst = setIVconst;
		Model = NULL;
		K0 = inK0;
		Degree = -1;
		outputRound = -1;
		NullificationStrategy.clear();
	}

	~Grain128DifferentDegrees()
	{
		delete Model;
		delete milpEnvironment;
	}

	void setNullificationStrategy(vector<nullifyOneCrucialBit> NullStrategy)
	{
		NullificationStrategy = NullStrategy;
	}

	void setK0ForModel(set<int> inK0)
	{
		K0 = inK0;
	}

	void setOutputRound(int setOutputRound)
	{
		outputRound = setOutputRound;
	}

	int getSPolyDegreeOfZ(std::ostream& o = std::cout)
	{
		if (outputRound == -1)
		{
			throw "The output round is unset!!!\n";
		}
		try
		{
			setNewModelForSuperpoly();
			GRBLinExpr KeySUM = 0;
			for (int i = 0; i < KEYLENGTH; ++i)
			{
				if (Model->Keydivision[i].F == _delta_Flag)
				{
					KeySUM += Model->Keydivision[i].val;
				}
			}
			Model->milpModel->addConstr(KeySUM >= 0);
			int iniRound = outputRound%32==0 ? outputRound : outputRound-(outputRound%32);
			DP_Structure z = setModelForOutputZ(iniRound);
			Model->milpModel->setObjective(KeySUM, GRB_MAXIMIZE);
			if (z.F == _0c_Flag && z.F == _1c_Flag)
			{
				Degree = 0;
			}
			else
			{
				Model->milpModel->optimize();
				if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
					int ResDeg = 0;
					for (int i = 0; i < KEYLENGTH; ++i)
					{
						if (Model->Keydivision[i].F == _delta_Flag && Model->Keydivision[i].val.get(GRB_DoubleAttr_X)>0.5)
						{
							++ResDeg;
						}
					}
#if PRINTRESULT
					o << dec << "Cube: ";
					for (set<int>::iterator ite = Cube.begin(); ite != Cube.end(); ++ite)
					{
						o << *ite<< ",";
					}
					o << endl;

					o << "Nullification:";
					for (int i = 0; i < NullificationStrategy.size(); ++i)
					{
						o << NullificationStrategy[i].IVIndex << "->";
						if (NullificationStrategy[i].isNFSR == true)o << "b" << NullificationStrategy[i].stateIndex << ",";
						else o << "s" << NullificationStrategy[i].stateIndex << ",";
					}
					o << endl;

					for (int i = 0; i < KEYLENGTH; ++i)
					{
						if (Model->Keydivision[i].F == _delta_Flag && Model->Keydivision[i].val.get(GRB_DoubleAttr_X)>0.5)
						{
							o << "K" << i << ",";
						}
					}
					o << endl;
					o <<"Degree:"<< ResDeg << endl;
#endif
					Degree = ResDeg; //return model.get(GRB_DoubleAttr_ObjVal);
					return Degree;
				}
				else if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_INFEASIBLE)
				{
#if PRINTRESULT
					o << "Infeasible\n";
#endif
					Degree = -1;
					return Degree;
				}
				else
				{
					o << "Solving Process Break Down!\n";
					Degree = -2;
					return Degree;
				}
			}
		}
		catch (GRBException e)
		{
			Degree = -1;
			o << e.getErrorCode() << endl;
			o << e.getMessage() << endl;
			return Degree;
		}
	}

	int getSPolyDegreeOfZ_DynamicWrong(std::ostream& o = std::cout)
	{
		if (outputRound == -1)
		{
			throw "The output round is unset!!!\n";
		}
		try
		{
			setNewModelForSuperpoly();
			GRBLinExpr KeySUM = 0;
			for (int i = 0; i < KEYLENGTH; ++i)
			{
				if (Model->Keydivision[i].F == _delta_Flag)
				{
					KeySUM += Model->Keydivision[i].val;
				}
			}
			Model->milpModel->addConstr(KeySUM >= 0);
			int iniRound = outputRound % 32 == 0 ? outputRound : outputRound - (outputRound % 32);
			DP_Structure z = setModelForOutputZ_DynamicWrong(iniRound);
			Model->milpModel->setObjective(KeySUM, GRB_MAXIMIZE);
			if (z.F == _0c_Flag && z.F == _1c_Flag)
			{
				Degree = 0;
			}
			else
			{
				Model->milpModel->optimize();
				if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
					int ResDeg = 0;
					for (int i = 0; i < KEYLENGTH; ++i)
					{
						if (Model->Keydivision[i].F == _delta_Flag && Model->Keydivision[i].val.get(GRB_DoubleAttr_X)>0.5)
						{
							++ResDeg;
						}
					}
#if PRINTRESULT
					o << dec << "Cube: ";
					for (set<int>::iterator ite = Cube.begin(); ite != Cube.end(); ++ite)
					{
						o << *ite << ",";
					}
					o << endl;

					o << "Nullification:";
					for (int i = 0; i < NullificationStrategy.size(); ++i)
					{
						o << NullificationStrategy[i].IVIndex << "->";
						if (NullificationStrategy[i].isNFSR == true)o << "b" << NullificationStrategy[i].stateIndex << ",";
						else o << "s" << NullificationStrategy[i].stateIndex << ",";
					}
					o << endl;
					for (int i = 0; i < KEYLENGTH; ++i)
					{
						if (Model->Keydivision[i].F == _delta_Flag && Model->Keydivision[i].val.get(GRB_DoubleAttr_X)>0.5)
						{
							o << "K" << i << ",";
						}
					}
					o << endl;
					o << "Degree:" << ResDeg<<endl;
#endif
					Degree = ResDeg; //return model.get(GRB_DoubleAttr_ObjVal);
					return Degree;
				}
				else if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_INFEASIBLE)
				{
#if PRINTRESULT
					o << "Infeasible\n";
#endif
					Degree = -1;
					return Degree;
				}
				else
				{
					o << "Solving Process Break Down!\n";
					Degree = -1;
					return Degree;
				}
			}
		}
		catch (GRBException e)
		{
			Degree = -1;
			o << e.getErrorCode() << endl;
			o << e.getMessage() << endl;
			return Degree;
		}
	}

	set<int> getCandidateK0(set<int> initK0 = {}, std::ostream& o = std::cout)
	{
		set<int> candidateK0;
		K0 = initK0;
		int minDeg = 128;
		vector<pair<int, int>> degreeTable;
		for (int key = 0; key < KEYLENGTH; ++key)
		{
			if (K0.find(key) != K0.end())
			{
				continue;
			}
			K0.insert(key);
			getSPolyDegreeOfZ(o);
			pair<int, int> p1(key, Degree);
			degreeTable.push_back(p1);
			K0.erase(key);
			minDeg = minDeg < Degree ? minDeg : Degree;
		}
		for (int i = 0; i < degreeTable.size(); ++i)
		{
			if (degreeTable[i].second == minDeg)
				candidateK0.insert(degreeTable[i].first);
		}
		Degree=minDeg;
		return candidateK0;
	}

	void getOneK0(set<int> initK0 = {}, std::ostream & o = std::cout)
	{
		o << "Cube:";
		for (set<int>::iterator ite = Cube.begin(); ite != Cube.end(); ++ite)
		{
			o << dec << *ite << ",";
		}
		o << endl;
		K0 = initK0;
		o << "Init K0:";
		for (set<int>::iterator ite = initK0.begin(); ite != initK0.end(); ++ite)
		{
			o << dec << *ite << ",";
		}
		o << endl;

		getSPolyDegreeOfZ(o);
		o << "Init Degree:" << Degree << endl;
		while (Degree >= 0)
		{
			set<int> candidateK0 = getCandidateK0(K0, o);
			o << "Candidate of Degree " << Degree << ":";
			for (set<int>::iterator ite = candidateK0.begin(); ite != candidateK0.end(); ++ite)
			{
				o << dec << *ite << ",";
			}
			o << endl;
			int in2K0 = randPick(candidateK0);
			o << "Pick " << in2K0 << endl;
			K0.insert(in2K0);
		}
		o << "Final K0:";
		for (set<int>::iterator ite = K0.begin(); ite != K0.end(); ++ite)
		{
			o << dec << *ite << ",";
		}
		o << endl;
	}

	set<int> getCandidateK0_DynamicWrong(set<int> initK0 = {}, std::ostream& o = std::cout)
	{
		set<int> candidateK0;
		K0 = initK0;
		int minDeg = 128;
		vector<pair<int, int>> degreeTable;
		for (int key = 0; key < KEYLENGTH; ++key)
		{
			if (K0.find(key) != K0.end())
			{
				continue;
			}
			K0.insert(key);
			getSPolyDegreeOfZ_DynamicWrong(o);
			pair<int, int> p1(key, Degree);
			degreeTable.push_back(p1);
			K0.erase(key);
			minDeg = minDeg < Degree ? minDeg : Degree;
		}
		for (int i = 0; i < degreeTable.size(); ++i)
		{
			if (degreeTable[i].second == minDeg)
				candidateK0.insert(degreeTable[i].first);
		}
		Degree = minDeg;
		return candidateK0;
	}
	
	void getOneK0_DynamicWrong(set<int> initK0 = {}, std::ostream & o = std::cout)
	{
		o << "Cube:";
		for (set<int>::iterator ite = Cube.begin(); ite != Cube.end(); ++ite)
		{
			o << dec << *ite << ",";
		}
		o << endl;
		K0 = initK0;
		o << "Init K0:";
		for (set<int>::iterator ite = initK0.begin(); ite != initK0.end(); ++ite)
		{
			o << dec << *ite << ",";
		}
		o << endl;

		getSPolyDegreeOfZ_DynamicWrong(o);
		o << "Init Degree:" << Degree << endl;
		while (Degree >= 0)
		{
			set<int> candidateK0 = getCandidateK0_DynamicWrong(K0, o);
			o << "Candidate of Degree " << Degree << ":";
			for (set<int>::iterator ite = candidateK0.begin(); ite != candidateK0.end(); ++ite)
			{
				o << dec << *ite << ",";
			}
			o << endl;
			int in2K0 = randPick(candidateK0);
			o << "Pick " << in2K0 << endl;
			K0.insert(in2K0);
		}
		o << "Final K0:";
		for (set<int>::iterator ite = K0.begin(); ite != K0.end(); ++ite)
		{
			o << dec << *ite << ",";
		}
		o << endl;
	}


protected:
	void setNewModelForSuperpoly()
	{
		Model = new Grain128MilpDivisionModel(milpEnvironment);
		Model->setStartRound(0);
		Model->initIVwithCubeAndIVconst(Cube, IVconst);
		Model->setK0(K0);
		Model->initKeyWithK0();
		for (int i = 0; i < NullificationStrategy.size(); ++i)
			Model->addOneDynamicIV(NullificationStrategy[i]);
	}

	DP_Structure setModelForOutputZ(int iniRound)
	{
		Model->setEndRound(iniRound);
		if (Model->NullificationStrategy.empty()==false)
		{
			Model->setKeyIV_Dynamic();
		}
		Model->setInitStateWithKeyIV();
		Model->UpdateModelByRounds();
		DP_Structure zo = Model->Zoutput(outputRound);
		int IniRounds = Model->endRound - Model->startRound;
		for (int i = 0; i < 128; ++i)
		{
			if (Model->Sreg[IniRounds + i].F==_delta_Flag)
				Model->milpModel->addConstr(Model->Sreg[IniRounds + i].val == 0);
			if (Model->Breg[IniRounds + i].F == _delta_Flag)
				Model->milpModel->addConstr(Model->Breg[IniRounds + i].val == 0);
		}
		if (zo.F.isDelta == 1)Model->milpModel->addConstr(zo.val == 1);
		return zo;
	}

	DP_Structure setModelForOutputZ_DynamicWrong(int iniRound)
	{
		Model->setEndRound(iniRound);
		if (Model->NullificationStrategy.empty()==false)
		{
			Model->setKeyIV_DynamicWrong();
		}
		Model->setInitStateWithKeyIV();
		Model->UpdateModelByRounds_DynamicWrong();
		DP_Structure zo = Model->Zoutput(outputRound);
		int IniRounds = Model->endRound - Model->startRound;
		for (int i = 0; i < 128; ++i)
		{
			if (Model->Sreg[IniRounds + i].F == _delta_Flag)
				Model->milpModel->addConstr(Model->Sreg[IniRounds + i].val == 0);
			if (Model->Breg[IniRounds + i].F == _delta_Flag)
				Model->milpModel->addConstr(Model->Breg[IniRounds + i].val == 0);
		}
		if (zo.F.isDelta == 1)Model->milpModel->addConstr(zo.val == 1);
		return zo;
	}


	int randPick(set<int> Free)
	{
		int pos = rand() % Free.size();
		int count = 0;
		for (set<int>::iterator ite = Free.begin(); ite != Free.end(); ite++)
		{
			if (count == pos)return *ite;
			count++;
		}
	}

};




class Grain128SuperpolyInvolvedKeysOfCube :public Grain128DifferentDegrees, public GRBCallback
{
public:
	set<int> InvolvedKeys;

	Grain128SuperpolyInvolvedKeysOfCube(set<int> setCube, vector<uint32> setIVconst = {}, set<int> inK0 = {}) :Grain128DifferentDegrees(setCube, setIVconst, inK0)
	{
		InvolvedKeys.clear();
	}

	int getInvolvedKeysOfZ(std::ostream& o = std::cout)
	{
		if (outputRound == -1)
		{
			throw "The output round is unset!!!\n";
		}
		try
		{
			setNewModelForSuperpoly();
			GRBLinExpr KeySUM = 0;
			for (int i = 0; i < KEYLENGTH; ++i)
			{
				if (Model->Keydivision[i].F == _delta_Flag)
				{
					KeySUM += Model->Keydivision[i].val;
				}
			}
			Model->milpModel->addConstr(KeySUM >= 1);
			int iniRound = outputRound % 32 == 0 ? outputRound : outputRound - (outputRound % 32);
			DP_Structure z = setModelForOutputZ(iniRound);
			Model->milpModel->setObjective(KeySUM, GRB_MAXIMIZE);

			if (z.F == _0c_Flag && z.F == _1c_Flag)
			{
				o << "The output bit is constant!" << endl;
				return 0;
			}
			else
			{
				Model->milpModel->setCallback(this);
				Model->milpModel->optimize();
				if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
					generateFinalReport();
					return InvolvedKeys.size();
				}
				else if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_INFEASIBLE)
				{
					o << "Infeasible\n";
					generateFinalReport();
					return InvolvedKeys.size();
				}
				else
				{
					o << "Solving Process Break Down!\n";
					return -1;
				}
			}
		}
		catch (GRBException e)
		{
			o << e.getErrorCode() << endl;
			o << e.getMessage() << endl;
			return -1;
		}
	}


protected:
	void callback()
	{
		try
		{
			if (where == GRB_CB_MIPSOL)
			{
				//cout << "CallBack\n";
				string filename = "TmpGrain128InvovledKeysCallBack" + CubeNameSet(Cube,IVLENGTH) + ".txt";
				ofstream file1;
				int maxDeg = 0;
				for (int i = 0; i < KEYLENGTH; ++i)
				{
					if (Model->Keydivision[i].F == _delta_Flag && getSolution(Model->Keydivision[i].val) > 0.5)
					{
						maxDeg++;
						cout << "K" << i << "Insert" << endl;
						addLazy(Model->Keydivision[i].val == 0);

						InvolvedKeys.insert(i);
						file1.open(filename, ios::app);
						file1 << i << ",";
						file1.close();
					}
				}
				if (maxDeg > Degree)Degree = maxDeg;
				file1.open(filename, ios::app);
				file1 << endl;
				file1.close();
			}
		}
		catch (GRBException e)
		{
			cout << "Error No. " << e.getErrorCode() << endl;
			cout << e.getMessage() << endl;
		}
	}

	void generateFinalReport(std::ostream & o = std::cout)
	{
		o << "CubeIndx:";
		for (int i = 0; i < IVLENGTH; ++i)
		{
			if (Cube.find(i) != Cube.end())o << i << ",";
		}
		o << endl << "Round " << outputRound << endl;
		if (IVconst.size() != 0)
		{
			o << "IVconst:";
			for (int j = 0; j < IVconst.size(); ++j)
			{
				o << IVconst[j] << ",";
			}
			o << endl;
		}
		if (K0.size() != 0)
		{
			o << "K0:";
			for (set<int>::iterator ite = K0.begin(); ite != K0.end(); ++ite)
			{
				o << dec << *ite << ",";
			}
			o << dec << "\n";
		}
		o << "Keys:";
		//	RelatedKeys = *cb.RelatedKeys;
		for (int i = 0; i < KEYLENGTH; ++i)
		{
			//如果第Ki包含在最高次项中，则把它存入RelatedKeys
			if (InvolvedKeys.find(i) != InvolvedKeys.end())o << i << ",";
		}
		o << endl << "#K=" << InvolvedKeys.size() << endl;
	}

	//private:
	//	vector<DP_Structure> KeyInit;
};



#if 1
//Precisely get the terms or approximately get the key bits involved in t-degree terms.
enum TermEnumManner
{
	Precise, //J^t for some degree t
	Approximate //Jsub^t for some degree t
};
#define STORETERMS 0
class Grain128SuperpolyTermEnumerateOfCube :public Grain128DifferentDegrees, public GRBCallback
{
public:
	vector<set<int>> InvolvedKeysByDegree;
	vector<int> termNumByDegree;
	Grain128SuperpolyTermEnumerateOfCube(set<int> setCube, vector<uint32> setIVconst = {}, set<int> inK0 = {}) :Grain128DifferentDegrees(setCube, setIVconst, inK0)
	{
		InvolvedKeysByDegree = {};
		termNumByDegree = {};
		currentEvaluatingDegree = -1;
		enumerateMode = Approximate;
	}

	void setDegree()
	{
		//Degree = toSetDegree;
		if (outputRound == -1)
		{
			throw "The output round is unset!!!\n";
		}
		if (Degree == -1)
		{
			int deg = getSPolyDegreeOfZ(std::cout);
			cout << "Degree determined as " << deg << endl;
			for (int i = 0; i < deg; ++i)
				InvolvedKeysByDegree.push_back({});
			for (int i = 0; i < deg; ++i)
				termNumByDegree.push_back(0);
		}
		else if (Degree > 0)
		{
			if (InvolvedKeysByDegree.empty() == true) {
				for (int i = 0; i < Degree; ++i)
					InvolvedKeysByDegree.push_back({});
			}
			if (termNumByDegree.empty() == true)
			{
				for (int i = 0; i < Degree; ++i)
					termNumByDegree.push_back(0);
			}
		}

	}


	int getInvolvedKeysByDegree(int TargetDegree, std::ostream & o = std::cout)
	{
		enumerateMode = Approximate;
		setDegree();
		if (TargetDegree<0 || TargetDegree>Degree)
		{
			throw "Illegal input of TargetDegree=" + to_string(TargetDegree) + " when Degree=" + to_string(Degree) + "!\n";
		}
		currentEvaluatingDegree = TargetDegree;
		try
		{
			setNewModelForSuperpoly();
			GRBLinExpr KeySUM = 0;
			for (int i = 0; i < KEYLENGTH; ++i)
			{
				if (Model->Keydivision[i].F == _delta_Flag)
				{
					KeySUM += Model->Keydivision[i].val;
				}
			}
			Model->milpModel->addConstr(KeySUM >= currentEvaluatingDegree);
			int iniRound = outputRound % 32 == 0 ? outputRound : outputRound - (outputRound % 32);
			DP_Structure z = setModelForOutputZ(iniRound);
			Model->milpModel->setObjective(KeySUM, GRB_MAXIMIZE);

			if (z.F == _0c_Flag && z.F == _1c_Flag)
			{
				o << "The output bit is constant!" << endl;
			}
			else
			{
				Model->milpModel->setCallback(this);
				Model->milpModel->optimize();
				if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
					generateFinalReport(TargetDegree, o);
					return InvolvedKeysByDegree[TargetDegree - 1].size();
				}
				else if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_INFEASIBLE)
				{
					o << "Infeasible\n";
					generateFinalReport(TargetDegree, o);
					return InvolvedKeysByDegree[TargetDegree - 1].size();
				}
				else
				{
					o << "Solving Process Break Down!\n";
					return -1;
				}
			}
		}
		catch (GRBException e)
		{
			o << e.getErrorCode() << endl;
			o << e.getMessage() << endl;
			return -1;
		}
	}

	
	int getPreciseTermsByDegree(int TargetDegree, std::ostream & o = std::cout)
	{
		enumerateMode = Precise;
		setDegree();
		if (TargetDegree<0 || TargetDegree>Degree)
		{
			throw "Illegal input of TargetDegree=" + to_string(TargetDegree) + " when Degree=" + to_string(Degree) + "!\n";
		}
		currentEvaluatingDegree = TargetDegree;
		try
		{
			setNewModelForSuperpoly();
			GRBLinExpr KeySUM = 0;
			for (int i = 0; i < KEYLENGTH; ++i)
			{
				if (Model->Keydivision[i].F == _delta_Flag)
				{
					KeySUM += Model->Keydivision[i].val;
				}
			}
			Model->milpModel->addConstr(KeySUM == currentEvaluatingDegree);
			int iniRound = outputRound % 32 == 0 ? outputRound : outputRound - (outputRound % 32);
			DP_Structure z = setModelForOutputZ(iniRound);
			Model->milpModel->setObjective(KeySUM, GRB_MAXIMIZE);

			if (z.F == _0c_Flag && z.F == _1c_Flag)
			{
				o << "The output bit is constant!" << endl;
			}
			else
			{
				Model->milpModel->setCallback(this);
				Model->milpModel->optimize();
				if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_OPTIMAL) {

					generateFinalReport(currentEvaluatingDegree, o);
					return termNumByDegree[currentEvaluatingDegree - 1];
				}
				else if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_INFEASIBLE)
				{
					o << "Infeasible\n";
					generateFinalReport(currentEvaluatingDegree, o);
					return termNumByDegree[currentEvaluatingDegree - 1];
				}
				else
				{
					o << "Solving Process Break Down!\n";
					return -1;
				}
			}
		}
		catch (GRBException e)
		{
			o << e.getErrorCode() << endl;
			o << e.getMessage() << endl;
			return -1;
		}
	}



protected:
	void callback()
	{
		try
		{
			if (where == GRB_CB_MIPSOL)
			{

				ofstream file1;

				GRBLinExpr sum = 0;
				int variableNo = 0;
				if (enumerateMode == Approximate)
				{
					string filename =
						"TmpGrain128ApproximateTermEnumerateAt" + to_string(currentEvaluatingDegree) + "CallBack" + CubeNameSet(Cube, IVLENGTH) + ".txt";

					string inputString;
					for (int i = 0; i < KEYLENGTH; ++i)
					{
						if (Model->Keydivision[i].F == _delta_Flag && getSolution(Model->Keydivision[i].val) > 0.5)
						{
							++variableNo;
							InvolvedKeysByDegree[currentEvaluatingDegree - 1].insert(i);
							inputString += to_string(i);
							inputString += ",";
						}
					}
					inputString.pop_back();
					file1.open(filename, ios::app);
					file1 << inputString << "\n";
					file1.close();
					GRBLinExpr sumRest = 0;
					for (int i = 0; i < KEYLENGTH; ++i)
					{
						if (Model->Keydivision[i].F == _delta_Flag
							&& InvolvedKeysByDegree[currentEvaluatingDegree - 1].find(i) == InvolvedKeysByDegree[currentEvaluatingDegree - 1].end())
						{
							sumRest += Model->Keydivision[i].val;
						}
					}
					//Model->milpModel->addConstr(sum == 0);// <= currentEvaluatingDegree - 1);
					addLazy(sumRest >= 1);
				}
				else if (enumerateMode == Precise)
				{
					string filename =
						"TmpGrain128PreciseTermEnumerateAt" + to_string(currentEvaluatingDegree) + "CallBack" + CubeNameSet(Cube, IVLENGTH) + ".txt";
					for (int i = 0; i < KEYLENGTH; ++i)
					{
						if (Model->Keydivision[i].F == _delta_Flag && getSolution(Model->Keydivision[i].val) > 0.5)
						{
							++variableNo;
							sum += Model->Keydivision[i].val;
							InvolvedKeysByDegree[currentEvaluatingDegree - 1].insert(i);
#if STORETERMS
							file1.open(filename, ios::app);
							file1 << dec << i;
							if (variableNo != currentEvaluatingDegree)file1 << ",";
							else file1 << endl;
							file1.close();
#endif
						}
					}
					termNumByDegree[currentEvaluatingDegree - 1]++;
					addLazy(sum <= currentEvaluatingDegree - 1);
				}
			}
		}
		catch (GRBException e)
		{
			cout << "Error No. " << e.getErrorCode() << endl;
			cout << e.getMessage() << endl;
		}
	}

	void generateFinalReport(int TargetDegree, std::ostream & o)
	{
		o << "CubeIndx:";
		for (int i = 0; i < IVLENGTH; ++i)
		{
			if (Cube.find(i) != Cube.end())o << i << ",";
		}
		o << endl << "Round " << outputRound << endl;
		if (IVconst.size() != 0)
		{
			o << "IVconst:";
			for (int j = 0; j < IVconst.size(); ++j)
			{
				o << IVconst[j] << ",";
			}
			o << endl;
		}
		if (K0.size() != 0)
		{
			o << "K0:";
			for (set<int>::iterator ite = K0.begin(); ite != K0.end(); ++ite)
			{
				o << dec << *ite << ",";
			}
			o << dec << "\n";
		}
		o << "KeysInDegree" << TargetDegree << ":";
		//	RelatedKeys = *cb.RelatedKeys;
		for (int i = 0; i < KEYLENGTH; ++i)
		{
			//如果第Ki包含在最高次项中，则把它存入RelatedKeys
			if (InvolvedKeysByDegree[TargetDegree - 1].find(i) != InvolvedKeysByDegree[TargetDegree - 1].end())o << i << ",";
		}
		o << endl << "#K" << TargetDegree << "=" << InvolvedKeysByDegree[TargetDegree - 1].size() << endl;
		if (enumerateMode == Precise)o << "#TermOfDegree" << TargetDegree << "=" << termNumByDegree[TargetDegree - 1] << endl;
	}


private:
	int currentEvaluatingDegree;
	TermEnumManner enumerateMode;
};
#endif





