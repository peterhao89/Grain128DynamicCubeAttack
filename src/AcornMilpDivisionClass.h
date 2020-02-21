#pragma once
#include"DivisionPropertyBasic.h"


#define CORENUM 6
#define KEYLENGTH 128
#define IVLENGTH 128




class AcornMilpDivisionModel
{
public:
	GRBModel * milpModel;
	vector<DP_Structure> Sreg, m;
	vector<DP_Structure> IVdivision;
	vector<DP_Structure> Keydivision;

	int startRound;
	int endRound;
	set<int> K0;//key bits with 0 value



	AcornMilpDivisionModel(GRBEnv * milpEnv)
	{
		milpModel = new GRBModel(*milpEnv);
		Sreg.clear();
		IVdivision.clear();
		Keydivision.clear();

		startRound = 0;
		endRound = 0;
	}


	~AcornMilpDivisionModel()
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

	//expand m to 256+Rounds
	void setInitStateWithKeyIV()
	{
		Sreg.clear();
		for (int i = 0; i < 293; ++i)
		{
			DP_Structure Stmp = DP_Structure(*milpModel, _0c_Flag);
			Sreg.push_back(Stmp);
		}
		for (int i = 0; i < KEYLENGTH; ++i)
		{
			m.push_back(Keydivision[i]);
		}
		for (int i = 0; i < IVLENGTH; ++i)
		{
			m.push_back(IVdivision[i]);
		}
		for (int i = 0; i < (endRound - startRound - 256); ++i)
		{
			vector<DP_Structure> tmpkey = COPY(*milpModel, m[i % 128], 2);
			if (i % 128 == 0)
			{
				m.push_back(Sum(*milpModel, { tmpkey[0],DP_Structure(*milpModel,_1c_Flag) }));
			}
			else
			{
				m.push_back(tmpkey[0]);
			}
			m[i % 128] = tmpkey[1];
		}
	}

	//endRound-startRound number of initialization and an additional call to Upd_LFSRs
	void UpdateModelByRounds()
	{
		int iniRound = endRound - startRound;
		for (int r = 0; r < iniRound; ++r)
		{
			UpdateState(r);
		}
		Upd_LFSRs();
	}

	void UpdateState(int r)
	{
		Upd_LFSRs();
		DP_Structure ks = KSG128();
		DP_Structure majv = MajVec(244, 23, 160);
		vector<DP_Structure> ToSum = { m[r],ks,majv };
		if (Sreg[196].F.isDelta == 1)
		{
			vector<DP_Structure> Tmp196 = COPY(*milpModel, Sreg[196], 2);
			Sreg[196] = Tmp196[0];
			ToSum.push_back(Tmp196[1]);
		}
		else
		{
			ToSum.push_back(DP_Structure(*milpModel, Sreg[196].F));
		}
		if (Sreg[107].F.isDelta == 1)
		{
			vector<DP_Structure> Tmp107 = COPY(*milpModel, Sreg[107], 2);
			Sreg[107] = Tmp107[0];
			ToSum.push_back(Tmp107[1]);
		}
		else
		{
			ToSum.push_back(DP_Structure(*milpModel, Sreg[107].F + _1c_Flag));
		}
		if (Sreg[0].F.isDelta == 1)
		{
			ToSum.push_back(Sreg[0]);
		}
		else
		{
			ToSum.push_back(DP_Structure(*milpModel, Sreg[0].F));
		}

		Sreg[0] = Sum(*milpModel, ToSum);

		LRotate(Sreg);
	}

	void Upd_LFSRs()
	{
		//s289+=s235+s230
		vector<DP_Structure> ToSum289 = { Sreg[289] };
		ToSum289.push_back(COPY_XOR(*milpModel, Sreg, { 235,230 }));
		Sreg[289] = Sum(*milpModel, ToSum289);

		//s230+=s196+s193
		vector<DP_Structure> ToSum230 = { Sreg[230] };
		ToSum230.push_back(COPY_XOR(*milpModel, Sreg, { 196,193 }));
		Sreg[230] = Sum(*milpModel, ToSum230);



		//s193+=s160+s154
		vector<DP_Structure> ToSum193 = { Sreg[193] };
		ToSum193.push_back(COPY_XOR(*milpModel, Sreg, { 160,154 }));
		Sreg[193] = Sum(*milpModel, ToSum193);

		//s154+=s111+s107
		vector<DP_Structure> ToSum154 = { Sreg[154] };
		ToSum154.push_back(COPY_XOR(*milpModel, Sreg, { 111,107 }));
		Sreg[154] = Sum(*milpModel, ToSum154);

		//s107+=s66+s61
		vector<DP_Structure> ToSum107 = { Sreg[107] };
		ToSum107.push_back(COPY_XOR(*milpModel, Sreg, { 66,61 }));
		Sreg[107] = Sum(*milpModel, ToSum107);

		//s61+=s23+s0
		vector<DP_Structure> ToSum61 = { Sreg[61] };
		ToSum61.push_back(COPY_XOR(*milpModel, Sreg, { 23,0 }));
		Sreg[61] = Sum(*milpModel, ToSum61);
	}

	DP_Structure KSG128()
	{
		DP_Structure Vmaj = MajVec(235, 61, 193);
		DP_Structure Vch = ChVec(230, 111, 66);
		vector<DP_Structure> ToSum = { Vmaj ,Vch };
		if (Sreg[12].F.isDelta == 1)
		{
			vector<DP_Structure> s12 = COPY(*milpModel, Sreg[12], 2);
			Sreg[12] = s12[0];
			ToSum.push_back(s12[1]);
		}

		if (Sreg[154].F.isDelta == 1)
		{
			vector<DP_Structure> s154 = COPY(*milpModel, Sreg[154], 2);
			Sreg[154] = s154[0];
			ToSum.push_back(s154[1]);
		}
		return Sum(*milpModel, ToSum);
	}

	DP_Structure outputZFromFinalState()
	{
		DP_Structure ks = KSG128();
		for (int i = 0; i < 293; ++i)
		{
			if (Sreg[i].F.isDelta == 1)milpModel->addConstr(Sreg[i].val == 0);
		}
		if (ks.F.isDelta == 1)milpModel->addConstr(ks.val == 1);
		return ks;
	}

private:
	//COPY and Ch
	//return {e,f,g,ch(e,f,g)}
	vector<DP_Structure> Ch(DP_Structure e, DP_Structure f, DP_Structure g)
	{
		vector<DP_Structure> res = { e,f,g };
		FlagValue One(0, 1);
		FlagValue Zro(0, 0);

		if ((f.F + g.F) == Zro)
		{
			vector<DP_Structure> tmp = COPY(*milpModel, g, 2);
			res[2] = tmp[0];
			res.push_back(tmp[1]);
			return res;
		}
		if ((e.F + One) == Zro)
		{
			DP_Structure ef1 = COPY_AND(*milpModel, res, { 0,1 });
			res.push_back(ef1);
			return res;
		}


		FlagValue weg = (e.F + FlagValue(0, 1))*g.F;
		vector<DP_Structure> ToBeSum;
		if ((e.F*f.F).isDelta == 1)
		{
			DP_Structure ef = COPY_AND(*milpModel, res, { 0,1 });
			ToBeSum.push_back(ef);

		}
		if (weg.isDelta == 1)
		{
			DP_Structure eg = COPY_AND(*milpModel, res, { 0,2 });
			ToBeSum.push_back(eg);
		}
		if (((e.F*f.F) + weg).isDelta == 1)res.push_back(Sum(*milpModel, ToBeSum));
		else
		{
			res.push_back(DP_Structure(*milpModel, (e.F*f.F) + weg));
		}
		return res;
	}

	DP_Structure ChVec(int eindx, int findx, int gindx)
	{
		vector<DP_Structure> res = Ch(Sreg[eindx], Sreg[findx], Sreg[gindx]);
		Sreg[eindx] = res[0];
		Sreg[findx] = res[1];
		Sreg[gindx] = res[2];
		return res[3];
	}

	vector<DP_Structure> Maj(DP_Structure e, DP_Structure f, DP_Structure g)
	{

		vector<DP_Structure> res = { e,f,g };
		FlagValue Zro(0, 0);
		if ((e.F + f.F) == Zro)
		{
			vector<DP_Structure> tmp = COPY(*milpModel, g, 2);
			res[2] = tmp[0];
			res.push_back(tmp[1]);
			return res;
		}
		if ((e.F + g.F) == Zro)
		{
			vector<DP_Structure> tmp = COPY(*milpModel, f, 2);
			res[1] = tmp[0];
			res.push_back(tmp[1]);
			return res;
		}
		if ((f.F + g.F) == Zro)
		{
			vector<DP_Structure> tmp = COPY(*milpModel, e, 2);
			res[0] = tmp[0];
			res.push_back(tmp[1]);
			return res;
		}
		DP_Structure ef = COPY_AND(*milpModel, res, { 0,1 });
		DP_Structure eg = COPY_AND(*milpModel, res, { 0,2 });
		DP_Structure fg = COPY_AND(*milpModel, res, { 1,2 });
		res.push_back(Sum(*milpModel, { ef,eg,fg }));
		return res;
	}

	DP_Structure MajVec(int eindx, int findx, int gindx)
	{
		vector<DP_Structure> res = Maj(Sreg[eindx], Sreg[findx], Sreg[gindx]);
		Sreg[eindx] = res[0];
		Sreg[findx] = res[1];
		Sreg[gindx] = res[2];
		return res[3];
	}

};


class AcornDifferentDegrees
{
public:
	GRBEnv * milpEnvironment;
	AcornMilpDivisionModel * Model;
	set<int> Cube;
	set<int> K0;
	vector<uint32> IVconst;
	int Degree;
	int outputRound;

	AcornDifferentDegrees(set<int> setCube, vector<uint32> setIVconst = {}, set<int> inK0 = {})
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

	~AcornDifferentDegrees()
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

	int getSPolyDegree(std::ostream& o = std::cout)
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


protected:
	void setNewModelForSuperpoly()
	{
		Model = new AcornMilpDivisionModel(milpEnvironment);
		Model->setStartRound(0);
		Model->initIVwithCubeAndIVconst(Cube, IVconst);
		Model->setK0(K0);
		Model->initKeyWithK0();
	}

	DP_Structure setModelForOutputZ()
	{
		Model->setEndRound(outputRound);
		Model->setInitStateWithKeyIV();
		Model->UpdateModelByRounds();
		DP_Structure z = Model->outputZFromFinalState();
		return z;
	}
};

class AcornSuperpolyInvolvedKeysOfCube :public AcornDifferentDegrees, public GRBCallback
{
public:
	set<int> InvolvedKeys;

	AcornSuperpolyInvolvedKeysOfCube(set<int> setCube, vector<uint32> setIVconst = {}, set<int> inK0 = {}) :AcornDifferentDegrees(setCube, setIVconst, inK0)
	{
		InvolvedKeys.clear();
	}

	int getInvolvedKeys(std::ostream& o = std::cout)
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
					generateFinalReport(o);
					return InvolvedKeys.size();
				}
				else if (Model->milpModel->get(GRB_IntAttr_Status) == GRB_INFEASIBLE)
				{
					o << "Infeasible\n";
					generateFinalReport(o);
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
				string filename = "TmpAcornInvovledKeysCallBack" + CubeNameSet(Cube, IVLENGTH) + ".txt";
				ofstream file1;
				//Clear the file;
				//file1.open(filename,ios::app);
				//file1.close();

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

	void generateFinalReport(std::ostream & o)
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
class AcornSuperpolyTermEnumerateOfCube :public AcornDifferentDegrees, public GRBCallback
{
public:
	vector<set<int>> InvolvedKeysByDegree;
	vector<int> termNumByDegree;
	AcornSuperpolyTermEnumerateOfCube(set<int> setCube, vector<uint32> setIVconst = {}, set<int> inK0 = {}) :AcornDifferentDegrees(setCube, setIVconst, inK0)
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
			int deg = getSPolyDegree(std::cout);
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
						"TmpAcornApproximateTermEnumerateAt" + to_string(currentEvaluatingDegree) + "CallBack" + CubeNameSet(Cube, IVLENGTH) + ".txt";

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
						"TmpAcornPreciseTermEnumerateAt" + to_string(currentEvaluatingDegree) + "CallBack" + CubeNameSet(Cube, IVLENGTH) + ".txt";
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


