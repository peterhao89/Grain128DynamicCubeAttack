#pragma once
#include"DivisionPropertyBasic.h"
#define CORENUM 6
#define KEYLENGTH 128
#define IVLENGTH 128

const vector<int> KreyviumOutputStateBits = { 242, 65, 161 , 176, 92, 287 };


class KreyviumMilpDivisionModel
{
public:
	GRBModel * milpModel;
	vector<DP_Structure> initState;
	vector<DP_Structure> finalState;
	vector<DP_Structure> IVdivision, ivStar;
	vector<DP_Structure> Keydivision, keyStar;

	int startRound;
	int endRound;

	set<int> K0;//key bits with 0 value



	KreyviumMilpDivisionModel(GRBEnv * milpEnv)
	{
		milpModel = new GRBModel(*milpEnv);
		initState.clear();
		finalState.clear();
		IVdivision.clear();
		Keydivision.clear();
		ivStar.clear();
		keyStar.clear();
		startRound = 0;
		endRound = 0;
	}


	~KreyviumMilpDivisionModel()
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
		int evalNumRounds = endRound - startRound;
		//寄存器中有93比特密钥
		for (int i = 0; i < 93; ++i)
		{
			initState.push_back(Keydivision[i]);
		}
		//////////////////////////   Set Key bits in Kstar  ////////////////////////////////////////////////
		//K*前128比特密钥中有93比特是从Srg中COPY来的，其余为初始Division Property
		if (evalNumRounds >= 128)
		{
			for (int r = 0; r < 128; r++)
			{
				if (127 - (r % 128) < 93)//COPY
				{
					vector<DP_Structure> tmpKeyvec = COPY(*milpModel, Keydivision[127 - (r % 128)], 2);
					initState[127 - (r % 128)] = tmpKeyvec[0];
					keyStar.push_back(tmpKeyvec[1]);
				}
				else//初始
				{
					keyStar.push_back(Keydivision[127 - (r % 128)]);
				}
			}

			//剩余的初始化轮数所需的K*比特都是通过COPY之前的128比特K*得到
			for (int r = 128; r < evalNumRounds; ++r)
			{
				vector<DP_Structure> tmpKeyvec = COPY(*milpModel, keyStar[r - 128], 2);
				keyStar[r - 128] = tmpKeyvec[0];
				keyStar.push_back(tmpKeyvec[1]);
			}
		}
		else
		{
			//轮数小于128轮的话，有些比特不会引入，不过这和咱实验无关，忽略之
			cout << "Evaluate Round " << evalNumRounds << "<128";

			vector<DP_Structure> tempKeyini;
			for (int r = 0; r < evalNumRounds; r++)
			{
				if (127 - (r % 128) <93)
				{
					vector<DP_Structure> tmpKeyvec = COPY(*milpModel, Keydivision[127 - (r % 128)], 2);
					initState[127 - (r % 128)] = tmpKeyvec[0];
					keyStar.push_back(tmpKeyvec[1]);
				}
				else
				{
					keyStar.push_back(Keydivision[127 - (r % 128)]);
				}
			}
		}

		//初始化IV
		//////////////////////////   Set IV bits in Srg  ////////////////////////////////////////////////
		for (int i = 93; i < 93 + 128; ++i)
		{
			initState.push_back(IVdivision[i - 93]);
		}
		///////////////////////   Set Constant bits in Srg /////////////////////////////////////////////
		for (int i = 93 + 128; i < 288 - 1; ++i)
		{
			initState.push_back(DP_Structure(*milpModel, _1c_Flag));
		}
		for (int i = 288 - 1; i < 288; ++i)
		{
			initState.push_back(DP_Structure(*milpModel, _0c_Flag));
		}
		//////////////////////////   Set IV bits in IVstar  ////////////////////////////////////////////////
		//初始化IV*
		if (evalNumRounds >= 128)
		{
			for (int r = 0; r < 128; r++)
			{
				vector<DP_Structure> tmpIVvec = COPY(*milpModel, IVdivision[127 - (r % 128)], 2);
				initState[93 + 127 - (r % 128)] = tmpIVvec[0];
				ivStar.push_back(tmpIVvec[1]);
			}

			for (int r = 128; r < evalNumRounds; ++r)
			{
				vector<DP_Structure> tmpIVvec = COPY(*milpModel, ivStar[r - 128], 2);
				ivStar[r - 128] = tmpIVvec[0];
				ivStar.push_back(tmpIVvec[1]);
			}
		}
		else
		{
			cout << "Evaluate Round " << evalNumRounds << "<128";
			for (int r = 0; r < evalNumRounds; r++)
			{
				vector<DP_Structure> tmpIVvec = COPY(*milpModel, IVdivision[127 - (r % 128)], 2);
				initState[93 + 127 - (r % 128)] = tmpIVvec[0];
				ivStar.push_back(tmpIVvec[1]);
			}
		}


	}

	
	void setFinalStateWithInitState()
	{
		if (initState.empty() == true)
		{
			throw "The initial state is not settled!!\n";
		}
		else
		{
			finalState = initState;
		}
	}


	//The updating function of Kreyvium
	//Indx={91,90,170,65, 92}
	//Indx={175,174,263,161, 176}
	//Indx={286,285,68,242, 287}
	void KreyviumCore(vector<int> Indx, DP_Structure *IV0 = NULL, DP_Structure *K0 = NULL)
	{
		FlagValue MultiTest = finalState[Indx[0]].F;
		MultiTest *= finalState[Indx[1]].F;
		//DP_Structure s91s90(model, MultiTest);
		vector<DP_Structure> ToBeSum;

		if (MultiTest.isDelta == 1)
		{
			vector<DP_Structure> s91 = COPY(*milpModel, finalState[Indx[0]], 2);
			finalState[Indx[0]] = s91[0];
			vector<DP_Structure> s90 = COPY(*milpModel, finalState[Indx[1]], 2);
			finalState[Indx[1]] = s90[0];
			vector<DP_Structure> ToBeMulti = { s91[1], s90[1] };
			DP_Structure s91s90 = Multi(*milpModel, ToBeMulti);
			ToBeSum.push_back(s91s90);
		}


		vector<DP_Structure> s170;
		MultiTest += finalState[Indx[2]].F;
		if (finalState[Indx[2]].F.isDelta == 1)
		{
			s170 = COPY(*milpModel, finalState[Indx[2]], 2);
			finalState[Indx[2]] = s170[0];
			ToBeSum.push_back(s170[1]);
		}
		//= COPY(model, Srg[Indx[2]], 2);

		vector<DP_Structure> s65;
		MultiTest += finalState[Indx[3]].F;
		if (finalState[Indx[3]].F.isDelta == 1)
		{
			s65 = COPY(*milpModel , finalState[Indx[3]], 2);
			finalState[Indx[3]] = s65[0];
			ToBeSum.push_back(s65[1]);
		}
		MultiTest += finalState[Indx[4]].F;
		if (finalState[Indx[4]].F.isDelta == 1)ToBeSum.push_back(finalState[Indx[4]]);


		if (IV0 != NULL)
		{
			MultiTest += IV0->F;
			if (IV0->F.isDelta == 1)
			{
				ToBeSum.push_back(*IV0);
			}
		}

		if (K0 != NULL)
		{
			MultiTest += K0->F;
			if (K0->F.isDelta == 1)
			{
				ToBeSum.push_back(*K0);
			}
		}
		if (ToBeSum.empty() == false)
		{
			DP_Structure t1 = Sum(*milpModel, ToBeSum);
			finalState[Indx[4]] = t1;
		}
		else
		{
			DP_Structure t1(*milpModel, MultiTest);
			finalState[Indx[4]] = t1;
		}
	}


	void UpdateModelByRounds()
	{
		DP_Structure stmp = DP_Structure(*milpModel, _0c_Flag);
		for (int r = startRound; r < endRound; ++r)
		{
			KreyviumCore({ 91, 90, 170, 65, 92 }, &ivStar[r], NULL);
			KreyviumCore({ 175, 174, 263, 161, 176 });
			KreyviumCore({ 286, 285, 68, 242, 287 }, NULL, &keyStar[r]);
			RRotate(finalState);
		}
	}

	DP_Structure outputSeperateStateBit(int bitNo)
	{
		DP_Structure stmp = DP_Structure(*milpModel, _0c_Flag);
		for (int i = 0; i < 288; i++)
		{
			if (finalState[i].F == _delta_Flag && i != bitNo)milpModel->addConstr(finalState[i].val == 0);
		}
		if (finalState[bitNo].F == _delta_Flag)milpModel->addConstr(finalState[bitNo].val == 1);
		return finalState[bitNo];
	}

	DP_Structure outputZ()
	{
		set<int> IndxSet = { 65, 92, 161, 176, 242, 287 };
		vector<int> Indx = { 65, 92, 161, 176, 242, 287 };

		vector<DP_Structure> toBeSum;
		for (int i = 0; i < 288; ++i)
		{
			if (IndxSet.find(i) == IndxSet.end())
			{
				if (finalState[i].F == _delta_Flag)milpModel->addConstr(finalState[i].val == 0);
			}
			else
			{
				toBeSum.push_back(finalState[i]);
			}
		}
		DP_Structure z = Sum(*milpModel, { toBeSum });
		if (z.F == _delta_Flag)milpModel->addConstr(z.val == 1);
		return z;
	}


};



class KreyviumDifferentDegrees
{
public:
	GRBEnv * milpEnvironment;
	KreyviumMilpDivisionModel * Model;
	set<int> Cube;
	set<int> K0;
	vector<uint32> IVconst;
	int Degree;
	int outputRound;

	KreyviumDifferentDegrees(set<int> setCube, vector<uint32> setIVconst = {}, set<int> inK0 = {})
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

	~KreyviumDifferentDegrees()
	{
		delete Model;
		delete milpEnvironment;
	}

	void setK0ForModel(set<int> inK0)
	{
		K0 = inK0;
	}

	void setOutputRound(int setOutputRound)
	{
		outputRound = setOutputRound;
	}

	int getAlgebraicDegreeOfZ(std::ostream& o = std::cout)
	{
		if (outputRound == -1)
		{
			throw "The output round is unset!!!\n";
		}
		try
		{
			setNewModelForFreeIVs();
			GRBLinExpr VariableSUM = 0;
			for (int i = 0; i < KEYLENGTH; ++i)
			{
				if (Model->Keydivision[i].F == _delta_Flag)
				{
					VariableSUM += Model->Keydivision[i].val;
				}
			}
			for (int i = 0; i < IVLENGTH; ++i)
			{
				if (Model->IVdivision[i].F == _delta_Flag)
				{
					VariableSUM += Model->IVdivision[i].val;
				}
			}
			//Model->milpModel->addConstr(KeySUM >= 1);
			DP_Structure z = setModelForOutputZ();
			Model->milpModel->setObjective(VariableSUM, GRB_MAXIMIZE);
			if (z.F == _0c_Flag && z.F == _1c_Flag)
			{
				Degree = 0;
				return Degree;
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
							o << "K" << i << ",";
							++ResDeg;
						}
					}
					for (int i = 0; i < IVLENGTH; ++i)
					{
						if (Model->IVdivision[i].F == _delta_Flag && Model->IVdivision[i].val.get(GRB_DoubleAttr_X)>0.5)
						{
							o << "V" << i << ",";
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


	int getAlgebraicDegreeOfStateBit(int stateBitPos, std::ostream & o = std::cout)
	{
		if (outputRound == -1)
		{
			throw "The output round is unset!!!\n";
		}
		try
		{
			setNewModelForFreeIVs();
			GRBLinExpr VariableSUM = 0;
			for (int i = 0; i < KEYLENGTH; ++i)
			{
				if (Model->Keydivision[i].F == _delta_Flag)
				{
					VariableSUM += Model->Keydivision[i].val;
				}
			}
			for (int i = 0; i < IVLENGTH; ++i)
			{
				if (Model->IVdivision[i].F == _delta_Flag)
				{
					VariableSUM += Model->IVdivision[i].val;
				}
			}
			DP_Structure z = setModelForSeperateStateBit(stateBitPos);
			Model->milpModel->setObjective(VariableSUM, GRB_MAXIMIZE);
			if (z.F == _0c_Flag || z.F == _1c_Flag)
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
							o << "K" << i << ",";
							++ResDeg;
						}
					}
					for (int i = 0; i < IVLENGTH; ++i)
					{
						if (Model->IVdivision[i].F == _delta_Flag && Model->IVdivision[i].val.get(GRB_DoubleAttr_X)>0.5)
						{
							o << "V" << i << ",";
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
			DP_Structure z = setModelForOutputZ();
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

	int getSPolyDegreeOfStateBit(int stateBitPos, std::ostream & o = std::cout)
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
			DP_Structure z = setModelForSeperateStateBit(stateBitPos);
			Model->milpModel->setObjective(KeySUM, GRB_MAXIMIZE);
			if (z.F == _0c_Flag || z.F == _1c_Flag)
			{
				Degree = 0;
			}
			else
			{
				Model->milpModel->optimize();
				if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
					int ResDeg = 0;
					o << stateBitPos << ":";
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


	int getSPolyOverallDegree_Seperately(std::ostream& o = std::cout)
	{
		int maxDeg = 0;
		for (int i = 0; i < KreyviumOutputStateBits.size(); ++i)
		{
			int deg = getSPolyDegreeOfStateBit(KreyviumOutputStateBits[i], o);
			maxDeg = deg > maxDeg ? deg : maxDeg;
		}
		Degree = maxDeg;
		return maxDeg;
	}



protected:
	void setNewModelForSuperpoly()
	{
		Model = new KreyviumMilpDivisionModel(milpEnvironment);
		Model->setStartRound(0);
		Model->initIVwithCubeAndIVconst(Cube, IVconst);
		Model->setK0(K0);
		Model->initKeyWithK0();
	}

	void setNewModelForFreeIVs()
	{
		Model = new KreyviumMilpDivisionModel(milpEnvironment);
		Model->setStartRound(0);
		Model->initIVwithFreeIVsAndIVconst(Cube, IVconst);
		Model->setK0(K0);
		Model->initKeyWithK0();
	}


	DP_Structure setModelForOutputZ()
	{
		Model->setEndRound(outputRound);
		Model->setInitStateWithKeyIV();
		Model->setFinalStateWithInitState();
		Model->UpdateModelByRounds();
		DP_Structure z = Model->outputZ();
		return z;
	}

	DP_Structure setModelForSeperateStateBit(int stateBit)
	{

		int evalNumRounds;
		int StBt;

		if (stateBit >= 0 && stateBit < 93)
		{
			evalNumRounds = outputRound - (stateBit);
			StBt = 0;
		}
		else if (stateBit >= 93 && stateBit < 177)
		{
			evalNumRounds = outputRound - (stateBit - 93);
			StBt = 93;
		}
		else
		{
			evalNumRounds = outputRound - (stateBit - 177);
			StBt = 177;
		}

		Model->setEndRound(evalNumRounds);
		Model->setInitStateWithKeyIV();
		Model->setFinalStateWithInitState();
		Model->UpdateModelByRounds();
		DP_Structure targetBit = Model->outputSeperateStateBit(StBt);
		return targetBit;
	}

};


class KreyviumSuperpolyInvolvedKeysOfCube :public KreyviumDifferentDegrees, public GRBCallback
{
public:
	set<int> InvolvedKeys;

	KreyviumSuperpolyInvolvedKeysOfCube(set<int> setCube, vector<uint32> setIVconst = {}, set<int> inK0 = {}) :KreyviumDifferentDegrees(setCube, setIVconst, inK0)
	{
		InvolvedKeys.clear();
		//		KeyInit = {};
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
			DP_Structure z = setModelForOutputZ();
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
					generateFinalReportOfZ(o);
					return InvolvedKeys.size();
				}
				else if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_INFEASIBLE)
				{
					o << "Infeasible\n";
					generateFinalReportOfZ(o);
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

	int getInvolvedKeysOfStateBit(int stateBitPos, std::ostream& o = std::cout)
	{
		if (outputRound == -1)
		{
			throw "The output round is unset!!!\n";
		}
		try
		{
			setNewModelForSuperpoly();
			GRBLinExpr KeySUM = 0;
			for (int i = 0; i < IVLENGTH; ++i)
			{
				if (Model->Keydivision[i].F == _delta_Flag)
				{
					KeySUM += Model->Keydivision[i].val;
				}
			}
			Model->milpModel->addConstr(KeySUM >= 1);
			DP_Structure z = setModelForSeperateStateBit(stateBitPos);
			Model->milpModel->setObjective(KeySUM, GRB_MAXIMIZE);
			if (z.F == _0c_Flag || z.F == _1c_Flag)
			{
				o << "The output bit is constant!" << endl;
				return 0;
			}
			else
			{
				Model->milpModel->setCallback(this);
				Model->milpModel->optimize();
				if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
					generateFinalReportOfStateBit(stateBitPos, o);
					return InvolvedKeys.size();
				}
				else if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_INFEASIBLE)
				{
					o << "Infeasible\n";
					generateFinalReportOfStateBit(stateBitPos, o);
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
			Degree = -1;
			o << e.getErrorCode() << endl;
			o << e.getMessage() << endl;
			return -1;
		}
	}

	int getAllInvolvedKeys_Seperately(std::ostream & o = std::cout)
	{
		int maxRes = 0;
		for (int i = 0; i < KreyviumOutputStateBits.size(); ++i)
		{
			int res = getInvolvedKeysOfStateBit(KreyviumOutputStateBits[i], o);
			maxRes = res > maxRes ? res : maxRes;
		}
		return maxRes;
	}

protected:
	void callback()
	{
		try
		{
			if (where == GRB_CB_MIPSOL)
			{
				//cout << "CallBack\n";
				string filename = "TmpKreyviumInvovledKeysCallBack" + CubeNameSet(Cube, IVLENGTH) + ".txt";
				ofstream file1;
				//Clear the file;
				file1.open(filename);
				file1.close();

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

	void generateFinalReportOfStateBit(int stateBitPos, std::ostream & o)
	{
		o << "CubeIndx:";
		for (int i = 0; i < IVLENGTH; ++i)
		{
			if (Cube.find(i) != Cube.end())o << i << ",";
		}
		o << endl << "Round " << outputRound << endl;
		o << "StateBit:" << stateBitPos << endl;
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
			if (InvolvedKeys.find(i) != InvolvedKeys.end())o << i << ",";
		}
		o << endl << "#K=" << InvolvedKeys.size() << endl;
	}

	void generateFinalReportOfZ(std::ostream & o)
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


//Precisely get the terms or approximately get the key bits involved in t-degree terms.
enum TermEnumManner
{
	Precise, //J^t for some degree t
	Approximate //Jsub^t for some degree t
};
#define STORETERMS 0
class KreyviumSuperpolyTermEnumerateOfCube :public KreyviumDifferentDegrees, public GRBCallback
{
public:
	vector<set<int>> InvolvedKeysByDegree;
	vector<int> termNumByDegree;
	KreyviumSuperpolyTermEnumerateOfCube(set<int> setCube, vector<uint32> setIVconst = {}, set<int> inK0 = {}) :KreyviumDifferentDegrees(setCube, setIVconst, inK0)
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
			int deg = getSPolyOverallDegree_Seperately(std::cout);
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

	int getInvolvedKeysByDegree_FromZ(int TargetDegree, std::ostream & o = std::cout)
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
			DP_Structure z = setModelForOutputZ();
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
					generateFinalReport_Z(TargetDegree, o);
					return InvolvedKeysByDegree[TargetDegree - 1].size();
				}
				else if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_INFEASIBLE)
				{
					o << "Infeasible\n";
					generateFinalReport_Z(TargetDegree, o);
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

	int getInvolvedKeysByDegree_FromStateBit(int stateBitPos, int TargetDegree, std::ostream & o = std::cout)
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
			DP_Structure z = setModelForSeperateStateBit(stateBitPos);
			Model->milpModel->setObjective(KeySUM, GRB_MAXIMIZE);
			if (z.F == _0c_Flag || z.F == _1c_Flag)
			{
				o << "The output bit is constant!" << endl;
				return 0;
			}
			else
			{
				Model->milpModel->setCallback(this);
				Model->milpModel->optimize();
				if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
					generateFinalReport_StateBit(stateBitPos, TargetDegree, o);
					return InvolvedKeysByDegree[currentEvaluatingDegree - 1].size();
				}
				else if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_INFEASIBLE)
				{
					o << "Infeasible\n";
					generateFinalReport_StateBit(stateBitPos, TargetDegree, o);
					return InvolvedKeysByDegree[currentEvaluatingDegree - 1].size();
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

	int getAllInvolvedKeysByDegree_Seperately(int TargetDegree, std::ostream & o = std::cout)
	{
		if (outputRound == -1)
		{
			throw "The output round is unset!!!\n";
		}
		vector<int> stateBitsToBeEvaluated;
		int maxDeg = 0;
		for (int i = 0; i < KreyviumOutputStateBits.size(); ++i)
		{
			int deg = getSPolyDegreeOfStateBit(KreyviumOutputStateBits[i], o);
			o << "S" << KreyviumOutputStateBits[i] << ":" << deg << endl;
			maxDeg = deg > maxDeg ? deg : maxDeg;
			if (deg >= TargetDegree)
			{
				stateBitsToBeEvaluated.push_back(KreyviumOutputStateBits[i]);
			}
		}
		Degree = maxDeg;
		for (int i = 0; i < Degree; ++i)
			InvolvedKeysByDegree.push_back({});
		for (int i = 0; i < stateBitsToBeEvaluated.size(); ++i)
		{
			getInvolvedKeysByDegree_FromStateBit(stateBitsToBeEvaluated[i], TargetDegree, o);
		}

		return InvolvedKeysByDegree[TargetDegree - 1].size();
	}

	int getPreciseTermsByDegree_FromZ(int TargetDegree, std::ostream & o = std::cout)
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
			DP_Structure z = setModelForOutputZ();
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

					generateFinalReport_Z(currentEvaluatingDegree, o);
					return termNumByDegree[currentEvaluatingDegree - 1];
				}
				else if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_INFEASIBLE)
				{
					o << "Infeasible\n";
					generateFinalReport_Z(currentEvaluatingDegree, o);
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

	int getPreciseTermsByDegree_FromStateBit(int stateBitPos, int TargetDegree, std::ostream & o = std::cout)
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
			DP_Structure z = setModelForSeperateStateBit(stateBitPos);
			Model->milpModel->setObjective(KeySUM, GRB_MAXIMIZE);
			if (z.F == _0c_Flag || z.F == _1c_Flag)
			{
				o << "The output bit is constant!" << endl;
				return 0;
			}
			else
			{
				Model->milpModel->setCallback(this);
				Model->milpModel->optimize();
				if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
					generateFinalReport_StateBit(stateBitPos, TargetDegree, o);
					return InvolvedKeysByDegree[currentEvaluatingDegree - 1].size();
				}
				else if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_INFEASIBLE)
				{
					o << "Infeasible\n";
					generateFinalReport_StateBit(stateBitPos, TargetDegree, o);
					return InvolvedKeysByDegree[currentEvaluatingDegree - 1].size();
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
						"TmpKreyviumApproximateTermEnumerateAt" + to_string(currentEvaluatingDegree) + "CallBack" + CubeNameSet(Cube, IVLENGTH) + ".txt";

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
						"TmpKreyviumPreciseTermEnumerateAt" + to_string(currentEvaluatingDegree) + "CallBack" + CubeNameSet(Cube, IVLENGTH) + ".txt";
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

	void generateFinalReport_Z(int TargetDegree, std::ostream & o)
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

	void generateFinalReport_StateBit(int stateBitPos, int TargetDegree, std::ostream & o)
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
		o << "StateBit:" << stateBitPos << endl;
		o << "KeysInDegree" << TargetDegree << ":";
		for (int i = 0; i < KEYLENGTH; ++i)
		{
			if (InvolvedKeysByDegree[TargetDegree - 1].find(i) != InvolvedKeysByDegree[TargetDegree - 1].end())o << i << ",";
		}
		o << endl << "#K" << TargetDegree << "=" << InvolvedKeysByDegree[TargetDegree - 1].size() << endl;
		if (enumerateMode == Precise)o << "#TermOfDegree" << TargetDegree << "=" << termNumByDegree[TargetDegree - 1] << endl;
	}


private:
	int currentEvaluatingDegree;
	TermEnumManner enumerateMode;
};






