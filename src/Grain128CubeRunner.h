#pragma once
#include"DivisionPropertyBasic.h"


class Grain128Runner
{
public:
	vector<uint16> LFSR;
	vector<uint16> NFSR;
	vector<uint16> Z;
	int currentRound;
	Grain128Runner()
	{
		currentRound = 0;
		Z = vector<uint16>(256+32, 0);
		NFSR= vector<uint16>(256+128+32, 0);
		LFSR = vector<uint16>(256 + 128+32, 0);
		for (int i = 96; i < 128; ++i)
		{
			LFSR[i] = 1;
		}
		
	}

	void ZroAll()
	{
		currentRound = 0;
		Z = vector<unsigned short>(256+32, 0);
		NFSR = vector<unsigned short>(256 + 128+32, 0);
		LFSR = vector<unsigned short>(256 + 128+32, 0);
		for (int i = 96; i < 128; ++i)
		{
			LFSR[i] = 1;
		}
	}


	void setKey(vector<uint32> key)
	{
		for (int i = 0; i<128; ++i)
		{
			NFSR[i] = bitW32(key, i);
		}
	}

	void setIV(vector<uint32> iv, set<int> cubeindex)
	{
		for (set<int>::iterator ite = cubeindex.begin(); ite != cubeindex.end(); ++ite)
		{
			LFSR[*ite] = bitW32(iv, *ite);
		}
	}

	void setIV(uint64 cubeCount, set<int> cubeindex)
	{
		int bitPos = 0;
		for (set<int>::iterator ite = cubeindex.begin(); ite != cubeindex.end(); ++ite)
		{
			LFSR[*ite] = bit64(cubeCount, bitPos);
			++bitPos;
		}
	}


	void setIV(vector<uint32> iv)
	{
		for (int i = 0; i<96; ++i)
		{
			LFSR[i] = bitW32(iv, i);
		}
	}

	void setOneBitByRound_LFSR(int r)
	{
		LFSR[r+128]=LFSR[r] ^ LFSR[r + 7] ^ LFSR[r + 38] ^ LFSR[r + 70] ^ LFSR[r + 81] ^ LFSR[r + 96];
	}

	uint16 output_LFSR(int r)
	{
		return (LFSR[r] ^ LFSR[r + 7] ^ LFSR[r + 38] ^ LFSR[r + 70] ^ LFSR[r + 81] ^ LFSR[r + 96]);
	}

	uint16 output_NFSR(int r)
	{
		uint16 LinPart = LFSR[r] ^ NFSR[r] ^ NFSR[r + 26] ^ NFSR[r + 56] ^ NFSR[r + 91] ^ NFSR[r + 96];
		uint16 QuaPart = (NFSR[r + 3] & NFSR[r + 67]) ^
			(NFSR[r + 11] & NFSR[r + 13]) ^
			(NFSR[r + 17] & NFSR[r + 18]) ^
			(NFSR[r + 27] & NFSR[r + 59]) ^
			(NFSR[r + 40] & NFSR[r + 48]) ^
			(NFSR[r + 61] & NFSR[r + 65]) ^
			(NFSR[r + 68] & NFSR[r + 84]);
		return (LinPart^QuaPart);
	}

	void setOneBitByRound_NFSR(int r)
	{
		/*
		uint32 LinPart = bitW32(LFSR, r) ^ bitW32(NFSR, r) ^ bitW32(NFSR, r + 26) ^ bitW32(NFSR, r + 56) ^ bitW32(NFSR, r + 91) ^ bitW32(NFSR, r + 96);
		uint32 QuaPart = (bitW32(NFSR, r + 3) & bitW32(NFSR, r + 67)) ^
			(bitW32(NFSR, r + 11) & bitW32(NFSR, r + 13)) ^
			(bitW32(NFSR, r + 17) & bitW32(NFSR, r + 18)) ^
			(bitW32(NFSR, r + 27) & bitW32(NFSR, r + 59)) ^
			(bitW32(NFSR, r + 40) & bitW32(NFSR, r + 48)) ^
			(bitW32(NFSR, r + 61) & bitW32(NFSR, r + 65)) ^
			(bitW32(NFSR, r + 68) & bitW32(NFSR, r + 84));
		*/
		uint16 LinPart = LFSR[r] ^ NFSR[r] ^ NFSR[r + 26] ^ NFSR[r + 56] ^ NFSR[r + 91] ^ NFSR[r + 96];
		uint16 QuaPart = (NFSR[r+3] & NFSR[r+67])^
			(NFSR[r + 11] & NFSR[r + 13]) ^
			(NFSR[r + 17] & NFSR[r + 18]) ^
			(NFSR[r + 27] & NFSR[r + 59]) ^
			(NFSR[r + 40] & NFSR[r + 48]) ^
			(NFSR[r + 61] & NFSR[r + 65]) ^
			(NFSR[r + 68] & NFSR[r + 84]);
		NFSR[r+128]=(LinPart^QuaPart);
	}

	void setOneBitByRound_Z(int r)
	{
		uint16 LinPart = NFSR[r + 2] ^ NFSR[r + 15] ^ NFSR[r + 36] ^ NFSR[r + 45] ^ 
			NFSR[r + 64] ^ NFSR[r + 73] ^ NFSR[r + 89] ^ LFSR[r + 93];
		uint16 QuaPart = (NFSR[r + 12] & LFSR[r + 8]) ^
			(LFSR[r + 13] & LFSR[r + 20]) ^
			(NFSR[r + 95] & LFSR[r + 42]) ^
			(LFSR[r + 60] & LFSR[r + 79]);
		uint16 TriPart = NFSR[r + 12] & NFSR[ r + 95] & LFSR[r + 95];
		Z[r] = (LinPart^QuaPart^TriPart);
	}
	void initByRound(int outputRound)
	{
		for (int r = 0; r < outputRound; ++r)
		{
			uint16 zr = outputZ(r);
			uint16 ssub = output_LFSR(r);
			uint16 bsub = output_NFSR(r);
			
			Z[r] = zr;
			NFSR[r + 128] = r < 256 ? bsub^zr : bsub;
			LFSR[r + 128] = r < 256 ? ssub^zr : ssub;
		}
	}


	uint16 outputZ(int r)
	{
		/*
		uint32 LinPart = bitW32(NFSR, r + 2) ^ bitW32(NFSR, r + 15) ^ bitW32(NFSR, r + 36) ^ bitW32(NFSR, r + 45)
			^ bitW32(NFSR, r + 64) ^ bitW32(NFSR, r + 73) ^ bitW32(NFSR, r + 89) ^ bitW32(LFSR, r + 93);
		uint32 QuaPart = (bitW32(NFSR, r + 12) & bitW32(LFSR, r + 8)) ^
			(bitW32(LFSR, r + 13) & bitW32(LFSR, r + 20)) ^
			(bitW32(NFSR, r + 95) & bitW32(LFSR, r + 42)) ^
			(bitW32(LFSR, r + 60) & bitW32(LFSR, r + 79));
		uint32 TriPart = (bitW32(NFSR, r + 12) & bitW32(NFSR, r + 95) & bitW32(LFSR, r + 95));
		uint32 zr = (LinPart^QuaPart^TriPart);
		*/
		uint16 LinPart = NFSR[r + 2] ^ NFSR[r + 15] ^ NFSR[r + 36] ^ NFSR[r + 45] ^
			NFSR[r + 64] ^ NFSR[r + 73] ^ NFSR[r + 89] ^ LFSR[r + 93];
		uint16 QuaPart = (NFSR[r + 12] & LFSR[r + 8]) ^
			(LFSR[r + 13] & LFSR[r + 20]) ^
			(NFSR[r + 95] & LFSR[r + 42]) ^
			(LFSR[r + 60] & LFSR[r + 79]);
		uint16 TriPart = NFSR[r + 12] & NFSR[r + 95] & LFSR[r + 95];
		uint16 zr = (LinPart^QuaPart^TriPart);
		return zr;
	}

	uint16 initRoundsAndOutputZ(int outputRound)
	{
		int initRound = outputRound - (outputRound % 32);
		for (int r = 0; r < initRound; ++r)
		{
			setOneBitByRound_Z(r);
			setOneBitByRound_NFSR(r);
			setOneBitByRound_LFSR(r);
			if(r<256)NFSR[r + 128] ^= Z[r];
			if(r<256)LFSR[r + 128] ^= Z[r];
		}
		setOneBitByRound_Z(outputRound);
		return Z[outputRound];
	}
};

extern nullifyOneCrucialBit;

struct CubeAndStrategy
{
	set<int> Cube;
	nullifyOneCrucialBit oneStrategy;
	vector<uint32> nonCubeIVs;
};

class DynamicCubeAttack
{
public:
	vector<nullifyOneCrucialBit> NullificationStrategy;
	set<int> Cube;
	vector<uint32> IV;
	int outputRound;
	int count0Sum;
	int testTime;

	DynamicCubeAttack(set<int> cube = { 6,7,37,46,49,59,69,85 }, vector<uint32> iv = { 0,0,0 }, vector<nullifyOneCrucialBit> strategies = { {90,158,true} }, int round = 179)
	{
		outputRound = round;
		Cube = cube;
		IV = iv;
		NullificationStrategy = strategies;
		count0Sum = 0;
		testTime = 0;
	}

	DynamicCubeAttack(CubeAndStrategy strategy, int round = 179)
	{
		outputRound = round;
		Cube = strategy.Cube;
		IV = strategy.nonCubeIVs;
		NullificationStrategy = { strategy.oneStrategy };
		count0Sum = 0;
		testTime = 0;
	}


	void setDynamicIVs(Grain128Runner & Gr, uint32 UserGuess = 0)
	{
		for (int i = 0; i < NullificationStrategy.size(); ++i)
		{
			vector<uint16> guess;
			if (NullificationStrategy[i].IVIndex == 90 && NullificationStrategy[i].stateIndex == 158 && NullificationStrategy[i].isNFSR == true)
			{
				guess.push_back(Gr.NFSR[30] ^ (Gr.NFSR[32]) ^ (Gr.NFSR[33] & Gr.NFSR[97]) ^ (Gr.NFSR[41] & Gr.NFSR[43]) ^ 
					(Gr.NFSR[42] & Gr.NFSR[125]) ^ (Gr.NFSR[45]) ^ (Gr.NFSR[47] & Gr.NFSR[48]) ^ (Gr.NFSR[56]) ^ 
					(Gr.NFSR[57] & Gr.NFSR[89]) ^ (Gr.NFSR[66]) ^ (Gr.NFSR[70] & Gr.NFSR[78]) ^ (Gr.NFSR[75]) ^ (Gr.NFSR[86]) ^ 
					(Gr.NFSR[91] & Gr.NFSR[95]) ^ (Gr.NFSR[94]) ^ (Gr.NFSR[98] & Gr.NFSR[114]) ^ (Gr.NFSR[103]) ^ (Gr.NFSR[119]) ^ 
					(Gr.NFSR[121]) ^ (Gr.NFSR[126]) ^ 1);
				guess.push_back(Gr.NFSR[42]);
				guess.push_back(Gr.NFSR[125]);
				for (int bitNo = 0; bitNo < 3; ++bitNo)
					guess[bitNo] ^= bit32(UserGuess, bitNo);
				Gr.LFSR[NullificationStrategy[i].IVIndex] = guess[0] ^ (guess[1] & Gr.LFSR[38]) ^ (guess[2] & Gr.LFSR[72])
					^ (Gr.LFSR[30]) ^ (Gr.LFSR[43] & Gr.LFSR[50]);
			}
			if (NullificationStrategy[i].IVIndex == 30 && NullificationStrategy[i].stateIndex == 158 && NullificationStrategy[i].isNFSR == true)
			{
				guess.push_back(Gr.NFSR[30] ^ (Gr.NFSR[32]) ^ (Gr.NFSR[33] & Gr.NFSR[97]) ^ (Gr.NFSR[41] & Gr.NFSR[43]) ^
					(Gr.NFSR[42] & Gr.NFSR[125]) ^ (Gr.NFSR[45]) ^ (Gr.NFSR[47] & Gr.NFSR[48]) ^ (Gr.NFSR[56]) ^
					(Gr.NFSR[57] & Gr.NFSR[89]) ^ (Gr.NFSR[66]) ^ (Gr.NFSR[70] & Gr.NFSR[78]) ^ (Gr.NFSR[75]) ^ (Gr.NFSR[86]) ^
					(Gr.NFSR[91] & Gr.NFSR[95]) ^ (Gr.NFSR[94]) ^ (Gr.NFSR[98] & Gr.NFSR[114]) ^ (Gr.NFSR[103]) ^ (Gr.NFSR[119]) ^
					(Gr.NFSR[121]) ^ (Gr.NFSR[126]) ^ 1);
				guess.push_back(Gr.NFSR[42]);
				guess.push_back(Gr.NFSR[125]);
				for (int bitNo = 0; bitNo < 3; ++bitNo)
					guess[bitNo] ^= bit32(UserGuess, bitNo);
				Gr.LFSR[NullificationStrategy[i].IVIndex] = guess[0] ^ (guess[1] & Gr.LFSR[38]) ^ (guess[2] & Gr.LFSR[72])
					^ (Gr.LFSR[90]) ^ (Gr.LFSR[43] & Gr.LFSR[50]);
			}
			/*
			if(NullificationStrategy[i].IVIndex == 18 && NullificationStrategy[i].stateIndex == 146 && NullificationStrategy[i].isNFSR == true)
			{
				guess.push_back(Gr.NFSR[18]^ (Gr.NFSR[20]) ^ (Gr.NFSR[21] & Gr.NFSR[85]) ^ (Gr.NFSR[29] & Gr.NFSR[31]) ^ (Gr.NFSR[30] & Gr.NFSR[113]) ^ (Gr.NFSR[33]) ^ (Gr.NFSR[35] & Gr.NFSR[36]) ^ (Gr.NFSR[44]) ^ (Gr.NFSR[45] & Gr.NFSR[77]) ^ (Gr.NFSR[54]) ^ (Gr.NFSR[58] & Gr.NFSR[66]) ^ (Gr.NFSR[63]) ^ (Gr.NFSR[74]) ^ (Gr.NFSR[79] & Gr.NFSR[83]) ^ (Gr.NFSR[82]) ^ (Gr.NFSR[86] & Gr.NFSR[102]) ^ (Gr.NFSR[91]) ^ (Gr.NFSR[107]) ^ (Gr.NFSR[109]) ^ (Gr.NFSR[114]) ^1);
				guess.push_back(Gr.NFSR[113]);
				guess.push_back(Gr.NFSR[30]);
				for (int bitNo = 0; bitNo < 3; ++bitNo)
					guess[bitNo] ^= bit32(UserGuess, bitNo);
				Gr.LFSR[NullificationStrategy[i].IVIndex] = guess[0] ^ (guess[1] & Gr.LFSR[60]) ^ (guess[2] & Gr.LFSR[26])
					^ (Gr.LFSR[31] & Gr.LFSR[38]) ^ Gr.LFSR[78];
			}
			if (NullificationStrategy[i].IVIndex == 78 && NullificationStrategy[i].stateIndex == 146 && NullificationStrategy[i].isNFSR == true)
			{
				guess.push_back(Gr.NFSR[18] ^ (Gr.NFSR[20]) ^ (Gr.NFSR[21] & Gr.NFSR[85]) ^ (Gr.NFSR[29] & Gr.NFSR[31]) ^ (Gr.NFSR[30] & Gr.NFSR[113]) ^ (Gr.NFSR[33]) ^ (Gr.NFSR[35] & Gr.NFSR[36]) ^ (Gr.NFSR[44]) ^ (Gr.NFSR[45] & Gr.NFSR[77]) ^ (Gr.NFSR[54]) ^ (Gr.NFSR[58] & Gr.NFSR[66]) ^ (Gr.NFSR[63]) ^ (Gr.NFSR[74]) ^ (Gr.NFSR[79] & Gr.NFSR[83]) ^ (Gr.NFSR[82]) ^ (Gr.NFSR[86] & Gr.NFSR[102]) ^ (Gr.NFSR[91]) ^ (Gr.NFSR[107]) ^ (Gr.NFSR[109]) ^ (Gr.NFSR[114]) ^ 1);
				guess.push_back(Gr.NFSR[113]);
				guess.push_back(Gr.NFSR[30]);
				for (int bitNo = 0; bitNo < 3; ++bitNo)
					guess[bitNo] ^= bit32(UserGuess, bitNo);
				Gr.LFSR[NullificationStrategy[i].IVIndex] = guess[0] ^ (guess[1] & Gr.LFSR[60]) ^ (guess[2] & Gr.LFSR[26])
					^ (Gr.LFSR[31] & Gr.LFSR[38]) ^ Gr.LFSR[18];
			}
			if (NullificationStrategy[i].IVIndex ==8 && NullificationStrategy[i].stateIndex == 136 && NullificationStrategy[i].isNFSR == true)
			{
				guess.push_back(Gr.NFSR[8] ^ (Gr.NFSR[10]) ^ (Gr.NFSR[11] & Gr.NFSR[75]) ^ (Gr.NFSR[19] & Gr.NFSR[21]) ^ (Gr.NFSR[20] & Gr.NFSR[103]) ^ (Gr.NFSR[23]) ^ (Gr.NFSR[25] & Gr.NFSR[26]) ^ (Gr.NFSR[34]) ^ (Gr.NFSR[35] & Gr.NFSR[67]) ^ (Gr.NFSR[44]) ^ (Gr.NFSR[48] & Gr.NFSR[56]) ^ (Gr.NFSR[53]) ^ (Gr.NFSR[64]) ^ (Gr.NFSR[69] & Gr.NFSR[73]) ^ (Gr.NFSR[72]) ^ (Gr.NFSR[76] & Gr.NFSR[92]) ^ (Gr.NFSR[81]) ^ (Gr.NFSR[97]) ^ (Gr.NFSR[99]) ^ (Gr.NFSR[104]) ^ 1);
				guess.push_back(Gr.NFSR[103]);
				guess.push_back(Gr.NFSR[20]);
				for (int bitNo = 0; bitNo < 3; ++bitNo)
					guess[bitNo] ^= bit32(UserGuess, bitNo);
				Gr.LFSR[NullificationStrategy[i].IVIndex] = guess[0] ^ (guess[1] & Gr.LFSR[50]) ^ (guess[2] & Gr.LFSR[16])
					^ (Gr.LFSR[21] & Gr.LFSR[28]) ^ (Gr.LFSR[68]&Gr.LFSR[87]);
			}
			*/
		}

	}

	int count0SumForOneGuess(uint32 guess, int TestTime=1000, std::ostream & o=std::cout)
	{
		outputSetting(guess, o);
		int count = 0;
		int count0 = 0;
		uint16 sum = 0;
		vector<uint32> randKey = { rand_32(), rand_32(), rand_32(), rand_32() };
		uint64 cubePointsNum = 1;
		cubePointsNum <<= Cube.size();
		Grain128Runner runner = Grain128Runner();
		runner.setIV(IV);
		while (count < TestTime)
		{
			randKey = { rand_32(), rand_32(), rand_32(), rand_32() };
			runner.setKey(randKey);
			sum = 0;
			for (uint64 i = 0; i < cubePointsNum; ++i)
			{
				runner.setIV(i, Cube);
				setDynamicIVs(runner, guess);
				sum ^= runner.initRoundsAndOutputZ(outputRound);
			}
			if (sum == 0)count0++;
			count++;
			/*
			if (count % (TestTime/10) == 0)
			{
				o << dec << count0 << "/" << count << endl;
			}
			*/
		}
		testTime = TestTime;
		count0Sum = count0;
		outputReport(guess, o);
		return count0;
	}
private:
	void outputSetting(uint32 guess, std::ostream & o)
	{
		o << "Experiment Setting:" << endl;
		o << "Cube:";
		for (set<int>::iterator ite = Cube.begin(); ite != Cube.end(); ++ite)
		{
			o << dec << *ite << ",";
		}
		o << endl;

		o << "IV:";
		for (int i = 0; i < IV.size(); ++i)
		{
			o << IV[i] << ",";
		}
		o << endl;

		o << "Nullification: ";
		for (int i = 0; i < NullificationStrategy.size(); ++i)
		{
			o << NullificationStrategy[i].IVIndex << "->";
			if (NullificationStrategy[i].isNFSR == true)o << "b" << NullificationStrategy[i].stateIndex << ",";
			else o << "s" << NullificationStrategy[i].stateIndex << ",";
		}
		o << endl;

		o << "Guess " << guess << endl;

	}

	void outputReport(uint32 guess, std::ostream & o)
	{
		o << dec << "Guess " << guess << " Final:" << dec << count0Sum << "/" << testTime << endl;
	}

};
