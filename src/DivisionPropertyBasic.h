#ifndef DIVISIONPROPERTYBASIC
#define DIVISIONPROPERTYBASIC


#include"gurobi_c.h"
#include"gurobi_c++.h"
#include<vector>
#include<set>
#include<time.h>
#include<cstring>
#include<iostream>
#include<string>
#include<fstream>
#include<map>
using namespace std;


typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef  unsigned long long uint64;
#define LROT32(x,s)  ((((x)&0xffffffff)<<(s))|(((x)&0xffffffff)>>(32-(s))))
#define RROT32(x,s)  ((((x)&0xffffffff)>>(s))|(((x)&0xffffffff)<<(32-(s))))
#define LROT8(x,s)  (((((x)&0xff)<<(s))|(((x)&0xff)>>(8-(s))))&0xff)
#define RROT8(x,s)  (((((x)&0xff)>>(s))|(((x)&0xff)<<(8-(s))))&0xff)
#define LROT64(x,s)  (((x)<<(s))|((x)>>(64-(s))))
#define RROT64(x,s)  (((x)>>(s))|((x)<<(64-(s))))
#define bit(x,n)   (((x)>>(n))&1)
#define bit2(x1,x2,pos) (bit(x1,pos)^bit(x2,pos))
#define bit32(x,n)   (((x)>>(n))&1)
#define bitW32(x,n)  bit32((x[((n)/32)]),((n)%32))
#define bit64(x,n)   (((x)>>(n))&1)
#define bitW64(x,n)  bit64((x[((n)/64)]),((n)%64))
#define bit2p(x1,pos1,x2,pos2) (bit(x1,pos1)^bit(x2,pos2))
#define maj(a,b,c) (((a)&(b))^((a)&(c))^((b)&(c)))
#define ch(a,b,c) (((a)&(b))^(((a)^1)&(c)))

void SetBit(uint32 & X, int bitNum, int zro_or_one)
{
	uint32 one;
	if (0 == zro_or_one)
	{
		one = 1;
		one <<= bitNum;
		one = (~one);
		X &= one;
	}
	else
	{
		one = 1;
		one <<= bitNum;
		X |= one;
	}
}

void SetDivBit(vector<uint32>  & InputDiv, int BitNum, uint32 O_or_1)
{
	uint32 mask = 0xffffffff;
	uint32 one = 1;
	one <<= (BitNum % 32);
	mask ^= one;
	if (O_or_1 == 0)
	{
		InputDiv[BitNum / 32] &= mask;
	}
	else
	{
		InputDiv[BitNum / 32] &= mask;
		InputDiv[BitNum / 32] ^= one;
	}
}


int RandPick(set<int> Free)
{
	int pos = rand() % Free.size();
	int count = 0;
	for (set<int>::iterator ite = Free.begin(); ite != Free.end(); ite++)
	{
		if (count == pos)return *ite;
		count++;
	}
}

set<int> RandSet(int dim, set<int> Free)
{
	if (dim >= Free.size())return Free;
	set<int> res;
	while (res.size() < dim)
	{
		res.insert(RandPick(Free));
	}
	return res;
}



string CubeNameSet(set<int> Cube, int IVlength)
{
	string str = "";
	vector<uint32> vc;
	while ((32 * vc.size()) < IVlength)
	{
		vc.push_back(0);
	}
	for (set<int>::iterator ite = Cube.begin(); ite != Cube.end(); ++ite)
	{
		SetDivBit(vc, *ite, 1);
	}
	for (int i = 0; i < vc.size(); ++i)
	{
		str.append(to_string(vc[i]));
		str.append(",");
	}
	str.erase(str.end() - 1);
	//str.append(".txt");
	return str;
}

uint64 rand_64()//产生64位随机数
{
	static uint64 Z[2] = { 0x375201345e7fa379, 0xcde9fe134e8af6b1 ^ (uint64(rand()) << 32) ^ rand() };//(rand() << 24) + (rand() << 12) + rand() 0xcde9fb8a
	uint64 temp = Z[1] ^ (Z[1] << 63) ^ (Z[0] >> 1);
	Z[0] = Z[1];
	Z[1] = temp;
	return Z[0];
}

uint32 rand_32()//产生32位随机数
{
	static unsigned int Z[2] = { 0x37520134, 0xcde9fb81 ^ rand() };//(rand() << 24) + (rand() << 12) + rand() 0xcde9fb8a
	unsigned int temp = Z[1] ^ (Z[1] << 31) ^ (Z[0] >> 1);
	Z[0] = Z[1];
	Z[1] = temp;
	return Z[0];
}



struct FlagValue
{
public:
	uint32 isDelta;// if isDelta==1->it's variable. isDelta==0->it's constant
	uint32 constValue;// if constValue==1->constant 1, constValue==0->constant 0
	FlagValue(uint32 whev = 0, uint32 v = 0)
	{
		isDelta = whev;
		constValue = v;
	}

	FlagValue & operator += (const FlagValue & a)//Check whether "this \oplus a" involves variables. 
	{
		isDelta |= a.isDelta;
		constValue ^= a.constValue;
		if (isDelta == 1)constValue = 0;
		return (*this);
	}
	FlagValue & operator *= (const FlagValue & a)//Check whether "this \oplus a" involves variables. 
	{
		if ((a.isDelta == 0 && a.constValue == 0) || (isDelta == 0 && constValue == 0))
		{
			isDelta = 0;
			constValue = 0;
		}
		else
		{
			isDelta |= a.isDelta;
			constValue &= a.constValue;
			if (isDelta == 1)constValue = 0;
		}
		return (*this);
	}

};
bool operator == (FlagValue a, FlagValue b)
{
	if (a.isDelta == b.isDelta  && a.constValue == b.constValue)
	{
		return true;
	}
	else if (a.isDelta == 1 && b.isDelta == 1)
	{
		return true;
	}
	else
		return false;
}
bool operator !=(FlagValue a, FlagValue b)
{
	if (a == b)return false;
	else return true;
}
FlagValue operator + (const FlagValue a, const FlagValue b)
{
	FlagValue res = a;
	res += b;
	return res;
}
FlagValue operator * (const FlagValue a, const FlagValue b)
{
	FlagValue res = a;
	res *= b;
	return res;
}


const FlagValue _0c_Flag = FlagValue(0, 0);
const FlagValue _1c_Flag = FlagValue(0, 1);
const FlagValue _delta_Flag = FlagValue(1, 0);

struct DP_Structure
{
public:
	FlagValue F;
	GRBVar val;
	DP_Structure(GRBModel & model, FlagValue whec)
	{
		F = whec;
		if (whec.isDelta == 1)
		{
			val = model.addVar(0, 1, 0, GRB_BINARY);
			model.update();
		}
	}
	DP_Structure()
	{
		F = _0c_Flag;
	}
};




vector<DP_Structure> COPY(GRBModel & model, DP_Structure in, int Num)
{
	vector<DP_Structure> outset;
	if (Num == 0)return outset;
	if (Num == 1)
	{
		outset.push_back(in);
		return outset;
	}
	for (int i = 0; i < Num; ++i)
	{
		DP_Structure cpy = DP_Structure(model, in.F);
		outset.push_back(cpy);
	}
	if (in.F.isDelta == 1)
	{
		GRBLinExpr sum = 0;
		for (int i = 0; i < Num; ++i)
		{
			sum += outset[i].val;
		}
		model.addConstr(sum == in.val);
	}
	return outset;
}


vector< vector<DP_Structure>> COPYvec(GRBModel & model, vector<DP_Structure> in, int Num)
{
	vector< vector<DP_Structure>> outset;
	for (int i = 0; i < in.size(); ++i)
	{
		outset.push_back(COPY(model, in[i], Num));
	}
	vector< vector<DP_Structure>> res;
	for (int i = 0; i < Num; ++i)
	{
		vector<DP_Structure> tmpVec;
		for (int j = 0; j < in.size(); ++j)
		{
			tmpVec.push_back(outset[j][i]);
		}
		res.push_back(tmpVec);

	}
	return res;
}



DP_Structure Sum(GRBModel & model, vector<DP_Structure>  in)
{
	//Filter the Constants
	int Length = in.size();
	if (Length == 0)return DP_Structure(model, FlagValue(0, 0));
	FlagValue wSum(0, 0);
	GRBLinExpr sum = 0;
	vector<int> SumIndx;
	for (int i = 0; i < Length; ++i)
	{
		wSum += in[i].F;
		//if (in[i].F.isDelta == 1)sum += in[i].val;
	}
	if (wSum.isDelta == 0)return DP_Structure(model, wSum);
	//wSum.Value = 0;//I don't know why this can affect the model
	for (int i = 0; i < Length; ++i)
	{
		if (in[i].F.isDelta == 1)SumIndx.push_back(i);
		//if (in[i].F.isDelta == 1)sum += in[i].val;
	}
	if (SumIndx.size() == 1)return in[SumIndx[0]];


	for (int i = 0; i < SumIndx.size(); ++i)
	{
		sum += in[SumIndx[i]].val;
	}

	DP_Structure res(model, wSum);
	model.addConstr(res.val == sum);
	return res;
}

DP_Structure Multi(GRBModel & model, vector<DP_Structure> in)
{
	//Filter the Constants
	int Length = in.size();
	if (Length == 0)return DP_Structure(model, FlagValue(0, 0));


	FlagValue wSum(0, 1);
	vector<int> indexSet;
	for (int i = 0; i < Length; ++i)
	{
		wSum *= in[i].F;
		//if (in[i].F.isDelta == 1)indexSet.push_back(i);
	}
	if (wSum.isDelta == 0)
	{
		for (int i = 0; i < Length; ++i)
		{
			if (in[i].F.isDelta == 1)
			{
				model.addConstr(in[i].val == 0);//Disable the previous COPY
			}
		}
		return DP_Structure(model, wSum);
	}

	for (int i = 0; i < Length; ++i)
	{
		if (in[i].F.isDelta == 1)indexSet.push_back(i);
	}

	if (indexSet.size() == 1)
		return in[indexSet[0]];
	else
	{
		DP_Structure res(model, wSum);
		GRBLinExpr vsum = 0;
		for (int i = 0; i < indexSet.size(); ++i)
		{
			model.addConstr(res.val >= in[indexSet[i]].val);
			vsum += in[indexSet[i]].val;
		}
		model.addConstr(res.val <= vsum);
		return res;
	}



}


DP_Structure AND2(GRBModel & model, DP_Structure a, DP_Structure b)
{
	DP_Structure res(model, _delta_Flag);
	model.addConstr(res.val >= a.val);
	model.addConstr(res.val >= b.val);
	model.addConstr(res.val <= a.val+b.val);
	return res;
}


DP_Structure COPY_AND(GRBModel & model, vector< DP_Structure > &  in, vector<int> index)
{
	FlagValue wres(0, 1);
	for (int i = 0; i < index.size(); ++i)
	{
		wres *= in[index[i]].F;
	}
	if (wres.isDelta == 0)return DP_Structure(model, wres);
	vector<DP_Structure> ToBeMulti;
	for (int i = 0; i < index.size(); ++i)
	{
		if (in[index[i]].F.isDelta == 1)
		{
			vector<DP_Structure> cp = COPY(model, in[index[i]], 2);
			in[index[i]] = cp[0];
			ToBeMulti.push_back(cp[1]);
		}
	}
	if (ToBeMulti.size() == 1)return ToBeMulti[0];
	DP_Structure resultt(model, wres);
	for (int i = 0; i < ToBeMulti.size(); ++i)
	{
		model.addConstr(resultt.val >= ToBeMulti[i].val);
	}
	return resultt;
}



DP_Structure COPY_XOR(GRBModel & model, vector< DP_Structure > &  in, vector<int> index)
{
	FlagValue wres(0, 0);
	for (int i = 0; i < index.size(); ++i)
	{
		wres += in[index[i]].F;
	}
	if (wres.isDelta == 0)return DP_Structure(model, wres);
	vector<DP_Structure> ToBeSum;
	for (int i = 0; i < index.size(); ++i)
	{
		if (in[index[i]].F.isDelta == 1)
		{
			vector<DP_Structure> cp = COPY(model, in[index[i]], 2);
			in[index[i]] = cp[0];
			ToBeSum.push_back(cp[1]);
		}
	}
	if (ToBeSum.size() == 1)return ToBeSum[0];
	DP_Structure resultt(model, wres);
	GRBLinExpr sum = 0;
	for (int i = 0; i < ToBeSum.size(); ++i)
	{
		sum += ToBeSum[i].val;
	}
	model.addConstr(sum == resultt.val);
	return resultt;
}



//For Trivium-Like Ciphers
void RRotate(vector<DP_Structure> & Srg)
{
	int Length = Srg.size();
	DP_Structure Tmp288 = Srg[Length - 1];
	for (int sft = Length - 1; sft > 0; --sft)
	{
		Srg[sft] = Srg[sft - 1];
	}
	Srg[0] = Tmp288;
}

void LRotate(vector<DP_Structure> & Srg)
{
	int Length = Srg.size();
	DP_Structure Tmp0 = Srg[0];
	for (int sft = 0; sft < Length - 1; ++sft)
	{
		Srg[sft] = Srg[sft + 1];
	}
	Srg[Length - 1] = Tmp0;
}


/////////////////////////////////     ARX related         /////////////////////////////////

// Rotate to LSB
void ARX_RRot(vector<DP_Structure> & Srg, int n)
{
	int Length = Srg.size();
	for (int i = 0; i<n; ++i)
	{
		DP_Structure Tmp = Srg[0];
		for (int sft = 0; sft <Length - 1; ++sft)
		{
			Srg[sft] = Srg[sft + 1];
		}
		Srg[Length - 1] = Tmp;
	}
}

// Rotate to MSB
void ARX_LRot(vector<DP_Structure> & Srg, int n)
{
	int Length = Srg.size();
	for (int i = 0; i < n; ++i)
	{
		DP_Structure Tmp = Srg[Length - 1];
		for (int sft = Length - 1; sft > 0; --sft)
		{
			Srg[sft] = Srg[sft - 1];
		}
		Srg[0] = Tmp;
	}
}




//The first bit of modular ADD
//x0+y0->(z0, c1)
//res[0]=z0, res[1]=c1
vector<DP_Structure> LSB_Add(GRBModel & model, DP_Structure x0, DP_Structure  y0)
{
	FlagValue wSum = x0.F + y0.F;
	FlagValue wMult = x0.F*y0.F;
	vector<DP_Structure> ResVec, ToBeSum, ToBeMult, x0vec, y0vec;
	if (wSum.isDelta == 0)//both x0 and y0 are variables
	{
		DP_Structure c1(model, wMult);
		DP_Structure z0(model, wSum);
		ResVec.push_back(z0);
		ResVec.push_back(c1);
		return ResVec;
	}
	else if (wMult.isDelta == 1)//The multiplication occurs and there is no constant-0 x0/y0. x0/y0=1 and y0/x0=var.
	{
		if (x0.F.isDelta == 1)
		{
			x0vec = COPY(model, x0, 2);
			ToBeSum.push_back(x0vec[0]);
			ToBeMult.push_back(x0vec[1]);
		}
		if (y0.F.isDelta == 1)
		{
			y0vec = COPY(model, y0, 2);
			ToBeSum.push_back(y0vec[0]);
			ToBeMult.push_back(y0vec[1]);
		}
		DP_Structure c1 = Multi(model, ToBeMult);
		DP_Structure z0 = Sum(model, ToBeSum);
		ResVec.push_back(z0);
		ResVec.push_back(c1);
		return ResVec;
	}
	else//either x0 or y0 is constant-0, the other is variable. 
	{
		if (x0.F.isDelta == 1)
		{
			ToBeSum.push_back(x0);
		}
		if (y0.F.isDelta == 1)
		{
			ToBeSum.push_back(y0);
		}
		DP_Structure c1(model, wMult);
		DP_Structure z0 = Sum(model, ToBeSum);
		ResVec.push_back(z0);
		ResVec.push_back(c1);
		return ResVec;
	}


}

//The intermediate bits of modular ADD.
vector<DP_Structure> INB_Add(GRBModel & model, DP_Structure x0, DP_Structure  y0, DP_Structure c0)
{
	FlagValue wSum = x0.F + y0.F + c0.F;
	FlagValue wSum_x0y0 = x0.F + y0.F;
	FlagValue wMult_x0y0 = x0.F*y0.F;
	FlagValue wc0_Mult_ADDx0y0 = (c0.F)*wSum_x0y0;
	FlagValue wc1 = wMult_x0y0 + wc0_Mult_ADDx0y0;
	vector<DP_Structure> ResVec, z0_ToBeSum, c0_x0y0ToBeMult, c1_ToBeSum, x0vec, y0vec, x0y0ToBeSum, x0y0ToBeMult, c0vec;;
	if (wSum.isDelta == 0)//All x0, y0, c0 are constants
	{
		DP_Structure c1(model, wSum);
		DP_Structure z0(model, wc1);
		ResVec.push_back(z0);
		ResVec.push_back(c1);
		return ResVec;
	}

	if (wSum_x0y0.isDelta == 1)//x0+y0=var
	{
		if (wc0_Mult_ADDx0y0.isDelta == 1)//x0 + y0 = var, c0*(x0 + y0) = var
		{
			if (wMult_x0y0.isDelta == 1)//x0+y0=var, c0*(x0+y0)=var, x0*y0=var
			{
				if (x0.F.isDelta == 1)
				{
					x0vec = COPY(model, x0, 2);
					x0y0ToBeSum.push_back(x0vec[0]);
					x0y0ToBeMult.push_back(x0vec[1]);
				}
				if (y0.F.isDelta == 1)
				{
					y0vec = COPY(model, y0, 2);
					x0y0ToBeSum.push_back(y0vec[0]);
					x0y0ToBeMult.push_back(y0vec[1]);
				}
				if (c0.F.isDelta == 1)
				{
					c0vec = COPY(model, c0, 2);
					//z0=c0
					z0_ToBeSum.push_back(c0vec[0]);
					//c0
					c0_x0y0ToBeMult.push_back(c0vec[1]);
				}
				//x0+y0
				DP_Structure ADD_x0y0 = Sum(model, x0y0ToBeSum);
				//COPY x0+y0
				vector<DP_Structure> ADD_x0y0vec = COPY(model, ADD_x0y0, 2);

				//x0*y0
				DP_Structure Mult_x0y0 = Multi(model, x0y0ToBeMult);
				//z0=c0+x0+y0
				z0_ToBeSum.push_back(ADD_x0y0vec[0]);

				//c0,(x0+y0)
				c0_x0y0ToBeMult.push_back(ADD_x0y0vec[1]);

				//c0*(x0+y0)
				DP_Structure c0_Mult_x0y0 = Multi(model, c0_x0y0ToBeMult);

				//x0y0
				c1_ToBeSum.push_back(Mult_x0y0);
				//x0y0, c0*(x0+y0)
				c1_ToBeSum.push_back(c0_Mult_x0y0);


				DP_Structure z0 = Sum(model, z0_ToBeSum);
				DP_Structure c1 = Sum(model, c1_ToBeSum);
				ResVec.push_back(z0);
				ResVec.push_back(c1);
				return ResVec;
			}
			else //x0+y0=var, c0*(x0+y0)=var, x0*y0=0
			{

				if (x0.F.isDelta == 1)x0y0ToBeSum.push_back(x0);
				if (y0.F.isDelta == 1)x0y0ToBeSum.push_back(y0);
				//x0+y0
				DP_Structure ADD_x0y0 = Sum(model, x0y0ToBeSum);
				//COPY x0+y0
				vector<DP_Structure> ADD_x0y0vec = COPY(model, ADD_x0y0, 2);
				//z0=c0+x0+y0
				z0_ToBeSum.push_back(ADD_x0y0vec[0]);
				c0_x0y0ToBeMult.push_back(ADD_x0y0vec[1]);

				if (c0.F.isDelta == 1)
				{
					c0vec = COPY(model, c0, 2);
					//z0=c0
					z0_ToBeSum.push_back(c0vec[0]);
					//c0
					c0_x0y0ToBeMult.push_back(c0vec[1]);
				}
				DP_Structure z0 = Sum(model, z0_ToBeSum);
				DP_Structure c1 = Multi(model, c0_x0y0ToBeMult);
				ResVec.push_back(z0);
				ResVec.push_back(c1);
				return ResVec;


			}
			//END OF //x0+y0=var, c0*(x0+y0)=var
		}
		else //x0+y0=var, c0*(x0+y0)=0
		{
			if (wMult_x0y0.isDelta == 1)//x0+y0=var, c0*(x0+y0)=0, x0y0=var
			{
				if (x0.F.isDelta == 1)
				{
					x0vec = COPY(model, x0, 2);
					x0y0ToBeSum.push_back(x0vec[0]);
					x0y0ToBeMult.push_back(x0vec[1]);
				}
				if (y0.F.isDelta == 1)
				{
					y0vec = COPY(model, y0, 2);
					x0y0ToBeSum.push_back(y0vec[0]);
					x0y0ToBeMult.push_back(y0vec[1]);
				}
				DP_Structure z0 = Sum(model, x0y0ToBeSum);
				DP_Structure c1 = Multi(model, x0y0ToBeMult);
				ResVec.push_back(z0);
				ResVec.push_back(c1);
				return ResVec;
			}
			else//x0+y0=var, c0*(x0+y0)=0, x0y0=0
			{
				if (x0.F.isDelta == 1)x0y0ToBeSum.push_back(x0);
				if (y0.F.isDelta == 1)x0y0ToBeSum.push_back(y0);
				DP_Structure z0 = Sum(model, x0y0ToBeSum);
				DP_Structure c1(model, wc1);
				ResVec.push_back(z0);
				ResVec.push_back(c1);
				return ResVec;
			}

		}//END OF x0+y0=var, c0*(x0+y0)=0
	}
	else//x0+y0=0/1
	{
		if (wc0_Mult_ADDx0y0.isDelta == 1)// x0+y0=1, c0*(x0+y0)=var, x0*y0=0
		{
			if (c0.F.isDelta == 1)
			{
				c0vec = COPY(model, c0, 2);
				//z0=c0
				z0_ToBeSum.push_back(c0vec[0]);
				//c0
				c0_x0y0ToBeMult.push_back(c0vec[1]);
			}
			DP_Structure z0 = Sum(model, z0_ToBeSum);
			DP_Structure c1 = Multi(model, c0_x0y0ToBeMult);
			ResVec.push_back(z0);
			ResVec.push_back(c1);
			return ResVec;
		}
		else// x0+y0=0, c0*(x0+y0)=0, x0*y0=0/1 but c0 is variable
		{
			DP_Structure z0 = c0;
			DP_Structure c1(model, wc1);
			ResVec.push_back(z0);
			ResVec.push_back(c1);
			return ResVec;
		}
	}



}

//The MSB do not generate carry
DP_Structure MSB_Add(GRBModel & model, DP_Structure x0, DP_Structure  y0, DP_Structure c0)
{
	FlagValue wSum = x0.F + y0.F + c0.F;
	vector<DP_Structure> ToBeSum;
	if (x0.F.isDelta == 1)ToBeSum.push_back(x0);
	if (y0.F.isDelta == 1)ToBeSum.push_back(y0);
	if (c0.F.isDelta == 1)ToBeSum.push_back(c0);
	if (wSum.isDelta == 1)
	{
		DP_Structure res = Sum(model, ToBeSum);
		return res;
	}
	else
	{
		return DP_Structure(model, wSum);
	}

}

//Add mod 2^n
vector<DP_Structure> Mod_Add(GRBModel & model, vector<DP_Structure> x0, vector<DP_Structure>  y0, int n)
{
	vector<DP_Structure> startDiv = LSB_Add(model, x0[0], y0[0]);
	vector<DP_Structure> Res;
	Res.push_back(startDiv[0]);
	for (int i = 1; i < n - 1; ++i)
	{
		vector<DP_Structure> TempDiv = INB_Add(model, x0[i], y0[i], startDiv[1]);
		startDiv = TempDiv;
		Res.push_back(startDiv[0]);
	}
	DP_Structure MSB_Div = MSB_Add(model, x0[n - 1], y0[n - 1], startDiv[1]);
	Res.push_back(MSB_Div);
	return Res;
}


vector<DP_Structure> Sum_Mod_Add(GRBModel & model, vector<vector<DP_Structure>> x0, int n = 32)
{
	if (x0.size() == 0)
	{
		vector<DP_Structure> res;
		for (int i = 0; i < n; ++i)res.push_back(DP_Structure(model, FlagValue(0, 0)));
		return res;
	}
	if (x0.size() == 1)return x0[0];
	vector<DP_Structure> out = x0[0];
	for (int i = 1; i < x0.size(); ++i)
	{
		vector<DP_Structure> tmpAdd = Mod_Add(model, out, x0[i], n);
		out = tmpAdd;
	}
	return out;
}




#endif // !DIVISIONPROPERTYBASIC
