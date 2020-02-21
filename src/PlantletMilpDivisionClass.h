#pragma once
#include"DivisionPropertyBasic.h"
#define KEYLENGTH 80
#define IVLENGTH 90
#define CORENUM 6


class PlantletMilpDivisionModel
{
public:
	GRBModel * milpModel;
	vector<DP_Structure> Sreg, Breg;
	vector<DP_Structure> IVdivision;
	vector<DP_Structure> Keydivision;
	vector<DP_Structure> KeyStar;
	uint32 C0_9;

	set<int> S_Nullified;
	set<int> B_Nullified;


	int startRound;
	int endRound;
	set<int> K0;//key bits with 0 value



	PlantletMilpDivisionModel(GRBEnv * milpEnv)
	{
		milpModel = new GRBModel(*milpEnv);
		Sreg.clear();
		Breg.clear();
		
		IVdivision.clear();
		Keydivision.clear();
		KeyStar.clear();

		startRound = 0;
		endRound = 0;
		S_Nullified.clear();
		B_Nullified.clear();

		C0_9 = 0;
	}


	~PlantletMilpDivisionModel()
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
	//LFSR: 60
	//NFSR: 40
	void setInitStateWithKeyIV()
	{
		Breg.clear();
		Sreg.clear();
		for (int i = 0; i < 40; ++i)
		{
			Breg.push_back(IVdivision[i]);
		}
		for (int i = 0; i < IVLENGTH-40; ++i)
		{
			Sreg.push_back(IVdivision[40 + i]);
		}
		for (int i = 50; i < 60; ++i)
		{
			if (i == 59)Sreg.push_back(DP_Structure(*milpModel, _0c_Flag));
			else Sreg.push_back(DP_Structure(*milpModel, _1c_Flag));
		}
	}

	void setNullified_S(set<int> S_NULL)
	{
		S_Nullified = S_NULL;
	}

	void setNullified_B(set<int> B_NULL)
	{
		B_Nullified = B_NULL;
	}

	void UpdateModelByRounds()
	{
		int iniRound = endRound - startRound;
		KeyExpand();
		for (uint32 r = 0; r < iniRound; ++r)
		{
			C0_9 = (r%80)^(r%0x80);
			UpdateInternalState(r);
		}
	}

	DP_Structure Zoutput(int Round)
	{
		FlagValue FinalWheCon(0, 0);
		vector<int> Bindex = { 1, 6, 15, 17, 23, 28, 34 };
		vector<int> Sindex = { 30 };
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
		/*
		0 N4
		1 L6
		2 L8
		3 L10
		4 L32
		5 L17
		6 L19
		7 L32
		8 L38
		x0x1 +x2x3 +x4x5 +x6x7 +x0x4x8
		N4L6
		L8L10
		L32L17
		*/
		ToBeSum.push_back(BS_Multi({4}, { 6 }, Round));
		ToBeSum.push_back(BS_Multi({}, { 8,10 }, Round));
		ToBeSum.push_back(BS_Multi({}, { 32,17 }, Round));
		ToBeSum.push_back(BS_Multi({}, { 19,32}, Round));
		ToBeSum.push_back(BS_Multi({ 4 }, { 32,38 }, Round));
		DP_Structure res = Sum(*milpModel, ToBeSum);
		return res;
	}


private:

	void KeyExpand()
	{
		if ((endRound - startRound) < 80)
		{
			throw "The number of initialization rounds is too small!!\n";
		}

		for (int r = 0; r < 80; ++r)
			KeyStar.push_back(Keydivision[r]);
		for (int r = 80; r < (endRound - startRound); ++r)
		{
			vector<DP_Structure> tmpCOPY = COPY(*milpModel, KeyStar[r - 80], 2);
			KeyStar[r - 80] = tmpCOPY[0];
			KeyStar.push_back(tmpCOPY[1]);
		}
	}


	void UpdateInternalState(int r)
	{
		vector<DP_Structure> sToBeSum, bToBeSum;
		if (r < 320) 
		{
			DP_Structure zr = Zoutput(r);
			vector<DP_Structure> zrVec = COPY(*milpModel, zr, 2);
			bToBeSum.push_back(zrVec[0]);
			sToBeSum.push_back(zrVec[1]);
		}
		//s0
		vector<DP_Structure> S0vec = COPY(*milpModel, Sreg[r], 2);
		sToBeSum.push_back(S0vec[0]);
		bToBeSum.push_back(S0vec[1]);

		//b', s'
		DP_Structure bsub = NFSR_Output(r);
		DP_Structure ssub = LFSR_Output(r);
		bToBeSum.push_back(bsub);
		sToBeSum.push_back(ssub);

		bToBeSum.push_back(KeyStar[r]);
		if (bit32(C0_9, 4) == 1)bToBeSum.push_back(DP_Structure(*milpModel, _1c_Flag));
		//Summation
		Breg.push_back(Sum(*milpModel, bToBeSum));
		Sreg.push_back(Sum(*milpModel, sToBeSum));

		if (r == 319)Sreg.push_back(DP_Structure(*milpModel, _1c_Flag));
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
	DP_Structure LFSR_Output(int Round)
	{
		vector<int> LFSRindx = {54,43,34,20,14};
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
	DP_Structure NFSR_Output(int Round)
	{
		FlagValue FinalWheCon(0, 0);
		vector<DP_Structure> ToBeSum;


		//b0 added without COPY
		if (Breg[Round].F.isDelta == 1)ToBeSum.push_back(Breg[Round]);


		//Other b bits are added with COPY
		vector<int> Bindex = { 13,19,35,39 };
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
			{ 2,25 },
			{ 3,5 },
			{ 7,8 },
			{ 14,21 },
			{ 16,18 },
			{ 22,24 },
			{ 26,32 },
			{33,36,37,38},
			{10,11,12},
			{27,30,31}
		};
		for (int term = 0; term < BMultiIndex.size(); term++)
		{
			DP_Structure Multires = B_Multi(BMultiIndex[term], Round);
			FinalWheCon += Multires.F;
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


};

class PlantletDifferentDegrees
{
public:
	GRBEnv * milpEnvironment;
	PlantletMilpDivisionModel * Model;
	set<int> Cube;
	set<int> K0;
	vector<uint32> IVconst;
	int Degree;
	int outputRound;
	set<int> S_Nullified, B_Nullified;

	PlantletDifferentDegrees(set<int> setCube, vector<uint32> setIVconst = {}, set<int> inK0 = {})
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
	}

	~PlantletDifferentDegrees()
	{
		delete Model;
		delete milpEnvironment;
	}

	void set_S_Nullified(set<int> NullfiedBitsValues)
	{
		S_Nullified = NullfiedBitsValues;
	}

	void set_B_Nullified(set<int> NullfiedBitsValues)
	{
		B_Nullified = NullfiedBitsValues;
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
			Model->milpModel->addConstr(KeySUM >= 1);
			int iniRound = outputRound;
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
					o << dec << "Cube: ";
					for (set<int>::iterator ite = Cube.begin(); ite != Cube.end(); ++ite)
					{
						o << *ite << ",";
					}
					o << endl;
					int ResDeg = 0;
					for (int i = 0; i < KEYLENGTH; ++i)
					{
						if (Model->Keydivision[i].F == _delta_Flag && Model->Keydivision[i].val.get(GRB_DoubleAttr_X)>0.5)
						{
							o << "K" << i << ",";
							++ResDeg;
						}
					}
					o << endl;
					Degree = ResDeg; //return model.get(GRB_DoubleAttr_ObjVal);
					return Degree;
				}
				else if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_INFEASIBLE)
				{
					o << "Infeasible\n";
					Degree = 0;
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



protected:
	void setNewModelForSuperpoly()
	{
		Model = new PlantletMilpDivisionModel(milpEnvironment);
		Model->setStartRound(0);
		Model->initIVwithCubeAndIVconst(Cube, IVconst);
		Model->setK0(K0);
		Model->initKeyWithK0();
		Model->setNullified_B(B_Nullified);
		Model->setNullified_S(S_Nullified);
	}

	DP_Structure setModelForOutputZ(int iniRound)
	{
		Model->setEndRound(iniRound);
		Model->setInitStateWithKeyIV();
		Model->UpdateModelByRounds();
		DP_Structure zo = Model->Zoutput(outputRound);
		int IniRounds = Model->endRound - Model->startRound;
		for (int i = 0; i < 40; ++i)
		{
			if (Model->Breg[IniRounds + i].F == _delta_Flag)
				Model->milpModel->addConstr(Model->Breg[IniRounds + i].val == 0);
		}
		for (int i = 0; i < 61; ++i)
		{
			if (Model->Sreg[IniRounds + i].F == _delta_Flag)
				Model->milpModel->addConstr(Model->Sreg[IniRounds + i].val == 0);
		}
		if (zo.F.isDelta == 1)Model->milpModel->addConstr(zo.val == 1);
		return zo;
	}


};

