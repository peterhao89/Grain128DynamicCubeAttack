#pragma once
#include"DivisionPropertyBasic.h"
#define CORENUM 6
#define KEYLENGTH 80
#define IVLENGTH 64



class GrainV1MilpDivisionModel
{
public:
	GRBModel * milpModel;
	vector<DP_Structure> Sreg, Breg;
	vector<DP_Structure> IVdivision;
	vector<DP_Structure> Keydivision;

	set<int> S_Nullified;
	set<int> B_Nullified;

	int startRound;
	int endRound;
	set<int> K0;//key bits with 0 value



	GrainV1MilpDivisionModel(GRBEnv * milpEnv)
	{
		milpModel = new GRBModel(*milpEnv);
		Sreg.clear();
		Breg.clear();
		IVdivision.clear();
		Keydivision.clear();
		S_Nullified.clear();
		B_Nullified.clear();
		startRound = 0;
		endRound = 0;
	}


	~GrainV1MilpDivisionModel()
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
		for (int i = 0; i < 80; ++i)
		{
			Breg.push_back(Keydivision[i]);
			if (i < IVLENGTH)Sreg.push_back(IVdivision[i]);
			else
			{
				Sreg.push_back(DP_Structure(*milpModel, _1c_Flag));
			}
		}
	}

	void setInitStateWith160Variables()
	{
		for (int i = 0; i < 80; ++i)
		{
			Sreg.push_back(DP_Structure(*milpModel, _delta_Flag));
			Breg.push_back(DP_Structure(*milpModel, _delta_Flag));
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
		for (int r = 0; r < endRound - startRound; ++r)
		{
			UpdateInternalState(r);
		}

	}

	DP_Structure Zoutput(int Round)
	{
		FlagValue FinalWheCon(0, 0);
		vector<int> Bindex = { 1, 2,4,10,31,43,56,63};
		vector<int> Sindex = { 25 };
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
ss	3	64	
ss	46	64	
bs	63	64	
sss	3	25	46
sss	3	46	64
ssb	3	46	63
ssb	25	46	63
ssb	46	64	63
*/
		ToBeSum.push_back(BS_Multi({}, { 3,64 }, Round));
		ToBeSum.push_back(BS_Multi({}, { 46,64 }, Round));
		ToBeSum.push_back(BS_Multi({}, { 3,25,46 }, Round));
		ToBeSum.push_back(BS_Multi({}, { 3,46,64 }, Round));
		ToBeSum.push_back(BS_Multi({ 63 }, { 3,46 }, Round));
		ToBeSum.push_back(BS_Multi({ 63 }, { 25,46 }, Round));
		ToBeSum.push_back(BS_Multi({ 63 }, { 46,64 }, Round));

		DP_Structure res = Sum(*milpModel, ToBeSum);
		return res;
	}


private:

	void UpdateInternalState(int r)
	{
		vector<DP_Structure> sToBeSum, bToBeSum;
		if (B_Nullified.find(r + 80) == B_Nullified.end() && S_Nullified.find(r + 80) == S_Nullified.end())
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
			DP_Structure bsub = NFSR80(r);
			DP_Structure ssub = LFSR80(r);
			bToBeSum.push_back(bsub);
			sToBeSum.push_back(ssub);

			//Summation
			Breg.push_back(Sum(*milpModel, bToBeSum));
			Sreg.push_back(Sum(*milpModel, sToBeSum));
		}
		else if (B_Nullified.find(r + 80) == B_Nullified.end() && S_Nullified.find(r + 80) != S_Nullified.end())
		{
			//zr
			DP_Structure zr = Zoutput(r);

			//b'
			DP_Structure bsub = NFSR80(r);
			//s[r+80]=0
			Sreg.push_back(DP_Structure(*milpModel, _0c_Flag));
			//b[r+80]=b'+s[r]+z[r]
			bToBeSum = { zr,bsub,Sreg[r] };
			Breg.push_back(Sum(*milpModel, bToBeSum));
		}
		else if (B_Nullified.find(r + 80) != B_Nullified.end() && S_Nullified.find(r + 80) == S_Nullified.end())
		{
			//zr
			DP_Structure zr = Zoutput(r);
			//s'
			DP_Structure ssub = LFSR80(r);
			//s[r+80]=s'+s[r]+z[r]
			sToBeSum = { zr,ssub,Sreg[r] };
			Sreg.push_back(Sum(*milpModel, sToBeSum));
			//b[r+80]=0
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
	//S0 has not been added
	DP_Structure LFSR80(int Round)
	{
		vector<int> LFSRindx = { 13, 23, 38, 51, 62 };
		vector<DP_Structure> ToBeSum;
		FlagValue FinalWheCon = _0c_Flag;
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


	//NFSR update. Bvec will expand by 1 variable--B[r+80]
	//S0 has not been added
	DP_Structure NFSR80(int Round)
	{
		FlagValue FinalWheCon(0, 0);
		vector<DP_Structure> ToBeSum;


		//b0 added without COPY
		if (Breg[Round].F.isDelta == 1)ToBeSum.push_back(Breg[Round]);


		//Other b bits are added with COPY
		vector<int> Bindex = {62,60,52,45,37,33,28,21,14,9};
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
			{ 63,60 },
			{ 37,33 },
			{ 15,9 },
			{ 60,52,45 },
			{ 33,28,21 },
			{ 63,45,28,9 },
			{ 60,52,37,33 },
			{ 63,60,21,15 },
			{ 63,60,52,45,37 },
			{ 33,28,21,15,9 },
			{ 52,45,37,33,28,21 }
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


};


class GrainV1DifferentDegrees
{
public:
	GRBEnv * milpEnvironment;
	GrainV1MilpDivisionModel * Model;
	set<int> Cube;
	set<int> K0;
	vector<uint32> IVconst;
	int Degree;
	int outputRound;
	set<int> S_Nullified, B_Nullified;

	GrainV1DifferentDegrees(set<int> setCube, vector<uint32> setIVconst = {}, set<int> inK0 = {})
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

	~GrainV1DifferentDegrees()
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
			int iniRound = outputRound % 16 == 0 ? outputRound : outputRound - (outputRound % 16);
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
		Model = new GrainV1MilpDivisionModel(milpEnvironment);
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
		for (int i = 0; i < 80; ++i)
		{
			if (Model->Sreg[IniRounds + i].F == _delta_Flag)
				Model->milpModel->addConstr(Model->Sreg[IniRounds + i].val == 0);
			if (Model->Breg[IniRounds + i].F == _delta_Flag)
				Model->milpModel->addConstr(Model->Breg[IniRounds + i].val == 0);
		}
		if (zo.F.isDelta == 1)Model->milpModel->addConstr(zo.val == 1);
		return zo;
	}


};




class GrainV1SuperpolyInvolvedKeysOfCube :public GrainV1DifferentDegrees, public GRBCallback
{
public:
	set<int> InvolvedKeys;

	GrainV1SuperpolyInvolvedKeysOfCube(set<int> setCube, vector<uint32> setIVconst = {}, set<int> inK0 = {}) :GrainV1DifferentDegrees(setCube, setIVconst, inK0)
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
			int iniRound = outputRound % 16 == 0 ? outputRound : outputRound - (outputRound % 16);
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
				string filename = "TmpGrainV1InvovledKeysCallBack" + CubeNameSet(Cube, IVLENGTH) + ".txt";
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
class GrainV1SuperpolyTermEnumerateOfCube :public GrainV1DifferentDegrees, public GRBCallback
{
public:
	vector<set<int>> InvolvedKeysByDegree;
	vector<int> termNumByDegree;
	GrainV1SuperpolyTermEnumerateOfCube(set<int> setCube, vector<uint32> setIVconst = {}, set<int> inK0 = {}) :GrainV1DifferentDegrees(setCube, setIVconst, inK0)
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
			int iniRound = outputRound % 16 == 0 ? outputRound : outputRound - (outputRound % 16);
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
			int iniRound = outputRound % 16 == 0 ? outputRound : outputRound - (outputRound % 16);
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
						"TmpGrainV1ApproximateTermEnumerateAt" + to_string(currentEvaluatingDegree) + "CallBack" + CubeNameSet(Cube, IVLENGTH) + ".txt";

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
						"TmpGrainV1PreciseTermEnumerateAt" + to_string(currentEvaluatingDegree) + "CallBack" + CubeNameSet(Cube, IVLENGTH) + ".txt";
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